// test for multiple async bufferify()s in parallel in a loop to detect leaks and other problems.

// ==================================
// = OOPS, SEGFAULTS EVERY TIME !!! =
// ==================================

// GRRR, Apple dice: (ExtAudioFileRead) This function works only on a single thread. If you want your application to read an audio file on multiple threads, use Audio File Services instead (en vez de *Extended* Audio File Services). :-(

var Sound= require('./build/default/sound');
var sounds= ['sound.wav', 'sound.aif', 'sound.au', 'sound.m4a', 'sound.mp3', 'Sous La Pluie.mp3', 'error.file', 'sound.cc'];

var ctr= 0;
var all= sounds.length;
function go () {
  process.stdout.write('\n');
  while (sounds.length) {
    var path= sounds.pop();
    Sound.bufferify(path, cb.bind(path));
  }
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
  
  if (sounds.push(path) === all) go();
}

go();

setTimeout(Date.now, 1e9);
