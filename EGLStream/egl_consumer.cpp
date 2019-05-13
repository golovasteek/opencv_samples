#include <cuda.h>
#include <cudaEGL.h>
#include <iostream>
#include <EGL/egl.h>

#include "egl_common.h"

EGLStreamKHR eglStream;
EGLDisplay eglDisplay;


int main(int argc, char** argv) {
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj;
    EGLint min;
    EGLBoolean initialized = eglInitialize(eglDisplay, &maj, &min);
    if (!initialized) {
        std::cout << "Egl is not initialized";
        return -1;
    }
    std::cout << "Hello EGL " << maj << "." << min << std::endl;
    
    std::cout << "EGL_CLIENT_APIS: " << eglQueryString(eglDisplay, EGL_CLIENT_APIS) << std::endl;
    std::cout << "EGL_EXTENSIONS: " << eglQueryString(eglDisplay, EGL_EXTENSIONS) << std::endl;
    std::cout << "EGL_VENDOR: " << eglQueryString(eglDisplay, EGL_VENDOR) << std::endl;
    std::cout << "EGL_VERSION: " << eglQueryString(eglDisplay, EGL_VERSION) << std::endl;

    size_t configSize = 100;
    EGLint numConfigs;
    EGLConfig configs[configSize];
    auto status = eglGetConfigs(eglDisplay, configs, configSize, &numConfigs);
    if (!status) {
        std::cout << "Can not get configs" << std::endl;
        return -1;
    }
    std::cout << "numConfigs: " << numConfigs << std::endl;

    EGLStreamExtInit();

    EGLint streamAttributeList[] = {
        EGL_SUPPORT_REUSE_NV, EGL_FALSE,
        EGL_CONSUMER_LATENCY_USEC_KHR, 16000,
        EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, 16000,
        EGL_NONE };
    EGLStreamKHR eglStream = eglCreateStreamKHR(eglDisplay, streamAttributeList);

    if (eglStream == EGL_NO_STREAM_KHR) {
        std::cout << "Can not create stream" << std::endl;
        return -1;
    }

    auto cudaResult = cuInit(0);
    if ( cudaResult != CUDA_SUCCESS) {
        const char* error;
        cuGetErrorString(cudaResult, &error);
        std::cout << "Can not initialize CUDA: " << error << std::endl;
        return -1;
    }

    int deviceCount = 0;
    cuDeviceGetCount(&deviceCount);
    if (deviceCount == 0) {
        std::cout << "No cuda devices found" << std::endl;
        return -1;
    }

    // Connecting to device 0;
    CUdevice device;
    cuDeviceGet(&device, 0);

    CUcontext cuContext;
    cuCtxCreate(&cuContext, 0, device);

    CUeglStreamConnection eglCudaConnection;
    cudaResult = cuEGLStreamConsumerConnect(&eglCudaConnection, eglStream);
    if (cudaResult != CUDA_SUCCESS) {
        const char* error;
        cuGetErrorString(cudaResult, &error);
        std::cout << "Can not connect to egl stream as consumer: " << error << std::endl;
        return -1;
    }

    cudaResult = cuEGLStreamConsumerDisconnect(&eglCudaConnection);
    if (cudaResult != CUDA_SUCCESS) {
        std::cout << "Can not disconnect consumer from eglStream" << std::endl;
        return -1;
    }

    status = eglDestroyStreamKHR(eglDisplay, eglStream);
    if (!status) {
        std::cout << "Can not destroy stream" << std::endl;
        return -1;
    } else {
        std::cout << "Stream is destroyed" << std::endl;
    }

    status = eglTerminate(eglDisplay);
    if (!status) {
        std::cout << "Termination failed" << std::endl;
    }

    std::cout << "EGL Termindated" << std::endl;
        
    return 0;
}