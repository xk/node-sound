// exercise callbacks;

var Sound= require('./build/default/sound');

var quit= setTimeout(function loop () {
  setTimeout(loop, 999);
  var r= [[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]],[[0]]];
  return quit= r;
}, 999); // don't quit

var howMany= 999;

function cb () {
  process.stdout.write('\n['+ this.id+ '].callback');
}


var buffer= new Buffer(44100*4);
var i= buffer.length;
while (i--) buffer[i]= 0;
Sound.create(buffer).loop(1e9).play();


var buffer= new Buffer(8);
var i= buffer.length;
while (i--) buffer[i]= Math.floor(256*Math.random());


var i= howMany;
function next () {
  var snd= Sound.create(buffer);
  process.stdout.write('\n['+ snd.id+ '].play(cb)');
  snd.play(cb);
  snd= null;
  if (--i) setTimeout(next, Math.floor(50*Math.random()));
};

next();