#include "SDLI.h"

#include <signal.h> // For kill, SIG_TSTP
#include <stdlib.h> // For putenv
#include <thread> // For std::thread
#include <unistd.h> // For sleep, execl

#include "Logger.h"
#include "AssetManager.h"
#include "FileByteSource.h"
#include "T2RootPanel.h"
#include "GridPanel.h"
#include "TextPanel.h"

#include "T2Tile.h"
#include "SimLogPanel.h"
#include "T2InfoPanel.h"
#include "ITCStatusPanel.h"
#include "ChooserPanel.h"
#include "T2GridPanel.h"
#include "T2TimeQueuePanel.h"
#include "T2UIComponents.h"
#include "T2CDMPanel.h"
#include "T2TracePanel.h"
#include "T2RadioButton.h"
#include "T2TitleCard.h"

namespace MFM
{
  static const double REDISPLAY_HZ = 6;
  static const u32 REDISPLAY_MS = (u32) (1000/REDISPLAY_HZ);
  static const u32 OVERFLOW_REDISPLAY_MS = (u32) (1.75*REDISPLAY_MS);
  static const double BACKAVG_FRAC=0.95;

  struct Event {
    SDL_Event m_sdlEvent;
  };

  //  SDLI * SDLI::mStaticInstance = 0;

  SDLI::SDLI(T2Tile& tile, const char * name)
    : mTile(tile)
    , mScreenWidth(T2_SCREEN_WIDTH)
    , mScreenHeight(T2_SCREEN_HEIGHT)
    , mScreen(0)
    , mShowCursor(false)
    , mMouseButtonsDown(0)
    , mKeyboardModifiers(0)
    , mLastKnownMousePosition(0,0)
    , mLastRedisplayMS(0)
    , mAvgMSPerRedisplay(REDISPLAY_MS)
    , mName(name)
    , mMenuManager(tile)
  {
  }

  SDLI::~SDLI() {
    for (auto itr = mPanels.begin(); itr != mPanels.end(); ++itr) {
      Panel * p = itr->second;
      delete p;
    }
    SDL_Quit();
  }

