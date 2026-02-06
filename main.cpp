// this code was made following this tutorial: https://libcamera.org/guides/application-developer.html

// C++ includes
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <memory>
#include <thread>

// libcamera includes
#include <libcamera/libcamera.h>

using namespace libcamera;
using namespace std::chrono_literals;

class RPiCam {

    private:

        std::shared_ptr<Camera> camera;

        std::unique_ptr<CameraConfiguration> config;
        std::unique_ptr<FrameBufferAllocator> allocator;
        Stream* stream = nullptr;
        std::vector<std::unique_ptr<Request>> requests;

        void requestComplete(Request *request);

    public:

        std::string id;
        std::string format;

        // stops camera, frees allocator and memory, and releases camera 
        void reset();
        int setup();
        int allocateBuffers();

        RPiCam(CameraManager &manager, std::string id);
};

RPiCam::RPiCam(CameraManager &manager, std::string id) {

    this->id = id;

    camera = manager.get(id);

}

void RPiCam::reset() {

    camera->stop();
    camera->release();
    allocator->free(stream);
    allocator.reset();
    camera.reset();

}

void RPiCam::requestComplete(Request *request) {

    // sees if request was cancelled
    if (request->status() == Request::RequestCancelled) { return; }

    // map of FrameBuffer instances associated with Stream that produced the images
    const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

    // loops through every buffer in the request and accesses its metadata
    for (auto bufferPair : buffers) {

        FrameBuffer *buffer = bufferPair.second;
        const FrameMetadata &metadata = buffer->metadata();

        // prints frame sequence # and details of planes (what's a plane?)
        std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

        unsigned int nplane = 0;

        for (const FrameMetadata::Plane &plane : metadata.planes()) {

            std::cout << plane.bytesused;
            if (++nplane < metadata.planes().size()) std::cout << "/";

        }

        std::cout << std::endl;

        // reuses request and re-queues it to the camera
        request->reuse(Request::ReuseBuffers);
        this->camera->queueRequest(request);

    }

}

int RPiCam::allocateBuffers() {

    for (StreamConfiguration &cfg : *(config)) {

        int ret = allocator->allocate(cfg.stream());

        if (ret < 0) {

            std::cerr << "Cannot allocate buffers." << std::endl;
            return -ENOMEM;

        }
        
        size_t allocated = allocator->buffers(cfg.stream()).size();
        // std::cout << "Allocated " << allocated << " buffers for stream." << std::endl;
    
    }

    return 0;

}

int RPiCam::setup() {

    // prevents another application from stealing and running off with the camera
    camera->acquire();
    
    config = camera->generateConfiguration( { StreamRole::Viewfinder } );


    
    // chooses the first (and only) config available for the camera
    StreamConfiguration& streamConfig = config->at(0);
    std::cout << "Default viewfinder config is: " << streamConfig.toString() << std::endl;

    // used if we were to adjust the output sizing stored in streamConfig
    config->validate();
    std::cout << "Validated viewfinder config is: " << streamConfig.toString() << std::endl;

    // provides a validated config
    camera->configure(config.get());

    format = streamConfig.pixelFormat.toString();

    allocator = std::make_unique<FrameBufferAllocator>(camera);

    // fills request vector by creating Request instances from camera and associates a buffer for each of them
    this->allocateBuffers();

    stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    // const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    // std::vector<std::unique_ptr<Request>> requests;

    // fills request vector by creating Request instances from camera and associates a buffer for each of them
    for (unsigned int i = 0; i < buffers.size(); ++i) {

        std::unique_ptr<Request> request = camera->createRequest();

        if (!request) {

            std::cerr << "Cannot create request." << std::endl;
            return -ENOMEM;

        }

        const std::unique_ptr<FrameBuffer> &buffer = buffers[i];
        int ret = request->addBuffer(stream, buffer.get());

        if (ret < 0) {

            std::cerr << "Cannot set buffer for request." << std::endl;
            return ret;

        }

        requests.push_back(std::move(request));

    }

    // uses concept of signals and slots (?)
    // Camera::bufferCompleted notifies apps that a buffer with img data is available
    // Camera::requestCompleted notifies apps that a request is completed and therefore all the buffers
    // within it are completed

    // connects slot function
    camera->requestCompleted.connect(this, &RPiCam::requestComplete);

    // std::cout << "hello" << std::endl;

    // starts camera and queues up all previously created requests
    camera->start();

    for (std::unique_ptr<Request> &request : requests) {
        std::cout << "hello" << std::endl;
        camera->queueRequest(request.get());

    }

    return 0;

}

static std::shared_ptr<Camera> camera;

// static void requestComplete(Request *request);
std::vector<std::string> getCameras(CameraManager& cameraManager);
// StreamConfiguration& setupCamera(Camera& cam, std::unique_ptr<CameraConfiguration>& config);

int main(int argc, char** argv) {

    // create a camera manager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // grabs all the cameras available and prints their names
    std::vector<std::string> cameraIDs;
    cameraIDs = getCameras(*cm);

    if (cameraIDs.size() == 0) { std::cout << "No cameras found." << std::endl; }

    // grabs the first camera available
    std::string cameraId = cameraIDs[0];

    RPiCam* cam1 = new RPiCam(*cm, cameraId);

    cam1->setup();

    std::this_thread::sleep_for(3000ms);

    cam1->reset();

    return 0;

}

// static void requestComplete(Request *request) {

//     // sees if request was cancelled
//     if (request->status() == Request::RequestCancelled) { return; }

//     // map of FrameBuffer instances associated with Stream that produced the images
//     const std::map<const Stream *, FrameBuffer *> &buffers = request->buffers();

//     // loops through every buffer in the request and accesses its metadata
//     for (auto bufferPair : buffers) {

//         FrameBuffer *buffer = bufferPair.second;
//         const FrameMetadata &metadata = buffer->metadata();

//         // prints frame sequence # and details of planes (what's a plane?)
//         std::cout << " seq: " << std::setw(6) << std::setfill('0') << metadata.sequence << " bytesused: ";

//         unsigned int nplane = 0;

//         for (const FrameMetadata::Plane &plane : metadata.planes()) {

//             std::cout << plane.bytesused;
//             if (++nplane < metadata.planes().size()) std::cout << "/";

//         }

//         std::cout << std::endl;

//         // reuses request and re-queues it to the camera
//         request->reuse(Request::ReuseBuffers);
//         camera->queueRequest(request);

//     }

// }

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

// StreamConfiguration& setupCamera(Camera &cam, std::unique_ptr<CameraConfiguration>& config) {

    // prevents another application from stealing and running off with the camera
//     cam.acquire();
    
//     config = cam.generateConfiguration( { StreamRole::Viewfinder } );

//     // chooses the first (and only) config available for the camera
//     StreamConfiguration &streamConfig = config->at(0);
//     std::cout << "Default viewfinder config is: " << streamConfig.toString() << std::endl;

//     // used if we were to adjust the output sizing stored in streamConfig
//     config->validate();
//     std::cout << "Validated viewfinder config is: " << streamConfig.toString() << std::endl;

//     // provides a validated config
//     cam.configure(config.get());

//     return streamConfig;

// }