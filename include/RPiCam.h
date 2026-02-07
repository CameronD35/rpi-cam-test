#include <libcamera/libcamera.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <thread>
#include <sys/mman.h>

using namespace libcamera;

#ifndef RPICAM_H
#define RPICAM_H

class RPiCam {

    private:

        std::shared_ptr<Camera> camera;
        std::unique_ptr<CameraConfiguration> config;
        std::unique_ptr<FrameBufferAllocator> allocator;
        std::vector<std::unique_ptr<Request>> requests;
        Stream* stream = nullptr;

        void requestComplete(Request *request);
        int allocateBuffers();
        void* mmapPlane(const FrameBuffer::Plane &plane);

    public:

        std::string id;
        std::string format;

        // stops camera, frees allocator and memory, and releases camera 
        void reset();

        // sets up camera with all the configurations needed
        int setup();

        // constructor
        RPiCam(CameraManager &manager, std::string id);

};

#endif