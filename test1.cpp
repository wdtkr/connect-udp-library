#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{
    cv::Mat img = cv::imread("apple.png");

    cv::namedWindow("test", cv::WINDOW_AUTOSIZE);
    cv::imshow("test", img);
    cv::waitKey(0);
    cv::destroyWindow("test");
    return 0;
}
