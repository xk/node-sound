// exercise recursive .play() from the callback; It's a bad idea, looping via the cb() produces glitches, should really use .loop() instead !

var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}

var quit= setTimeout(function nop(){}, 1e9); // don't quit

var howMany= 999;

function cb () {
  if (this.again === 10) {
    process.stdout.write('\n['+ this.id+ '].callback: NO_MORE');
  }
  else {
    this.again= (this.again || 0) + 1;
    this.play(cb);
    process.stdout.write('\n['+ this.id+ '].callback: PLAY_IT_AGAIN'); // Sam.
  }
}


var buffer= new Buffer(8192);
var i= buffer.length;
while (i--) buffer[i]= i%256;


var i= howMany;
function next () {
  var snd= Sound.create(buffer);
  process.stdout.write('\n['+ snd.id+ '].play(cb)');
  snd.play(cb);
  if (--i) setTimeout(next, 3333);
};

next();