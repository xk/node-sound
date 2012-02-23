// Test bufferifySync()

var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}
var sounds= ['sound.wav', 'sound.m4a', 'sound.aif', 'sound.mp3', 'sound.au', 'Sous La Pluie.mp3'];

var i= sounds.length;
(function next () {
  if (i--) {
    var path= __dirname+ '/'+ sounds[i];
    process.stdout.write('\n******************************* '+ path+ " ->");
    console.log(" -> buffer.length -> "+ Sound.create(Sound.bufferifySync(path)).play(next).data.length);
  }
  else {
    clearTimeout(quitTimer);
  }
})();

var quitTimer= setTimeout(Date.now, 1e9);

