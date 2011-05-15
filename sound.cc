// 
//  sound.cc
//  node_sound
//  
//  Created by Jorge@jorgechamorro.com on 2011-05-11.
//  Copyright 2011 Proyectos Equis Ka. All rights reserved.
// 

/*
  TODO 
  - Throw exceptions instead of just writing ERRORs to stderr.
  - provide a vumeter per sound
  - play callbacks (DONE)
  - bufferify(filePath, cb) returns a sound to the cb(err, soundObject);
*/

#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <ev.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#if defined (__APPLE__)
  #include <AudioToolbox/AudioToolbox.h>
  #include <CoreFoundation/CoreFoundation.h>
  
  typedef struct playerStruct {
    
    int paused;
    long int id;
    int playing;
    int destroying;
    unsigned long loop;
    ssize_t bufferLength;
    
    AudioQueueRef AQ;
    UInt32 AQBuffer1Length;
    UInt32 AQBuffer2Length;
    AudioQueueBufferRef AQBuffer1;
    AudioQueueBufferRef AQBuffer2;
    AudioStreamBasicDescription format;
    
    int hasCallback;
    int callbackIsPending;
    v8::Persistent<v8::Object> pendingJSCallback;
    v8::Persistent<v8::Object> JSObject;
    v8::Persistent<v8::Object> JSCallback;
    
  } playerStruct;
  
  static AudioStreamBasicDescription gFormato;
  static playerStruct* fondoSnd;
  
#endif

using namespace node;
using namespace v8;

typedef struct queueStruct {
  playerStruct* player;
  queueStruct* next;
  queueStruct* last;
} queueStruct;
static queueStruct* callbacksQueue= NULL;
static ev_async eio_sound_async_notifier;
pthread_mutex_t callbacksQueue_mutex = PTHREAD_MUTEX_INITIALIZER;

static Persistent<String> play_symbol;
static Persistent<String> pause_symbol;
static Persistent<String> data_symbol;
static Persistent<String> volume_symbol;
static Persistent<String> loop_symbol;
static Persistent<String> player_symbol;
static Persistent<String> callback_symbol;
static Persistent<String> id_symbol;
static Persistent<String> bufferify_symbol;

static Persistent<Function> volume_function;
static Persistent<Function> loop_function;
static Persistent<Function> play_function;
static Persistent<Function> pause_function;
static Persistent<Function> create_function;
static Persistent<Function> bufferify_function;

static long int createdCtr= 0;
static long int destroyedCtr= 0;
static long int wasPlayingCtr= 0;
static long int playingNow= 0;








// ================
// = tracker(int) =
// ================

void tracker (int i) {
  playingNow+= i;
  
  if (playingNow > 0) {
    if (fondoSnd->playing) {
      fondoSnd->playing= 0;
      AudioQueuePause(fondoSnd->AQ);
      //fprintf(stderr, "\nfondoSnd->pause()");
    }
  }
  else {
      if (!fondoSnd->playing) {
      fondoSnd->playing= 1;
      AudioQueueStart(fondoSnd->AQ, NULL);
      //fprintf(stderr, "\nfondoSnd->play()");
    }
  }
}







// ===============
// = newPlayer() =
// ===============

playerStruct* newPlayer () {
  playerStruct* player;
  
  player= (playerStruct*) calloc(1, sizeof(playerStruct));
  V8::AdjustAmountOfExternalAllocatedMemory(sizeof(playerStruct));
  
  player->id= createdCtr++;
  player->format= gFormato;
  player->loop= 0;
  player->paused= 0;
  player->playing= 0;
  player->destroying= 0;
  player->hasCallback= 0;
  player->callbackIsPending= 0;
  
  return player;
}







// ====================
// = **** Volume **** =
// ====================

