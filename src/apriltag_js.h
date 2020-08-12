/** @file apriltag_js.h
*  @brief Definitions for the apriltag detector
*
*  Use apriltag library to implement a detector that runs in the browser
*  using WASM
*
* Copyright (C) Wiselab CMU.
* @date July, 2020
*/

#ifndef _APRILTAG_JS_
#define _APRILTAG_JS_

#include "apriltag.h"
#include "apriltag_pose.h"
#include "str_json.h"

// maximum size of string for each detection
#define STR_DET_LEN 1500

/**
 * @brief Init the apriltag detector with given family and default options
 * default options: quad_decimate=2.0; quad_sigma=0.0; nthreads=1; refine_edges=1; return_pose=1
 * @sa set_detector_options for meaning of options
 *
 * @return 0=success; -1 on failure
 */
int atagjs_init();

/**
 * @brief Releases resources
 *
 * @return 0=success
 */
int atagjs_destroy();

/**
 * @brief Sets the given detector options
 *
 * @param decimate Decimate input image by this factor
 * @param sigma Apply low-pass blur to input; negative sharpens
 * @param nthreads Use this many CPU threads
 * @param refine_edges Spend more time trying to align edges of tags
 * @param max_detections Maximum number of detections to return (0=no max)
 * @param return_pose Detect returns pose of detected tags (0=does not return pose; returns pose otherwise)
 * @param return_solutions Detect returns details about both solutions of the pose estimation, if available
 *
 * @return 0=success
 */
int atagjs_set_detector_options(float decimate, float sigma, int nthreads, int refine_edges, int max_detections, int return_pose, int return_solutions);

/**
 * @brief Sets camera intrinsics (in pixels) for tag pose estimation
 *
 * @param fx x focal lenght in pixels
 * @param fy y focal lenght in pixels
 * @param cx x principal point in pixels
 * @param cy y principal point in pixels
 *
 * @return 0=success
 */
int atagjs_set_pose_info(double fx, double fy, double cx, double cy);

/**
 * @brief Creates/changes size of the image buffer where we receive the images to process
 *
 * @param width Width of the image
 * @param height Height of the image
 * @param stride How many pixels per row (=width typically)
 *
 * @return the pointer to the image buffer
 *
 * @warning caller of detect is responsible for putting *grayscale* image pixels in this buffer
 */
uint8_t *atagjs_set_img_buffer(int width, int height, int stride);

/**
 * @brief Detect tags in image stored in the buffer (g_img_buf)
 *
 * @return pointer to str_json structure. The data in this memory location must be consumed before the next call to detect()
 *
 * @warning caller is responsible for putting *grayscale* image pixels in the input buffer (g_img_buf)
 * @warning caller *should not* release return pointer (it's reused at every detect() call); data returned must be consumed before the next call to detect()
 */
t_str_json *atagjs_detect();

#endif