  void SDLI::checkInput() {
    Event absevent;
    //    static u32 eventCount = 0;
    while (SDL_PollEvent(&absevent.m_sdlEvent)) {
      SDL_Event & event = absevent.m_sdlEvent;
      //      if (DoSpecialEventHandling(event)) continue;

      switch(event.type) {
        default:
          LOG.Debug("Unhandled SDL event type %d", event.type);
          break;

        case SDL_VIDEORESIZE:
          LOG.Warning("Ignoring screen resize to (%d,%d)",
                   event.resize.w,
                   event.resize.h);
          break;

        case SDL_QUIT:
          LOG.Warning("Quitting on SDL request");
          // Clean up a few FDs since ~T2Tile etc is not going to run
          mTile.closeFDs();
          if (1) {
            execl("/opt/scripts/t2/RUN_SDL",
                  "/opt/scripts/t2/RUN_SDL",
                  "/opt/scripts/t2/sdlsplash",
                  "/opt/scripts/t2/t2-splash-inverted.png",
                  (char *) 0);
          }
          mTile.requestExit();  // (But we execld so we're not really here.)
          return;  // No more event polling for you

        case SDL_MOUSEBUTTONUP:
          mLastKnownMousePosition.Set(event.button.x, event.button.y);
          mMouseButtonsDown &= ~(1<<(event.button.button));
          mDragStartPositions[event.button.button].Set(-1,-1);
          goto mousebuttondispatch;

        case SDL_MOUSEBUTTONDOWN:
          mLastKnownMousePosition.Set(event.button.x, event.button.y);
          mMouseButtonsDown |= 1<<(event.button.button);
          mDragStartPositions[event.button.button].Set(event.button.x,event.button.y);
          // FALL THROUGH

        mousebuttondispatch:
          {
            MouseButtonEvent mbe(mKeyboardModifiers, event);
            Rect hitrect(SPoint(), UPoint(mScreenWidth,mScreenHeight));
            mRootPanel->Dispatch(mbe, hitrect);
          }
          break;

        case SDL_MOUSEMOTION:
          {
            mLastKnownMousePosition.Set(event.motion.x, event.motion.y);
            MouseMotionEvent mme(mKeyboardModifiers, event,
                                 mMouseButtonsDown, mDragStartPositions);
            mRootPanel->Dispatch(mme,
                                Rect(SPoint(),
                                     UPoint(mScreenWidth,mScreenHeight)));
          }
          break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
          {
            u32 mod = 0;
            switch (event.key.keysym.sym) {
            case SDLK_LSHIFT:  mod = KMOD_SHIFT; goto mod;
            case SDLK_RSHIFT:  mod = KMOD_SHIFT; goto mod;
            case SDLK_LCTRL:   mod = KMOD_CTRL; goto mod;
            case SDLK_RCTRL:   mod = KMOD_CTRL; goto mod;
            case SDLK_LALT:    mod = KMOD_ALT; goto mod;
            case SDLK_RALT:    mod = KMOD_ALT; goto mod;
            mod:
              if (event.type == SDL_KEYDOWN)
                mKeyboardModifiers |= mod;
              else
                mKeyboardModifiers &= ~mod;
              break;
            case SDLK_MENU:
              {
                KeyboardEvent kbe(event.key, mLastKnownMousePosition);
                Panel * hardp = mPanels["HardButtonPanel"];
                if (!hardp)
                  fatal("Undefined hard button panel\n");

                if (hardp->Handle(kbe))
                  return;
              }
              break;

            default:
              {
                KeyboardEvent kbe(event.key, mLastKnownMousePosition);
                Panel * rootp = mPanels["Root"];
                if (!rootp)
                  fatal("Undefined Root panel\n");

                if (rootp->Dispatch(kbe,
                                    Rect(SPoint(),
                                         UPoint(T2_SCREEN_WIDTH,T2_SCREEN_HEIGHT))))
                  return;
              }
    
              LOG.Debug("Ignoring keyboard event %d mod %d at (%d,%d)",
                        event.key.keysym.sym,
                        mod,
                        mLastKnownMousePosition.GetX(),
                        mLastKnownMousePosition.GetY());
              /*KeyboardUpdate(event.key, this->GetGrid(), mLastKnownMousePosition);  */
              break;
            }
          }
          break;
      }
    }
  }

  void SDLI::onTimeout(TimeQueue& srcTQ) {
    insert(srcTQ, REDISPLAY_MS,0);
    checkInput();
    redisplay();
  }

#define HACK_SIGNAL_NUM SIGXCPU  /* Don't think we're likely to really need this? */

  void hackSignalHandler(int sig) {
    signal(HACK_SIGNAL_NUM,SIG_DFL);
  }

  void sleeperSignalThread() {
    signal(HACK_SIGNAL_NUM, hackSignalHandler);
    usleep(300*1000); // 0.3sec
    kill(getpid(), HACK_SIGNAL_NUM);
  }

  void SDLI::doEarlyStartup() {
    message("SDL early startup");
    static char envvars[][50] =
      {
       "SDL_NOMOUSE=1",
       "TSLIB_CONSOLEDEVICE=none",
       "TSLIB_TSDEVICE=/dev/input/event1",
       "SDL_VIDEODRIVER=fbcon",
       "SDL_MOUSEDRV=TSLIB",
       "SDL_MOUSEDEV=/dev/input/event1",
       "SDL_NO_RAWKBD=1",
       0
      };
    for (u32 i = 0; i < sizeof(envvars)/sizeof(envvars[0]); ++i)
      if (putenv(envvars[i])) abort();
    debug("Set envvars");

    u32 flags;
    flags = SDL_INIT_TIMER | SDL_INIT_VIDEO;
    int ret = SDL_Init(flags);
    if (ret) {
      LOG.Error("SDL_Init(0x%x) failed: %s",
                flags,
                SDL_GetError());
      FAIL(ILLEGAL_STATE);
    }
    debug("TTF startup");
    TTF_Init();

  }

