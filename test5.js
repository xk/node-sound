// tests volume().

var buffer= new Buffer(256*256);
var len= buffer.length;
//while (len--) buffer[len]= Math.floor(Math.random()*256); // noise
while (len--) buffer[len]= len%256; // saw

var Sound= require('./build/default/sound');

var sound= Sound.create(buffer).volume(0).loop(1e9).play();

var i= 0;
var ii= 0.25;
(function loop () {
  setTimeout(loop, 7);
  i+= ii;
  if (i<0) {
    ii= 0.25;
    i= 0;
  }
  else if (i>1) {
    ii= -0.01;
    i= 1;
  }
  sound.volume(i);
})();

