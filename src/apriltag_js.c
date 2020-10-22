/** @file apriltag_js.c
 *  @brief Apriltag detection to be compile with emscripten
 *  @see documentation in apriltag_js.h
 *
 *  Uses the apriltag library; exposes a simple interface for a web app to
 *  use apriltags once it is compiled to WASM using emscripten
 *
 *  Copyright (C) Wiselab CMU.
 *  @date Nov, 2019
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "apriltag.h"
#include "apriltag_pose.h"
#include "tag36h11.h"
#include "tag25h9.h"
#include "tag16h5.h"
#include "tagCircle21h7.h"
#include "tagStandard41h12.h"
#include "common/getopt.h"
#include "common/image_u8.h"
#include "common/image_u8x4.h"
#include "common/pjpeg.h"
#include "common/zarray.h"
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "apriltag_js.h"
#include "str_json.h"

// global pointers to the tag family and detector
static apriltag_family_t *g_tf = NULL;
static apriltag_detector_t *g_td;

// size and stride f the image to process
static int g_width;
static int g_height;
static int g_stride;

// return structure for a json string we reuse in each detect() call
static t_str_json g_det_json = STR_JSON_INITIALIZER;

// pointer to the image grayscale pixels
static uint8_t *g_img_buf = NULL;

// max number of detections returned (0=no max)
static int g_max_detections = 0;

// if we are returning pose (=0 does not output; output otherwise)
static int g_return_pose = 1;

// if we are returning details about both solutions (see estimate_tag_pose_with_solution; =0 does not output; output otherwise)
static int g_return_solutions = 0;

// apriltag_detection_info
// defaults set for 2020 ipad, with 1280x720 images
static apriltag_detection_info_t g_det_pose_info = {.cx=636.9118, .cy=360.5100, .fx=997.2827, .fy=997.2827};

// declare static calls, implemented at the end of this file
static double estimate_tag_pose_with_solution(apriltag_detection_info_t *info, apriltag_pose_t *pose, char *s, int ssize);
static double tagsize_from_id(int tagid);

EMSCRIPTEN_KEEPALIVE
int atagjs_init()
{
    g_tf = tag36h11_create();
    if (g_tf == NULL)
    {
        printf("Error initializing tag family.");
        return -1;
    }
    g_td = apriltag_detector_create();
    if (g_td == NULL)
    {
        printf("Error initializing detector.");
        return -1;
    }
    apriltag_detector_add_family_bits(g_td, g_tf, 1);
    g_td->quad_decimate = 2.0;
    g_td->quad_sigma = 0.0;
    g_td->nthreads = 1;
    g_td->debug = 0; // Enable debugging output (slow)
    g_td->refine_edges = 1;
    g_return_pose = 1;
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int atagjs_destroy()
{
    apriltag_detector_destroy(g_td);
    tag36h11_destroy(g_tf);
    if (g_img_buf != NULL)
        free(g_img_buf);

    str_json_destroy(&g_det_json);

    return 0;
}

EMSCRIPTEN_KEEPALIVE
int atagjs_set_detector_options(float decimate, float sigma, int nthreads, int refine_edges, int max_detections, int return_pose, int return_solutions)
{
    g_td->quad_decimate = decimate;
    g_td->quad_sigma = sigma;
    g_td->nthreads = nthreads;
    g_td->refine_edges = refine_edges;
    g_max_detections = max_detections;
    g_return_pose = return_pose;
    g_return_solutions = return_solutions;
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int atagjs_set_pose_info(double fx, double fy, double cx, double cy)
{
    g_det_pose_info.fx = fx;
    g_det_pose_info.fy = fy;
    g_det_pose_info.cx = cx;
    g_det_pose_info.cy = cy;
    return 0;
}

EMSCRIPTEN_KEEPALIVE
uint8_t *atagjs_set_img_buffer(int width, int height, int stride)
{
    int w = (stride < width) ? width : stride; // stride should always be >= width...
    if (g_img_buf != NULL)
    {
        if (g_width == width && g_height == height && g_stride == stride)
            return g_img_buf;
        free(g_img_buf);
        g_width = width;
        g_height = height;
        g_stride = stride;
        g_img_buf = (uint8_t *)calloc(height*w, sizeof(uint8_t));

    }
    else
    {
        g_width = width;
        g_height = height;
        g_stride = stride;
        g_img_buf = (uint8_t *)calloc(height*w, sizeof(uint8_t));
    }
    return g_img_buf;
}

EMSCRIPTEN_KEEPALIVE
t_str_json *atagjs_detect()
{
    char str_tmp_det[STR_DET_LEN+1];

    // clear the json string
    str_json_destroy(&g_det_json); // IMPORTANT: make sure g_det_json is initialized properly with: t_str_json g_det_json = STR_JSON_INITIALIZER;

    if (g_tf == NULL || g_td == NULL || g_img_buf == NULL)
    {
        if (str_json_create(&g_det_json, 50) == 0) { // try to allocate string to return error string
          str_json_printf(&g_det_json, fmt_error, "Detector not initizalized. (did you call init and set_img_buffer ?)");
        }
        return &g_det_json;
    }

    image_u8_t im = {
        .width = g_width,
        .height = g_height,
        .stride = g_stride,
        .buf = g_img_buf};

    zarray_t *detections = apriltag_detector_detect(g_td, &im);

    int n = zarray_size(detections);

    if (n <= 0) {
      if (str_json_create(&g_det_json, 50) == 0) { // try to allocate string to return error string
        str_json_printf(&g_det_json, "[ ]");
      }
      apriltag_detections_destroy(detections);
      return &g_det_json; // return empty string or string with empty array
    }

    // limit detections returned according to g_max_detections
    if (g_max_detections > 0 && g_max_detections < n) n = g_max_detections;

    // start the json array
    if (str_json_create(&g_det_json, n*STR_DET_LEN) != 0) {
      if (str_json_create(&g_det_json, 50) == 0) { // try to allocate string to return error string
        str_json_printf(&g_det_json, fmt_error, "Could not allocate memory for %d detections", n);
      }
      apriltag_detections_destroy(detections);
      return &g_det_json;
    }
    str_json_concat(&g_det_json, "[ ");

    for (int i = 0; i < n; i++)
    {
        apriltag_detection_t *det;
        zarray_get(detections, i, &det);

        // size of the tag is determined from its id:
        double tagsize = tagsize_from_id(det->id);

        if (g_return_pose == 0)
        {
            snprintf(str_tmp_det, STR_DET_LEN, fmt_det_point, det->id, tagsize, det->p[0][0], det->p[0][1], det->p[1][0], det->p[1][1], det->p[2][0], det->p[2][1], det->p[3][0], det->p[3][1], det->c[0], det->c[1]);
        }
        else
        {
            // return pose ..
            apriltag_pose_t pose;
            double pose_err;
            g_det_pose_info.det = det;
            g_det_pose_info.tagsize = tagsize;
            char *s = malloc(STR_DET_LEN);
            pose_err = estimate_tag_pose_with_solution(&g_det_pose_info, &pose, s, STR_DET_LEN);
            // column major R:
            snprintf(str_tmp_det, STR_DET_LEN, fmt_det_point_pose, det->id, tagsize, det->p[0][0], det->p[0][1], det->p[1][0], det->p[1][1], det->p[2][0], det->p[2][1], det->p[3][0], det->p[3][1], det->c[0], det->c[1], matd_get(pose.R, 0, 0), matd_get(pose.R, 1, 0), matd_get(pose.R, 2, 0), matd_get(pose.R, 0, 1), matd_get(pose.R, 1, 1), matd_get(pose.R, 2, 1), matd_get(pose.R, 0, 2), matd_get(pose.R, 1, 2), matd_get(pose.R, 2, 2), matd_get(pose.t, 0, 0), matd_get(pose.t, 1, 0), matd_get(pose.t, 2, 0), pose_err, s);
            matd_destroy(pose.R);
            matd_destroy(pose.t);
            free(s);
        }
        if (i > 0) str_json_concat(&g_det_json, ", ");
        str_json_concat(&g_det_json, str_tmp_det);
    }

    str_json_concat(&g_det_json, " ]");

    apriltag_detections_destroy(detections);

    return &g_det_json;
}

/**
 * Our implementation of estimate tag pose to return the solution selected (1=homography method; 2=potential second local minima; see: apriltag_pose.h)
 *
 * @param info detection info
 * @param pose where to return the pose estimation
 * @param s the other solution (homography method; potential second local minima; see: apriltag_pose.h)
 *
 * return the object-space error of the pose estimation
 */