  SDL_Surface * SDLI::doLateStartup() {
    message("SDL late startup");

    /********
     For some reason, running without a keyboard causes SDL1.2 to hang
     inside some ioctl while initializing, and no magic environmental
     variable has been found that avoids that behavior.

     However, the ioctl can be interrupted by a signal, and then
     everything seems to work fine (modulo possibly implicated
     possible weirdness during program shutdown).

     So this pitiful code launches a thread that sleeps for a second
     -- letting SDL's ioctl hang -- and then signals ourself, which we
     catch and restore the default signal behavior.

     What a hackmare.
    ********/

    debug("Launching signal thread");
    std::thread kilr(sleeperSignalThread);
    //    u32 flags = SDL_SWSURFACE | SDL_FULLSCREEN;
    u32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN;
    SDL_Surface* screen = SDL_SetVideoMode(T2_SCREEN_WIDTH, T2_SCREEN_HEIGHT, 32, flags);

    debug("Joining signal thread");
    kilr.join();

    if (screen == 0) {
      fatal("SDL_SetVideoMode(%d,%d,32,0x%x) failed: %s",
            mScreenWidth, mScreenHeight, flags,
            SDL_GetError());
    }

    u32 gotWidth = SDL_GetVideoSurface()->w;
    u32 gotHeight = SDL_GetVideoSurface()->h;
    if (gotWidth != mScreenWidth || gotHeight != mScreenHeight)
      LOG.Message("Screen %dx%d (wanted %dx%d)",
                  gotWidth, gotHeight,
                  mScreenWidth, mScreenHeight);

    AssetManager::Initialize();

    SDL_ShowCursor(SDL_DISABLE);

    return screen;
  }

  void SDLI::init() {
    if (mScreen != 0) FAIL(ILLEGAL_STATE);
    doEarlyStartup();
    mScreen = doLateStartup();
    mRootPanel = configureWindows();
  }

  Panel & SDLI::getRootPanel() {
    MFM_API_ASSERT_NONNULL(mRootPanel);
    return *mRootPanel;
  }

  Panel * SDLI::makePanelType(OString128& type) {
    if (type.Equals("Root")) return new T2RootPanel();
    //    if (type.Equals("Grid")) return new GridPanel();
    if (type.Equals("Panel")) return new Panel();
    if (type.Equals("Label")) return new Label();
    if (type.Equals("ITC")) return new ITCStatusPanel(); // XXX FIX ME ITCPanel
    if (type.Equals("Text")) return new TextPanel<80,40>(); 
    if (type.Equals("HistoPanel")) return new HistoPanel(); 
    if (type.Equals("StatusPanel")) return new StatusPanel(); 
    if (type.Equals("StaticPanel")) return new StaticPanel(); 
    if (type.Equals("SimLog")) return new SimLogPanel(); 
    if (type.Equals("T2Info")) return new T2InfoPanel(); 
    if (type.Equals("ChooserPanel")) return new ChooserPanel(); 
    if (type.Equals("MenuItem")) return new MenuItem(); 
    if (type.Equals("T2GridPanel")) return new T2GridPanel(); 
    if (type.Equals("TQPanel")) return new T2TimeQueuePanel(); 
    if (type.Equals("CDMPanel")) return new T2CDMPanel(); 
    if (type.Equals("T2TileLiveCheckbox")) return new T2TileLiveCheckbox(); 
    if (type.Equals("T2TileListenCheckbox")) return new T2TileListenCheckbox(); 
    if (type.Equals("HardButton")) return new T2HardButton(); 
    if (type.Equals("T2TracePanel")) return new T2TracePanel(); 
    if (type.Equals("T2TraceCtlButton")) return new T2TraceCtlButton(); 
    if (type.Equals("T2SeedPhysicsButton")) return new T2SeedPhysicsButton(); 
    if (type.Equals("T2DebugSetupButton")) return new T2DebugSetupButton(); 
    if (type.Equals("T2ClearTileButton")) return new T2ClearTileButton(); 
    if (type.Equals("T2ShowCardButton")) return new T2ShowCardButton(); 
    if (type.Equals("QuitButton")) return new T2QuitButton(); 
    if (type.Equals("CrashButton")) return new T2CrashButton(); 
    if (type.Equals("DumpButton")) return new T2DumpButton(); 
    if (type.Equals("OffButton")) return new T2OffButton(); 
    if (type.Equals("BootButton")) return new T2BootButton(); 
    if (type.Equals("T2RadioButton")) return new T2RadioButton(); 
    if (type.Equals("FlashCommandLabel")) return new T2FlashCommandLabel(); 
    if (type.Equals("XCDMButton")) return new T2KillCDMButton(); 
    if (type.Equals("T2TitleCard")) return new T2TitleCard(); 
    if (type.Equals("WrappedText")) return new WrappedText(); 
    return 0;
  }

