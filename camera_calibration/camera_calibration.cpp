#include <opencv2/videoio.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/cudawarping.hpp>

#include <chrono>

#include <iostream>

using namespace std::chrono_literals;

namespace {
cv::Size boardSize{7, 5};
int chessBoardFlags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;

const float SQUARE_SIZE = 28;

const size_t PATTERNS = 20;

std::vector<cv::Point3f> calcCornersPositions()
{
    std::vector<cv::Point3f> result;
    for (int i = 0; i < boardSize.height; ++i) {
        for (int j = 0; j < boardSize.width; ++j) {
            result.push_back(
                cv::Point3f(j * SQUARE_SIZE, i * SQUARE_SIZE, 0)
            );
        }
    }
    return result;
}

std::chrono::steady_clock::time_point now() {
    return std::chrono::steady_clock::now();
}

}

int main(int argc, char** argv)
{

    cv::VideoCapture camera(0);
    cv::namedWindow("Display image");

    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distCoeff = cv::Mat::zeros(8, 1, CV_64F);

    cv::Mat map1;
    cv::Mat map2;
    cv::Mat cameraFrame;
    camera >> cameraFrame;
    cv::Size imageSize = cameraFrame.size();

    bool showUndistored = true;

    auto prevFrame = now();
    for (;;) {
        cv::initUndistortRectifyMap(
            cameraMatrix, distCoeff,
            cv::Mat(), cameraMatrix,
            imageSize, CV_32FC1, map1, map2);

        cv::cuda::GpuMat xmap(map1);
        cv::cuda::GpuMat ymap(map2);
        for (;;) {
            cv::Mat cameraFrame;
            camera >> cameraFrame;
            std::vector<cv::Point2f> pointBuf;
            bool found = cv::findChessboardCorners(cameraFrame, boardSize, pointBuf, chessBoardFlags);

            auto displayFrame = cameraFrame;

            if (found) {
                cv::drawChessboardCorners(
                    displayFrame, boardSize, cv::Mat(pointBuf), found
                );

                if (now() - prevFrame > 500ms) {
                    imagePoints.push_back(pointBuf);
                    prevFrame = now();
                    std::cout << "Captured pattern: " << imagePoints.size() << std::endl;
                    if (imagePoints.size() > PATTERNS) {
                        break;
                    }
                }
            }
            cv::cuda::GpuMat gpuDisplayFrame(displayFrame);
            if (showUndistored) {
                cv::cuda::GpuMat temp;
                cv::remap(gpuDisplayFrame, temp, map1, map2, cv::INTER_LINEAR);
                gpuDisplayFrame = temp;
            }
            cv::imshow("Display image", gpuDisplayFrame);
            int key = cv::waitKey(30);
            if (key == 27) {
                return 0;
            }
            if (key != -1) {
                std::cerr << "Key: " << key << std::endl;
            }
            if (key == 'u') {
                showUndistored = !showUndistored;
                std::cout << "Undistortion " << (showUndistored ? "on" : "off") << std::endl;
            }
        }

        
        std::vector<std::vector<cv::Point3f>> objectPoints(
            imagePoints.size(), calcCornersPositions());

        std::vector<cv::Mat> rvecs;
        std::vector<cv::Mat> tvecs;

        auto rms = cv::calibrateCamera(
            objectPoints, imagePoints,
            imageSize,
            cameraMatrix, distCoeff,
            rvecs, tvecs,
            CV_CALIB_FIX_K4|CV_CALIB_FIX_K5
        );
        imagePoints.clear();

        std::cout << "Calibrated camera with error:" << rms;
    }

    return 0;
}