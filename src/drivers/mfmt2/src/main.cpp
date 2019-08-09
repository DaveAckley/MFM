#include "main.h"

#include <sys/resource.h>  // For getrlimit / setrlimit

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

void * XXXDRIVER = 0;


namespace MFM
{
  template <class GC> struct T2MenuButton; // FORWARD
  template <class GC> struct MFMT2Driver;  // FORWARD

  template <class GC, u32 W, u32 H>
  struct Model {
    typedef GC GRID_CONFIG;
    enum { WIDTH = W, HEIGHT = H };
  };

  struct GridConfigCode {

    enum TileType { TileUNSPEC
#define XX(A,B,C) , Tile##A
#include "TileSizes.inc"
#undef XX
      , TileUPPER_BOUND
    };

    static u8 GetTileTypeCode(TileType t)
    {
      switch (t)
      {
#define XX(A,B,C) case Tile##A: return *#A;
#include "TileSizes.inc"
#undef XX
      default: FAIL(ILLEGAL_ARGUMENT);
      }
    }

    static u8 GetMinTypeCode()
    {
      return GetTileTypeCode((TileType) (TileUNSPEC + 1));
    }

    static u8 GetMaxTypeCode()
    {
      return GetTileTypeCode((TileType) (TileUPPER_BOUND - 1));
    }

    TileType tileType;
    u32 gridWidth;
    u32 gridHeight;
    GridLayoutPattern gridLayout;

    GridConfigCode(TileType t = TileUNSPEC, u32 width = 0, u32 height = 0, GridLayoutPattern layout = GRID_LAYOUT_CHECKERBOARD)
      : tileType(t)
      , gridWidth(width)
      , gridHeight(height)
      , gridLayout(layout)
    { }

    bool Set(TileType t, u32 width, u32 height, GridLayoutPattern layout)
    {
      return SetTileType(t) && SetGridWidth(width) && SetGridHeight(height) && SetGridLayout(layout);
    }

    bool SetTileType(TileType t)
    {
      if (tileType != TileUNSPEC || t == TileUNSPEC)
        return false;
      tileType = t;
      return true;
    }

    bool SetGridWidth(u32 width) {
      if (gridWidth != 0 || width == 0)
        return false;
      gridWidth = width;
      return true;
    }

    bool SetGridHeight(u32 height) {
      if (gridHeight != 0 || height == 0)
        return false;
      gridHeight = height;
      return true;
    }

    bool SetGridLayout(GridLayoutPattern layout) {
      gridLayout = layout;
      return true;
    }

    bool Read(ByteSource & bs)
    {
      u32 w, h;
      u8 ch;
      bool doublebrace = false;

      if (bs.Scanf("{") != 1) return false;
      if (bs.Scanf("{") == 1)
	doublebrace = true;
      else
	bs.Unread();

      if (bs.Scanf("%d%c%d}", &w, &ch, &h) != 4)
        return false;
      if (doublebrace && bs.Scanf("}") != 1) return false;

      if (ch < GridConfigCode::GetMinTypeCode() ||
          ch > GridConfigCode::GetMaxTypeCode())
        return false;

      if (bs.Read() >= 0)  // need EOF here
        return false;

      SetGridWidth(w);
      SetGridHeight(h);
      SetGridLayout(doublebrace ? GRID_LAYOUT_STAGGERED : GRID_LAYOUT_CHECKERBOARD);
      SetTileType((GridConfigCode::TileType)
                  ((ch - GridConfigCode::GetMinTypeCode()) + GridConfigCode::TileUNSPEC + 1));
      return true;
    }

  };

  /////
  // For all models

  typedef P3Atom OurAtomAll;
  typedef Site<P3AtomConfig> OurSiteAll;
  typedef EventConfig<OurSiteAll,4> OurEventConfigAll;