  Panel * SDLI::parsePanelDecl(LineCountingByteSource& bs) {
    OString128 name, type, parent;
    s32 w,h,x,y;

    // Name
    if (bs.Scanf("[%[^ ] ",&name) != 3) {
      fatal(bs,"Missing window name");
    }
    // Type
    if (bs.Scanf("%w%[^ ] ",&type) != 3) {
      fatal(bs,"Missing window type for '%s'",name.GetZString());
    }
    // Parent
    if (bs.Scanf("%w%[^ ] ",&parent) != 3) {
      fatal(bs,"Missing parent window for '%s'",name.GetZString());
    }
    if (bs.Scanf("%d%d%d%d", &w,&h,&x,&y) != 4) {
      fatal(bs,"Bad geometry for %s %s",
            name.GetZString(),
            type.GetZString());
    }
    if (bs.Scanf("%w]") != 2) {
      fatal(bs,"Missing ']'");
    }
    if (!Panel::IsLegalPanelName(name.GetZString()))
      fatal(bs,"'%s' is an illegal panel name", name.GetZString());
    
    /*
    debug(bs,"Name:%s, Type:%s, Parent:%s, %dx%d@(%d,%d)",
          name.GetZString(),
          type.GetZString(),
          parent.GetZString(),
          w,h,x,y);
    */
    std::string sname(name.GetZString());
    std::string sparent(parent.GetZString());
    if (mPanels[sname] != 0) {
      fatal(bs,"Duplicate window name '%s'", name.GetZString());
    }
    Panel * parentPanel = 0;
    if (name.Equals("Root")) {
      if  (!parent.Equals("-"))
        fatal(bs,"The parent of 'Root' must be '-', not '%s'",
              parent.GetZString());
    } else {
      if (mPanels[sparent] == 0)
        fatal(bs,"Parent '%s' must exist before '%s' can be defined",
              parent.GetZString(),
              name.GetZString());
      parentPanel = mPanels[sparent];
    }
      
    Panel * nup = makePanelType(type);
    if (!nup) {
      fatal(bs,"Unrecognized window type '%s' (for %s)",
            type.GetZString(),
            name.GetZString());
    }
    
    nup->SetName(name.GetZString());
    nup->SetDimensions(w,h);
    nup->SetDesiredSize(w,h);
    nup->SetRenderPoint(SPoint(x,y));

    if (parentPanel != 0) {
      parentPanel->Insert(nup, 0);
      parentPanel->RaiseToTop(nup);
    }

    mPanels[sname] = nup;
    return nup;
  }

  bool SDLI::parsePanelProperty(LineCountingByteSource& bs,
                                Panel * forPanel) {
    OString128 prop;
    if (bs.Scanf("%w%[A-Za-z0-9]%w=%w",&prop) != 5) {
      fatal(bs,"Property name not found for '%s'",forPanel->GetName());
    }

    OString128 val;
    if (bs.Peek() == '\'') {
      if (bs.Scanf("'%[^']'%w",&val) != 4) {
        fatal(bs,"Bad single-quoted value for '%s' in '%s'",
              prop.GetZString(),
              forPanel->GetName());
      }
    } else if (bs.Peek() == '"') {
      if (bs.Scanf("\"%[^\"]\"%w",&val) != 4) {
        fatal(bs,"Bad double-quoted value for '%s' in '%s'",
              prop.GetZString(),
              forPanel->GetName());
      }
    } else {
      if (bs.Scanf("%[^ \n]%W",&val) != 2) {
        fatal(bs,"Bad space-delimited value for '%s' in '%s'",
              prop.GetZString(),
              forPanel->GetName());
      }
    }
    return configurePanelProperty(bs,forPanel,
                                  prop.GetZString(),
                                  val.GetZString());
  }

