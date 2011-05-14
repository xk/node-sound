// test loop() to see if it makes any glitches when playing in a loop.

var buffer= new Buffer(256*256); // 1 seconds.
var len= buffer.length;
while (len--) buffer[len]= len%256; // saw;


require('./build/default/sound').create(buffer).loop(1e6).volume(1).play(); // loop

setTimeout(Date.now, 1e9); // forever

