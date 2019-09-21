#include "main.h"

#include <sys/resource.h>  // For getrlimit / setrlimit

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

void * XXXDRIVER = 0;

namespace MFM
{
  template <class GC> struct T2MenuButton; // FORWARD
  template <class GC> struct T2ITCIndicator; // FORWARD
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

  template <class GC>
  struct MFMT2Driver : public AbstractDualDriver<GC>,
                       public ITCDelegate<typename GC::EVENT_CONFIG>
  {
  private:

    typedef AbstractDualDriver<GC> Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    typedef Grid<GC> OurGrid;

    RandomDirIterator m_dirIterator;
    MFMT2Driver()
      : m_dirIterator(Dirs::DIR_COUNT)
    { }

    /// For ITCDelegate
    
  private:
    u8 m_locksetTaken;
    u8 m_locksetFreed;
    LonglivedLock<EC> m_intertileLocks[Dirs::DIR_COUNT];

  public:

    ///@Override ITCDelegate
    virtual void InitializeTile(Tile<EC> & tile) {
      LOG.Message("InitializeTile running");
      // Set all cacheprocessors idle.  We will activate them per event.
      for (u32 dir8 = 0; dir8 < Dirs::DIR_COUNT; ++dir8) {
        if (dir8 == Dirs::NORTH || dir8 == Dirs::SOUTH) continue; // T2 don't got those
        LonglivedLock<EC> & lll = m_intertileLocks[dir8];
        CacheProcessor<EC> & cp = tile.GetCacheProcessor(dir8);
        lll.SetITCDelegate(this);
        cp.ClaimCacheProcessorForT2(tile, dir8, m_intertileLocks[dir8]);
      }
    }

    ///@Override ITCDelegate ConsiderEventWindow
    virtual bool ConsiderEventWindow(EventWindow<EC> & ew) {
      Tile<EC> & tile = ew.GetTile();
      const SPoint tileCenter = ew.GetCenterInTile();
      u32 eventWindowBoundary = ew.GetBoundary();
      UPoint ctr = MakeUnsigned(tileCenter);
      UPoint size(tile.TILE_WIDTH,tile.TILE_HEIGHT);
      if (m_locksetTaken != 0 || m_locksetFreed != 0) {
        LOG.Error("ConsiderEventWindow with nonzero taken=%02x and/or freed=%02x; freeing",m_locksetTaken,m_locksetFreed);
        m_itcLocks.freeLocks();
        m_locksetTaken = 0;
        return false;
      }
      bool ret = this->m_itcLocks.tryLocksFor(4u, ctr, size, eventWindowBoundary,m_locksetTaken);
      if (!ret) return false;
      m_locksetFreed = 0;
      return true;
          
    }

    ///@Override ITCDelegate AdvanceCommunicationPredicate
    virtual bool AdvanceCommunicationPredicate(Tile<EC> & tile) {
      return m_locksetTaken == 0 && m_locksetFreed == 0;
    }
    
    ///@Override ITCDelegate
    virtual bool TryLock(LonglivedLock<EC> & lll, const CacheProcessor<EC> &cp) {
      u32 dir8 = cp.GetCacheDir();
      u32 dir6 = mapDir8ToDir6(dir8);
      u32 bit = 1<<dir6;
      return bit & m_locksetTaken; //ConsiderEventWindow took all locks we can need
    }

    ///@Override ITCDelegate
    virtual bool Unlock(LonglivedLock<EC> & lll, const CacheProcessor<EC> &cp) {
      u32 dir8 = cp.GetCacheDir();
      u32 dir6 = mapDir8ToDir6(dir8);
      u32 bit = 1<<dir6;
      if (bit & m_locksetFreed) {
        LOG.Message("Unlock already freed dir8=%d/dir6=%d, taken=%02x freed=%02x",dir8,dir6,m_locksetTaken, m_locksetFreed);
        return false;
      }
      m_locksetFreed |= bit;
      if (m_locksetFreed == m_locksetTaken) {
        m_itcLocks.freeLocks();
        m_locksetTaken = m_locksetFreed = 0;
      }
      return true;
    }

    ///@Override ITCDelegate
    virtual bool ReceivePacket(CacheProcessor<EC> &cp, PacketBuffer & to) {
      u32 dir8 = cp.GetCacheDir();
      //      LOG.Message("ReceivePacket running for %d",dir8);
      return m_mfmio.tryReceivePacket(dir8, to);
    }

    ///@Override ITCDelegate
    virtual bool ShipBufferAsPacket(CacheProcessor<EC> & cp, PacketBuffer & pb) {
      OString128 packet;
      CharBufferByteSource cbs = pb.AsByteSource();
      u8 dir8 = cp.GetCacheDir();
      u8 cmd = 0xa0|dir8; // STD MFM pkt hdr 
      packet.Printf("%c",cmd);
      packet.Copy(cbs);
      //      LOG.Message("SBAP %d",packet.GetLength());
      return m_mfmio.trySendPacket((const unsigned char *) packet.GetZString(),
                                   packet.GetLength());
    }

    ///@Override ITCDelegate
    virtual bool IsConnected(const CacheProcessor<EC> &cp) {
      bool ret = isConnected(cp);
      //      LOG.Message("IsConnected = %d",ret);
      return ret;
    }

    bool isConnected(u8 dir6, bool allowRefresh) {
      u8 connected = m_itcLocks.connected(allowRefresh);
      bool ret = (connected & (1<<dir6));
      return ret;
    }

  private:

    bool isConnected(const CacheProcessor<EC> &cp) {
      u8 dir6 = mapDir8ToDir6(cp.GetCacheDir());
      return isConnected(dir6,true);
    }

    bool advanceCP(CacheProcessor<EC> &cp) {
      switch (cp.GetState()) {
      case CacheProcessor<EC>::SHIPPING: return cp.AdvanceShipping();
      case CacheProcessor<EC>::IDLE:    // During these states we just receive
      case CacheProcessor<EC>::PASSIVE: // which is handled elsewhere now
      case CacheProcessor<EC>::RECEIVING: return false;
      case CacheProcessor<EC>::BLOCKING: return cp.AdvanceBlocking();
      default: FAIL(ILLEGAL_STATE);
      }
      /* NOT REACHED */
    }

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
      u32 bgColor = m_opGlobal ? 0xffff3333 : Drawing::GREEN;
      m_t2MenuGridButton.SetEnabledBg(bgColor);
/*
      m_t2MenuQuitButton.SetEnabledBg(bgColor);
      m_t2MenuRebootButton.SetEnabledBg(bgColor);
      m_t2MenuResetButton.SetEnabledBg(bgColor);
      m_t2MenuShutdownButton.SetEnabledBg(bgColor);
*/
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
      case DRIVER_NO_OP: 
        break;
      case STATSON_OP:
      case STATSOFF_OP:
        this->SetInfoBoxVisible(op == STATSON_OP);
        break;
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
      this->SetDelayedDriverOp(DRIVER_NO_OP,-1); // Clear
    }

