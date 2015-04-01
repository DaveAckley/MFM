#include "main.h"

#include <sys/resource.h>  // For getrlimit / setrlimit

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

namespace MFM
{
  template <class GC, u32 W, u32 H>
  struct Model {
    typedef GC GRID_CONFIG;
    enum { WIDTH = W, HEIGHT = H };
  };

  struct GridConfigCode {

    enum TileType { TileUNSPEC, TileA, TileB, TileC, TileD, TileE, TileF, TileG, TileH, TileI };
    enum { MIN_TYPE_CHAR = 'A', MAX_TYPE_CHAR = 'I' };

    TileType tileType;
    u32 gridWidth;
    u32 gridHeight;

    GridConfigCode(TileType t = TileUNSPEC, u32 width = 0, u32 height = 0)
      : tileType(t)
      , gridWidth(width)
      , gridHeight(height)
    { }

    bool Set(TileType t, u32 width, u32 height)
    {
      return SetTileType(t) && SetGridWidth(width) && SetGridHeight(height);
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

    bool Read(ByteSource & bs)
    {
      u32 w, h;
      u8 ch;

      if (bs.Scanf("{%d%c%d}", &w, &ch, &h) != 5)
        return false;

      if (ch < GridConfigCode::MIN_TYPE_CHAR ||
          ch > GridConfigCode::MAX_TYPE_CHAR)
        return false;

      if (bs.Read() >= 0)  // need EOF here
        return false;

      SetGridWidth(w);
      SetGridHeight(h);
      SetTileType((GridConfigCode::TileType)
                  ((ch - GridConfigCode::MIN_TYPE_CHAR) + GridConfigCode::TileA));
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
  typedef GridConfig<OurEventConfigAll, 24> OurGridConfigTileA; // 256 sites/tile
  typedef GridConfig<OurEventConfigAll, 32> OurGridConfigTileB; // 576 sites/tile
  typedef GridConfig<OurEventConfigAll, 40> OurGridConfigTileC; // 1024 sites/tile
  typedef GridConfig<OurEventConfigAll, 54> OurGridConfigTileD; // 2116 sites/tile
  typedef GridConfig<OurEventConfigAll, 72> OurGridConfigTileE; // 4096 sites/tile
  typedef GridConfig<OurEventConfigAll, 98> OurGridConfigTileF; // 8100 sites/tile
  typedef GridConfig<OurEventConfigAll,136> OurGridConfigTileG; // 16384 sites/tile
  typedef GridConfig<OurEventConfigAll,188> OurGridConfigTileH; // 32400 sites/tile
  typedef GridConfig<OurEventConfigAll,264> OurGridConfigTileI; // 65536 sites/tile

  /////
  // Standard models
  static const GridConfigCode gccModelStd(GridConfigCode::TileC, 5, 3);     // Default
  static const GridConfigCode gccModelAlt(GridConfigCode::TileE, 3, 2);     // Alternate model (flatter space)
  static const GridConfigCode gccModelTiny(GridConfigCode::TileB, 2, 2);    // Tiny model
  static const GridConfigCode gccModelBig(GridConfigCode::TileD, 8, 5);     // Larger model
  static const GridConfigCode gccModelBig1(GridConfigCode::TileH, 1, 1);    // BigTile model
  static const GridConfigCode gccModelMedium1(GridConfigCode::TileF, 1, 1); // MediumTile model
  static const GridConfigCode gccModelSmall1(GridConfigCode::TileE, 1, 1);  // SmallerTile model

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
#endif /* ULAM_CUSTOM_ELEMENTS */

      //Always include the old buddies?
      this->NeedElement(&Element_Empty<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Wall<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Res<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Dreg<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Sorter<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Data<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Emitter<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Consumer<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Block<EC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb1<EC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb2<EC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb3<EC>::THE_INSTANCE);
      this->NeedElement(&Element_AntiForkBomb<EC>::THE_INSTANCE);
      this->NeedElement(&Element_MQBar<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Mover<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Indexed<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Fish<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Shark<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_Sq1<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_L12<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_R12<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_General<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Creg<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Dmover<EC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkBlue<EC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkRed<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Cyan<EC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Magenta<EC>::THE_INSTANCE);
    }

    ThreadStamper m_stamper;

  public:

    MFMCDriver(u32 gridWidth, u32 gridHeight)
      : Super(gridWidth, gridHeight)
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
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

    virtual void ReinitEden()
    { }
  };

  template <class CONFIG>
  int SimRunner(int argc, const char** argv,u32 gridWidth,u32 gridHeight)
  {
    MFMCDriver<CONFIG> sim(gridWidth,gridHeight);
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
  int SimCheckAndRun(int argc, const char** argv, u32 gridWidth, u32 gridHeight)
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
    return SimRunner<CONFIG>(argc,argv,gridWidth,gridHeight);
  }

  int SimRunConfig(const GridConfigCode & gcc, int argc, const char** argv)
  {
    u32 w = gcc.gridWidth;
    u32 h = gcc.gridHeight;
    switch (gcc.tileType)
    {
#define XX(CODE) case GridConfigCode::CODE: return SimCheckAndRun<OurGridConfig##CODE>(argc, argv, w, h)
      XX(TileA);
      XX(TileB);
      XX(TileC);
      XX(TileD);
      XX(TileE);
      XX(TileF);
      XX(TileG);
      XX(TileH);
      XX(TileI);
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
      gcc.SetTileType(GridConfigCode::TileC);
      gcc.SetGridWidth(5);
      gcc.SetGridHeight(3);
    }

    return SimRunConfig(gcc, argc, argv);
  }
}

int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}
