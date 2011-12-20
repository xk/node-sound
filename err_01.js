// exercise callbacks;

var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}

var quit= setTimeout(Date.now, 1e9); // don't quit

var howMany= 9999;


function cb () {
  process.stdout.write('\ncb['+ this.id+ ']');
}


var buffer= new Buffer(44100);
var i= buffer.length;
while (i--) buffer[i]= Math.floor(256*Math.random());

var i= howMany;
function next () {
  var snd= Sound.create(buffer);
  process.stdout.write('\n['+ snd.id+ '].play(cb)');
  snd.play(cb);
  snd= null;
  if (--i) setTimeout(next, Math.floor(1000*Math.random()));
};

next();