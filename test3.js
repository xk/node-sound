// tests multiple sounds playing at once.

// ===================================================================
// =  OJO DA SEGMENTATION FAULT DE VEZ EN CUANDO NO SE SABE POR QUE  =
// ===================================================================

function noise (v,i,o) {
  o[i]= Math.floor(Math.random()*256);
}

function sin (v,i,o) {
  o[i]= Math.floor(Math.sin(i/5)*256);
}

var Sound= require('./build/default/sound');
var snds= [];
var bufs= [];

(function () {
  var i= 222;
  while (i--) {
    var buf= new Buffer(8+4*Math.floor(4000*Math.random()));
    //console.log(buf.length);
    var len= buf.length;
    
    if (Math.random() < 0.5) while (len--) buf[len]= len%256;
    else while (len--) buf[len]= Math.floor(256*Math.random());
    
    snds.push(Sound.create(buf));
    process.stdout.write('+');
  }
})();




(function loop () {
  process.stdout.write('.');
  snds[Math.floor(snds.length*Math.random())].play();
  process.nextTick(loop);
})();

