// exercise recursive .play() from the callback; It's a bad idea, you'll get big glitches, should use .loop() instead !

var Sound= require('./build/default/sound');

var quit= setTimeout(function nop(){}, 1e9); // don't quit

var howMany= 999;

function cb () {
  if (this.again) {
    process.stdout.write('\n['+ this.id+ '].callback: NO_MORE');
  }
  else {
    this.again= 1;
    this.play(cb);
    process.stdout.write('\n['+ this.id+ '].callback: PLAY_IT_AGAIN'); // Sam.
  }
}


var buffer= new Buffer(2048);
var i= buffer.length;
while (i--) buffer[i]= Math.floor(256*Math.random());


var i= howMany;
function next () {
  var snd= Sound.create(buffer);
  process.stdout.write('\n['+ snd.id+ '].play(cb)');
  snd.play(cb);
  snd= null;
  if (--i) setTimeout(next, Math.floor(10*Math.random()));
};

next();