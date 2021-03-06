// exercise callbacks;

var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}

setTimeout(function nop(){}, 1e9); // don't quit

var howMany= 3e3;

function cb () {
  process.stdout.write('\n['+ this.id+ '].callback');
}


var buffer= new Buffer(2048);
var i= buffer.length;
while (i--) buffer[i]= Math.floor(256*Math.random()); // noise


var i= howMany;
function next () {
  var snd= Sound.create(buffer).play(cb);
  process.stdout.write('\n['+ snd.id+ '].play(cb)');
  if (--i) setTimeout(next, Math.floor(10*Math.random()));
};

next();