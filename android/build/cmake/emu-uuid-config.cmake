# Copyright 2018 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

get_filename_component(PREBUILT_ROOT "${ANDROID_QEMU2_TOP_DIR}/../../prebuilts/android-emulator-build/common/e2fsprogs/${ANDROID_TARGET_TAG}" ABSOLUTE)

if ("${ANDROID_TARGET_TAG}" MATCHES "darwin.*")
    # Apple does its own thing, the headers should be on the path, so no need to set them explicitly.
elseif ("${ANDROID_TARGET_TAG}" MATCHES "linux.*")
    set(UUID_INCLUDE_DIR "${PREBUILT_ROOT}/include")
    set(UUID_INCLUDE_DIRS "${UUID_INCLUDE_DIR}")
    set(UUID_LIBRARIES "${PREBUILT_ROOT}/lib/libuuid.a")
elseif ( ("${ANDROID_TARGET_TAG}" MATCHES "windows.*") )
    # In windows you include rpc.h
    set(UUID_LIBRARIES "-lrpcrt4")
endif ()
set(PACKAGE_EXPORT "UUID_INCLUDE_DIR;UUID_INCLUDE_DIRS;UUID_LIBRARIES;UUID_FOUND")
set(UUID_FOUND TRUE)
