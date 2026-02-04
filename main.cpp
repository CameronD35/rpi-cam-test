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

static std::shared_ptr<Camera> camera;

static void requestComplete(Request *request);

int main(int argc, char** argv) {

    // create a camer manager
    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();
    cm->start();

    // grabs all the cameras available and prints their names
    auto cameras = cm->cameras();
    for (auto const &camera : cameras) {

        std::cout << camera->id() << std::endl;

    }

    if (cameras.empty()) {

        std::cout << "No cameras found." << std::endl;
        cm->stop();

        return EXIT_FAILURE;

    }

    // grabs the first camera available
    std::string cameraId = cameras[0]->id();
    camera = cm->get(cameraId);

    // prevents another application from stealing and running off with the camera
    camera->acquire();
    
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration( { StreamRole::Viewfinder } );

    // chooses the first (and only) config available for the camera
    StreamConfiguration &streamConfig = config->at(0);
    std::cout << "Default viewfinder config is: " << streamConfig.toString() << std::endl;

    // used if we were to adjust the output sizing stored in streamConfig
    config->validate();
    std::cout << "Validated viewfinder config is: " << streamConfig.toString() << std::endl;

    // provides a validated config
    camera->configure(config.get());

    // allocates memory for the camera and returns the amount of buffers created
    FrameBufferAllocator *allocator = new FrameBufferAllocator(camera);

    for (StreamConfiguration &cfg : *config) {

        int ret = allocator->allocate(cfg.stream());

        if (ret < 0) {

            std::cerr << "Cannot allocate buffers." << std::endl;
            return -ENOMEM;

        }
        
        size_t allocated = allocator->buffers(cfg.stream()).size();
        std::cout << "Allocated " << allocated << " buffers for stream." << std::endl;
    
    }

    // retrieves list of buffers using the allocator
    // creates vector of requests to be submitted to camera
    Stream *stream = streamConfig.stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator->buffers(stream);
    std::vector<std::unique_ptr<Request>> requests;

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
    camera->requestCompleted.connect(requestComplete);

    // starts camera and queues up all previously created requests
    camera->start();
    for (std::unique_ptr<Request> &request : requests) {

        camera->queueRequest(request.get());

    }

    // since cameramanager spawns its own thread, our app can whatever it likes and only needs to respond to signals emitted by libcamera
    // henchforth, we pause the main app for 3 seconds so it doesn't automatically terminate
    std::this_thread::sleep_for(3000ms);

    // stops camera, frees allocator and memory, releases camera, then stops camera manager
    camera->stop();
    allocator->free(stream);
    delete allocator;
    camera->release();
    camera.reset();
    cm->stop();

    return 0;

}

static void requestComplete(Request *request) {

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
        camera->queueRequest(request);

    }

}