  /////
  // Tile types
#define XX(A,B,C)							\
  typedef GridConfig<OurEventConfigAll, B, C, EVENT_HISTORY_SIZE> OurGridConfigTile##A;
#include "TileSizes.inc"
#undef XX
  /*
  typedef GridConfig<OurEventConfigAll, 24> OurGridConfigTileA; // 256 sites/tile
  typedef GridConfig<OurEventConfigAll, 32> OurGridConfigTileB; // 576 sites/tile
  typedef GridConfig<OurEventConfigAll, 40> OurGridConfigTileC; // 1024 sites/tile
  typedef GridConfig<OurEventConfigAll, 54> OurGridConfigTileD; // 2116 sites/tile
  typedef GridConfig<OurEventConfigAll, 72> OurGridConfigTileE; // 4096 sites/tile
  typedef GridConfig<OurEventConfigAll, 98> OurGridConfigTileF; // 8100 sites/tile
  typedef GridConfig<OurEventConfigAll,136> OurGridConfigTileG; // 16384 sites/tile
  typedef GridConfig<OurEventConfigAll,188> OurGridConfigTileH; // 32400 sites/tile
  typedef GridConfig<OurEventConfigAll,264> OurGridConfigTileI; // 65536 sites/tile
  typedef GridConfig<OurEventConfigAll,370> OurGridConfigTileJ; // 131044 sites/tile
  */

  /////
  // Standard models
  static const GridConfigCode gccModelStd(GridConfigCode::TileH, 1, 1);     // Default
  //  static const GridConfigCode gccModelBig(GridConfigCode::TileI, 1, 1);     // LargerTile
  //  static const GridConfigCode gccModelSmall(GridConfigCode::TileG, 1, 1);  // SmallerTile

  static const char MFM_DEV_PATH[] = "/dev/itc/mfm";

  struct FlashTraffic {
    FlashTraffic(u8 pkthdr, u8 cmd, u8 index, u8 ttl)
      : mPktHdr(pkthdr)
      , mCommand(cmd)
      , mIndex(index)
      , mTimeToLive(ttl)
      , mChecksum(computeChecksum())
    { }

    u8 computeChecksum() {
      u32 num = 0;
      num = (num << 5) ^ mCommand;
      num = (num << 5) ^ mIndex;
      num = (num << 5) ^ mTimeToLive;
      return (u8) (num ^ (num>>7) ^ (num>>14));
    }
    void updateChecksum() {
      mChecksum = computeChecksum();
    }
    bool checksumValid() {
      return mChecksum == computeChecksum();
    }
    bool executable(s32 & lastCommandIndex) {
      if (lastCommandIndex >= 0) {
        u8 advance = mIndex - (u8) lastCommandIndex;
        if (advance == 0 || advance >= U8_MAX/3) return false;
      }
      lastCommandIndex = mIndex;
      return true;
    }

    u8 mPktHdr;
    u8 mCommand;
    u8 mIndex;
    u8 mTimeToLive;
    u8 mChecksum;
  };

  template <class GC>
  struct MFMIO {
    MFMT2Driver<GC> &mDriver;
    s32 mMfmPacketFD;
    s32 mLastCommandIndex;
    bool mFlushAvailable;
    MFMIO(MFMT2Driver<GC> & driver)
      : mDriver(driver)
      , mMfmPacketFD(-1)
      , mLastCommandIndex(-1)
      , mFlushAvailable(true)
    { }

    bool open() {
      if (mMfmPacketFD >= 0) abort();
      mMfmPacketFD = ::open(MFM_DEV_PATH, O_RDWR | O_NONBLOCK);
      if (mMfmPacketFD < 0) {
        fprintf(stderr,"Can't open %s: %s\n", MFM_DEV_PATH, strerror(errno));
        return false;
      }
      return true;
    }

    bool close() {
      if (mMfmPacketFD < 0) return false;
      if (::close(mMfmPacketFD) < 0) {
        fprintf(stderr,"Can't close %s: %s\n", MFM_DEV_PATH, strerror(errno));
        return false;
      }

      mMfmPacketFD = -1;
      return true;
    }

    bool trySendPacket(unsigned char * buf, unsigned len) {
      ssize_t amt = write(mMfmPacketFD, buf, len);
      if (amt < 0) {
        if (errno == EHOSTUNREACH || errno == EAGAIN || errno == EWOULDBLOCK)
          return false;
        abort();
      }
      return true;
    }

