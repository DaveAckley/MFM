#include "main.h"

#include <sys/resource.h>  // For getrlimit / setrlimit

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

void * XXXDRIVER = 0;

namespace MFM
{
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

  enum { EVENT_HISTORY_SIZE = 100000 };

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
  static const GridConfigCode gccModelBig(GridConfigCode::TileI, 1, 1);     // LargerTile
  static const GridConfigCode gccModelSmall(GridConfigCode::TileG, 1, 1);  // SmallerTile

  template <class GC>
  struct MFMCDriver : public AbstractDualDriver<GC>
  {
  private:

    typedef AbstractDualDriver<GC> Super;
    typedef typename GC::EVENT_CONFIG EC;
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;

    struct ThreadStamper : public DateTimeStamp
    {
      typedef DateTimeStamp Super;
      MFMCDriver &driver;
      ThreadStamper(MFMCDriver &driver) : driver(driver) { }
      virtual Result PrintTo(ByteSink & byteSink, s32 argument = 0)
      {
        Super::PrintTo(byteSink, argument);
        byteSink.Printf("%dAEPS [%x]",
                        (u32) driver.GetAEPS(),
                        (u32) (((u64) pthread_self())>>8));
        return SUCCESS;
      }
    };

    virtual void DefineNeededElements()
    {
#ifdef ULAM_CUSTOM_ELEMENTS
      DefineNeededUlamCustomElements(this);
#endif
    }
    
    ThreadStamper m_stamper;

    static void PrintTileTypes(const char* not_needed, void* nullForShort)
    {
      fprintf(stderr, "Supported tile types\n");

#define XX(A,B,C)							\
    fprintf(stderr, "  Type '%s': %d non-cache sites (%d x %d site storage)\n", #A, (B-8)*(C-8), B, C);
#include "TileSizes.inc"
#undef XX

      exit(0);
    }


  public:

    MFMCDriver(u32 gridWidth, u32 gridHeight, GridLayoutPattern gridLayout)
      : Super(gridWidth, gridHeight, gridLayout)
      , m_stamper(*this)
    {
      MFM::LOG.SetTimeStamper(&m_stamper);
    }

    virtual void DoEpochEvents(Grid<GC>& grid, u32 epochs, u32 epochAEPS)
    {
      /* Write custom epoch code here */

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

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
      if (this->m_includeCPPDemos && this->m_elementRegistry.GetLibraryPathsCount() > 0)
        args.Die("Cannot include ulam elements when using --cpp-demos");
    }

    virtual void ReinitEden()
    { }
  };

  template <class CONFIG>
  int SimRunner(int argc, const char** argv,u32 gridWidth,u32 gridHeight, GridLayoutPattern gridLayout)
  {
    SizedTile<typename CONFIG::EVENT_CONFIG, CONFIG::TILE_WIDTH, CONFIG::TILE_HEIGHT, CONFIG::EVENT_HISTORY_SIZE>::SetGridLayoutPattern(gridLayout); //static before sim (next line)

    MFMCDriver<CONFIG> sim(gridWidth,gridHeight,gridLayout);
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
      const rlim_t needed = 110*sizeof(MFMCDriver<CONFIG>) / 100;  // Ready to give 110% sir
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
}

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