  u32 SDLI::parseColor(LineCountingByteSource& lcbs,
                       ByteSource& fs) {
    if (fs.Peek() == '#') {
      u32 col;
      if (fs.Scanf("#%08x",&col)==2) return col;
      fatal(lcbs,"Bad #color");
    }
    fatal(lcbs,"Looking for '#' to introduce hex color");
  }

  bool SDLI::parseBool(LineCountingByteSource& lcbs,
                       ByteSource& fs) {
    u32 val;
    if (fs.Scanf("%d",&val)==1) {
      if (val <= 1) return val!=0;
      fatal(lcbs,"Only 0 or 1 legal as numeric boolean values");
    }
    if (fs.Scanf("true")==4) return true;
    if (fs.Scanf("True")==4) return true;
    if (fs.Scanf("false")==5) return false;
    if (fs.Scanf("False")==5) return false;
    fatal(lcbs,"Unrecognized boolean value");
  }

  u32 SDLI::parseUnsigned(LineCountingByteSource& lcbs,
                          ByteSource& fs) {
    u32 val;
    if (fs.Scanf("%d",&val)!=1) 
      fatal(lcbs,"Expected unsigned decimal");
    return val;
  }

  s32 SDLI::parseSigned(LineCountingByteSource& lcbs,
                        ByteSource& fs) {
    s32 val;
    if (fs.Scanf("%d",&val)!=1) 
      fatal(lcbs,"Expected signed decimal");
    return val;
  }

  const char * SDLI::parseString(LineCountingByteSource& lcbs,
                                 ByteSource& fs)
  {
    const u32 BUF_SIZ = 128;
    static char buf[BUF_SIZ+1];
    u32 i;
    for (i = 0; i < BUF_SIZ; ++i) {
      s32 ch = fs.Read();
      if (ch < 0) break;
      buf[i] = (char) ch;
    }
    buf[i] = '\0';
    return buf;
  }

  bool SDLI::configurePanelProperty(LineCountingByteSource & lcbs,
                                    Panel * forPanel,
                                    const char * prop,
                                    const char * val) {
    CharBufferByteSource cbbs(val,strlen(val));
    if (!strcmp(prop,"bgcolor")) 
      forPanel->SetBackground(parseColor(lcbs, cbbs));
    else if (!strcmp(prop,"fgcolor")) 
      forPanel->SetForeground(parseColor(lcbs, cbbs));
    else if (!strcmp(prop,"bdcolor")) 
      forPanel->SetBorder(parseColor(lcbs, cbbs));
    else if (!strcmp(prop,"doc")) 
      forPanel->SetDoc(val);
    else if (!strcmp(prop,"visible")) 
      forPanel->SetVisible(parseBool(lcbs, cbbs));
    else if (!strcmp(prop,"font")) 
      forPanel->SetFont((FontAsset) parseUnsigned(lcbs, cbbs));
    else if (!strcmp(prop,"zfont")) {
      u32 proportional = parseUnsigned(lcbs, cbbs);
      if (proportional) proportional = 1; // Anything non-zero means 1
      // Eat white
      cbbs.Scanf("%w");

      // Look for ,
      s32 delim = cbbs.Read();
      MFM_API_ASSERT_STATE(delim == ',');

      // Get second arg
      u32 size = parseUnsigned(lcbs, cbbs);
      
      TTF_Font * font = AssetManager::GetZFont(proportional, size);
      MFM_API_ASSERT_NONNULL(font);
      forPanel->SetFontReal(font);
    }
#if 0
    else if (!strcmp(prop,"anchor")) 
      forPanel->SetAnchor(parseAnchor(lcbs, cbbs));
#endif
    else if (!strcmp(prop,"iconslot")) {
      Label * l = dynamic_cast<Label*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to Labels",prop);
      l->SetIconSlot((MasterIconZSheetSlot) parseUnsigned(lcbs,cbbs));
    }
    else if (!strcmp(prop,"text")) {
      Label * l = dynamic_cast<Label*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to Labels",prop);
      l->SetText(parseString(lcbs,cbbs));
    }
    else if (!strcmp(prop,"fontheightadjust")) {
      GenericTextPanel * l = dynamic_cast<GenericTextPanel*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to TextPanels",prop);
      l->SetFontHeightAdjust(parseSigned(lcbs,cbbs));
    }
    else if (!strcmp(prop,"elevatorwidth")) {
      GenericTextPanel * l = dynamic_cast<GenericTextPanel*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to TextPanels",prop);
      l->SetElevatorWidth(parseUnsigned(lcbs,cbbs));
    }
    else if (!strcmp(prop,"action")) {
      MenuItem * l = dynamic_cast<MenuItem*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to MenuItems",prop);
      l->parseAction(val);
    }
    else if (!strcmp(prop,"enabledfg") || !strcmp(prop,"enabledbg")) {
      AbstractButton * l = dynamic_cast<AbstractButton*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to AbstractButtons",prop);
      if (!strcmp(prop,"enabledfg"))
        l->SetEnabledFg(parseColor(lcbs, cbbs));
      else
        l->SetEnabledBg(parseColor(lcbs, cbbs));
    }
    else if (!strcmp(prop,"radiogroup")) {
      AbstractRadioButton * l = dynamic_cast<AbstractRadioButton*>(forPanel);
      if (!l) fatal(lcbs,"'%s' applies only to AbstractRadioButtons",prop);
      l->SetRadioGroup(parseString(lcbs,cbbs));
    }
    else
      fatal(lcbs,"Unknown property '%s'",prop);
    return true;
  }

