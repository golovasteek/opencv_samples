#include <cuda.h>
#include <cudaEGL.h>
#include <iostream>

#include "EGLStream/egl_common.h"
#include <thread>
#include <chrono>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>

using namespace std::chrono_literals;

constexpr int WIDTH = 720;
constexpr int HEIGHT = 480;

int main(int argc, char** argv) {
    egl::Display display;
    egl::Framework eglFramework;

    egl::Stream eglStream("/tmp/egl-stream.sock", egl::Stream::Endpoint::producer, eglFramework, display);

    EGLint streamState = 0;
    do {
        streamState = eglStream.queryState();
        std::cout << std::hex << "Stream state: " << streamState << std::endl;
        std::this_thread::sleep_for(30ms);
    } while (streamState != EGL_STREAM_STATE_CONNECTING_KHR);

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

    // Crate video capturing device and acquire one frame
    // to know dimentions of frames.
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Can not open camera" << std::endl;
    }

    cv::Mat frame;
    cv::cuda::GpuMat gpuFrame;
    cap >> frame;

    CUeglStreamConnection eglCudaConnection;
    cudaResult = cuEGLStreamProducerConnect(&eglCudaConnection, eglStream.get(), frame.size().width, frame.size().height);
    if (cudaResult != CUDA_SUCCESS) {
        const char* error;
        cuGetErrorString(cudaResult, &error);
        std::cout << "Can not connect to egl stream as producer: " << error << std::endl;
        return -1;
    }

    while (eglStream.queryState() != EGL_STREAM_STATE_DISCONNECTED_KHR) {
        gpuFrame.upload(frame);
        CHECK(gpuFrame.type() == CV_8UC3);
        std::cerr << "Image width: " << gpuFrame.size().width << std::endl;
        std::cerr << "Image height: " << gpuFrame.size().height << std::endl;
        std::cerr << "Image step: " << gpuFrame.step << std::endl;
        std::cerr << "Data prtr: " << std::hex << reinterpret_cast<uint64_t>(gpuFrame.data) << std::endl;

        CUeglFrame eglFrame;
        eglFrame.cuFormat = CU_AD_FORMAT_UNSIGNED_INT8;
        eglFrame.depth = 0;
        eglFrame.eglColorFormat = CU_EGL_COLOR_FORMAT_RGB;
        eglFrame.frameType = CU_EGL_FRAME_TYPE_PITCH;
        eglFrame.height = gpuFrame.size().height;
        eglFrame.width = gpuFrame.size().width;
        eglFrame.frame.pPitch[0] = gpuFrame.data;
        eglFrame.pitch = gpuFrame.step;
        eglFrame.planeCount = 1;
        eglFrame.numChannels = 3;

        CUstream cudaStream;
        cuStreamCreate(&cudaStream, 0);

        cudaResult = cuEGLStreamProducerPresentFrame(&eglCudaConnection, eglFrame, &cudaStream);
        if (cudaResult != CUDA_SUCCESS) {
            const char* errorName;
            cuGetErrorName(cudaResult, &errorName);
            const char* error;
            cuGetErrorString(cudaResult, &error);
            std::cout << "Failed to present frame: " << errorName << ": " << error << std::endl;
            return -1;
        }

        std::cout << "Presented frame..." << std::endl;
        cap >> frame;
        std::cout << "Captured next frame" << std::endl;
        
        while (true) {
            cudaResult = cuEGLStreamProducerReturnFrame(&eglCudaConnection, &eglFrame, &cudaStream);
            if (cudaResult == CUDA_ERROR_LAUNCH_TIMEOUT) {
                streamState = eglStream.queryState();
                std::cout << "Launch timeout, continue waiting. Stream state: 0x" << streamState << std::endl;
                if (streamState == EGL_STREAM_STATE_DISCONNECTED_KHR) {
                    std::cout << "Other stream end is disconnected, stopping" << std::endl;
                    break;
                } else {
                    std::this_thread::sleep_for(5ms);
                    continue;
                }
            }
            if (cudaResult != CUDA_SUCCESS) {
                const char* error;
                cuGetErrorString(cudaResult, &error);
                std::cout << "Return frame: " << error << std::endl;
                return -1;
            }
            break;
        }
    }
    std::this_thread::sleep_for(3s);
    cudaResult = cuEGLStreamProducerDisconnect(&eglCudaConnection);
    if (cudaResult != CUDA_SUCCESS) {
        std::cout << "Can not disconnect producer from eglStream" << std::endl;
        return -1;
    }

    return 0;
}