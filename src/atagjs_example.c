/** @file apriltag_example.c
 *  @brief Example program that get the detrector output by giving it image files
 *
 *  This file is based on apritag library examples. Copyright notice below.
 *  @date June, 2020
 */

/* Copyright (C) 2013-2016, The Regents of The University of Michigan.
   All rights reserved.

   This software was developed in the APRIL Robotics Lab under the
   direction of Edwin Olson, ebolson@umich.edu. This software may be
   available under alternative licensing terms; contact the address above.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are those
   of the authors and should not be interpreted as representing official policies,
   either expressed or implied, of the Regents of The University of Michigan.
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>

#include "apriltag.h"
#include "tag36h11.h"

#include "common/getopt.h"
#include "common/image_u8.h"
#include "common/image_u8x4.h"
#include "common/pjpeg.h"
#include "common/zarray.h"

#include "apriltag_js.h"

int main(int argc, char *argv[])
{
        getopt_t *getopt = getopt_create();

        getopt_add_bool(getopt, 'h', "help", 0, "Show this help");
        getopt_add_bool(getopt, 'd', "debug", 0, "Enable debugging output (slow)");
        getopt_add_bool(getopt, 'q', "quiet", 0, "Reduce output");
        getopt_add_int(getopt, 'i', "iters", "1", "Repeat processing on input set this many times");
        getopt_add_int(getopt, 't', "threads", "1", "Use this many CPU threads");
        getopt_add_int(getopt, 'a', "hamming", "1", "Detect tags with up to this many bit errors.");
        getopt_add_double(getopt, 'x', "decimate", "2.0", "Decimate input image by this factor");
        getopt_add_double(getopt, 'b', "blur", "0.0", "Apply low-pass blur to input; negative sharpens");
        getopt_add_bool(getopt, '0', "refine-edges", 1, "Spend more time trying to align edges of tags");
        getopt_add_int(getopt, 'm', "max-detections", "0", "Maximum detections to return (0=return all)");
        getopt_add_bool(getopt, 'p', "output-pose", 1, "Return pose");
        getopt_add_bool(getopt, 's', "output-pose-sol", 1, "Return pose solutions");

        if (argc==1 || !getopt_parse(getopt, argc, argv, 1) || getopt_get_bool(getopt, "help"))
        {
                printf("Usage: %s [options] <input files>\n", argv[0]);
                getopt_do_usage(getopt);
                exit(0);
        }

        const zarray_t *inputs = getopt_get_extra_args(getopt);

        double quad_decimate = getopt_get_double(getopt, "decimate");
        double quad_sigma = getopt_get_double(getopt, "blur");
        int nthreads = getopt_get_int(getopt, "threads");
        bool debug = getopt_get_bool(getopt, "debug");
        bool refine_edges = getopt_get_bool(getopt, "refine-edges");

        int max_detections = getopt_get_int(getopt, "max-detections");
        bool output_pose = getopt_get_bool(getopt, "output-pose");
        bool output_pose_solutions = getopt_get_bool(getopt, "output-pose-sol");
        if (!output_pose) output_pose_solutions = 0;
        int quiet = getopt_get_bool(getopt, "quiet");

        // init apriltag detector
        atagjs_init();

        // options: float decimate, float sigma, int nthreads, int refine_edges, int max_detections, int return_pose, int return_solutions
        atagjs_set_detector_options(quad_decimate, quad_sigma, nthreads, refine_edges, max_detections, output_pose, output_pose_solutions);

        // camera parameters from ipad where tag photos were taken, for the sake of outputing some pose values
        atagjs_set_pose_info(997.5703125, 997.5703125, 636.783203125, 360.4857482910); // double fx, double fy, double cx, double cy

        for (int input = 0; input < zarray_size(inputs); input++)
        {
                char *path;
                zarray_get(inputs, input, &path);
                if (!quiet)
                        printf("loading %s\n", path);

                image_u8_t *im = NULL;
                if (str_ends_with(path, "pnm") || str_ends_with(path, "PNM") ||
                    str_ends_with(path, "pgm") || str_ends_with(path, "PGM"))
                        im = image_u8_create_from_pnm(path);
                else if (str_ends_with(path, "jpg") || str_ends_with(path, "JPG"))
                {
                        int err = 0;
                        pjpeg_t *pjpeg = pjpeg_create_from_file(path, 0, &err);
                        if (pjpeg == NULL)
                        {
                                printf("pjpeg error %d\n", err);
                                continue;
                        }

                        im = pjpeg_to_u8_baseline(pjpeg);

                        pjpeg_destroy(pjpeg);
                }

                if (im == NULL)
                {
                        printf("couldn't load %s\n", path);
                        continue;
                }

                if (debug)
                        image_u8_write_pnm(im, "detect_input.pnm");

                // copy the image into the detector buffer
                // reproduce what needs to happen when the detector is running in a WASM module

                // get pointer to buffer
                uint8_t * dimg = atagjs_set_img_buffer(im->width, im->height, im->stride);

                // copy data
                memcpy(dimg, im->buf, im->height*im->stride);

                // call apriltag detect
                t_str_json *detjson = atagjs_detect();

                printf("(%lu) %s\n",  detjson->len, detjson->str);

                image_u8_destroy(im);

        }

        printf("\n");

        atagjs_destroy();

        getopt_destroy(getopt);

        return 0;
}
