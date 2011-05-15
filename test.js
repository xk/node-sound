// latency tests. not too good, it's about 30 ms.


var Sound= require('./build/default/sound');
var sounds= [];

var tinyBuffer= new Buffer(532); // 532/(44100*4) -> 0,003015873016 s -> ~3 ms
var len= tinyBuffer.length;
while (len--) tinyBuffer[len]= Math.floor(256*Math.random());


var i= 99; // Create this many identical sounds.
while (i--) sounds[i]= Sound.create(tinyBuffer);

var sndIndex= 0;
function demo (ms) {
  // .play() them sequentially at exactly ms intervals, during no more than a second.
  var t= Date.now();
  var quit= t+ 1e3;
  do {
    
    t+= ms;
    var playNext= sounds[sndIndex];
    while (Date.now() < t) ;
    playNext.play();
    
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
