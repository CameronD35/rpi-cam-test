# RPi Cam Test (NoIR RPi Cam Module 3)
This program/class is used to detect the first two cameras attached to a Raspberry Pi, connect their output to an OpenCV file stream, and, for debugging, display the footage on the screen.

This program uses the following libraries:
* [OpenCV](https://docs.opencv.org/4.12.0/index.html)
* [libcamera](https://libcamera.org)

## Building the Program
To build the project use [CMake](https://cmake.org).


## What You Can Quickly Change
_`main.cpp` contains the key parameters. Below are the changes you can make without changing the `RPiCam` class._
* If you wish to change the resolution, refer to line 23 and change the resolution to your desired width and height with:
  ```cpp
  int res[2] = {width, height};
  ```
* If you wish to change the framerate on a specific camera (and yes, you can have different frame rates on each camera), refer to lines 29 and 30 to make a change on cam 1 or 2, respectively:

  ```cpp
                                             vvvvvvvvv
  RPiCam* cam1 = new RPiCam(*cm, cameraId_1, framerate, res);
  RPiCam* cam2 = new RPiCam(*cm, cameraId_2, framerate, res);
  ```
* To turn off one camera, you can simply comment out line 35 (see TODO):

  ```cpp
  // A feature to parameterize this will come soon!
  std::thread cam2_thread{runCam, cam2};
  cam2_thread.detach();
  ```
## What You Can Otherwise Change
_`/src/RPiCam.cpp` contains all of the code the `RPiCam` class uses under the hood. You can make more changes here, but be careful, it may cause bugs!_
* If you wish to chaneg the video output format you can head to line 130 (I had trouble with the `.mp4` format, but it's likely possible!):

  ```cpp
  writer = cv::VideoWriter(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), static_cast<double>(fps), cv::Size(streamConfig.size.width, streamConfig.size.height));
  ```
* To adjust autofocus, head to line 150:

```cpp
controls.set(controls::AfMode, controls::AfModeContinuous);
```
_It may be helpful to refer to the official [libcamera documentation on libcamera::ControlList class](https://libcamera.org/api-html/classlibcamera_1_1ControlList.html)._

* While you may change anything else, anything not mentioned is pretty much core to how the program runs and is unlikely to change the output!

## TODO
_Below are features I wish/need to add_
#### * Add much more detail to the build process! *
* Add a rolling file system
  * Create a folder for each run of the application
  * In each folder, save a file of the left and right camera with the filename format `cam_{L/R}_{seq#}_{res}.avi`
* ~~Optimise the camera output~~
  * ~~Currently, the camera _can_ output a 1280x720 (720p) resolution but seems to only record at 15fps in such cases~~
* ~~Add arguments that can be passed into the application for~~
  * ~~Resolution~~
  * ~~Framerate~~
  * ~~\# of cameras~~
  * ~~Length of recording~~
  * ~~Footage GUI display on/off~~
* Add much more detail to the build process
