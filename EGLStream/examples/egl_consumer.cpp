#include <cuda.h>
#include <cudaEGL.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "EGLStream/egl_common.h"
#include <opencv2/core/cuda.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace std::chrono_literals;

int main(int argc, char** argv) {
    egl::Display display;
    egl::Framework eglFramework;

    egl::Stream eglStream("/tmp/egl-stream.sock", egl::Stream::Endpoint::consumer, eglFramework, display);


    EGLint streamState = 0;
    do {
        streamState = eglStream.queryState();
        std::cout << std::hex << "Waiting producer to connect. Stream state: " << streamState << std::endl;
        std::this_thread::sleep_for(30ms);
    } while (streamState == EGL_STREAM_STATE_INITIALIZING_NV);

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

    CUstream cudaStream;
    cuStreamCreate(&cudaStream, 0);


    cv::namedWindow("Frame", cv::WINDOW_NORMAL);
    while(true) {
        do {
            streamState = eglStream.queryState();
            std::cout << std::hex << "Waiting for frames. Stream state: " << streamState << std::endl;
            if(cv::waitKey(5) == 27) {
                break;
            }
            std::this_thread::sleep_for(30ms);
        } while(streamState != EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR && streamState != EGL_STREAM_STATE_DISCONNECTED_KHR);

        if (streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
            std::cout << "Stream disconnected" << std::endl;
            break;
        }

        CUgraphicsResource cudaResource;
        cudaResult = cuEGLStreamConsumerAcquireFrame(&eglCudaConnection, &cudaResource, &cudaStream, 16000);
        if (cudaResult != CUDA_SUCCESS) {
            const char* error;
            cuGetErrorString(cudaResult, &error);
            std::cout << "Can not acquire cuda frame: " << error << std::endl;
            return -1;
        }
        std::cout << "Frame acquired" << std::endl;

        CUeglFrame eglFrame;
        cudaResult = cuGraphicsResourceGetMappedEglFrame(&eglFrame, cudaResource, 0, 0);
        if (cudaResult != CUDA_SUCCESS) {
            const char* error;
            cuGetErrorString(cudaResult, &error);
            std::cout << "Can not get EGL frame from resource: " << error << std::endl;
            return -1;
        }

        cv::Mat cpuMat;

        {
            cv::cuda::GpuMat frameWrapper(eglFrame.height, eglFrame.width, CV_8UC3, eglFrame.frame.pPitch[0], eglFrame.pitch);
            frameWrapper.download(cpuMat);
            cv::imshow("Camera frame", cpuMat);
        }
        
        cudaResult = cuEGLStreamConsumerReleaseFrame(&eglCudaConnection, cudaResource, &cudaStream);
        if (cudaResult != CUDA_SUCCESS) {
            const char* error;
            cuGetErrorString(cudaResult, &error);
            std::cout << "Can not release frame: " << error << std::endl;
            return -1;
        }
        std::cout << "Frame released" << std::endl;
    }

    cudaResult = cuEGLStreamConsumerDisconnect(&eglCudaConnection);
    if (cudaResult != CUDA_SUCCESS) {
        std::cout << "Can not disconnect consumer from eglStream" << std::endl;
        return -1;
    }

    return 0;
}