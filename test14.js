// test for async bufferify()s IN SERIES in a loop to detect leaks and other problems.

var Sound= require('./build/default/sound');
var sounds= ['sound.wav', 'sound.aif', 'sound.au', 'sound.mp3', 'sound.m4a'];


var ctr= 0;
var i= sounds.length;
function next () {
  if (--i<0) i= sounds.length-1;
  var path= sounds[i];
  Sound.bufferify(path, cb.bind(path));
}


function cb (err, buffer) {
  var path= ''+ this;
  process.stdout.write('\n['+ (ctr++)+ '] *** -> ');
  
  if (err) {
    process.stdout.write('ERROR -> '+ path);
  }
  else {
    process.stdout.write('OK    -> '+ path);
  }
  
  next();
}

next();

setTimeout(Date.now, 1e9);
