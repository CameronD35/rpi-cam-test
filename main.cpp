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

    Mat frame;

    VideoCapture capture;

    int deviceID = 0;
    int apiID = CAP_ANY;

    capture.open(deviceID, apiID);

    if (!capture.isOpened()) {

        cerr << "Cannot find camera." << endl;

    }

    for (;;) {

        capture.read(frame);

        if (frame.empty()) {

            cerr << "Recieved empty frame." << endl;
            break;

        }

        imshow("Live", frame);

        if (waitKey(5) >= 0) {

            break;

        }

    }

    return 0;
    
}