/* -*- C++ -*- */
#ifndef SDLI_H
#define SDLI_H

#include <map>
#include <signal.h>

#include "LineCountingByteSource.h"

#include "SDL.h"
#include "SDL_ttf.h"
#include "Panel.h"

#include "itype.h"

#include "T2VizConstants.h"
#include "TimeoutAble.h"
#include "Menu.h"

namespace MFM {

  struct T2Tile; // FORWARD

  struct SDLI : public TimeoutAble {
    static struct SDLI * mStaticInstance;

    SDLI(T2Tile&,const char *) ;
    ~SDLI() ;

    virtual void onTimeout(TimeQueue& srcTQ) ;

    virtual const char * getName() const { return mName; }

    void checkInput() ;

    void redisplay() ;

    void handleInput(u32 maxEvents) ;

    T2Tile & getTile() { return mTile; }

    SDL_Surface* getScreen() { return mScreen; }

    void init() ;

    Panel * configureWindows() ;

    Panel * makePanelType(OString128& type) ;

    Panel * parsePanelDecl(LineCountingByteSource& bs) ;

    bool parsePanelProperty(LineCountingByteSource& bs, Panel * forPanel) ;

    bool parseWindowConfig(LineCountingByteSource& bs) ;

    bool configurePanelProperty(LineCountingByteSource& bs,
                                Panel * forPanel,
                                const char * prop,
                                const char * val) ;

    u32 parseColor(LineCountingByteSource& lcbs,
                   ByteSource& fs) ;

    const char * parseString(LineCountingByteSource& lcbs,
                             ByteSource& fs) ;

    bool parseBool(LineCountingByteSource& lcbs,
                   ByteSource& fs) ;

    u32 parseUnsigned(LineCountingByteSource& lcbs,
                      ByteSource& fs) ;

    s32 parseSigned(LineCountingByteSource& lcbs,
                    ByteSource& fs) ;

    void stop() ;

    Panel * lookForPanel(const char * named) ;
    MenuManager & getMenuManager() { return mMenuManager; }
  private:

    void doEarlyStartup() ;
    SDL_Surface * doLateStartup() ;
    
    T2Tile & mTile;
    const u32 mScreenWidth;
    const u32 mScreenHeight;
    SDL_Surface* mScreen;

    bool mShowCursor;

    typedef std::map<std::string,Panel*> PanelMap;
    PanelMap mPanels;

    u32 mMouseButtonsDown;
    u32 mKeyboardModifiers;
    ButtonPositionArray mDragStartPositions;
    SPoint mLastKnownMousePosition;

    Panel * mRootPanel;

    u32 mLastRedisplayMS;
    double mAvgMSPerRedisplay;
    const char * mName;

    MenuManager mMenuManager;
  };

  struct SDLIConfigurable {
    virtual void configure(SDLI& sdli) = 0 ;
  };

}


#endif /* SDLI_H */
