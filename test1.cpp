#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>

int main()
{
    // openCVでカメラを起動する
    cv::VideoCapture capture(0); // 0 is the ID for the default camera
    if (!capture.isOpened())
    {
        std::cerr << "Error opening camera" << std::endl;
        return -1;
    }

    // Set up for encoding with ffmpeg
    int frame_width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = 10; // Frames per second

    // Start child process for ffmpeg input
    std::string ffmpeg_command = "ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt bgr24 -s " +
                                 std::to_string(frame_width) + "x" + std::to_string(frame_height) +
                                 " -r " + std::to_string(fps) +
                                 " -i - -c:v libx264 -pix_fmt yuv420p -f rtp rtp://127.0.0.1:1234";

    // Open pipe to ffmpeg
    FILE *ffmpeg = popen(ffmpeg_command.c_str(), "w");
    if (!ffmpeg)
    {
        std::cerr << "Could not open pipe to ffmpeg" << std::endl;
        return -1;
    }

    // Using chrono for timing
    auto start_time = std::chrono::high_resolution_clock::now();
    cv::Mat frame;

    // openCVで5秒間カメラをキャプチャする
    while (std::chrono::high_resolution_clock::now() - start_time < std::chrono::seconds(5))
    {
        capture >> frame; // Capture a frame from the camera

        // Check if the frame is empty
        if (frame.empty())
        {
            std::cerr << "Received empty frame" << std::endl;
            break;
        }

        // Write frame to ffmpeg
        size_t count = fwrite(frame.data, 1, frame.total() * frame.elemSize(), ffmpeg);
        if (count != frame.total() * frame.elemSize())
        {
            std::cerr << "Error writing frame data to ffmpeg" << std::endl;
            break;
        }

        // Introduce a delay for the specified fps
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }

    // Flush and close pipe to ffmpeg
    fflush(ffmpeg);
    pclose(ffmpeg);

    // Release the camera
    capture.release();

    return 0;
}
