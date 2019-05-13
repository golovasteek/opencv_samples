#include "egl_common.h"

#include <stdexcept>

eglCreateStreamKHR_type eglCreateStreamKHR = nullptr;
eglDestroyStreamKHR_type eglDestroyStreamKHR = nullptr;

void EGLStreamExtInit()
{
    eglCreateStreamKHR = eglCreateStreamKHR_type(eglGetProcAddress("eglCreateStreamKHR"));
    if (eglCreateStreamKHR == nullptr) {
        throw std::runtime_error("eglCreateStream function is not found");
    }

    eglDestroyStreamKHR = eglDestroyStreamKHR_type(eglGetProcAddress("eglDestroyStreamKHR"));
    if (eglDestroyStreamKHR == nullptr) {
        throw std::runtime_error("eglDestroyStreamKHR function is not found");
    }
}