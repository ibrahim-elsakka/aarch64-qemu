LOCAL_PATH := $(call my-dir)

host_common_SRC_FILES :=     \
     etc.cpp                 \
     FramebufferData.cpp     \
     GLBackgroundLoader.cpp  \
     GLDispatch.cpp          \
     GLutils.cpp             \
     GLEScontext.cpp         \
     GLESvalidate.cpp        \
     GLESpointer.cpp         \
     GLESbuffer.cpp          \
     NamedObject.cpp         \
     ObjectData.cpp          \
     ObjectNameSpace.cpp     \
     PaletteTexture.cpp      \
     RangeManip.cpp          \
     SaveableTexture.cpp     \
     ScopedGLState.cpp       \
     ShareGroup.cpp          \
     TextureData.cpp         \
     TextureUtils.cpp

host_GL_COMMON_LINKER_FLAGS :=
host_common_LDLIBS :=
host_common_LDFLAGS :=

ifeq ($(BUILD_TARGET_OS),linux)
#    host_common_LDFLAGS += -Wl,--whole-archive
    host_common_LDLIBS += -ldl
    host_common_LDFLAGS += -Wl,-Bsymbolic
endif

ifeq ($(BUILD_TARGET_OS),windows)
    host_common_LDLIBS += -lgdi32
    host_common_LDFLAGS += -Wl,--add-stdcall-alias
endif

### EGL host implementation ########################

$(call emugl-begin-static-library,libGLcommon)

LOCAL_SRC_FILES := $(host_common_SRC_FILES)
$(call emugl-export,LDLIBS,$(host_common_LDLIBS))
$(call emugl-export,LDFLAGS,$(host_common_LDFLAGS))
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH)/../include $(EMUGL_PATH)/shared $(METRICS_PROTO_INCLUDES) $(EMUGL_PATH)/../../../astc-codec/include $(generated-proto-sources-dir) $(PROTOBUF_INCLUDES))
$(call emugl-export,STATIC_LIBRARIES, libemugl_common $(ALL_PROTOBUF_LIBS) $(PROTOBUF_STATIC_LIBRARIES))

$(call emugl-end-module)

### EGL unit tests ########################

$(call emugl-begin-executable,lib$(BUILD_TARGET_SUFFIX)GLcommon_unittests)

LOCAL_SRC_FILES := Etc2_unittest.cpp
$(call emugl-import,libGLcommon libemugl_gtest)
$(call local-link-static-c++lib)
$(call emugl-end-module)