v8::Handle<Value> Volume (const Arguments &args) {
  
  v8::HandleScope scope;
  
#if defined (__APPLE__)
  
  OSStatus err;
  double volumen;
  
  playerStruct* player;
  player= (playerStruct*) (v8::External::Unwrap(args.This()->ToObject()->GetHiddenValue(player_symbol)));
  
  if (args.Length() && args[0]->IsNumber()) {
    volumen= args[0]->NumberValue();
    
    if (volumen < 0) volumen= 0;
    else if (volumen > 1) volumen= 1;
    
    err= AudioQueueSetParameter (player->AQ, kAudioQueueParam_Volume, (AudioQueueParameterValue) volumen);
  }
  
#else
  fprintf(stderr, "\nERROR *** Sound::volume() LINUX not implemented argghh");
  fflush(stderr);
  
#endif


  end:
  return scope.Close(args.This());
}









// ==================
// = **** Loop **** =
// ==================

v8::Handle<Value> Loop (const Arguments &args) {
  
  v8::HandleScope scope;
  
#if defined (__APPLE__)
  
  double loop;
  
  playerStruct* player;
  player= (playerStruct*) (v8::External::Unwrap(args.This()->ToObject()->GetHiddenValue(player_symbol)));
  
  if (args.Length() && args[0]->IsNumber()) {
    loop= args[0]->NumberValue();
    
    if (loop < 0) loop= 0;
    else if (loop > 9007199254740992) loop= 9007199254740992; // 2^53
    
    player->loop= truncl(loop);
  }
  
#else
  fprintf(stderr, "\nERROR *** Sound::loop() LINUX not implemented argghh");
  fflush(stderr);
  
#endif


  end:
  return scope.Close(args.This());
}










// ==================
// = **** Play **** =
// ==================

v8::Handle<Value> Play (const Arguments &args) {
  
  v8::HandleScope scope;
  
#if defined (__APPLE__)
  
  OSStatus err;
  playerStruct* player;
  
  player= (playerStruct*) (v8::External::Unwrap(args.This()->GetHiddenValue(player_symbol)));
  
  //fprintf(stderr, "\n*** Sound::play() [%ld]", player->id);
  
  if (player->hasCallback) {
    player->hasCallback= 0;
    player->JSCallback.Dispose();
  }
  
  
  if (args.Length()) {
    if (args[0]->IsFunction()) {
      player->hasCallback= 1;
      player->JSCallback= v8::Persistent<v8::Object>::New(args[0]->ToObject());
    }
    else {
      return ThrowException(Exception::TypeError(String::New("Sound::Play(): The callback must be a function")));
    }
  }
  
  
  if (player->paused) {
    player->paused= 0;
    player->playing= 1;
    tracker(+1);
    
    err= AudioQueueStart(player->AQ, NULL);
    if (err) {
      fprintf(stderr, " ERROR:AudioQueueStart:[%d]", err);
    }
    
    goto end;
  }
  
  if (player->playing) goto end;
  
  //fprintf(stderr, "\n[%d] PLAY", player->id);
  //fflush(stderr);
  
  player->playing= 1;
  tracker(+1);
  
  player->AQBuffer1->mAudioDataByteSize= player->AQBuffer1Length;
  err= AudioQueueEnqueueBuffer(player->AQ, player->AQBuffer1, 0, NULL);
  if (err) {
    fprintf(stderr, " ERROR:AudioQueueEnqueueBuffer:[%d]", err);
  }
  
  player->AQBuffer2->mAudioDataByteSize= player->AQBuffer2Length;
  err= AudioQueueEnqueueBuffer(player->AQ, player->AQBuffer2, 0, NULL);
  if (err) {
    fprintf(stderr, " ERROR:AudioQueueEnqueueBuffer:[%d]", err);
  }
  
  err= AudioQueueStart(player->AQ, NULL);
  if (err) {
    fprintf(stderr, " ERROR:AudioQueueStart:[%d]", err);
  }
  
#else
  fprintf(stderr, "\nERROR *** Sound::play() LINUX not implemented argghh");
  fflush(stderr);
  
#endif


  end:
  
  //fflush(stderr);
  return scope.Close(args.This());
}







// ======================================
// = **** AudioQueueBufferCallback **** =
// ======================================

#if defined (__APPLE__)

