/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found at:
 *  https://github.com/webrtc/samples/blob/gh-pages/LICENSE.md
 */

'use strict';

// Put variables in global scope to make them available to the browser console.
const video = window.video = document.getElementById('webcam_canvas');
const canvas = window.canvas = document.getElementById('out_canvas');

canvas.width = 480;
canvas.height = 360;

const constraints = {
  audio: false,
  video: true
};

function handleSuccess(stream) {
  window.stream = stream; // make stream available to browser console
  video.srcObject = stream;
}

function handleError(error) {
  console.log('navigator.MediaDevices.getUserMedia error: ', error.message, error.name);
}

navigator.mediaDevices.getUserMedia(constraints).then(handleSuccess).catch(handleError);
