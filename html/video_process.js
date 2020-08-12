import * as Comlink from "https://unpkg.com/comlink/dist/esm/comlink.mjs";

var detections=[];

window.onload = (event) => {
  init();
}

async function init() {
  // WebWorkers use `postMessage` and therefore work with Comlink.
  const Apriltag = Comlink.wrap(new Worker("apriltag.js"));

  // must call this to init apriltag detector; argument is a callback for when the detector is ready
  window.apriltag = await new Apriltag(Comlink.proxy(() => {

    // set camera info; we must define these according to the device and image resolution
    // set_camera_info(double fx, double fy, double cx, double cy)
    window.apriltag.set_camera_info(997.5703125, 997.5703125, 636.783203125, 360.4857482910); 

    // start processing frames
    setTimeout(process_frame, 500);

  }));
}

async function process_frame() {

  canvas.width = video.videoWidth;
  canvas.height = video.videoHeight;

  let ctx = canvas.getContext("2d");

  ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

  let imageData = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height);
  let imageDataPixels = imageData.data;
  let grayscaleArray = new Uint8Array(ctx.canvas.width * ctx.canvas.height);

  for (var i = 0, j = 0; i < imageDataPixels.length; i += 4, j++) {
    let grayscale = Math.round((imageDataPixels[i] + imageDataPixels[i + 1] + imageDataPixels[i + 2]) / 3);
    grayscaleArray[j] = grayscale;
    imageDataPixels[i] = grayscale;
    imageDataPixels[i + 1] = grayscale;
    imageDataPixels[i + 2] = grayscale;
  }
  ctx.putImageData(imageData, 0, 0);

  // draw previous detection
  detections.forEach(det => {
    ctx.beginPath();
    ctx.lineWidth = "2";
    ctx.strokeStyle = "blue";
    ctx.moveTo(det.corners[0].x, det.corners[0].y);
    ctx.lineTo(det.corners[1].x, det.corners[1].y);
    ctx.lineTo(det.corners[2].x, det.corners[2].y);
    ctx.lineTo(det.corners[3].x, det.corners[3].y);
    ctx.lineTo(det.corners[0].x, det.corners[0].y);
    ctx.font = "bold 20px Arial";
    var txt = ""+det.id;
    ctx.fillStyle = "blue";
    ctx.textAlign = "center";
    ctx.fillText(txt, det.center.x, det.center.y+5);
    ctx.stroke();
  });

  // detect aprilTag
  detections = await apriltag.detect(grayscaleArray, ctx.canvas.width, ctx.canvas.height);

  window.requestAnimationFrame(process_frame);
}
