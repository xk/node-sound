// tests loop() glitches with short sounds.

var buffer= new Buffer(4096); // 4096/(44100*4) == 0,02321995465 s.
var len= buffer.length;
var i= 0;
var flipflop= 1;
var onda= 0;
while (len--) {
  
  process.stdout.write(onda+ ' ');
  
  buffer[len--]= onda;
  buffer[len--]= onda;
  buffer[len--]= onda;
  buffer[len]= onda;
  
  if (flipflop) onda+= 2;
  else onda-= 2;
  if (onda > 255) {
    onda= 255;
    flipflop= !flipflop;
  }
  else if (onda < 0) {
    onda= 0;
    flipflop= !flipflop;
  }
  
}


require('sound').create(buffer).loop(1e6).volume(1).play(); // loop
setTimeout(Date.now, 1e9); // forever

