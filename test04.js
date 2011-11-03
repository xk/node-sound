// test .play() and .pause();

var Sound= require('./build/default/sound');

var buf= new Buffer(256*256);
var i= buf.length;
while (i--) buf[i]= i%256;
//while (i--) buf[i]= Math.floor(Math.random()*256);

var snd= Sound.create(buf);
snd.loop(1e9);

var flipflop= 0;
function loop () {
  if (++flipflop % 2) {
    process.stdout.write('PLAY ');
    snd.play();
  }
  else {
    process.stdout.write('PAUSE ');
    snd.pause();
  }
  
  setTimeout(loop, 444);
}

loop();

