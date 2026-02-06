#include "RPiCam.h"

using namespace libcamera;

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