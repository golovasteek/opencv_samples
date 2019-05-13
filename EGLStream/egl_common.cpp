#include "egl_common.h"

#include <stdexcept>
#include <iostream>

namespace egl {

template<class FunctionType>
FunctionType getExtFunction(const std::string& name)
{
    auto result = FunctionType(eglGetProcAddress(name.c_str()));
    if (result == nullptr) {
        throw std::runtime_error(name + " function is not found");
    }
    return result;
}

#define INIT_EXT_FUNCTION(name) \
    name(getExtFunction<name ## _type>(#name))


Framework::Framework()
    : INIT_EXT_FUNCTION(eglCreateStreamKHR)
    , INIT_EXT_FUNCTION(eglDestroyStreamKHR)
{

}

Display::Display()
    : display_(eglGetDisplay(EGL_DEFAULT_DISPLAY))
{
    EGLint maj;
    EGLint min;
    EGLBoolean initialized = eglInitialize(display_, &maj, &min);
    if (!initialized) {
        throw Error("Egl is not initialized");
    }
    std::cerr << "Hello EGL " << maj << "." << min << std::endl;
    
    std::cerr << "EGL_CLIENT_APIS: " << eglQueryString(display_, EGL_CLIENT_APIS) << std::endl;
    std::cerr << "EGL_EXTENSIONS: " << eglQueryString(display_, EGL_EXTENSIONS) << std::endl;
    std::cerr << "EGL_VENDOR: " << eglQueryString(display_, EGL_VENDOR) << std::endl;
    std::cerr << "EGL_VERSION: " << eglQueryString(display_, EGL_VERSION) << std::endl;
}

Display::~Display()
{
    auto status = eglTerminate(display_);
    if (!status) {
        std::cerr << "Termination failed" << std::endl;
    }

    std::cerr << "EGL Termindated" << std::endl;
}

Stream::Stream(const Framework& framework, const Display& d)
    : framework_(framework)
    , display_(d)
{
    EGLint streamAttributeList[] = {
        EGL_SUPPORT_REUSE_NV, EGL_FALSE,
        EGL_CONSUMER_LATENCY_USEC_KHR, 16000,
        EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, 16000,
        EGL_NONE };
    stream_ = framework_.eglCreateStreamKHR(d.get(), streamAttributeList);

    if (stream_ == EGL_NO_STREAM_KHR) {
        throw Error("Can not create stream");
    }
}

Stream::~Stream()
{
    auto status = framework_.eglDestroyStreamKHR(display_.get(), stream_);
    if (!status) {
        std::cerr << "Can not destroy stream" << std::endl;
    } else {
        std::cerr << "Stream is destroyed" << std::endl;
    }
}

}