static double estimate_tag_pose_with_solution(apriltag_detection_info_t *info, apriltag_pose_t *pose, char *s, int ssize)
{
    double err1, err2;
    apriltag_pose_t pose1, pose2;
    estimate_tag_pose_orthogonal_iteration(info, &err1, &pose1, &err2, &pose2, 50);

    if (err1 <= err2)
    {
        pose->R = pose1.R;
        pose->t = pose1.t;
        if (g_return_solutions) {
            if (pose2.R != NULL && pose2.t !=  NULL) {
                // return other alternative solution; uniquesol indicates if there are multiple solutions
                snprintf(s, ssize, ", \"asol\": {\"R\": [[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]], \"t\": [%f,%f,%f], \"e\": %f, \"uniquesol\": true }",
                    matd_get(pose2.R, 0, 0), matd_get(pose2.R, 1, 0), matd_get(pose2.R, 2, 0), matd_get(pose2.R, 0, 1), matd_get(pose2.R, 1, 1), matd_get(pose2.R, 2, 1), matd_get(pose2.R, 0, 2), matd_get(pose2.R, 1, 2), matd_get(pose2.R, 2, 2), matd_get(pose2.t, 0, 0), matd_get(pose2.t, 1, 0), matd_get(pose2.t, 2, 0), err2);
            } else snprintf(s, ssize, ", \"asol\": \"R\": [[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]], \"t\": [%f,%f,%f], \"e\": %f, \"uniquesol\": false }", // return the same solution
                    matd_get(pose1.R, 0, 0), matd_get(pose1.R, 1, 0), matd_get(pose1.R, 2, 0), matd_get(pose1.R, 0, 1), matd_get(pose1.R, 1, 1), matd_get(pose1.R, 2, 1), matd_get(pose1.R, 0, 2), matd_get(pose1.R, 1, 2), matd_get(pose1.R, 2, 2), matd_get(pose1.t, 0, 0), matd_get(pose1.t, 1, 0), matd_get(pose1.t, 2, 0), err1);
        } else s[0]='\0'; // return empty string
        if (pose2.R)
        {
            matd_destroy(pose2.t);
        }
        matd_destroy(pose2.R);
        return err1;
    }
    else
    {
        pose->R = pose2.R;
        pose->t = pose2.t;
        if (g_return_solutions) {
            // return other alternative solution; uniquesol indicates if there are multiple solutions
            snprintf(s, ssize, ", \"asol\": {\"R\": [[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]], \"t\": [%f,%f,%f], \"e\": %f, \"uniquesol\": true }",
                 matd_get(pose1.R, 0, 0), matd_get(pose1.R, 1, 0), matd_get(pose1.R, 2, 0), matd_get(pose1.R, 0, 1), matd_get(pose1.R, 1, 1), matd_get(pose1.R, 2, 1), matd_get(pose1.R, 0, 2), matd_get(pose1.R, 1, 2), matd_get(pose1.R, 2, 2), matd_get(pose1.t, 0, 0), matd_get(pose1.t, 1, 0), matd_get(pose1.t, 2, 0), err1);
        } else s[0]='\0'; // return empty string
        matd_destroy(pose1.R);
        matd_destroy(pose1.t);
        return err2;
    }
}

/**
 * @brief Determine size of the tag from its id:
 *  [0,150]=150mm;  ]150,300]=100mm; ]300,450]=50mm; ]450,587]=20mm;
 *
 * @param tagid tag id
 *
 * return the tag size, in meters
 */
static double tagsize_from_id(int tagid) {
  if (tagid <= 150) return 0.150;
  if (tagid <= 300) return 0.100;
  if (tagid <= 450) return 0.050;
  return 0.020;
}
