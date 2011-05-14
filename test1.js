// tests proper destruction/absence of leaks when there's unreferenced sound (and buffer) objects that are still play()ing.

var Sound= require('./build/default/sound');

(function loop () {
  process.nextTick(loop);
  Sound.create(new Buffer(65536)).play();
  process.stdout.write('.');
})();