  bool SDLI::parseWindowConfig(LineCountingByteSource& bs) {
    Panel * prevPanel = 0;
    debug("Parsing window config");
    while (true) {
      // Eat white
      bs.Scanf("%w");

      // Check for EOF
      if (bs.Peek() < 0) break;

      // Check for #COMMENT
      if (bs.Peek() == '#') {
        s32 ch;
        while ((ch = bs.Read()) >= 0 && ch != '\n') { /* empty */ }
        continue;
      }

      if (bs.Peek() == '[')
        prevPanel = parsePanelDecl(bs);
      else {
        if (!prevPanel)
          fatal(bs,"Properties cannot appear before a [Panel] declaration");
        parsePanelProperty(bs, prevPanel);
      }
    }
    {
      Panel * root = mPanels["Root"];
      if (!root)
        fatal("Window 'Root' must be defined");
      T2RootPanel * rp = dynamic_cast<T2RootPanel*>(root);
      if (!rp)
        fatal("Window 'Root' must be type T2RootPanel");
    }
    for (auto itr = mPanels.begin(); itr != mPanels.end(); ++itr) {
      Panel * p = itr->second;
      SDLIConfigurable * scfg = dynamic_cast<SDLIConfigurable*>(p);
      if (scfg)
        scfg->configure(*this);
      TimeoutAble * ta = dynamic_cast<TimeoutAble*>(p);
      if (ta)
        mTile.insertOnMasterTimeQueue(*ta,250);
    }
    return true;
  }

  Panel * SDLI::lookForPanel(const char * named) {
    return mPanels[named];
  }

  Panel* SDLI::configureWindows() {
    const char * path = mTile.getWindowConfigPath();
    if (path == 0) {
      LOG.Warning("No window configuration path");
      mPanels["Root"] = new Label("No window configuration");
      return mPanels["Root"];
    }
    message("Parsing window config %s",path);
    FileByteSource fbs(path);
    if (!fbs.IsOpen()) fatal("Couldn't open %s",path);
    LineCountingByteSource lcbs;
    lcbs.SetLabel(path);
    lcbs.SetByteSource(fbs);
    ByteSink * bs = LOG.GetByteSink();
    if (bs)
      lcbs.SetErrorByteSink(*bs);
    else
      lcbs.SetErrorByteSink(STDERR);
    if (!parseWindowConfig(lcbs))
      fatal("Window configuration failed");
    Panel * rootp = mPanels["Root"];
    if (!rootp)
      fatal("Undefined Root panel\n");
    return rootp;
  }
  
