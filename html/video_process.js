import * as Comlink from "https://unpkg.com/comlink/dist/esm/comlink.mjs";

import * as Base64 from "./base64.js";

var detections=[];
var imgSaveRequested=0;
var cameraInfo;

window.onload = (event) => {
  init();

  loadImg('saved_det');
}

async function init() {
  // WebWorkers use `postMessage` and therefore work with Comlink.
  const Apriltag = Comlink.wrap(new Worker("apriltag.js"));

  // must call this to init apriltag detector; argument is a callback for when the detector is ready
  window.apriltag = await new Apriltag(Comlink.proxy(() => {

    // set camera info; we must define these according to the device and image resolution
    // set_camera_info(double fx, double fy, double cx, double cy)
    let cameraInfo = window.cameraInfo;

    // start processing frames
    window.requestAnimationFrame(process_frame);
  }));
}

async function process_frame() {

  canvas.width = video.videoWidth;
  canvas.height = video.videoHeight;
  let ctx = canvas.getContext("2d");

  let imageData;
  try {
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
    imageData = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height);
  } catch (err) {
    console.log("Failed to get video frame. Video not started ?");
    setTimeout(process_frame, 500); // try again in 0.5 s
    return;
  }
  let imageDataPixels = imageData.data;
  let grayscalePixels = new Uint8Array(ctx.canvas.width * ctx.canvas.height); // this is the grayscale image we will pass to the detector

  for (var i = 0, j = 0; i < imageDataPixels.length; i += 4, j++) {
    let grayscale = Math.round((imageDataPixels[i] + imageDataPixels[i + 1] + imageDataPixels[i + 2]) / 3);
    grayscalePixels[j] = grayscale; // single grayscale value
    imageDataPixels[i] = grayscale;
    imageDataPixels[i + 1] = grayscale;
    imageDataPixels[i + 2] = grayscale;
  }
  ctx.putImageData(imageData, 0, 0);

  // draw previous detection
  detections.forEach(det => {

/*
    // attempt to draw an axis line indicating the pose
    var r = math.matrix(det.pose.R); // apritag rotation (from detector); a 3x3
    var t = math.transpose(math.matrix(det.pose.t)); // apriltag translation (from detector); transposing into a 3x1
    t.resize([4,1], 1); // make t a 4x1, adding a 1
    r.resize([4,4], 0); // make r a 4x4, adding 0s
    r.subset(math.index([0,1,2,3], 3), t); // make t the 4th column of r
    console.log("r=", r.valueOf());
    let len = 0.15; // length of the axis line in meters
    var p = math.matrix([[0],[0],[-len],[1]]); // end point of z axis line
    var pcc = math.multiply(r, p); // r x p
    console.log("pcc=", pcc.valueOf());
    window.cameraInfo.camera_matrix[0][0] = -window.cameraInfo.camera_matrix[0][0];
    var cm = math.matrix(window.cameraInfo.camera_matrix); // camera matrix a 3x3 [[fx, 0, cx],[0, fy, cy], [0, 0, 1]]
    console.log("cm=", cm.valueOf());
    cm.resize([3,4], 0); // make cm a 3x4, adding 0s
    var pc = math.multiply(cm, pcc); // cm x pcc
    var pc = math.divide(pc, pc.subset(math.index(2,0))).valueOf(); // pc / pc[2]
    console.log("pc=", pc);

    // draw pose
    ctx.beginPath();
    ctx.lineWidth = "8";
    ctx.strokeStyle = "red";
    ctx.moveTo(det.center.x, det.center.y);
    ctx.lineTo(det.center.x+pc[0], det.center.y+pc[1]);
    ctx.stroke();
*/
    // draw tag borders
    ctx.beginPath();
      ctx.lineWidth = "5";
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

  // detect aprilTag in the grayscale image given by grayscalePixels
  detections = await apriltag.detect(grayscalePixels, ctx.canvas.width, ctx.canvas.height);

  if (imgSaveRequested && detections.length > 0) {
      console.log("det",detections.length);
      //saveImg(canvas);

      //let enc = new TextDecoder("utf-8");
      console.log(imageDataPixels.length);
      //let gp = enc.decode(grayscalePixels);
      //imageData = ;
      let savep = Base64.bytesToBase64(ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height).data);
      var det = JSON.stringify({
        det_data: detections[0],
        //img_data: canvas.toDataURL("image/png"),
        img_data: LZString.compressToUTF16(savep),
        img_width:  ctx.canvas.width,
        img_height: ctx.canvas.height
      });

      console.log("Saving detection data.");
      localStorage.setItem("detectData", det);
      buttonToggle();
  }

  window.requestAnimationFrame(process_frame);
}