void AQBufferCallback (void* priv, AudioQueueRef AQ, AudioQueueBufferRef AQBuffer) {
  
  OSStatus err;
  playerStruct* player= (playerStruct*) priv;
  
  //fprintf(stderr, "\n[%ld] AQBufferCallback() [%d]", player->id, 1+(AQBuffer == player->AQBuffer2));
  
  
  if (player->destroying) {
    
    //fprintf(stderr, "\n[%ld] DESTROYING", player->id);
    
    goto end;
  };
  
  if (!player->playing) {
    
    //fprintf(stderr, "\n[%ld] IGNORE", player->id);
    
    goto end;
  }
  
  if (player->loop && (AQBuffer == player->AQBuffer1)) {
    
    //fprintf(stderr, "\n[%ld] loop--", player->id);
    
    player->loop--;
  }
  
  if (player->loop) {
    
    //fprintf(stderr, "\n[%ld] RE_QUEUE[%d]", player->id, 1+(AQBuffer == player->AQBuffer2));
    
    // Si estamos en loop simplemente hay que volver a meterlo en la cola.
    
    AQBuffer->mAudioDataByteSize= (AQBuffer == player->AQBuffer1) ? player->AQBuffer1Length : player->AQBuffer2Length;

    err= AudioQueueEnqueueBuffer(player->AQ, AQBuffer, 0, NULL);
    if (err) {
      fprintf(stderr, " ERROR:AQBufferCallback AudioQueueEnqueueBuffer:[%d] ", err);
    }
    
  }
  else if (AQBuffer == player->AQBuffer2) {
    
    //fprintf(stderr, "\n[%ld] CLEANUP", player->id);
    
    err= AudioQueueStop(player->AQ, true);
    if (err) {
      fprintf(stderr, " ERROR:AudioQueueStop:[%d]", err);
    }
    
    player->playing= 0;
    
    if (player->hasCallback) {
      player->callbackIsPending= 1;
      player->pendingJSCallback= player->JSCallback;
      player->hasCallback= 0;
      queueStruct* theItem= (queueStruct*) calloc(1, sizeof(queueStruct));
      theItem->player= player;
      
      pthread_mutex_lock(&callbacksQueue_mutex);
      if (callbacksQueue == NULL) callbacksQueue= theItem;
      else callbacksQueue->last->next= theItem;
      callbacksQueue->last= theItem;
      pthread_mutex_unlock(&callbacksQueue_mutex);
      
      ev_async_send(EV_DEFAULT_UC_ &eio_sound_async_notifier);
    }
  }
  else {
    
    //fprintf(stderr, "\n[%ld] STOP", player->id);
    
    tracker(-1);
    
    err= AudioQueueStop(player->AQ, false);
    if (err) {
      fprintf(stderr, " ERROR:AudioQueueStop:[%d]", err);
    }
  }
  
  end:
  
  //fflush(stderr);
  return;
}

#endif









// ===================
// = **** Pause **** =
// ===================

v8::Handle<Value> Pause (const Arguments &args) {
  
  v8::HandleScope scope;

#if defined (__APPLE__)

  OSStatus err;
  playerStruct* player;
  player= (playerStruct*) (v8::External::Unwrap(args.This()->ToObject()->GetHiddenValue(player_symbol)));
  
  //fprintf(stderr, "\n*** Sound::pause [%ld]", player->id);
  
  if (player->paused) {
    //fprintf(stderr, " IGNORED_WAS_PAUSED");
    goto end;
  }
  
  if (!player->playing) {
    //fprintf(stderr, " IGNORED_WAS_NOT_PLAYING");
    goto end;
  }
  
  player->paused= 1;
  tracker(-1);
  err= AudioQueuePause(player->AQ);
  if (err) {
    fprintf(stderr, " ERROR:AudioQueuePause:[%d]", err);
  }

#else
  fprintf(stderr, "\nERROR *** Sound::pause() LINUX not implemented argghh");
  fflush(stderr);
  
  
#endif

  end:
  //fflush(stderr);
  return scope.Close(args.This());
}








