# Copyright (c) 2018 The Android Open Source Project
# Copyright (c) 2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from copy import deepcopy

from .common.codegen import CodeGen
from .common.vulkantypes import \
        VulkanAPI, makeVulkanTypeSimple, iterateVulkanType

from .wrapperdefs import VulkanWrapperGenerator

# No real good way to automatically infer the most important Vulkan API
# functions as it relates to which getProcAddress function to use, plus
# we might want to control which function to use depending on our
# performance needs.

# This is based on the minimum set of functions needed to be directly
# queried with dlsym and not returning null.
getProcAddrFuncs = [
    "vkGetInstanceProcAddr",
    "vkDestroyInstance",
    "vkEnumeratePhysicalDevices",
    "vkGetPhysicalDeviceFeatures",
    "vkGetPhysicalDeviceFormatProperties",
    "vkGetPhysicalDeviceImageFormatProperties",
    "vkGetPhysicalDeviceProperties",
    "vkGetPhysicalDeviceQueueFamilyProperties",
    "vkGetPhysicalDeviceMemoryProperties",
    "vkCreateDevice",
    "vkDestroyDevice",
    "vkEnumerateDeviceExtensionProperties",
    "vkEnumerateDeviceLayerProperties",
    "vkGetDeviceQueue",
    "vkQueueSubmit",
    "vkQueueWaitIdle",
    "vkDeviceWaitIdle",
]

getInstanceProcAddrNoInstanceFuncs = [
    "vkCreateInstance",
    "vkEnumerateInstanceExtensionProperties",
    "vkEnumerateInstanceLayerProperties",
]

getInstanceProcAddrFuncs = [
    "vkGetDeviceProcAddr",
    "vkCreateSwapchainKHR",
    "vkDestroySwapchainKHR",
    "vkGetSwapchainImagesKHR",
    "vkAcquireNextImageKHR",
    "vkQueuePresentKHR",
    "vkCreateMacOSSurfaceMVK",
    "vkCreateWin32SurfaceKHR",
    "vkGetPhysicalDeviceWin32PresentationSupportKHR",
    "vkCreateXlibSurfaceKHR",
    "vkGetPhysicalDeviceXlibPresentationSupportKHR",
    "vkCreateXcbSurfaceKHR",
    "vkGetPhysicalDeviceXcbPresentationSupportKHR",
]

# Implicitly, everything else is going to be obtained
# with vkGetDeviceProcAddr,
# unless it has instance in the arg.

def isGetProcAddressAPI(vulkanApi):
    return vulkanApi.name in getProcAddrFuncs

def isGetInstanceProcAddressNoInstanceAPI(vulkanApi):
    return vulkanApi.name in getInstanceProcAddrNoInstanceFuncs

def isGetInstanceProcAddressAPI(vulkanApi):
    if vulkanApi.name in getInstanceProcAddrFuncs:
        return True

    if vulkanApi.parameters[0].typeName == "VkInstance":
        return True

    return False

def isGetDeviceProcAddressAPI(vulkanApi):
    if isGetProcAddressAPI(vulkanApi):
        return False

    if isGetInstanceProcAddressAPI(vulkanApi):
        return False

    return True

def inferProcAddressFuncType(vulkanApi):
    if isGetProcAddressAPI(vulkanApi):
        return "global"
    if isGetInstanceProcAddressNoInstanceAPI(vulkanApi):
        return "global-instance"
    if isGetInstanceProcAddressAPI(vulkanApi):
        return "instance"
    return "device"

