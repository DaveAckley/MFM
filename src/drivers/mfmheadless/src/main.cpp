#include "main.h"

namespace MFM
{
  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom, OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig, 5, 3> OurGridConfig;

  struct MFMSimHeadlessDemo : public AbstractDualDriver<OurGridConfig>
  {
  private: typedef AbstractDualDriver<OurGridConfig> Super;

  public:
    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

    virtual void DefineNeededElements()
    {
      NeedElement(&Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Wall<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb1<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb2<OurCoreConfig>::THE_INSTANCE);
    }

    virtual void ReinitEden()
    {
      OurGrid& m_grid = GetGrid();

      OurAtom dreg(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      SPoint dregPt(5, 5);

      m_grid.PlaceAtom(dreg, dregPt);
    }
  };
}

int main(int argc, const char** argv)
{
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.ALL);

  MFM::MFMSimHeadlessDemo sim;
  sim.Init(argc, argv);

  sim.Reinit();

  sim.Run();

  return 0;
}