async function loadImg(targetHtmlElemId) {
  var detectData = localStorage.getItem('detectData');
  if (detectData) {
     let detectDataObj = JSON.parse(detectData);
     let savedPixels = Base64.base64ToBytes(LZString.decompressFromUTF16(detectDataObj.img_data));
     delete detectDataObj.img_data;

     const canvasSaved = document.getElementById(targetHtmlElemId+"_canvas");
     let ctx = canvasSaved.getContext("2d");
     canvasSaved.width = detectDataObj.img_width;
     canvasSaved.height = detectDataObj.img_height;
     let imageData = ctx.getImageData(0, 0, ctx.canvas.width, ctx.canvas.height);
     imageData.data.set(savedPixels);
     ctx.putImageData(imageData, 0, 0);

     console.log(detectDataObj.det_data);
     let detDataSaved = document.getElementById(targetHtmlElemId+"_data");
     detDataSaved.value=JSON.stringify(detectDataObj, null, 2);


     //var r = math.matrix([[0.399586, 0.297737, -0.866997],
      //  [-0.910373, 0.017994, -0.413397],
      //  [-0.107483, 0.954478, 0.278242]]); // apritag rotation (from detector)
/*
       var r = math.matrix(det.pose.R); // apritag rotation (from detector); a 3x3
       var t = math.transpose(math.matrix(det.pose.t)); // apriltag translation (from detector); transposing into a 3x1
       t.resize([4,1], 1); // make t a 4x1, adding a 1
       r.resize([4,4], 0); // make r a 4x4, adding 0s
       r.subset(math.index([0,1,2,3], 3), t); // make t the 4th column of r
       let len = 50; // length of the axis line
       var p = math.matrix([0, 0, -len, 1]); // end point of z axis line
       var pcc = math.multiply(p, r); // p . r
       var cm = math.matrix(window.cameraInfo.camera_matrix); // camera matrix a 3x3 [[fx, 0, cx],[0, fy, cy], [0, 0, 1]]
       var tm = math.matrix([[0],[0],[0],[1]]); // 4x1
       cm.resize([4,4], 0); // make cm a 4x4, adding 0s
       cm.subset(math.index([0,1,2,3], 3), tm); // make tm the 4th column of cm
       var pc = math.multiply(pcc, cm); // pcc . cm
*/
/*
      var r = math.matrix(det.pose.R); // apritag rotation (from detector); a 3x3
      var t = math.transpose(math.matrix(det.pose.t)); // apriltag translation (from detector); transposing into a 3x1
      t.resize([4,1], 1); // make t a 4x1, adding a 1
      r.resize([4,4], 0); // make r a 4x4, adding 0s
      r.subset(math.index([0,1,2,3], 3), t); // make t the 4th column of r
      console.log("r=", r.valueOf());
      let len = 0.15; // length of the axis line in meters
      var p = math.matrix([[0],[0],[-len],[1]]); // end point of z axis line
      var pcc = math.multiply(r, p); // r x p
      console.log("pcc=", pcc.valueOf());
      var cm = math.matrix(window.cameraInfo.camera_matrix); // camera matrix a 3x3 [[fx, 0, cx],[0, fy, cy], [0, 0, 1]]
      console.log("cm=", cm.valueOf());
      cm.resize([3,4], 0); // make cm a 3x4, adding 0s
      var pc = math.multiply(cm, pcc); // cm x pcc
      console.log("pc=", pc);
      var pcf = pc.valueOf();
      pcf[0] = pcf[0] / pcf[2];
      pcf[1] = pcf[1] / pcf[2];
      console.log("pcf=", pcf);
*/
  } else console.log("detectData not found");
}

var button = document.getElementById('req_save');
button.addEventListener('click', function() {
  buttonToggle();
  console.log("setImgSaveRequested", imgSaveRequested);
});

function buttonToggle() {
  if (imgSaveRequested == 0) {
    button.innerHTML = "Saving next detection... (press to cancel)";
    imgSaveRequested = 1;
    button.className += " active";
  } else {
    button.innerHTML = "Save next detection (local storage)";
    imgSaveRequested = 0;
    button.className.replace(" active", "");
  }
}