  public:

    /** Override AbstractGUIDriver method
     */
    virtual void DoPerUpdateSpecialTasks()
    {
      //      this->m_itcLocks.fakeEvent();
      this->m_mfmio.processFlashTraffic();
      if (m_opCountdownTimer > 0) {
        fprintf(stderr,"oct apf %d %d\n",this->GetAEPSPerFrame(), m_opCountdownTimer);
        if (--m_opCountdownTimer == 0) {
          this->DoDriverOpLocally(m_localOp);

        }
      }
    }

    void SetDelayedDriverOp(DriverOp op, s32 delayUpdates) {
      if (delayUpdates <= 0) {
        m_localOp = DRIVER_NO_OP;
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
      , m_locksetTaken(0)
      , m_stamper(*this)
      , m_mfmio(*this)
      , m_keepRunning(true)
      , m_opCountdownTimer(0)
      , m_localOp(DRIVER_NO_OP)
      , m_opGlobal(false)
      , m_t2NORTHEAST(*this,Dirs::NORTHEAST,MENU_WIDTH/2-ITC_RIGHT_INDENT,ITC_WIDTH,MENU_WIDTH/2,0)
      , m_t2EAST     (*this,     Dirs::EAST,ITC_WIDTH,MENU_HEIGHT,MENU_WIDTH-ITC_WIDTH-ITC_RIGHT_INDENT,0)
      , m_t2SOUTHEAST(*this,Dirs::SOUTHEAST,MENU_WIDTH/2-ITC_RIGHT_INDENT,ITC_WIDTH,MENU_WIDTH/2,MENU_HEIGHT-ITC_WIDTH)
      , m_t2SOUTHWEST(*this,Dirs::SOUTHWEST,MENU_WIDTH/2,ITC_WIDTH,0,MENU_HEIGHT-ITC_WIDTH)
      , m_t2WEST     (*this,     Dirs::WEST,ITC_WIDTH,MENU_HEIGHT,0,0)
      , m_t2NORTHWEST(*this,Dirs::NORTHWEST,MENU_WIDTH/2,ITC_WIDTH,0,0)
        /* Going for:
                         Cancel   ..      Quit
                         Reset   Clear    Stats
                         300MHz  720MHz   1HGz
                         Reboot  Shutdown GRID
         */
      , m_t2MenuCancelButton("Cancel",CANCEL_OP,       0*MENU_WIDTH/3, 0*MENU_HEIGHT/4, *this)
      , m_t2MenuStatsButton("Stats",STATSON_OP,        2*MENU_WIDTH/3, 1*MENU_HEIGHT/4, *this)
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

      MFM::LOG.Message("Freeing all locks");
      m_itcLocks.freeLocks();

      Grid<GC> & grid = this->GetGrid();
      SPoint origin;
      Tile<EC> & tile = grid.GetTile(origin);
      tile.SetITCDelegate(this);
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
            u64 thisticks = curticks - lastticks;
            double deltaaeps = thisAEPS - lastAEPS;
            fprintf(fd,"%f %f\n",
                    1000.0*deltaaeps/thisticks,
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
    {
      if (!this->m_haveStartSymbol) return;
      
      OurGrid & mainGrid = this->GetGrid();
      const Element<EC> * elt = mainGrid.LookupElementFromSymbol(this->m_startSymbol);
      if (!elt) {
        LOG.Error("Start symbol '%s' not found", this->m_startSymbol);
        return;
      }

      u32 realWidth = OurGrid::OWNED_WIDTH;
      u32 realHeight = OurGrid::OWNED_HEIGHT;

      SPoint ctr(realWidth/4,realHeight/2);
      mainGrid.PlaceAtom(elt->GetDefaultAtom(), ctr);

      LOG.Message("Starting with one '%s' at (%d,%d)", this->m_startSymbol, ctr.GetX(), ctr.GetY());
      this->SetAEPSPerFrame(1);
      SDL_ShowCursor(false);
    }

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

      // Flush any existing packets
      m_mfmio.flushPendingPackets();
      
    }

    T2ITCIndicator<GC> m_t2NORTHEAST;
    T2ITCIndicator<GC> m_t2EAST;
    T2ITCIndicator<GC> m_t2SOUTHEAST;
    T2ITCIndicator<GC> m_t2SOUTHWEST;
    T2ITCIndicator<GC> m_t2WEST;
    T2ITCIndicator<GC> m_t2NORTHWEST;

    Panel m_t2MenuPanel;
    T2MenuButton<GC> m_t2MenuCancelButton;
    T2MenuButton<GC> m_t2MenuStatsButton;
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

      this->GetRootPanel().Insert(&m_t2NORTHEAST, NULL); this->GetRootPanel().RaiseToTop(&m_t2NORTHEAST);
      this->GetRootPanel().Insert(&m_t2EAST, NULL); this->GetRootPanel().RaiseToTop(&m_t2EAST);
      this->GetRootPanel().Insert(&m_t2SOUTHEAST, NULL); this->GetRootPanel().RaiseToTop(&m_t2SOUTHEAST);
      this->GetRootPanel().Insert(&m_t2SOUTHWEST, NULL); this->GetRootPanel().RaiseToTop(&m_t2SOUTHWEST);
      this->GetRootPanel().Insert(&m_t2WEST, NULL); this->GetRootPanel().RaiseToTop(&m_t2WEST);
      this->GetRootPanel().Insert(&m_t2NORTHWEST, NULL); this->GetRootPanel().RaiseToTop(&m_t2NORTHWEST);

      this->GetRootPanel().Insert(&m_t2MenuPanel, NULL);
      this->GetRootPanel().RaiseToTop(&m_t2MenuPanel);

      m_t2MenuPanel.Insert(&m_t2MenuCancelButton, NULL);
      m_t2MenuPanel.Insert(&m_t2MenuStatsButton, NULL);
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
  struct T2ITCIndicator : public Panel {
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    typedef Grid<GC> OurGrid;
    typedef Tile<EC> OurTile;
    typedef CacheProcessor<EC> OurCacheProcessor;

    MFMT2Driver<GC> & m_driver;
    const u8 m_dir8;
    T2ITCIndicator(MFMT2Driver<GC> & driver, int dir8, int w, int h, int x, int y)
      : Panel()
      , m_driver(driver)
      , m_dir8(dir8)
    {
      char windowLabel[100];
      snprintf(windowLabel,100-1,"ITC_%s", Dirs::GetName(dir8));
      this->SetName(windowLabel);
      this->SetVisible(true);
      this->SetBackground(Drawing::BLACK);
      this->SetForeground(Drawing::RED);

      this->SetDimensions(w,h);
      this->SetDesiredSize(w,h);

      const SPoint pos(x,y);
      this->SetRenderPoint(pos);
    }

    virtual void PaintBorder(Drawing & config) { /* empty */ }

    virtual void PaintComponent(Drawing & config)
    {
      u8 dir6 = mapDir8ToDir6(m_dir8);
      bool con = m_driver.isConnected(dir6, false); /* Don't refresh connection info from paint thread*/
      u32 oldbg = config.GetBackground();
      config.SetBackground(con ? Drawing::GREY05 : Drawing::RED);
      config.Clear();
      config.SetBackground(oldbg);
    }

  };
    
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
      this->SetEnabledBg(Drawing::GREEN);
      this->SetEnabledFg(Drawing::BLACK);

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
      DriverOp todo = m_driverOp;
      // We want to toggle stats but stats button always reports 'on'
      if (todo == STATSON_OP && m_driver.IsInfoBoxVisible()) 
          todo = STATSOFF_OP;

      if (todo == GLOBAL_OP) {
        m_driver.ToggleGlobalOpFlag();
      } else {
        if (m_driver.GetGlobalOpFlag()) {
          m_driver.ToggleGlobalOpFlag();

          for (u32 dir = 0; dir < 8; ++dir) {
            if (!(dir&3)) continue;
            u8 pkthdr = '\xa0' + dir;
            u8 index = m_driver.GetMFMIO().mLastCommandIndex + 1u;
            FlashTraffic ft(pkthdr,todo,index,FLASH_TRAFFIC_TTL);
            m_driver.GetMFMIO().trySendPacket((unsigned char*) &ft,sizeof(ft));
          }
        }
        m_driver.SetDelayedDriverOp(todo, DRIVER_OP_DELAY);
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