  void SDLI::showFail(const char * file, int line, const char * msg) {
    Drawing draw;
    draw.Reset(mScreen, FONT_ASSET_ELEMENT_MEDIUM);
    draw.Clear();

    for (u32 y = 0; y < 320; ++y) {
      for (u32 x = 0; x < 480; ++x) {
        u32 sum = y + x;
        u32 color = ((sum>>5)&1) ? Drawing::YELLOW : Drawing::BLACK;
        draw.FillRect(x, y, 1, 1, color);
      }
    }
    const char * postslash;
    for (postslash = file; *postslash != 0; ++postslash) {
      if (*postslash == '/') {
        ++postslash;
        break;
      }
    }
    if (*postslash != 0) file = postslash;
    OString128 buf1, buf2;
    buf1.Printf("%s:%d:", file, line);
    buf2.Printf("%s", msg ? msg : "UNKNOWN ERROR");
    FontAsset font1 = FONT_ASSET_ELEMENT_MEDIUM;
    SPoint siz1 = draw.GetTextSizeInFont(buf1.GetZString(), font1);
    siz1 += SPoint(10,10);
    FontAsset font2 = FONT_ASSET_ELEMENT_MEDIUM;
    SPoint siz2 = draw.GetTextSizeInFont(buf2.GetZString(), font2);
    siz2 += SPoint(20,20);
    SPoint size(siz1.GetX() > siz2.GetX() ? siz1.GetX() : siz2.GetX(), siz1.GetY() + siz2.GetY());
    SPoint at = (SPoint(480,320)-size)/2;
    draw.FillRect(at.GetX(),at.GetY(),size.GetX(),size.GetY(), Drawing::RED);
    draw.SetForeground(Drawing::WHITE);
    draw.SetBackground(Drawing::GREY50);

    draw.SetFont(font1);
    draw.BlitBackedTextCentered(buf1.GetZString(), at, UPoint(size.GetX(), siz1.GetY()));
    draw.SetFont(font2);
    draw.BlitBackedTextCentered(buf2.GetZString(), at+SPoint(0,siz1.GetY()), UPoint(size.GetX(), siz2.GetY()));
    SDL_Flip(mScreen);
    sleep(2);
  }

  void SDLI::redisplay() {
    Drawing draw;
    draw.Reset(mScreen, FONT_ASSET_ELEMENT);
    draw.Clear();
    Panel * root = mPanels["Root"];
    root->Paint(draw);

    u32 lastms = mLastRedisplayMS;
    mLastRedisplayMS = mTile.now();
    if (lastms != 0) {
      static u32 updates = 0;
      u32 deltams = mLastRedisplayMS - lastms;
      ++updates;
      if (deltams > OVERFLOW_REDISPLAY_MS) {
        LOG.Warning("Big deltams %d on update %d",deltams,updates);
      }
      mAvgMSPerRedisplay = BACKAVG_FRAC*mAvgMSPerRedisplay + (1.0-BACKAVG_FRAC)*deltams;
      if (false /*mAvgMSPerRedisplay != 0*/) {
        const double secPerFrame =  mAvgMSPerRedisplay / 1000.0;
        const double fps =  1.0 / secPerFrame;
        char buf[100];
        //        snprintf(buf,100,"%c FPS=%5.3f %dms", "/<\\^/>\\v"[updates&7],fps,deltams);
        snprintf(buf,100,"[%*c%2d%*c%5.3f",
                 (updates&7)+1,deltams/100+'0',
                 deltams%100,
                 8-(updates&7),']',
                 //                 +'A',(updates&7)+'A',(updates&7)+'A',(updates&7)+'A',
                 //                 " < ^ > v"[updates&7]," < ^ > v"[updates&7]," < ^ > v"[updates&7]," < ^ > v"[updates&7],
                 fps);
        draw.BlitBackedText(buf, SPoint(60,150), UPoint(200,100));
      }
    }

    {
      static u32 flips;
      u32 before = mTile.now();
      SDL_Flip(mScreen);
      ++flips;
      u32 after = mTile.now();
      if (time_before(before+OVERFLOW_REDISPLAY_MS,after))
        fprintf(stderr,"Big flip %d on #%d\n",after-before,flips);
    }
  }

  void SDLI::handleInput(u32 maxEvents) {
    DIE_UNIMPLEMENTED();
  }

  void SDLI::stop() {
    SDL_Event fake;
    memset(&fake,0,sizeof(fake));
    fake.type = SDL_QUIT;
    SDL_PushEvent(&fake);
  }
}