    void processPacket(unsigned char * buf, unsigned len) {
      if (len < sizeof(FlashTraffic)) {
        fprintf(stderr,"Packet length %d too small '%*s'\n", len, len, buf);
        return;
      }
      FlashTraffic *ft = (FlashTraffic*) buf;
      if (ft->mCommand < CANCEL_OP || ft->mCommand >= DRIVER_OP_COUNT) {
        fprintf(stderr,"MFM packet type '%c' unrecognized\n", ft->mCommand);
        return;
      }
      if (!ft->checksumValid()) {
        fprintf(stderr,"Invalid flash traffic checksum %02x in '%5s', expected %02x\n",
                ft->mChecksum,
                (char*) ft,
                ft->computeChecksum());
        return;
      }
      bool maybeForward = true;
      if (ft->executable(mLastCommandIndex)) {
        fprintf(stderr,"EXECUTING #%d:%c\n", mLastCommandIndex, ft->mCommand);
        mDriver.DoDriverOpLocally((DriverOp) ft->mCommand);  // BAM
      } else {
        fprintf(stderr,"OBSOLETE #%d:%02x\n", mLastCommandIndex, ft->mCommand);
        maybeForward = false;
      }
      if (maybeForward && ft->mTimeToLive > 0) {
        u8 fromDir = ft->mPktHdr&7;
        ft->mTimeToLive--;
        for (unsigned i = 0; i < 8; ++i) {
          if ((i&3) == 0 || i == fromDir) continue; // Skip 0, 4, and fromDir
          ft->mPktHdr = (ft->mPktHdr&~7) | i;
          ft->updateChecksum();
          trySendPacket(buf, len);
        }
      }
    }

    bool update() {
      u32 packetsHandled = 0;
      if (mMfmPacketFD < 0) abort();

      do {
        unsigned char buf[256];
        ssize_t amt = read(mMfmPacketFD, buf, sizeof(buf));

        if (amt == 0) {
          fprintf(stderr,"EOF on %d\n", mMfmPacketFD);
          exit(5);
        }

        if (amt < 0) {
          if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (mFlushAvailable) {
              mFlushAvailable = false;
              LOG.Message("Flushed %d stale packet(s)", packetsHandled);
            }
            mLastCommandIndex = -1;
            return false; /* nothing to read or flush finished */
          } else {
            abort();
          }
        }

        ++packetsHandled;

        if (!mFlushAvailable)
          processPacket(buf,amt);

      } while(mFlushAvailable);

