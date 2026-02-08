// this code was made following this tutorial: https://libcamera.org/guides/application-developer.html
#include "RPiCam.h"

using namespace libcamera;
using namespace std::chrono_literals;

std::vector<std::string> getCameras(CameraManager& cameraManager);
int runCam(RPiCam* cam);

int main(int argc, char** argv) {

    // create a camera manager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // grabs all the cameras available and prints their names
    std::vector<std::string> cameraIDs;
    cameraIDs = getCameras(*cm);

    // ensures we actually got cameras fr
    if (cameraIDs.size() == 0) { std::cout << "No cameras found." << std::endl; cm->stop(); return -1; }

    // grabs the first camera available
    std::string cameraId_1 = cameraIDs[0];
    std::string cameraId_2 = cameraIDs[1];

    RPiCam* cam1 = new RPiCam(*cm, cameraId_1);
    RPiCam* cam2 = new RPiCam(*cm, cameraId_2);

    std::thread cam1_thread{runCam, cam1};
    cam1_thread.detach();

    std::thread cam2_thread{runCam, cam2};
    cam2_thread.detach();

    std::this_thread::sleep_for(12000ms);

    return 0;

}

int runCam(RPiCam* cam) {

    std::cout << "hello" << std::endl;

    cam->setup();

    cam->record();

    std::this_thread::sleep_for(10000ms);

    cam->reset();

    return 0;

}

// gets all of the cameras using the camera manager 
std::vector<std::string> getCameras(CameraManager& cameraManager) {

    auto cameras = cameraManager.cameras();
    std::vector<std::string> cameraIDs;

    if (cameras.empty()) {

        std::cout << "No cameras found." << std::endl;
        cameraManager.stop();

        // returns empty vector
        return std::vector<std::string>();

    }

    for (const auto &camera : cameras) {

        cameraIDs.push_back(camera->id());

    }

    return cameraIDs;

}