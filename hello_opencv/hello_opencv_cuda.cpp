#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <image file>" << std::endl;
        return 1;
    }
    std::string imageName = argv[1];

    cv::Mat image;
    image = cv::imread(imageName, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Couldn't read the image file " << imageName;
        return 2;
    }

    cv::imshow("Display window", image);
    cv::waitKey(0);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Can not open camera" << std::endl;
    }

    cv::Mat frame;
    cv::cuda::GpuMat gpuFrame;
    cv::cuda::GpuMat edges;
    
    cv::namedWindow("edges", cv::WINDOW_OPENGL);
    auto blurFilter = cv::cuda::createGaussianFilter(
        gpuFrame.type(),
        gpuFrame.type(),
        cv::Size(7, 7),
        1.5, 1.5
    );

    auto edgeDetector = cv::cuda::createCannyEdgeDetector(
        0, 30, 3
    );

    for(;;) {
        cap >> frame;
        gpuFrame.upload(frame);

        cv::cuda::cvtColor(gpuFrame, edges, cv::COLOR_BGR2GRAY);
        blurFilter->apply(edges, edges);
        edgeDetector->detect(edges, edges);
        cv::imshow("edges", edges);
        if(cv::waitKey(30) >= 0) {
            break;
        }

    }

    return 0;
}
