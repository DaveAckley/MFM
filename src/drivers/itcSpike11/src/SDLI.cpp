#include "SDLI.h"

#include "Logger.h"
#include "AssetManager.h"
#include "ITCSpikeDriver.h"
#include "TileModel.h"

namespace MFM
{
  SDLI * SDLI::mStaticInstance = 0;

  SDLI::SDLI()
    : mScreenWidth(T2_SCREEN_WIDTH)
    , mScreenHeight(T2_SCREEN_HEIGHT)
    , mScreen(init())
    , mRunning(true)
    , mShowCursor(false)
  { }
    
  SDL_Surface* SDLI::init() {
    LOG.Message("SDL early startup");
    u32 flags;
    flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
    int ret = SDL_Init(flags);
    if (ret) {
      LOG.Error("SDL_Init(0x%x) failed: %s",
                flags,
                SDL_GetError());
      FAIL(ILLEGAL_STATE);
    }
    LOG.Message("TTF startup");
    TTF_Init();

    LOG.Message("SDL late startup");
    flags = SDL_SWSURFACE | SDL_FULLSCREEN;
    SDL_Surface* screen = SDL_SetVideoMode(T2_SCREEN_WIDTH, T2_SCREEN_HEIGHT, 32, flags);

    if (screen == 0) {
      LOG.Error("SDL_SetVideoMode(%d,%d,32,0x%x) failed: %s",
                mScreenWidth, mScreenHeight, flags,
                SDL_GetError());
      FAIL(ILLEGAL_STATE);
    }

    u32 gotWidth = SDL_GetVideoSurface()->w;
    u32 gotHeight = SDL_GetVideoSurface()->h;
    if (gotWidth != mScreenWidth || gotHeight != mScreenHeight)
      LOG.Message("Screen %dx%d (wanted %dx%d)",
                  gotWidth, gotHeight,
                  mScreenWidth, mScreenHeight);

    AssetManager::Initialize();
    return screen;
  }

  void SDLI::mainLoop(ITCSpikeDriver & driver) {
    TileModel &model = driver.GetTileModel();
    Event event;
    u32 lastFrame = SDL_GetTicks();

    while (mRunning) {
      SDL_ShowCursor(mShowCursor ? SDL_ENABLE : SDL_DISABLE);

      while (SDL_PollEvent(&event.m_sdlEvent)) {

        switch (event.m_sdlEvent.type) {
        default:
          model.input(event);
          break;

        case SDL_QUIT:
          mRunning = false;
          break;
        }
      }
        
      driver.update();

      //      model.output();

      SDL_Flip(mScreen);

      /* Limit framerate */
      const s32 MS_PER_FRAME = 200;
      u32 thisFrame = SDL_GetTicks();
      lastFrame += MS_PER_FRAME;
      s32 extraMS = (s32) (lastFrame - thisFrame);
      if (extraMS > 0) SDL_Delay(extraMS);
    }
  }

  void SDLI::stop() {
    SDL_Quit();
  }
}
