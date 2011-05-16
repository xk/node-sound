var Sound= require('./build/default/sound');

var files= ['sound.wav', 'sound.m4a', 'sound.aif', 'sound.mp3'];

var i= files.length;
while (i--) {
  process.stdout.write('\n******************************* '+ files[i]+ " ->");
  Sound.bufferify(files[i]);
}

