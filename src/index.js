
import RayKarstenModule from "./RayKarsten.js";

const WIDTH = 512;
const HEIGHT = 512;

document.addEventListener('DOMContentLoaded', function(event) {
  


let mWASMReady = false;
let up = false;
let right = false;
let down = false;
let left = false;
let speed = 0.25;

class Vec2 {
  constructor() {
    this.x = 0.0;
    this.y = 0.0;
  }

  normalize() {
    let len = Math.sqrt(this.dirX * this.dirX + this.dirY * this.dirY);
    if (len > 0.0) {
      this.dirX = this.dirX / len;
      this.dirY = this.dirY / len;
    }
  }
}

class Player {
  constructor() {
    this.posX = 16.0;
    this.posY = 5.0;
    this.viewAngle = 0.09;
    this.dir = new Vec2();
    this.dir.x = Math.cos(this.viewAngle);
    this.dir.y = Math.sin(this.viewAngle);
  }

  updateViewAngle(delta) {
    this.viewAngle += delta;
    this.dir.x = Math.cos(this.viewAngle);
    this.dir.y = Math.sin(this.viewAngle);
  }

  updatePosition(speed) {
    this.posX += this.dir.x * speed;
    this.posY += this.dir.y * speed;
  }
}

const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
let wrapper = new RayKarstenModule();
const imgData = ctx.createImageData(WIDTH, HEIGHT);
let player = new Player();

wrapper['onRuntimeInitialized'] = function () {
  mWASMReady = true;
  renderScene();
};

function renderScene() {
  wrapper.rayCastImage(player.posX, player.posY, player.dir.x, player.dir.y, 60.0);
  let img = wrapper.getImage();
  for (let i = 0; i < WIDTH * HEIGHT * 4; i++) {
    imgData.data[i] = img[i];
  }
  ctx.putImageData(imgData, 0, 0);
}

function gameLoop() {

  if (mWASMReady) {
    let updated = false;
    if (up) {
      let nextMapTile = wrapper.getMapType(player.posX + player.dir.x * speed * 4.0,player.posY + player.dir.y * speed * 4.0);
      if(nextMapTile == 0)
      {
        player.updatePosition(speed);
      }
      updated = true;
    }

    if (down) {
      let nextMapTile = wrapper.getMapType(player.posX - player.dir.x * speed * 4.0,player.posY - player.dir.y * speed * 4.0);
      if(nextMapTile == 0)
      {
        player.updatePosition(-speed);
      }
     
      updated = true;
    }

    if (right) {
      player.updateViewAngle(speed * 0.2);
      updated = true;
    }

    if (left) {
      player.updateViewAngle(-speed * 0.2);
      updated = true;
    }

    if (updated) {
      renderScene();
      // document.getElementById('results').innerHTML = "<b>" + player.posX + "," + player.posY + "</b>";
    }
  }
  window.requestAnimationFrame(gameLoop)
}

window.requestAnimationFrame(gameLoop);

document.addEventListener('keydown', press)
function press(e) {
  if (e.keyCode === 87 /* w */) {
    up = true
  }
  if (e.keyCode === 68 /* d */) {
    right = true
  }
  if (e.keyCode === 83 /* s */) {
    down = true
  }
  if (e.keyCode === 65 /* a */) {
    left = true
  }
}

document.addEventListener('keyup', release)
function release(e) {
  if (e.keyCode === 87 /* w */) {
    up = false
  }
  if (e.keyCode === 68 /* d */) {
    right = false
  }
  if (e.keyCode === 83 /* s */) {
    down = false
  }
  if (e.keyCode === 65 /* a */) {
    left = false
  }
}

document.getElementById("leftButton").addEventListener('mousedown', function(event){  left = true});
document.getElementById("leftButton").addEventListener('mouseup', function(event){  left = false});
document.getElementById("leftButton").addEventListener('mouseleave', function(event){  left = false});
document.getElementById("leftButton").addEventListener('touchstart', function(event){  left = true});
document.getElementById("leftButton").addEventListener('touchend', function(event){  left = false});

document.getElementById("rightButton").addEventListener('mousedown', function(event){  right = true});
document.getElementById("rightButton").addEventListener('mouseup', function(event){  right = false});
document.getElementById("rightButton").addEventListener('mouseleave', function(event){  right = false});
document.getElementById("rightButton").addEventListener('touchstart', function(event){ right = true });
document.getElementById("rightButton").addEventListener('touchend', function(event){  right = false});

})
