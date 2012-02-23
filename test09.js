// tests loop() glitches with TOO short sounds. FAIL

var buffer= new Buffer(2048); // 2048/(44100*4) == 0,01160997732 s.
var len= buffer.length;
var flipflop= 1;
var onda= 0;
var inc= 4;
do {
  
  process.stdout.write(onda+ ' ');
  
  buffer[--len]= onda;
  buffer[--len]= onda;
  buffer[--len]= onda;
  buffer[--len]= onda;
  
  onda+= flipflop ? inc : -inc;
  
  if (onda > 255) {
    onda= 255;
    flipflop= !flipflop;
  }
  else if (onda < 0) {
    onda= 0;
    flipflop= !flipflop;
  }
  
} while (len);


require('sound').create(buffer).loop(1e6).volume(1).play(); // loop

setTimeout(Date.now, 1e9); // forever