// =========================
// = **** DestroyerCB **** =
// =========================

void destroyerCB (Persistent<Value> object, void* parameter) {
  
#if defined (__APPLE__)
  
  OSStatus err;
  playerStruct* player= (playerStruct*) parameter;
  
  //fprintf(stderr, "\n*** destroyerCB() [%ld]", player->id);
  
  if (player->playing || player->hasCallback || player->callbackIsPending) {
    // Habrá que esperar a que termine antes de cargárselo...
    
    //fprintf(stderr, "\n[%ld] NOT_DESTROYED_WAS_PLAYING_OR_HAS_PENDING_CB", player->id);
    
    object.MakeWeak(player, destroyerCB);
    wasPlayingCtr++;
    goto end;
  }
  
  
  player->destroying= 1;
  err= AudioQueueDispose(player->AQ, true);
  if (err) {
    
    //fprintf(stderr, "\n[%ld] DESTROYING: AudioQueueDispose ERROR:[%d]", player->id, err);
    
    player->destroying= 0;
    object.MakeWeak((void*) player, destroyerCB);
    goto end;
  }
  else {
    
    //fprintf(stderr, "\n[%ld] DESTROYED", player->id);
    
    V8::AdjustAmountOfExternalAllocatedMemory(-(2*player->bufferLength + sizeof(playerStruct)));
    free(player);
    object.Dispose();
    destroyedCtr++;
  }
  
  
#else
  fprintf(stderr, "\nERROR *** destroyerCB() LINUX not implemented argghh");
  fflush(stderr);
  
  
#endif

  end:
  //fflush(stderr);
  return;
}





// ====================
// = **** Create **** =
// ====================

v8::Handle<Value> Create (const Arguments &args) {
  
  //fprintf(stderr, "\nOK *** Sound::Create() BEGIN");
  //fflush(stderr);
  
  v8::HandleScope scope;
  
  if (args.Length() != 1) {
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer): bad number of arguments")));
  }
  
  if (!Buffer::HasInstance(args[0])) {
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer): The argument must be a Buffer() instance")));
  }
  
  //v8::Persistent<Object> buffer= v8::Persistent<Object>::New(args[0]->ToObject());
  v8::Local<Object> buffer= args[0]->ToObject();
  char* bufferData= node::Buffer::Data(buffer);
  size_t bufferLength= node::Buffer::Length(buffer);
  
  if (!bufferLength) {
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer): buffer has length == 0")));
  }
  
  if (bufferLength < 8) {
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer): buffer.length must be >= 8")));
  }
  
  if (bufferLength % 4) {
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer): buffer.length must a multiple of 4")));
  }
  
