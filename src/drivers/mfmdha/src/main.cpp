#include "main.h"
#include "P0Atom.h"

namespace MFM {

  typedef ParamConfig<> OurParamConfig;
  typedef P0Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig,5,3> OurGridConfig;

  struct MFMSimCloudDemo : public AbstractDriver<OurGridConfig>
  {
    MFMSimCloudDemo(DriverArguments & args) 
      : AbstractDriver(args) 
    {
    }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();

      mainGrid.Needed(Element_Empty<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Boids1<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Boids2<OurCoreConfig>::THE_INSTANCE);
    }

    void ReinitEden() 
    {
      OurGrid & mainGrid = GetGrid();
      StatsRenderer & srend = GetStatsRenderer();

      srend.DisplayStatsForType(Element_Empty<OurCoreConfig>::TYPE);
      srend.DisplayStatsForType(Element_Dreg<OurCoreConfig>::TYPE);
      srend.DisplayStatsForType(Element_Res<OurCoreConfig>::TYPE);
      srend.DisplayStatsForType(Element_Boids1<OurCoreConfig>::TYPE);
      srend.DisplayStatsForType(Element_Boids2<OurCoreConfig>::TYPE);

      OurAtom aBoid1(Element_Boids1<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom aBoid2(Element_Boids2<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom aDReg(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<OurCoreConfig>::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurCoreConfig>));

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint eloc(GRID_WIDTH*realWidth-2, 10);
      SPoint cloc(1, 10);

      u32 wid = mainGrid.GetWidth()*realWidth;
      u32 hei = mainGrid.GetHeight()*realWidth;
      for(u32 x = 7*wid/16; x < 9*wid/16; x+=2) {
        for(u32 y = 7*hei/16; y < 9*hei/16; y+=2) {
          aloc.Set(x,y);
          sloc.Set(x+1,y+1);

          if (((x+y)/2)&1)
            mainGrid.PlaceAtom(aBoid1, aloc);
          else
            mainGrid.PlaceAtom(aBoid2, aloc);
          mainGrid.PlaceAtom(aDReg, sloc);
        }
      }
    }
  };
} /* namespace MFM */

int main(int argc, char** argv)
{
  MFM::DriverArguments args(argc,argv);

  MFM::MFMSimCloudDemo sim(args);

  sim.Reinit();

  sim.Run();

  return 0;
}
