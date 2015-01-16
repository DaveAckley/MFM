#include "main.h"

#ifdef ULAM_CUSTOM_ELEMENTS
#include "UlamCustomElements.h"
#endif

namespace MFM
{

  /////
  // Standard model

  typedef ParamConfig<96,4,8,40> OurParamConfigStd;
  typedef P3Atom<OurParamConfigStd> OurAtomStd;
  typedef CoreConfig<OurAtomStd, OurParamConfigStd> OurCoreConfigStd;
  typedef GridConfig<OurCoreConfigStd, 5, 3> OurGridConfigStd;

  /////
  // Alternate model (flatter space)

  typedef ParamConfig<96,4,8,68> OurParamConfigAlt;
  typedef P3Atom<OurParamConfigAlt> OurAtomAlt;
  typedef CoreConfig<OurAtomAlt, OurParamConfigAlt> OurCoreConfigAlt;
  typedef GridConfig<OurCoreConfigAlt, 3, 2> OurGridConfigAlt;

  /////
  // Tiny model

  typedef ParamConfig<96,4,8,32> OurParamConfigTiny;
  typedef P3Atom<OurParamConfigTiny> OurAtomTiny;
  typedef CoreConfig<OurAtomTiny, OurParamConfigTiny> OurCoreConfigTiny;
  typedef GridConfig<OurCoreConfigTiny, 2, 2> OurGridConfigTiny;

  /////
  // Larger model

  typedef ParamConfig<96,4,8,48> OurParamConfigBig;
  typedef P3Atom<OurParamConfigBig> OurAtomBig;
  typedef CoreConfig<OurAtomBig, OurParamConfigBig> OurCoreConfigBig;
  typedef GridConfig<OurCoreConfigBig, 8, 5> OurGridConfigBig;

  /////
  // BigTile model

  typedef ParamConfig<96,4,8,176> OurParamConfigBigTile;
  typedef P3Atom<OurParamConfigBigTile> OurAtomBigTile;
  typedef CoreConfig<OurAtomBigTile, OurParamConfigBigTile> OurCoreConfigBigTile;
  typedef GridConfig<OurCoreConfigBigTile, 1, 1> OurGridConfigBigTile;

  /////
  // MediumTile model

  typedef ParamConfig<96,4,8,96> OurParamConfigMediumTile;
  typedef P3Atom<OurParamConfigMediumTile> OurAtomMediumTile;
  typedef CoreConfig<OurAtomMediumTile, OurParamConfigMediumTile> OurCoreConfigMediumTile;
  typedef GridConfig<OurCoreConfigMediumTile, 1, 1> OurGridConfigMediumTile;

  template <class GC>
  struct MFMCDriver : public AbstractDualDriver<GC>
  {
  private:

    typedef AbstractDualDriver<GC> Super;
    typedef typename Super::CC CC;
    typedef typename Super::CC::PARAM_CONFIG P;
    typedef typename Super::CC::ATOM_TYPE T;

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
#else
      //#error no custom wsu
      this->NeedElement(&Element_Empty<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wall<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Res<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Dreg<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Sorter<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Data<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Emitter<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Consumer<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Block<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb1<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb2<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb3<CC>::THE_INSTANCE);
      this->NeedElement(&Element_AntiForkBomb<CC>::THE_INSTANCE);
      this->NeedElement(&Element_MQBar<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Mover<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Indexed<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Fish<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Shark<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_Sq1<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_L12<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_R12<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_General<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Creg<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Dmover<CC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkBlue<CC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkRed<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Cyan<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Magenta<CC>::THE_INSTANCE);
#endif /* ULAM_CUSTOM_ELEMENTS */
    }

    ThreadStamper m_stamper;

  public:

    void SwitchToInternalLogging()
    {
      const char* path = this->GetSimDirPathTemporary("log/log.txt");
      LOG.Message("Switching log target to: %s", path);
      FILE* fp = fopen(path, "w");
      if (!fp) FAIL(IO_ERROR);
      {
        static FileByteSink fbs(fp);
        LOG.SetByteSink(fbs);
      }
      LOG.Message("Switched to log target: %s", path);
      LOG.Message("Command line: %s", this->GetCommandLine());
    }

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

  static int RunSmall(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigTiny> sim;

    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static int RunMedium(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigStd> sim;
    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static int RunAlternate(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigAlt> sim;
    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static int RunBig(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigBig> sim;
    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static int RunBigTile(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigBigTile> sim;
    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }

  static int RunMediumTile(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigMediumTile> sim;
    sim.ProcessArguments(argc, argv);
    sim.SwitchToInternalLogging();
    sim.Init();
    sim.Run();
    return 0;
  }
}

static bool EndsWith(const char *string, const char* suffix)
{
  MFM::u32 slen = strlen(string);
  MFM::u32 xlen = strlen(suffix);
  return xlen <= slen && !strcmp(suffix, &string[slen - xlen]);
}

int main(int argc, const char** argv)
{
  // Early early logging
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.MESSAGE);

  if (EndsWith(argv[0],"_s"))
  {
    return MFM::RunSmall(argc, argv);
  }

  if (EndsWith(argv[0],"_l"))
  {
    return MFM::RunBig(argc, argv);
  }

  if (EndsWith(argv[0],"_1l"))
  {
    return MFM::RunBigTile(argc, argv);
  }

  if (EndsWith(argv[0],"_1m"))
  {
    return MFM::RunMediumTile(argc, argv);
  }

  if (EndsWith(argv[0],"_a"))
  {
    return MFM::RunAlternate(argc, argv);
  }

  return MFM::RunMedium(argc, argv);
}