      return true;
    }
  };

  template <class GC>
  struct MFMT2Driver : public AbstractDualDriver<GC>
  {
  private:

    typedef AbstractDualDriver<GC> Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    struct ThreadStamper : public DateTimeStamp
    {
      typedef DateTimeStamp Super;
      MFMT2Driver &driver;
      ThreadStamper(MFMT2Driver &driver) : driver(driver) { }
      virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
      {
        Super::PrintTo(byteSink, argument);
        byteSink.Printf("%dAEPS [%x]",
                        (u32) driver.GetAEPS(),
                        (u32) (((u64) pthread_self())>>8));
        return SUCCESS;
      }
    };

  public:
    virtual void DefineNeededElements()
    {
#ifdef ULAM_CUSTOM_ELEMENTS
      DefineNeededUlamCustomElements(this);
#endif
    }

  private:
    ThreadStamper m_stamper;
    MFMIO<GC> m_mfmio;
    ITCLocks m_itcLocks;

  public:
    MFMIO<GC>& GetMFMIO() { return m_mfmio; }
    ITCLocks& GetITCLocks() { return m_itcLocks; }

    static void PrintTileTypes(const char* not_needed, void* nullForShort)
    {
      fprintf(stderr, "Supported tile types\n");

#define XX(A,B,C)							\
    fprintf(stderr, "  Type '%s': %d non-cache sites (%d x %d site storage)\n", #A, (B-8)*(C-8), B, C);
#include "TileSizes.inc"
#undef XX

      exit(0);
    }

    bool m_keepRunning;

  public:
    virtual bool RunHelperExiter() {
      if (!Super::RunHelperExiter() || !m_keepRunning)
        return false;
      return true;
    }

  private:
    u32 m_opCountdownTimer;
    DriverOp m_localOp;
    bool m_opGlobal; /*next op applies to whole grid*/

  public:
    bool GetGlobalOpFlag() const { return m_opGlobal; }

    void ToggleGlobalOpFlag() {
      m_opGlobal = !m_opGlobal;
      u32 bgColor = m_opGlobal ? 0xffff3333 : 0xff888888;
      m_t2MenuQuitButton.SetBackground(bgColor);
      m_t2MenuRebootButton.SetBackground(bgColor);
      m_t2MenuResetButton.SetBackground(bgColor);
      m_t2MenuShutdownButton.SetBackground(bgColor);
    }

    void setSpeedMHz(u32 mhz) {
      char buff[100];
      snprintf(buff,100,"cpufreq-set -f %dMHz",mhz);
      system(buff);
    }

    void turnOnT2Viz() {
      fprintf(stderr,"turnOnT2Viz UNIMPLEMENTIO\n");
    }

    void DoDriverOpLocally(DriverOp op) {
      fprintf(stderr,"DoDriverOpLocally(%d)\n",op);
      switch (op) {
      case CANCEL_OP:
        this->LowerMenuIfNeeded();
        break;
      case CLEAR_OP:
        this->SetTileEmpty();
        break;
      case RESET_OP:
        this->ReloadCurrentConfigurationPath();
        this->SetKeyboardPaused(false);
        break;
      case QUIT_OP:
        m_keepRunning = false;
        break;
      case REBOOT_OP:
        system("reboot");
        break;
      case SHUTDOWN_OP:
        system("poweroff");
        break;
      case T2VIZ_OP:
        turnOnT2Viz();
        break;

      case MHZ300_OP: setSpeedMHz(300); break;
      case MHZ600_OP: setSpeedMHz(600); break;
      case MHZ720_OP: setSpeedMHz(720); break;
      case MHZ800_OP: setSpeedMHz(800); break;
      case MHZ1000_OP: setSpeedMHz(1000); break;

      case GLOBAL_OP:
        fprintf(stderr,"poof\n");
        break;
      default:
        FAIL(ILLEGAL_ARGUMENT);
      }
      this->SetDelayedDriverOp(DRIVER_OP_COUNT,-1); // Clear
    }

  public:

    /** Override AbstractGUIDriver method
     */
    virtual void DoPerUpdateSpecialTasks()
    {
      this->m_itcLocks.fakeEvent();
      this->m_mfmio.update();
      if (m_opCountdownTimer > 0) {
        fprintf(stderr,"oct %d\n",m_opCountdownTimer);
        if (--m_opCountdownTimer == 0) {
          this->DoDriverOpLocally(m_localOp);

        }
      }
    }

    void SetDelayedDriverOp(DriverOp op, s32 delayUpdates) {
      if (delayUpdates <= 0) {
        m_localOp = DRIVER_OP_COUNT;
        m_opCountdownTimer = 0;
      } else {
        m_localOp = op;
        m_opCountdownTimer = (u32) delayUpdates;
      }
    }

    virtual bool DoSpecialEventHandling(SDL_Event & event) {
      if (event.type == SDL_MOUSEBUTTONUP) {
        if (RaiseMenuIfNeeded()) return true;
        fprintf(stderr,"Mouse up @ (%d,%d)\n",
                event.button.x, event.button.y);
      }
      return Super::DoSpecialEventHandling(event);
    }

    MFMT2Driver(u32 gridWidth, u32 gridHeight, GridLayoutPattern gridLayout)
      : Super(gridWidth, gridHeight, gridLayout)
      , m_stamper(*this)
      , m_mfmio(*this)
      , m_keepRunning(true)
      , m_opCountdownTimer(0)
      , m_localOp(DRIVER_OP_COUNT)
      , m_opGlobal(false)
        /* Going for:
                         Cancel   ..      Quit
                         Reset   Clear    ..
                         300MHz  720MHz   1HGz
                         Reboot  Shutdown GRID
         */
      , m_t2MenuCancelButton("Cancel",CANCEL_OP,       0*MENU_WIDTH/3, 0*MENU_HEIGHT/4, *this)
      , m_t2MenuClearButton("Clear",CLEAR_OP,          1*MENU_WIDTH/3, 1*MENU_HEIGHT/4, *this)
      , m_t2MenuGridButton("Grid",GLOBAL_OP,           2*MENU_WIDTH/3, 3*MENU_HEIGHT/4, *this)
      , m_t2MenuMHz0300Button("300MHz",MHZ300_OP,      0*MENU_WIDTH/3, 2*MENU_HEIGHT/4, *this)
      , m_t2MenuMHz0720Button("720MHz",MHZ720_OP,      1*MENU_WIDTH/3, 2*MENU_HEIGHT/4, *this)
      , m_t2MenuMHz1000Button("1GHz",MHZ1000_OP,       2*MENU_WIDTH/3, 2*MENU_HEIGHT/4, *this)

      , m_t2MenuQuitButton("Quit",QUIT_OP,             2*MENU_WIDTH/3, 0*MENU_HEIGHT/4, *this)
      , m_t2MenuRebootButton("Reboot",REBOOT_OP,       0*MENU_WIDTH/3, 3*MENU_HEIGHT/4, *this)
      , m_t2MenuResetButton("Reset",RESET_OP,          0*MENU_WIDTH/3, 1*MENU_HEIGHT/4, *this)
      , m_t2MenuShutdownButton("Shutdown",SHUTDOWN_OP, 1*MENU_WIDTH/3, 3*MENU_HEIGHT/4, *this)
    {
      MFM::LOG.SetTimeStamper(&m_stamper);
      if (!m_itcLocks.open()) {
        MFM::LOG.Message("Can't open ITCLocks");
      }
    }

    virtual void DoEpochEvents(Grid<GC>& grid, u32 epochs, u32 epochAEPS)
    {
      /* Write custom epoch code here */
      static u64 lastticks = 0;
      static double lastAEPS = 0;
      u64 curticks = this->GetTicksSinceEpoch(); // in ms
      if (lastticks + SECS_PER_STATUS*1000 <= curticks) {
#define XXDIR "/run/mfmt2"
#define XXFILE "status.dat"
        double thisAEPS = this->GetAEPS();
        if (lastAEPS > 0) {
          int ret = mkdir(XXDIR,0x644);
          if (ret == 0) {
            LOG.Message("Created " XXDIR);
          }
          FILE * fd = fopen(XXDIR "/" XXFILE,"w");
          if (fd) {
            double deltaaeps = thisAEPS - lastAEPS;
            fprintf(fd,"%f %f\n",
                    deltaaeps/SECS_PER_STATUS,
                    thisAEPS);
            fclose(fd);
          }
        }
        lastAEPS = thisAEPS;
        lastticks = curticks;
#undef XXDIR
#undef XXFILE
      }

      /* Leave this line here so the Superclass can run as well. */
      Super::DoEpochEvents(grid, epochs, epochAEPS);
    }

    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
      this->RegisterArgumentSection("Driver-specific switches");

      this->RegisterArgument("Display the supported tile types, then exit.",
                             "--tiles", &PrintTileTypes, NULL, false);


    }

    virtual void ReinitEden()
    { }

    virtual void PostUpdate()
    {
      Super::PostUpdate();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);

      if (this->m_includeCPPDemos && this->m_elementRegistry.GetLibraryPathsCount() > 0)
        args.Die("Cannot include ulam elements when using --cpp-demos");

      // Add our extra panels and such
      OnceOnlyGraphics();

      // Set up intertile packet spike
      if (!m_mfmio.open()) {
        abort();
      }
    }

    Panel m_t2MenuPanel;
    T2MenuButton<GC> m_t2MenuCancelButton;
    T2MenuButton<GC> m_t2MenuClearButton;
    T2MenuButton<GC> m_t2MenuGridButton;
    T2MenuButton<GC> m_t2MenuMHz0300Button;
    T2MenuButton<GC> m_t2MenuMHz0720Button;
    T2MenuButton<GC> m_t2MenuMHz1000Button;
    T2MenuButton<GC> m_t2MenuQuitButton;
    T2MenuButton<GC> m_t2MenuRebootButton;
    T2MenuButton<GC> m_t2MenuResetButton;
    T2MenuButton<GC> m_t2MenuShutdownButton;

    bool RaiseMenuIfNeeded() {
      if (m_t2MenuPanel.IsVisible()) return false;
      m_t2MenuPanel.SetVisible(true);
      return true;
    }

    bool LowerMenuIfNeeded() {
      if (!m_t2MenuPanel.IsVisible()) return false;
      m_t2MenuPanel.SetVisible(false);
      return true;
    }

    bool SetTileEmpty() {
      SPoint origin(0,0);
      this->GetGrid().EmptyTile(origin);
      return true;
    }

    void OnceOnlyGraphics() {
      m_t2MenuPanel.SetName("T2Menu");
      m_t2MenuPanel.SetVisible(false);
      m_t2MenuPanel.SetFont(FONT_ASSET_ELEMENT);
      m_t2MenuPanel.SetBackground(Drawing::BLACK);
      m_t2MenuPanel.SetAnchor(ANCHOR_WEST);
      m_t2MenuPanel.SetAnchor(ANCHOR_NORTH);

      m_t2MenuPanel.SetDimensions(MENU_WIDTH,MENU_HEIGHT);
      m_t2MenuPanel.SetDesiredSize(MENU_WIDTH,MENU_HEIGHT);
      const SPoint pos(0,0);
      m_t2MenuPanel.SetRenderPoint(pos);

      //this->InitButtons(&m_t2MenuPanel);

      this->GetRootPanel().Insert(&m_t2MenuPanel, NULL);
      this->GetRootPanel().RaiseToTop(&m_t2MenuPanel);

      m_t2MenuPanel.Insert(&m_t2MenuCancelButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuClearButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuGridButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuMHz0300Button, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuMHz0720Button, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuMHz1000Button, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuQuitButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuRebootButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuResetButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuShutdownButton, NULL);

  
      this->HandleResize(); //Repack
    }

  };

  template <class CONFIG>
  int SimRunner(int argc, const char** argv,u32 gridWidth,u32 gridHeight, GridLayoutPattern gridLayout)
  {
    SizedTile<typename CONFIG::EVENT_CONFIG, CONFIG::TILE_WIDTH, CONFIG::TILE_HEIGHT, CONFIG::EVENT_HISTORY_SIZE>::SetGridLayoutPattern(gridLayout); //static before sim (next line)

    MFMT2Driver<CONFIG> sim(gridWidth,gridHeight,gridLayout);
    XXXDRIVER = &sim;
    sim.ProcessArguments(argc, argv);
    sim.AddInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  /* Note that all the stack size checking is likely moot at this
     point because we've moved the grid to the heap to allow runtime
     grid sizing (since we're not in core/ here).  But it shouldn't
     hurt anything so for now anyway we're leaving it in. */
  template <class CONFIG>
  int SimCheckAndRun(int argc, const char** argv, u32 gridWidth, u32 gridHeight, GridLayoutPattern gridLayout)
  {
    struct rlimit lim;
    if (getrlimit(RLIMIT_STACK, &lim))
      fprintf(stderr,"WARNING: Unable to check stack size (may segfault): %s\n",
              strerror(errno));
    else {
      const rlim_t needed = 110*sizeof(MFMT2Driver<CONFIG>) / 100;  // Ready to give 110% sir
      const rlim_t have = lim.rlim_cur;
      if (have < needed) {
        if (lim.rlim_max < needed)
          fprintf(stderr,"WARNING: Max stack size too small (%ld, need %ld): May segfault!\n",
                  have, needed);
        else {
          // Let's try upping the limit..
          lim.rlim_cur = needed;
          if (setrlimit(RLIMIT_STACK, &lim))
            fprintf(stderr,"WARNING: Unable to increase stack limit (may segfault): %s\n",
                    strerror(errno));
          else
            LOG.Message("NOTE: Simulation needs more space; stack limit increased from %dK to %dK",
                        (u32) (have/1024), (u32) (needed/1024));
        }
      }
    }

    return SimRunner<CONFIG>(argc,argv,gridWidth,gridHeight,gridLayout);
  }

  int SimRunConfig(const GridConfigCode & gcc, int argc, const char** argv)
  {
    u32 w = gcc.gridWidth;
    u32 h = gcc.gridHeight;
    GridLayoutPattern l = gcc.gridLayout;

    switch (gcc.tileType)
    {
#define XX(A,B,C) case GridConfigCode::Tile##A: return SimCheckAndRun<OurGridConfigTile##A>(argc, argv, w, h, l);
#include "TileSizes.inc"
#undef XX
    default:
      FAIL(ILLEGAL_STATE);
    }
  }

  bool CheckForConfigCode(GridConfigCode & gcc, int argc, const char ** argv)
  {
    if (argc < 2) return false;

    GridConfigCode tmp;
    CharBufferByteSource cbs(argv[1], strlen(argv[1]));

    if (!tmp.Read(cbs)) return false;

    gcc = tmp;
    return true;

#if 0
    if (EndsWith(argv[0],"_s")) gcc.Set(gccModelTiny);
    if (EndsWith(argv[0],"_l")) gcc.Set(gccModelBig);
    if (EndsWith(argv[0],"_1l")) gcc.Set(gccModelBig1);
    if (EndsWith(argv[0],"_1m")) gcc.Set(gccModelMedium1);
    if (EndsWith(argv[0],"_1s")) gcc.Set(gccModelSmall1);
    if (EndsWith(argv[0],"_a")) gcc.Set(gccModelAlt);
#endif
  }

  int MainDispatch(int argc, const char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    GridConfigCode gcc;
    if (!CheckForConfigCode(gcc,argc,argv))
    {
      // Default anything still unset
      gcc.SetTileType(GridConfigCode::TileH);
      gcc.SetGridLayout(GRID_LAYOUT_STAGGERED);
      gcc.SetGridWidth(2);
      gcc.SetGridHeight(2);
    }

    return SimRunConfig(gcc, argc, argv);
  }

  template<class GC>
  struct T2MenuButton : public AbstractButton {
    MFMT2Driver<GC> & m_driver;
    const DriverOp m_driverOp;
    T2MenuButton(const char * label, DriverOp op, int x, int y, MFMT2Driver<GC> & driver)
      : AbstractButton(label)
      , m_driver(driver)
      , m_driverOp(op)
    {
      char windowLabel[100];
      snprintf(windowLabel,100-1,"P_%s", label);
      this->SetName(windowLabel);
      this->SetVisible(true);
      this->SetFont(FONT_ASSET_ELEMENT);
      this->SetBackground(Drawing::GREEN);
      this->SetForeground(Drawing::BLACK);

      const u32 INDENT = 2;
      SPoint size(MENU_WIDTH/3-2*INDENT,MENU_HEIGHT/4-2*INDENT);
      this->SetDimensions(size);
      this->SetDesiredSize(size.GetX(),size.GetY());

      const SPoint pos(x+INDENT/2,y+INDENT/2);
      this->SetRenderPoint(pos);
    }

    virtual s32 GetSection() { FAIL(UNSUPPORTED_OPERATION); return -1;  }
    virtual const char * GetDoc() { return "T2 main menu button"; }
    virtual bool GetKey(u32& keysym, u32& mods) { return false; }
    virtual bool ExecuteFunction(u32 keysym, u32 mods) {
      fprintf(stderr, "EXECUTE %s %d %d\n", this->GetText(), keysym, mods);
      return true;
    }

    virtual void OnClick(u8 button) {
      if (m_driverOp == GLOBAL_OP) {
        m_driver.ToggleGlobalOpFlag();
      } else {
        if (m_driver.GetGlobalOpFlag()) {
          m_driver.ToggleGlobalOpFlag();

          for (u32 dir = 0; dir < 8; ++dir) {
            if (!(dir&3)) continue;
            u8 pkthdr = '\xa0' + dir;
            u8 index = m_driver.GetMFMIO().mLastCommandIndex + 1u;
            FlashTraffic ft(pkthdr,m_driverOp,index,FLASH_TRAFFIC_TTL);
            m_driver.GetMFMIO().trySendPacket((unsigned char*) &ft,sizeof(ft));
          }
        }
        m_driver.SetDelayedDriverOp(m_driverOp, DRIVER_OP_DELAY);
        m_driver.LowerMenuIfNeeded();
      }
    }
  };

} // namespace MFM


