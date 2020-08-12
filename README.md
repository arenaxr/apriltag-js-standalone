# Apriltag Detector Standalone Repo

Apriltag detector using the apriltag C library at [https://github.com/AprilRobotics/apriltag](https://github.com/AprilRobotics/apriltag), and compiled to WASM using emscripten.

This is the main WASM apriltag detector source, with additional tests and a standalone javascript page that displays the detector output. This allows to develop and test the detector, and then transfer the source to the main ARENA-core source.

## Contents

- **apriltag**: submodule of the apriltag library source repository ([https://github.com/AprilRobotics/apriltag](https://github.com/AprilRobotics/apriltag))
- **bin**: where the resulting binaries are placed
- **docs**: doxygen documentation of the detector source (under ```src```)
- **html**: standalone javascript page that displays the detector output
- **log**: where valgrind logs are placed
- **src**: the detector source
- **test**: cmocka tests
- **test/tag-imgs**: test input images

## Quick Start

Install make, gcc, [emscripten](https://emscripten.org/docs/getting_started/downloads.html), [cmocka](https://cmocka.org/), [valgrind](https://www.valgrind.org/downloads/?src=www.discoversdk.com), and [doxygen](https://www.doxygen.nl/manual/install.html).  Cmocka and valgrind are only necessary to run the tests and memory checks. Doxygen is needed if you want to build the documentation.

To compile and run tests, use make:

```make <target>```

The Makefile has the following targets:

- **all**: Builds the example binary (atagjs_example) and the WASM files (apriltag_wasm.js).
- **atagjs_example** (default): Creates a binary (at ```bin/atagjs_example```) of an example program that get the detrector output by giving it image files. The image files are indicated as arguments to the program (requires gcc).
- **apriltag_wasm.js**: Builds the WASM detector (requires emscripten). The resulting files (**apriltag_wasm.js** and **apriltag_wasm.wasm**) are placed under the ```html``` folder so they are run with the javascript example there.
- **tests**: Builds the cmocka test runner as executes it (requires cmocka).
- **valgrind**: Runs the test program under valgrind for several input images in test/tag-imgs (requires valgrind).
- **clean**: Cleans non-source files.
- **help**: outputs description of targets.

# Detector Details

The apriltag detector uses the [tag36h11](http://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/edu/umich/eecs/april/tag/Tag36h11.html) family ([pre-generated tags](https://github.com/conix-center/apriltag-gen)). For tag pose estimation, tag sizes are assumed to be fixed, according to the tag id, as shown in the table.

| Tag ID Range | Tag Size (mm) |
| ------------ | ------------- |
| [0,150]      | 150           |
| [151,300]    | 100           |
| [301,450]    | 50            |
| [451,586]    | 20            |

See pre-generated tags with the right size here: https://github.com/conix-center/apriltag-gen

## Detector API

The C detector html documentation is under [docs/html](docs/html). A usage example can be seen in [atagjs_example](src/atagjs_example.c). When running in a browser, the C code is compiled to WASM and wrapped by the javascript class ```Apriltag``` (in ```html/apriltag.js```) using emscripten's [cwrap()](https://emscripten.org/docs/api_reference/preamble.js.html#cwrap). The detector C calls are private to the ```Apriltag``` class, which exposes the folowing calls:

- Apriltag() constructor. Accepts a callback that will be called when the detector code is fully loaded:

```javascript
constructor(onDetectorReadyCallback)
```

- The ```detect()``` call receives a grayscale image (```grayscaleImg```) with dimensions given by the arguments ```imgWidth``` and ```imgHeight``` in pixels:

```javascript
detect(grayscaleImg, imgWidth, imgHeight)
```

> ```detect()``` will return an array of JSON objects with information about the tags detected.
>
> Example detection:
>
> ```
> [
> {
>  "id": 151,
>  "size": 0.1,
>  "corners": [
>    { "x": 777.52, "y": 735.39},
>    { "x": 766.05, "y": 546.94},
>    { "x": 578.36, "y": 587.88},
>    { "x": 598, "y": 793.42}
>  ],
>  "center": { "x": 684.52, "y": 666.51 },
>  "pose": {
>    "R": [
>      [ 0.91576, -0.385813, 0.111941 ],
>      [ -0.335306, -0.887549, -0.315954 ],
>      [ -0.221252, -0.251803, 0.942148 ] ],
>    "t": [ 0.873393, 0.188183, 0.080928 ],
>    "e": 0.000058,
>    "s": 2
>  }
> }
> ]
> ```
>
> Where:
>
> * *id* is the tag id,
> * *size* is the tag size in meters (based on the tag id)
> * *corners* are x and y corners of the tag (in fractional pixel coordinates)
> * *center* is the center of the tag (in fractional pixel coordinates)
> * *pose*:
>   * *R* is the rotation matrix (**column major**)
>   * *t* is the translation
>   * *e* is the object-space error of the pose estimation
>   * *s* is the solution returned (1=homography method; 2=potential second local minima; see: [apriltag_pose.h](https://github.com/AprilRobotics/apriltag/blob/master/apriltag_pose.h))

- To compute the pose, the detector needs the camera parameters. To set them, before calling ```detect()```, use ```set_camera_info()```, where
  * *fx*, *fy* is the focal lenght, in pixels
  * *cx*, *cy* is the principal point offset, in pixels

```javascript
set_camera_info(fx, fy, cx, cy);
```

See the source for a javascript example [of detecting tags from the webcam](html/video_process.js).
