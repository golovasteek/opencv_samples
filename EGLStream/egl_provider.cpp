#include <cuda.h>
#include <cudaEGL.h>
#include <iostream>

#include "egl_common.h"

int main(int argc, char** argv) {
    egl::Display display;
    egl::Framework eglFramework;

    egl::Stream eglStream(eglFramework, display);

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
    cudaResult = cuEGLStreamConsumerConnect(&eglCudaConnection, eglStream.get());
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

    return 0;
}