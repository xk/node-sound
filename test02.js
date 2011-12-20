// tests proper destruction/absence of leaks when there's unreferenced sound objects that are still play()ing.

function noise (v,i,o) {
  o[i]= Math.floor(256*Math.random());
}

function saw (v,i,o) {
  o[i]= i%256;
}

var buffers= [];

var i= 50;
while (i--) {
  var buf= new Buffer(4096+ 4*Math.floor(1024*Math.random()));
  [].forEach.call(buf, i%2 ? noise : saw);
  buffers.push(buf);
}


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
  Sound.create(buffers[Math.floor(buffers.length*Math.random())]).loop(2).play();
  Sound.create(buffers[Math.floor(buffers.length*Math.random())]).loop(2).play();
  Sound.create(buffers[Math.floor(buffers.length*Math.random())]).loop(2).play();
  Sound.create(buffers[Math.floor(buffers.length*Math.random())]).loop(2).play();
  process.stdout.write('.');
})();

