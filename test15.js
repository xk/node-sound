// test for multiple async bufferify()s IN PARALLEL in a loop to detect leaks and other problems.

var Sound= require('./build/default/sound');
var sounds= ['sound.wav', 'sound.aif', 'sound.au', 'sound.m4a', 'sound.mp3'];

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
    process.stdout.write(ctrStr+ 'OK    -> '+ path+ ' -> buffer.length:'+ buffer.length);
  }
  
  if (sounds.push(path) === all) go();
}

go();

setTimeout(Date.now, 1e9);
