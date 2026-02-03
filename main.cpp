// code built with help of https://docs.opencv.org/4.x/d5/dc4/tutorial_video_input_psnr_ssim.html

// C++ includes
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

// OpenCV includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

    if (argc != 3) {

        cout << "Missing parameters." << endl;
        return -1;

    }

    Mat image;
    image = imread(argv[1], IMREAD_COLOR);

    if (!image.data) {

        printf("No image data.\n");
        return -1;

    }

    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", image);

    waitKey(0);

    return 0;
    
}