# VulkanDispatch defines a struct, VulkanDispatch,
# that is populated by function pointers from the Vulkan
# loader. No attempt is made to do something different
# for instance vs device functions.
class VulkanDispatch(VulkanWrapperGenerator):
    def __init__(self, module, typeInfo):
        VulkanWrapperGenerator.__init__(self, module, typeInfo)

        self.apisToGet = {}

        self.cgenHeader = CodeGen()
        self.cgenImpl = CodeGen()
        self.typeInfo = typeInfo

        self.currentFeature = ""
        self.featureForCodegen = ""

    def onBegin(self):
        self.cgenHeader.line("""
void init_vulkan_dispatch_from_system_loader(
    DlOpenFunc dlOpenFunc,
    DlSymFunc dlSymFunc,
    VulkanDispatch* dispatch_out);
""")
        self.cgenHeader.line("struct VulkanDispatch {")
        self.module.appendHeader(self.cgenHeader.swapCode())

    def syncFeature(self, cgen, feature):
        if self.featureForCodegen != feature:
            if feature == "":
                cgen.leftline("#endif")
                self.featureForCodegen = feature
                return

            if self.featureForCodegen != "":
                cgen.leftline("#endif")

            cgen.leftline("#ifdef %s" % feature)
            self.featureForCodegen = feature

    def makeDlsymCall(self, cgen, apiname, typedecl):
        cgen.stmt( \
            "out->%s = (%s)dlSymFunc(lib, \"%s\")" % \
            (apiname, typedecl, apiname))

    def onEnd(self):
        self.cgenHeader.line("};")
        self.module.appendHeader(self.cgenHeader.swapCode())

        self.cgenImpl.line("""
void init_vulkan_dispatch_from_system_loader(
    DlOpenFunc dlOpenFunc,
    DlSymFunc dlSymFunc,
    VulkanDispatch* out)""")

        self.cgenImpl.beginBlock()
        
        self.cgenImpl.stmt("memset(out, 0x0, sizeof(VulkanDispatch))")

        self.cgenImpl.stmt("void* lib = dlOpenFunc()")
        self.cgenImpl.stmt("if (!lib) return")

        apis = \
            self.apisToGet["global"] + \
            self.apisToGet["global-instance"] + \
            self.apisToGet["instance"] + \
            self.apisToGet["device"] + \
            self.apisToGet["global"] + \
            self.apisToGet["global"]

        for vulkanApi, typeDecl, feature in apis:
            self.syncFeature(self.cgenImpl, feature)
            self.makeDlsymCall(self.cgenImpl, vulkanApi.name, typeDecl)

        self.syncFeature(self.cgenImpl, "")
        self.cgenImpl.endBlock()
        self.module.appendImpl(self.cgenImpl.swapCode())

    def onBeginFeature(self, featureName):
        self.currentFeature = featureName

    def onGenType(self, typeXml, name, alias):
        VulkanWrapperGenerator.onGenType(self, typeXml, name, alias)

    def onGenCmd(self, cmdinfo, name, alias):
        VulkanWrapperGenerator.onGenCmd(self, cmdinfo, name, alias)

        vulkanApi = self.typeInfo.apis[name]

        typeDecl = "PFN_%s" % name

        procAddressType = inferProcAddressFuncType(vulkanApi)

        self.cgenHeader.stmt("%s %s" % (typeDecl, name));
        self.module.appendHeader(self.cgenHeader.swapCode())

        current = self.apisToGet.get(procAddressType, [])
        if current == []:
            self.apisToGet[procAddressType] = current
        current.append((vulkanApi, typeDecl, self.currentFeature))