#if defined (__APPLE__)

  OSStatus err;
  playerStruct* player= newPlayer();
  
  player->AQBuffer1Length= bufferLength/2;
  (player->AQBuffer1Length % 4) && (player->AQBuffer1Length-= (player->AQBuffer1Length % 4));
  player->AQBuffer2Length= bufferLength- player->AQBuffer1Length;
  
  //fprintf(stderr, "\nOK *** AQBufferSize -> [%ld, %ld]", player->AQBuffer1Length, player->AQBuffer2Length);
  //fflush(stderr);
  
  
  
  err= AudioQueueNewOutput(
    &player->format,         // const AudioStreamBasicDescription   *inFormat
    AQBufferCallback,        // AudioQueueOutputCallback            inCallbackProc
    player,                  // void                                *inUserData
    NULL,                    // CFRunLoopRef                        inCallbackRunLoop
    kCFRunLoopDefaultMode,   // CFStringRef                         inCallbackRunLoopMode
    0,                       // UInt32                              inFlags
    &player->AQ              // AudioQueueRef                       *outAQ
  );
  
  if (err) {
    free(player);
    V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(playerStruct));
    fprintf(stderr, "\nERROR *** Sound::create AudioQueueNewOutput:[%d]\n", err);
    fflush(stderr);
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer) AudioQueueNewOutput error")));
  }
  
  /*
    TODO 
    - Hay que hacer un método rewind() ?.
  */
  
  err= AudioQueueAllocateBuffer (
    player->AQ,               // AudioQueueRef inAQ
    player->AQBuffer1Length,  // UInt32 inBufferByteSize
    &player->AQBuffer1        // AudioQueueBufferRef *outBuffer
  );
  
  if (err) {
    AudioQueueDispose (player->AQ, true);
    free(player);
    V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(playerStruct));
    fprintf(stderr, "\nERROR *** Sound::create AudioQueueAllocateBuffer:[%d]\n", err);
    fflush(stderr);
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer) AudioQueueAllocateBuffer error")));
  }
  
  err= AudioQueueAllocateBuffer (
    player->AQ,                // AudioQueueRef inAQ
    player->AQBuffer2Length,   // UInt32 inBufferByteSize
    &player->AQBuffer2         // AudioQueueBufferRef *outBuffer
  );
  
  if (err) {
    AudioQueueDispose (player->AQ, true);
    free(player);
    V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(playerStruct));
    fprintf(stderr, "\nERROR *** Sound::create AudioQueueAllocateBuffer:[%d]\n", err);
    fflush(stderr);
    return ThrowException(Exception::TypeError(String::New("Sound::create(buffer) AudioQueueAllocateBuffer error")));
  }
  
  V8::AdjustAmountOfExternalAllocatedMemory(bufferLength);
  memcpy(player->AQBuffer1->mAudioData, bufferData, player->AQBuffer1Length);
  memcpy(player->AQBuffer2->mAudioData, (bufferData+ player->AQBuffer1Length), player->AQBuffer2Length);
  
  //fprintf(stderr, "\nOK *** Sound::Create() END");
  
#else
  fprintf(stderr, "\nERROR *** Sound::create() LINUX not implemented argghh.");
  fflush(stderr);

  void* player= newPlayer();

#endif
  
  v8::Persistent<v8::Object> JSObject= v8::Persistent<v8::Object>::New(v8::Object::New());
  JSObject.MakeWeak(player, destroyerCB);
  player->JSObject= JSObject;
  
  
  JSObject->Set(id_symbol, v8::Integer::New(player->id));
  JSObject->Set(play_symbol, play_function);
  JSObject->Set(pause_symbol, pause_function);
  JSObject->Set(volume_symbol, volume_function);
  JSObject->Set(loop_symbol, loop_function);
  JSObject->Set(data_symbol, buffer);
  JSObject->SetHiddenValue(player_symbol, v8::External::Wrap(player));
  
  
  if ((createdCtr % 500) == 0) {
    fprintf(stderr, "\nGC *** Sound::create [Created:%ld, Destroyed:%ld, WerePlaying:%ld]\n", createdCtr, destroyedCtr, wasPlayingCtr);
    fflush(stderr);
  }
  
  //fprintf(stderr, "\nOK *** Create\n");
  //fflush(stderr);
  
  return scope.Close(JSObject);
}












// =========================
// = **** Bufferify() **** =
// =========================

v8::Handle<Value> Bufferify (const Arguments &args) {
  
  v8::HandleScope scope;
  
  
  end:
  //fprintf(stderr, "\nOK *** Bufferify\n");
  //fflush(stderr);

  return v8::Undefined();
}











// ===================================
// = **** Callback into node.js **** =
// ===================================

// this is the async libev event callback that runs in node.js main thread
static void Callback (EV_P_ ev_async *watcher, int revents) {
  
  //fprintf(stderr, "*** JSCallback");
  //fflush(stderr);
  
  HandleScope scope;

  assert(watcher == &eio_sound_async_notifier);
  assert(revents == EV_ASYNC);
  
  queueStruct* next;
  queueStruct* item;
  // Grab the queue.
  pthread_mutex_lock(&callbacksQueue_mutex);
  item= callbacksQueue;
  callbacksQueue= NULL;
  pthread_mutex_unlock(&callbacksQueue_mutex);
  
  /*
    TODO 
    - ver qué hay que hacer exactamente cuando cb() throws.
  */
  
  //TryCatch try_catch;
  
  while (item != NULL) {
    
    if (item->player->callbackIsPending) {
      Persistent<Object> cb= item->player->pendingJSCallback;
      Persistent<Function>::Cast(cb)->Call(item->player->JSObject, 0, NULL);
      item->player->callbackIsPending= 0;
      cb.Dispose();
      //if (try_catch.HasCaught()) FatalException(try_catch);
    }
    
    next= item->next;
    free(item);
    item= next;
  }
}







