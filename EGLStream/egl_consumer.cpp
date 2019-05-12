#include <cudaEGL.h>
#include <iostream>
#include <EGL/egl.h>

EGLStreamKHR eglStream;
EGLDisplay eglDisplay;


int main(int argc, char** argv) {
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj;
    EGLint min;
    EGLBoolean initialized = eglInitialize(eglDisplay, &maj, &min);
    if (initialized) {
        std::cout << "Hello EGL " << maj << "." << min << std::endl;
        eglTerminate(eglDisplay);
    } else {
        std::cout << "Egl is not initialized";
    }
    return 0;
}