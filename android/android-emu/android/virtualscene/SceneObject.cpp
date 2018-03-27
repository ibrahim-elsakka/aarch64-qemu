/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "android/virtualscene/SceneObject.h"

#include "android/base/files/PathUtils.h"
#include "android/base/files/ScopedStdioFile.h"
#include "android/base/system/System.h"
#include "android/utils/debug.h"
#include "android/virtualscene/Renderer.h"

#include <tiny_obj_loader.h>

#include <unordered_map>
#include <vector>

#define E(...) derror(__VA_ARGS__)
#define W(...) dwarning(__VA_ARGS__)
#define D(...) VERBOSE_PRINT(virtualscene, __VA_ARGS__)
#define D_ACTIVE VERBOSE_CHECK(virtualscene)

using android::base::PathUtils;
using android::base::ScopedStdioFile;
using android::base::System;

static constexpr android::virtualscene::VertexPositionUV kQuadVerts[] = {
    { glm::vec3(-0.5f, -0.5f, 0.f), glm::vec2(0.f, 0.f) },
    { glm::vec3( 0.5f, -0.5f, 0.f), glm::vec2(1.f, 0.f) },
    { glm::vec3(-0.5f,  0.5f, 0.f), glm::vec2(0.f, 1.f) },
    { glm::vec3( 0.5f,  0.5f, 0.f), glm::vec2(1.f, 1.f) },
};

static constexpr GLuint kQuadIndices[] = {
    0, 1, 2, 2, 1, 3
};

namespace android {
namespace virtualscene {

SceneObject::SceneObject() = default;
SceneObject::~SceneObject() = default;

std::unique_ptr<SceneObject> SceneObject::loadFromObj(Renderer& renderer,
                                                      const char* filename) {
    const std::string resourcesDir =
            PathUtils::addTrailingDirSeparator(PathUtils::join(
                    System::get()->getLauncherDirectory(), "resources"));
    const std::string filePath = PathUtils::join(
            System::get()->getLauncherDirectory(), "resources", filename);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    const bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                      filePath.c_str(), resourcesDir.c_str());
    if (!ret) {
        E("%s: Error loading obj %s: %s", __FUNCTION__, filename,
          err.empty() ? "<no message>" : err.c_str());
        return nullptr;
    } else if (!err.empty()) {
        W("%s: Warnings loading obj %s: %s", __FUNCTION__, filename,
          err.c_str());
    }

    std::unique_ptr<SceneObject> result(new SceneObject());

    const size_t vertexCount = attrib.vertices.size() / 3;
    const size_t texcoordCount = attrib.texcoords.size() / 2;

    for (const tinyobj::shape_t& shape : shapes) {
        const tinyobj::mesh_t& mesh = shape.mesh;

        std::vector<VertexPositionUV> vertices;
        std::unordered_map<VertexPositionUV, GLuint, VertexPositionUVHash>
                existingVertexToIndex;
        std::vector<GLuint> indices;
        Texture texture;

        bool useCheckerboardMaterial = false;

        if (!mesh.material_ids.empty()) {
            const int material_id = mesh.material_ids[0];
            if (material_id >= 0 && material_id < materials.size()) {
                if (strstr(materials[material_id].diffuse_texname.c_str(),
                           "TV")) {
                    useCheckerboardMaterial = true;
                } else {
                    texture = renderer.loadTexture(
                            result.get(),
                            materials[material_id].diffuse_texname.c_str());
                }
            }
        }

        for (size_t i = 0; i < mesh.indices.size(); i++) {
            tinyobj::index_t index = mesh.indices[i];
            VertexPositionUV vertex;

            if (index.vertex_index < 0 || index.vertex_index >= vertexCount) {
                E("%s: Error parsing %s, invalid vertex index %d, expected "
                  "less than %d",
                  __FUNCTION__, filename, index.vertex_index, vertexCount);
                return nullptr;
            }

            vertex.pos = glm::vec3(attrib.vertices[3 * index.vertex_index],
                                   attrib.vertices[3 * index.vertex_index + 1],
                                   attrib.vertices[3 * index.vertex_index + 2]);

            if (index.texcoord_index >= 0) {
                if (index.texcoord_index >= texcoordCount) {
                    E("%s: Error parsing %s, invalid vertex index %d, expected "
                      "less than %d",
                      __FUNCTION__, filename, index.texcoord_index,
                      texcoordCount);
                    return nullptr;
                }

                // Flip Y coord, OpenGL and .obj are reversed.
                vertex.uv = glm::vec2(
                        attrib.texcoords[2 * index.texcoord_index],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);
            }

            auto existingEntry = existingVertexToIndex.find(vertex);
            if (existingEntry != existingVertexToIndex.end()) {
                indices.push_back(existingEntry->second);
            } else {
                vertices.push_back(vertex);

                const GLuint index = vertices.size() - 1;
                indices.push_back(index);

                existingVertexToIndex[vertex] = index;
            }
        }

        D("%s: Creating mesh with %d vertices, %d indices", __FUNCTION__,
          vertices.size(), indices.size());

        Renderable renderable;
        renderable.material = useCheckerboardMaterial ?
                renderer.createMaterialCheckerboard(result.get()) :
                renderer.createMaterialTextured();
        renderable.mesh = renderer.createMesh(result.get(), vertices, indices);
        renderable.texture = texture;

        result.get()->mRenderables.emplace_back(std::move(renderable));
    }

    return result;
}

std::unique_ptr<SceneObject> SceneObject::createQuad(
        Renderer& renderer,
        const char* textureFilename) {
    std::unique_ptr<SceneObject> result(new SceneObject());

    Texture texture = renderer.loadTexture(result.get(), textureFilename);
    if (!texture.isValid()) {
        W("%s: Could not load texture '%s'", __FUNCTION__, textureFilename);
        return nullptr;
    }

    Renderable renderable;
    renderable.material = renderer.createMaterialTextured();
    renderable.mesh =
            renderer.createMesh(result.get(), kQuadVerts, kQuadIndices);
    renderable.texture = texture;

    result.get()->mRenderables.emplace_back(std::move(renderable));

    return result;
}

void SceneObject::setTransform(const glm::mat4& transform) {
    mTransform = transform;
}

glm::mat4 SceneObject::getTransform() const {
    return mTransform;
}

const std::vector<Renderable>& SceneObject::getRenderables() const {
    return mRenderables;
}

}  // namespace virtualscene
}  // namespace android