#define XX(A,B,C) \
void XXXCC##A()  __attribute__ ((used)) ;  \
void XXXCC##A() { \
  if (!XXXDRIVER) abort(); \
  ((MFM::AbstractDriver<MFM::OurGridConfigTile##A>*) XXXDRIVER)->XXXCHECKCACHES(); \
}
#include "TileSizes.inc"
#undef XX


void DP(const MFM::UlamContext<MFM::OurEventConfigAll>& ruc,
        const MFM::UlamRef<MFM::OurEventConfigAll>& rur) __attribute__ ((used)) ;
void DP(const MFM::UlamContext<MFM::OurEventConfigAll>& ruc,
        const MFM::AtomBitStorage<MFM::OurEventConfigAll>& abs) __attribute__ ((used)) ;

void DP(const MFM::UlamContext<MFM::OurEventConfigAll>& ruc,
        const MFM::AtomBitStorage<MFM::OurEventConfigAll>& abs)
{
  MFM::DebugPrint<MFM::OurEventConfigAll>(ruc, abs, MFM::STDERR);
  MFM::STDERR.Printf("\n");
}


void DP(const MFM::UlamContext<MFM::OurEventConfigAll>& ruc,
        const MFM::UlamRef<MFM::OurEventConfigAll>& rur)
{
  MFM::DebugPrint<MFM::OurEventConfigAll>(ruc, rur, MFM::STDERR);
  MFM::STDERR.Printf("\n");
}

int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}
