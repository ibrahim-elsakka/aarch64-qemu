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

get_filename_component(PREBUILT_ROOT "${ANDROID_QEMU2_TOP_DIR}/../../prebuilts/android-emulator-build/qemu-android-deps/${ANDROID_TARGET_TAG}" ABSOLUTE)

set(SDL2_INCLUDE_DIRS "${PREBUILT_ROOT}/include")
set(SDL2_INCLUDE_DIR "${PREBUILT_ROOT}/include")
set(SDL2_LIBRARIES "${PREBUILT_ROOT}/lib/SDL2.a")
set(SDL_FOUND TRUE)

set(PACKAGE_EXPORT "SDL2_INCLUDE_DIR;SDL2_INCLUDE_DIRS;SDL2_LIBRARIES;SDL2_FOUND")

