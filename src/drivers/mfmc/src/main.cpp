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

#ifdef MFM_GUI_DRIVER
      {
        Element<CC>* elem = &Element_Dreg<OurCoreConfig>::THE_INSTANCE;
        m_dregSliderConfig.SetElement(elem);
        AbstractGUIDriver::RegisterSliderConfig(&m_dregSliderConfig);
      }
      {
        Element<CC>* elem = &Element_ForkBomb1<OurCoreConfig>::THE_INSTANCE;
        m_forkBombRedSliderConfig.SetElement(elem);
        AbstractGUIDriver::RegisterSliderConfig(&m_forkBombRedSliderConfig);
      }
      {
        Element<CC>* elem = &Element_ForkBomb2<OurCoreConfig>::THE_INSTANCE;
        m_forkBombBlueSliderConfig.SetElement(elem);
        AbstractGUIDriver::RegisterSliderConfig(&m_forkBombBlueSliderConfig);
      }

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
        sliders[0].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetResOddsPtr());
        sliders[0].SetSnapResolution(10);

        sliders[1].SetText("DREG Spawn Odds");
        sliders[1].SetMinValue(1);
        sliders[1].SetMaxValue(1000);
        sliders[1].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregCreateOddsPtr());
        sliders[1].SetSnapResolution(10);

        sliders[2].SetText("Delete Odds");
        sliders[2].SetMinValue(1);
        sliders[2].SetMaxValue(100);
        sliders[2].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregDeleteOddsPtr());
        sliders[2].SetSnapResolution(10);

        sliders[3].SetText("Delete DREG Odds");
        sliders[3].SetMinValue(1);
        sliders[3].SetMaxValue(100);
        sliders[3].SetExternalValue(Element_Dreg<CC>::THE_INSTANCE.GetDregDeleteDregOddsPtr());
        sliders[3].SetSnapResolution(10);

        return 4;
      }
    }m_dregSliderConfig;

    class ForkBombRedSliderConfig : public OurSliderConfig
    {
    public:
      virtual u32 SetupSliders(Slider* sliders, u32 maxSliders) const
      {
        sliders[0].SetText("Bomb Radius");
        sliders[0].SetMinValue(0);
        sliders[0].SetMaxValue(4);
        sliders[0].SetExternalValue(Element_ForkBomb1<CC>::THE_INSTANCE.GetBombRangePtr());

        return 1;
      }
    }m_forkBombRedSliderConfig;

    class ForkBombBlueSliderConfig : public OurSliderConfig
    {
    public:
      virtual u32 SetupSliders(Slider* sliders, u32 maxSliders) const
      {
        sliders[0].SetText("Bomb Radius");
        sliders[0].SetMinValue(0);
        sliders[0].SetMaxValue(4);
        sliders[0].SetExternalValue(Element_ForkBomb2<CC>::THE_INSTANCE.GetBombRangePtr());

        return 1;
      }
    }m_forkBombBlueSliderConfig;

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
