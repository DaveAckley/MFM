#include "main.h"

#if 0 // Fri Sep  5 03:03:03 2014 No longer used?
#ifdef MFM_GUI_DRIVER
  #include "AbstractSliderConfig.h"
#endif
#endif

namespace MFM
{
  typedef ParamConfig<96,4,8,40> OurParamConfig;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom, OurParamConfig> OurCoreConfig;
  //  typedef GridConfig<OurCoreConfig, 5, 3> OurGridConfig;
  typedef GridConfig<OurCoreConfig, 5, 3> OurGridConfig;

#if 0 // Fri Sep  5 03:03:18 2014 No longer used?
#ifdef MFM_GUI_DRIVER
  typedef AbstractSliderConfig<OurCoreConfig> OurSliderConfig;
#endif
#endif

  struct MFMCDriver : public AbstractDualDriver<OurGridConfig>
  {
  private:

    typedef AbstractDualDriver<OurGridConfig> Super;

    virtual void DefineNeededElements()
    {
      NeedElement(&Element_Empty<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Wall<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Sorter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Data<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Emitter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Consumer<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Block<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb1<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb2<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb3<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_AntiForkBomb<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_MQBar<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Mover<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Indexed<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Fish<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Shark<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Xtal_Sq1<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Xtal_L12<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Xtal_R12<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Xtal_General<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Creg<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Dmover<OurCoreConfig>::THE_INSTANCE);
    }

  public:
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
}

int main(int argc, const char** argv)
{
  MFM::DateTimeStamp stamper;
  MFM::LOG.SetTimeStamper(&stamper);
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.MESSAGE);

  MFM::MFMCDriver sim;
  sim.Init(argc, argv);

  sim.Reinit();

  sim.Run();

  return 0;
}
