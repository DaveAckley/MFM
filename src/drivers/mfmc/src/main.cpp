#include "main.h"

#ifdef MFM_GUI_DRIVER
  #include "AbstractSliderConfig.h"
#endif

namespace MFM
{
  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom, OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig, 5, 3> OurGridConfig;

#ifdef MFM_GUI_DRIVER
  typedef AbstractSliderConfig<OurCoreConfig> OurSliderConfig;
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

#ifdef MFM_GUI_DRIVER
      Element<CC>* elem = &Element_Dreg<OurCoreConfig>::THE_INSTANCE;
      m_dregSliderConfig.SetElement(elem);

      /* Register sliders */
      AbstractGUIDriver::RegisterSliderConfig(&m_dregSliderConfig);
#endif
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

#ifdef MFM_GUI_DRIVER

    class DregSliderConfig : public OurSliderConfig
    {
    public:
      virtual u32 SetupSliders(Slider* sliders, u32 maxSliders) const
      {
        sliders[0].SetText("RES Spawn Odds");
        sliders[0].SetMinValue(1);
        sliders[0].SetMaxValue(1000);
        sliders[0].SetSnapResolution(10);
        sliders[0].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetResOddsPtr());

        sliders[1].SetText("DREG Spawn Odds");
        sliders[1].SetMinValue(1);
        sliders[1].SetMaxValue(1000);
        sliders[1].SetSnapResolution(10);
        sliders[1].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregCreateOddsPtr());

        sliders[2].SetText("Delete Odds");
        sliders[2].SetMinValue(1);
        sliders[2].SetMaxValue(100);
        sliders[2].SetSnapResolution(10);
        sliders[2].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregDeleteOddsPtr());

        sliders[3].SetText("Delete DREG Odds");
        sliders[3].SetMinValue(1);
        sliders[3].SetMaxValue(100);
        sliders[3].SetSnapResolution(10);
        sliders[3].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregDeleteDregOddsPtr());

        return 4;
      }
    }m_dregSliderConfig;

#endif

    virtual void ReinitEden()
    { }
  };
}

int main(int argc, const char** argv)
{
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.ALL);

  MFM::MFMCDriver sim;
  sim.Init(argc, argv);

  sim.Reinit();

  sim.Run();

  return 0;
}
