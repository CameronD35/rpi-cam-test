// C++ includes
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory>
#include <thread>

// libcamera includes
#include <libcamera/libcamera.h>

// OpenCV includes
// #include <opencv2/core.hpp>
// #include <opencv2/imgproc.hpp>
// #include <opencv2/videoio.hpp>
// #include <opencv2/highgui.hpp>

// using namespace cv;
using namespace libcamera;

static std::shared_ptr<Camera> camera;

int main(int argc, char** argv) {

    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    for (auto const &camera : cm->cameras()) {

        std::cout << "hi there " << camera->id() << std::endl;

    }

    auto cameras = cm->cameras();

    if (cameras.empty()) {

        std::cout << "No cameras found." << std::endl;
        cm->stop();

        return EXIT_FAILURE;

    }

    std::string cameraId = cameras[0]->id();

    camera = cm->get(cameraId);

    // prevents another application from stealing the camera
    camera->acquire();
    
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );

    StreamConfiguration &streamConfig = config->at(0);

    std::cout << "Default viewfinder config is: " << streamConfig.toString() << std::endl;

    config->validate();
    std::cout << "Validated viewfinder config is: " << streamConfig.toString() << std::endl;

    camera->configure(config.get());


    camera->stop();
    camera->release();
    camera.reset();
    cm->stop();

    return 0;

}