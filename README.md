#node-sound The best sound library for node.js
<p>&nbsp;</p>
Node-sound es un módulo nativo para node.js que sirve para crear/sintetizar y reproducir sonidos en tu Mac, en JavaScript, a partir de un buffer, o a partir de cualquier fichero de música (.aiff, .mp3, .m4a, .aac, .mov, ...) usando las librerías de sonido incorporadas en el Mac OSX.

##Instalar

**NOTA: Sólo funciona con node >= v0.3.6. Sólo funciona con Mac OSX**

**You need node >= v0.3.6 to run this program. Only for Mac OSX**

(Puedes encontrar cualquier versión de node, desde la primera hasta la última, en: http://nodejs.org/dist/ )

Que un módulo sea nativo significa fundamentalmente dos cosas: que está escrito en C y que para poder usarlo hay que compilarlo primero.

Es muy fácil de hacer, suponiendo que ya tengas node.js instalado (y compilado y funcionando).

Lo primero es descargar el .zip o el .tar.gz dando al botón *DOWNLOADS* que hay ahí arriba a la derecha en esta misma página:

<img src= "https://github.com/xk/node-sound/raw/master/imgs/downloads.png" border= "0">

Después descomprimes ese fichero y creará una carpeta *xk-node-sound-xxxx*. Puedes borrar el -xxxx y dejarla en *xk-node-sound* a secas.

Por último, para compilar el módulo teclea:

    cd xk-node-sound
    node-waf configure uninstall distclean configure build install

En mi Mac eso produce algo así, donde lo más importante es la última línea: *'build' finished successfully*:

<img src= "https://github.com/xk/node-sound/raw/master/imgs/node-waf-output.png" border= "0">

Si todo ha ido bien, el módulo compilado se encontrará, dependiendo de la versión de node que tengas instalada, en *xk-node-sound/build/default/sound.node* (versiones más antiguas de node), o en *xk-node-sound/build/release/sound.node* (versiones más modernas de node)

Si algo no va bien, puedes abrir un ticket haciendo click en "issues" arriba, en esta misma página. Describe el problema lo mejor posible y yo recibiré un email automáticamente, y trataré de resolverlo lo antes posible.

<p>&nbsp;</p>
##Manual de instrucciones:

### Require('sound')

Lo primero es cargar el módulo y asignarle un nombre (por ejemplo *Sound*) en la aplicación:

    var Sound= require('sound');

Si node no es capaz de encontralo, tienes dos opciones.

La mejor opción: translada el fichero *sound.node* a la carpeta *node_modules* (si no existe, simplemente créala), o bien, especifica el path completo hasta la carpeta en la que se encuentra *sound.node* :

    require('/absolute/path/to/sound.node's/folder/sound'); // ugh !
    
Por ejemplo, los tests usan:
    
    require('./build/default/sound');

El módulo (que una vez `require()`d se llama *Sound*) tiene 4 métodos:


<p>&nbsp;</p>
###Sound.create(buffer)

Crea un sonido a partir de un *buffer*.

    var buffer= new Buffer(8192);       // Crear un buffer de 8kB
    
    var i= buffer.length;
    while (i--) buffer[i]= i%256;       // Rellenar el buffer con algo que "suene"
    
    var sonido1= Sound.create(buffer);  // Crear el sonido.
    
    sonido1.loop(5).volume(0.5).play(); // Y hacerlo sonar 5 veces seguidas con el volumen al 50%


<p>&nbsp;</p>
###Sound.bufferifySync(path)

Lee un fichero de sonido, preferiblemente :-) y lo transforma en un *buffer*. Admite casi cualquier formato de sonido: .wav, .mp3, .aif, .m4a, etc.

    var buffer = Sound.bufferifySync('unPingüinoEnMiAscensor.mp3');
    var sonido2= Sound.create(buffer);
    
    //o simplemente:
    
    var sonido2= Sound.create( Sound.bufferifySync(path) );
    
    //Y luego le damos a play:
    
    sonido2.play();

<p>&nbsp;</p>
###Sound.bufferify(path, callback)

Es la versión asíncrona de `bufferifySync()`, hace lo mismo pero (en una thread en paralelo) sin bloquear, y cuando ha acabado llama a *callback* y le pasa el *buffer* si no ha habido ningún *error* :

    Sound.bufferify('/path/to/a/sound.file', cb)
    function cb (error, buffer) {
      if (!error) {
        var sonido2= Sound.create(buffer);
      }
    }

<p>&nbsp;</p>
###Sound.stream(path)

Aún no va (2011-05-19). Mejor lo dejamos para otro momento.

    var sonido3= Sound.stream(path)

<p>&nbsp;</p>
###Los métodos de los sonidos:

`Sound.create(buffer)` devuelve un objeto sonido que tiene los siguentes métodos:

    .play()             // evidente.
    .play(callback)     // Igual, pero al acabar llama a callback
    .loop(veces)        // repite el sonido en bucle *veces* veces
    .volume( 0..1 )     // 0 es silencio, 1 es a tope, cualquier cosa intermedia vale también.
    .pause()            // pues eso.

Cada vez que se llama a cualquiera de ellos, devuelve el objeto sonido otra vez, lo que permite encadenar las llamadas:

En vez de:

    sonido.loop(5);
    sonido.volume(1);
    sonido.play();
    
Puedes hacerlo en una sola línea:

    sonido.loop(5).volume(1).play();

Además, cada objeto sonido tiene estos otros 2 atributos:
     
     .id                // Un número de serie que se asigna secuencialmente.
     .data              // Una referencia al buffer con el que se ha creado.
     
En resumen:

<img src= "https://github.com/xk/node-sound/raw/master/imgs/resumen.png" border= "0">

***
© Jorge Chamorro Bieling, 2011. Ver la <a href = "https://github.com/xk/node-sound/raw/master/LICENSE">Licencia</a>