// tests proper destruction/absence of leaks when there's unreferenced sound (and buffer) objects that are still play()ing.

var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}

(function loop () {
  process.nextTick(loop);
  Sound.create(new Buffer(8192)).play();
  process.stdout.write('.');
})();