// =================================
// = **** Initialization code **** =
// =================================

// Esto se llama una sola vez, al hacer require('sound');
extern "C" {
  void init (v8::Handle<Object> target) {
    
    v8::HandleScope scope;

    data_symbol= v8::Persistent<String>::New(v8::String::New("data"));
    play_symbol= v8::Persistent<String>::New(v8::String::New("play"));
    id_symbol= v8::Persistent<String>::New(v8::String::New("id"));
    pause_symbol= v8::Persistent<String>::New(v8::String::New("pause"));
    volume_symbol= v8::Persistent<String>::New(v8::String::New("volume"));
    loop_symbol= v8::Persistent<String>::New(v8::String::New("loop"));
    player_symbol= v8::Persistent<String>::New(v8::String::New("_player"));
    bufferify_symbol= v8::Persistent<String>::New(v8::String::New("bufferify"));
    
    volume_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Volume)->GetFunction());
    loop_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Loop)->GetFunction());
    play_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Play)->GetFunction());
    pause_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Pause)->GetFunction());
    create_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Create)->GetFunction());
    bufferify_function= v8::Persistent<Function>::New(v8::FunctionTemplate::New(Bufferify)->GetFunction());
    
    target->Set(v8::String::New("create"), create_function);
    target->Set(v8::String::New("bufferify"), bufferify_function);
    
    // Start async events for callbacks.
    ev_async_init(&eio_sound_async_notifier, Callback);
    ev_async_start(EV_DEFAULT_UC_ &eio_sound_async_notifier);
    ev_unref(EV_DEFAULT_UC);
    
#if defined (__APPLE__)
    gFormato.mSampleRate= 44100;
    gFormato.mFormatID= kAudioFormatLinearPCM;
    gFormato.mFormatFlags= kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    gFormato.mBytesPerPacket= 4;
    gFormato.mFramesPerPacket= 1;
    gFormato.mBytesPerFrame= 4;
    gFormato.mChannelsPerFrame= 2;
    gFormato.mBitsPerChannel= 16;
    
    /*
      This is a dummy player to init the sound machinery and to keep it up and running when no other sounds are playing.
      It does not need any buffers, it just plays silence. It's paused when not needed (when other sounds are playing)
      and restarted when no other sounds are playing (tracker() keeps track of that).
    */
    
    OSStatus err;
    fondoSnd= newPlayer();
    err= AudioQueueNewOutput(
      &fondoSnd->format,         // const AudioStreamBasicDescription   *inFormat
      AQBufferCallback,          // AudioQueueOutputCallback            inCallbackProc
      fondoSnd,                  // void                                *inUserData
      NULL,                      // CFRunLoopRef                        inCallbackRunLoop
      kCFRunLoopDefaultMode,     // CFStringRef                         inCallbackRunLoopMode
      0,                         // UInt32                              inFlags
      &fondoSnd->AQ              // AudioQueueRef                       *outAQ
    );
    
    err= AudioQueueStart(fondoSnd->AQ, NULL);
    if (err) {
      fprintf(stderr, "\nERROR *** Sound::Init AudioQueueStart:[%d]\n", err);
    }
    
#endif

    fprintf(stderr, "\nOK *** Sound::init");
    
#if defined (__APPLE__)
    fprintf(stderr, " [MAC] ");
#else
    fprintf(stderr, " [LINUX] ");
#endif

    fprintf(stderr, " © 2011 Jorge@jorgechamorro.com\n");
    fflush(stderr);
  }

  NODE_MODULE(sound, init);
}
