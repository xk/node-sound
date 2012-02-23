// test loop() to see if it makes any glitches when playing in a loop.

// the buffer contains 1/3rd silence + 1/3rd noise + 1/3rd silence

var buffer= new Buffer(44100*4); // 1 seconds.
var len= buffer.length;
while (len--) {
  if (len < (buffer.length/3) || len >= (2*buffer.length/3)) {
    buffer[len]= 0; // silence
  }
  else {
    buffer[len]= Math.floor(Math.random()*256); // noise
  }
}


var S= require('sound');
S.create(buffer).loop(1e6).volume(1).play(); // loop

setTimeout(Date, 1e9); // forever

