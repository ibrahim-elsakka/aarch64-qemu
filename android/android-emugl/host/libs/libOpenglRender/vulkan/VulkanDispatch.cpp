// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VulkanDispatch.h"

#include "android/base/memory/LazyInstance.h"

using android::base::LazyInstance;

namespace emugl {

class VulkanInitialization {
public:
    VulkanInitialization() {
    }

    VulkanDispatch* dispatch() { return 0; }
};

static LazyInstance<VulkanInitialization> sVulkanInitalization =
    LAZY_INSTANCE_INIT;

VulkanDispatch* getInstanceDeviceAndDispatch() {
    return sVulkanInitalization->dispatch();
}

}
