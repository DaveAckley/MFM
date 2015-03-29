#include "main.h"

#include <sys/resource.h>  // For getrlimit / setrlimit

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

namespace MFM
{

  /////
  // For all models

  typedef P3Atom OurAtomAll;
  typedef Site<P3AtomConfig> OurSiteAll;
  typedef EventConfig<OurSiteAll,4> OurEventConfigAll;

  /////
  // Standard model
  typedef GridConfig<OurEventConfigAll, 40, 5, 3> OurGridConfigStd;

  /////
  // Alternate model (flatter space)
  typedef GridConfig<OurEventConfigAll, 68, 3, 2> OurGridConfigAlt;

  /////
  // Tiny model
  typedef GridConfig<OurEventConfigAll, 32, 2, 2> OurGridConfigTiny;

  /////
  // Larger model
  typedef GridConfig<OurEventConfigAll, 48, 8, 5> OurGridConfigBig;

  /////
  // BigTile model
  typedef GridConfig<OurEventConfigAll, 176, 1, 1> OurGridConfigBigTile;

  /////
  // MediumTile model
  typedef GridConfig<OurEventConfigAll, 96, 1, 1> OurGridConfigMediumTile;

  /////
  // SmallerTile model
  typedef GridConfig<OurEventConfigAll, 58, 1, 1> OurGridConfigSmallTile; //==50x50 after cache edge

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

    MFMCDriver() : m_stamper(*this)
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
  int SimRunner(int argc, const char** argv)
  {
    MFMCDriver<CONFIG> sim;
    sim.ProcessArguments(argc, argv);
    sim.AddInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static bool EndsWith(const char *string, const char* suffix)
  {
    MFM::u32 slen = strlen(string);
    MFM::u32 xlen = strlen(suffix);
    return xlen <= slen && !strcmp(suffix, &string[slen - xlen]);
  }


  template <class CONFIG>
  int SimCheckAndRun(int argc, const char** argv)
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
    return SimRunner<CONFIG>(argc,argv);
  }

  int MainDispatch(int argc, const char** argv)
  {
    // Early early logging
    LOG.SetByteSink(STDERR);
    LOG.SetLevel(LOG.MESSAGE);

    if (EndsWith(argv[0],"_s"))  return SimCheckAndRun<OurGridConfigTiny>(argc, argv);
    if (EndsWith(argv[0],"_l"))  return SimCheckAndRun<OurGridConfigBig>(argc, argv);
    if (EndsWith(argv[0],"_1l")) return SimCheckAndRun<OurGridConfigBigTile>(argc, argv);
    if (EndsWith(argv[0],"_1m")) return SimCheckAndRun<OurGridConfigMediumTile>(argc, argv);
    if (EndsWith(argv[0],"_1s")) return SimCheckAndRun<OurGridConfigSmallTile>(argc, argv);
    if (EndsWith(argv[0],"_a"))  return SimCheckAndRun<OurGridConfigAlt>(argc, argv);
    //if (EndsWith(argv[0],"_m"))   ..or anything else
    return SimCheckAndRun<OurGridConfigStd>(argc, argv);
  }
}

int main(int argc, const char** argv)
{
  return MFM::MainDispatch(argc,argv);
}
