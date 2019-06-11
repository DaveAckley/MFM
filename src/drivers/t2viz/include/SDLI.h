/* -*- C++ -*- */
#ifndef SDLI_H
#define SDLI_H

#include <map>
#include <signal.h>

#include "SDL.h"
#include "SDL_ttf.h"

#include "itype.h"

#include "T2VizConstants.h"
#include "Model.h"

namespace MFM {
  struct Model; // Forward

  struct SDLI {
    static struct SDLI * mStaticInstance;
    const u32 mScreenWidth;
    const u32 mScreenHeight;
    SDL_Surface* const mScreen;

    bool mRunning;
    bool mShowCursor;

#if 0
    typedef void (SDLI::*HandlerFunc)(int); // ptr to member func
    typedef std::map<s32,HandlerFunc> SigMap;

    static void SDLISignalHandler(int sig) {
      if (!mStaticInstance) abort();
      mStaticInstance->deliverSignal(sig);
    }

    SigMap mSigs;
    void signal(s32 sig, HandlerFunc hdlr) {
      if (mStaticInstance == 0) mStaticInstance = this;
      if (mStaticInstance != this) abort();
      mSigs[sig] = hdlr;
      ::signal(sig, SDLISignalHandler);
    }

    void deliverSignal(int sig) {
      SigMap::const_iterator itr = mSigs.find(sig);
      if (itr == mSigs.end()) abort();
      HandlerFunc func = itr->second;
      if (func)
        (this->*func)(sig);
    }
#endif

    SDLI() ;

    SDL_Surface* getScreen() { return mScreen; }

    SDL_Surface* init() ;

    void mainLoop(Model&) ;

    void stop() ;

  };

}


#endif /* SDLI_H */
