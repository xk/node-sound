// latency tests. not too good, it's about 30 ms.


var Sound= require('./build/default/sound');
var sounds= [];

var bufferShort= new Buffer(532); // 532/(44100*4) -> 0,003015873016 s -> ~3 ms
[].forEach.call(bufferShort, noise);

function noise (v,i,o) {
  o[i]= Math.floor(Math.random()*256);
}

var buffer= new Buffer(44100*4*2);
var i= buffer.length;
while (i--) buffer[i]= 0;
Sound.create(buffer).loop(1e9).play();


var i= 99; // Create this many identical sounds.
while (i--) sounds[i]= Sound.create(bufferShort);

var sndIndex= 0;
function demo (ms) {
  // .play() them sequentially at exactly ms intervals, during no more than a second.
  var t= Date.now();
  var quit= t+ 1e3;
  do {
    
    t+= ms;
    while (Date.now() < t) ;
    sounds[sndIndex].play();
    
    sndIndex= ++sndIndex % sounds.length;
  } while (Date.now() < quit);
  
  setTimeout(next, 999);
}


var i= 10;
function next () {
  console.log(i+ 'ms');
  demo(i);
  i= (i<100) ? i+5 : 10;
}

next();
