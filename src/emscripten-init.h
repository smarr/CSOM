#ifndef EMSCRIPTEN_INIT_H_
#define EMSCRIPTEN_INIT_H_

#if __EMSCRIPTEN__

#include <emscripten.h>

void init_filesystem() {
  EM_ASM(
         FS.mkdir('/Smalltalk');
         FS.mount(NODEFS, {
              root: './core-lib/Smalltalk'
            }, '/Smalltalk');
         FS.mkdir('/TestSuite');
         FS.mount(NODEFS, {
              root: './core-lib/TestSuite'
            }, '/TestSuite');
         FS.mkdir('/Examples');
         FS.mount(NODEFS, {
              root: './core-lib/Examples'
            }, '/Examples');
  );
}

#else

void init_filesystem() {}

#endif

#endif // EMSCRIPTEN_INIT_H_
