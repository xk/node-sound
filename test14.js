// test for async bufferify()s in series in a loop to detect leaks and other problems.

var Sound= require('./build/default/sound');
var sounds= ['Sous La Pluie.mp3', 'sound.wav', 'sound.aif', 'sound.mp3', 'sound.m4a', 'error.file'];


var ctr= 0;
var i= sounds.length;
function next () {
  if (--i<0) i= sounds.length-1;
  Sound.bufferify(sounds[i], cb.bind(sounds[i]));
}


function cb (err, buffer) {
  var path= ''+ this;
  var ctrStr= '\n['+ (ctr++)+ '] *** ';
  
  if (err) {
    process.stdout.write(ctrStr+ 'ERROR -> '+ path);
  }
  else {
    process.stdout.write(ctrStr+ 'OK    -> '+ path);
  }
  
  next();
}

next();

setTimeout(Date.now, 1e9);