# VulkanDispatchFast allows one to get the optimal function pointers
# for a given Vulkan API call, in order to improve performance.
#
# We can optionally query VkDevices to get function pointers that are
# closer to the ICD and have fewer levels of indirection from the loader
# to get there.
# See
# https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md
# for more info.
#
# This requires the calling C++ code to provide functions to
# generate the desired instances and devices, otherwise we won't know
# which instance or device to pass to vkGet(Instance|Device)ProcAddr,
# so it does push more complexity to the user.
class VulkanDispatchFast(VulkanDispatch):

    def __init__(self, module, typeInfo):
        VulkanDispatch.__init__(self, module, typeInfo)

    def onBegin(self):
        self.cgenHeader.line("""
void init_vulkan_dispatch_from_system_loader(
    DlOpenFunc dlOpenFunc,
    DlSymFunc dlSymFunc,
    InstanceGetter instanceGetter,
    DeviceGetter deviceGetter,
    VulkanDispatch* dispatch_out);
""")

        self.cgenHeader.line("struct VulkanDispatch {")
        self.cgenHeader.line("VkInstance instance;")
        self.cgenHeader.line("VkPhysicalDevice physicalDevice;")
        self.cgenHeader.line("uint32_t physicalDeviceQueueFamilyInfoCount;")
        self.cgenHeader.line("VkQueueFamilyProperties* physicalDeviceQueueFamilyInfos;")
        self.cgenHeader.line("VkDevice device;")
        self.cgenHeader.line("bool presentCapable;")
        self.module.appendHeader(self.cgenHeader.swapCode())

    def makeGetProcAddr(self, cgen, dispatchLevel, dispatch, apiname, typedecl):
        if dispatchLevel == "instance":
            funcname = "vkGetInstanceProcAddr"
        elif dispatchLevel == "device":
            funcname = "vkGetDeviceProcAddr"
        else:
            raise

        cgen.stmt( \
            "out->%s = (%s)out->%s(%s, \"%s\")" % \
            (apiname, typedecl, funcname, dispatch, apiname))

    def onEnd(self):
        self.cgenHeader.line("};")
        self.module.appendHeader(self.cgenHeader.swapCode())

        self.cgenImpl.line("""
void init_vulkan_dispatch_from_system_loader(
    DlOpenFunc dlOpenFunc,
    DlSymFunc dlSymFunc,
    InstanceGetter instanceGetter,
    DeviceGetter deviceGetter,
    VulkanDispatch* out)""")

        self.cgenImpl.beginBlock()
        
        self.cgenImpl.stmt("out->instance = nullptr")
        self.cgenImpl.stmt("out->physicalDevice = nullptr")
        self.cgenImpl.stmt("out->physicalDeviceQueueFamilyInfoCount = 0")
        self.cgenImpl.stmt("out->physicalDeviceQueueFamilyInfos = nullptr")
        self.cgenImpl.stmt("out->device = nullptr")
        self.cgenImpl.stmt("out->presentCapable = false")

        self.cgenImpl.stmt("void* lib = dlOpenFunc()")
        self.cgenImpl.stmt("if (!lib) return")

        for vulkanApi, typeDecl, feature in self.apisToGet["global"]:
            self.syncFeature(self.cgenImpl, feature)
            self.makeDlsymCall(self.cgenImpl, vulkanApi.name, typeDecl)

        self.syncFeature(self.cgenImpl, "")
        self.cgenImpl.stmt("if (!out->vkGetInstanceProcAddr) return")

        for vulkanApi, typeDecl, feature in self.apisToGet["global-instance"]:
            self.syncFeature(self.cgenImpl, feature)
            self.makeGetProcAddr( \
                self.cgenImpl, "instance", "nullptr", vulkanApi.name, typeDecl);

        self.syncFeature(self.cgenImpl, "")
        self.cgenImpl.stmt("if (!instanceGetter(out, &out->instance)) return")

        for vulkanApi, typeDecl, feature in self.apisToGet["instance"]:
            self.syncFeature(self.cgenImpl, feature)
            self.makeGetProcAddr( \
                self.cgenImpl, "instance", "out->instance", vulkanApi.name, typeDecl);

        self.syncFeature(self.cgenImpl, "")

        self.cgenImpl.stmt("if (!deviceGetter(out, out->instance, &out->physicalDevice, &out->physicalDeviceQueueFamilyInfoCount, nullptr, &out->device, &out->presentCapable)) return")
        self.cgenImpl.stmt("out->physicalDeviceQueueFamilyInfos = (VkQueueFamilyProperties*)malloc(out->physicalDeviceQueueFamilyInfoCount * sizeof(VkQueueFamilyProperties))");
        self.cgenImpl.stmt("if (!deviceGetter(out, out->instance, &out->physicalDevice, &out->physicalDeviceQueueFamilyInfoCount, out->physicalDeviceQueueFamilyInfos, &out->device, &out->presentCapable)) return")

        for vulkanApi, typeDecl, feature in self.apisToGet["device"]:
            self.syncFeature(self.cgenImpl, feature)
            self.makeGetProcAddr( \
                self.cgenImpl, "device", "out->device", vulkanApi.name, typeDecl);

        self.syncFeature(self.cgenImpl, "")

        self.cgenImpl.endBlock()

        self.module.appendImpl(self.cgenImpl.swapCode())

    def onBeginFeature(self, featureName):
        VulkanDispatch.onBeginFeature(self, featureName);

    def onGenType(self, typeXml, name, alias):
        VulkanDispatch.onGenType(self, typeXml, name, alias);

    def onGenCmd(self, cmdinfo, name, alias):
        VulkanDispatch.onGenCmd(self, cmdinfo, name, alias);
