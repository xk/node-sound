//2011-11-03 Jorge@jorgechamorro.com
//Hace una musiquilla sintetizando unas cuantas randommente


function createSquareWave (f /*frequency in Hz*/, seconds /*duration in seconds*/) {
  //returns a buffer containing a sound of a sine wave of frequency f and duration seconds in PCM format

  function floatToSignedInteger (value) {
    //converts from float value in the range -1..1 to a 16bits signed integer

    function lonibble (integer) {
      return integer & 0x00ff;
    }

    function hinibble (integer) {
      return (integer & 0xff00) >>> 8;
    }

    if (value > 1) value= 1;
    else if (value < -1) value= -1;
    value= Math.floor(32767*value);
    return {hi:hinibble(value), lo:lonibble(value), v:value};
  }
  
  var kChannels= 2;
  var kBytesPerSample= 2;
  var kSamplesPerSecond= 44100;
  var buffer= new Buffer(Math.floor(seconds*kSamplesPerSecond)* kChannels* kBytesPerSample);

  var i= 0;
  var step= kChannels* kBytesPerSample;
  do {
    var α= (f* 2* Math.PI* i/ kSamplesPerSecond/ step) % (2* Math.PI);
    var sample= floatToSignedInteger((α > Math.PI) ? 1 : -1);
    //process.stdout.write([i/step, α, sample.v, sample.hi, sample.lo] + "\r\n");
    buffer[i]= buffer[i+2]= sample.lo;
    buffer[i+1]= buffer[i+3]= sample.hi;
    i+= step;
  } while (i < buffer.length);

  return buffer;
}


var DONE= 0;
var Sound;
var paths= ['./build/default/sound', './build/release/sound', 'sound'];
while (paths.length) {
  var p= paths.pop();
  try { Sound= require(p) } catch (e) { continue }
  console.log("Módulo de sonido encontrado en: '"+ p+ "'");
  break;
}

var musiquilla= [];
var i= 20;
while (i--) {
  var f= 222+ (3e3* Math.random());
  var t= 0.05+ (Math.random()/20);
  var buffer= createSquareWave(f/*f in Hz*/, t/*duration in seconds*/);
  musiquilla.push(Sound.create(buffer));
}

(function cb () {
  if (!musiquilla.length) return (DONE= 1);
  musiquilla.pop().play(cb);
})();

(function cb () { if (!DONE) setTimeout(cb, 333); })();
