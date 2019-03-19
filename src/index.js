

import RayKarstenModule from "./RayKarsten.js";

const WIDTH = 512;
const HEIGHT = 512;
var index = 2;
var mWASMReady = false;


class Player
{
  constructor() {
    this.name = "Chuck Norris";
    this.posX = 10.0;
    this.posY = 10.0;

    this.dirX = 0.5;
    this.dirY = 0.5;

    let len = Math.sqrt(this.dirX*this.dirX + this.dirY*this.dirY);
    this.dirX = this.dirX/len;
    this.dirY = this.dirY/len;

    this.Fov = 1.0;
  }
}

const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
let wrapper = new RayKarstenModule();
const imgData = ctx.createImageData(WIDTH, HEIGHT);
let player = new Player();



var Key = {
  LEFT:   37,
  UP:     38,
  RIGHT:  39,
  DOWN:   40
};

function _addEventListener(evt, element, fn) {
  if (window.addEventListener)
  {
    element.addEventListener(evt, fn, false);
  }
  else 
  {
    element.attachEvent('on'+evt, fn);
  }
}



wrapper['onRuntimeInitialized'] = function()
  {
   mWASMReady = true;
    console.log(wrapper);
    wrapper.rayCastImage(16.0,5.0,0.9,-0.1,60.0);
    var img = wrapper.getImage();

    _addEventListener('keydown', document, handleKeyboardEvent);

    for (let i = 0; i < WIDTH*HEIGHT*4; i++) {
      imgData.data[i] = img[i];
    }

    ctx.putImageData(imgData, 0, 0);
  };


  function updateImage()
  {
    ctx.beginPath();
    ctx.moveTo(player.posX, player.posY);
    ctx.lineTo(300, 150);
    ctx.stroke();
  }

  function handleKeyboardEvent(evt) {
    if (!evt) {evt = window.event;} // for old IE compatible
    var keycode = evt.keyCode || evt.which; // also for cross-browser compatible
    var info = document.getElementById("info");
  
    switch (keycode) {
      case Key.LEFT:
      console.log("left");
        break;
      case Key.UP:
      console.log("up");
        break;
      case Key.RIGHT:
      console.log("right");
        break;
      case Key.DOWN:
      updateImage();
      console.log("down");
        break;
      default:
        break;
    }
  }


