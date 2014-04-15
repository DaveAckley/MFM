#include "main.h"
#include "P1Atom.h"
#include "Element_Bar.h"
#include "Element_Mover.h"

namespace MFM {

  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P1Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig,5,3> OurGridConfig;
  typedef StatsRenderer<OurGridConfig> OurStatsRenderer;

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
      mainGrid.Needed(Element_Bar<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Mover<OurCoreConfig>::THE_INSTANCE);
    }

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();
      OurStatsRenderer & srend = GetStatsRenderer();

      srend.DisplayStatsForElement(mainGrid,Element_Empty<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Res<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Bar<OurCoreConfig>::THE_INSTANCE);
      //      srend.DisplayStatsForType(Element_Mover<OurCoreConfig>::TYPE);

      const SPoint BAR_SIZE(100,50);
      const SPoint center = BAR_SIZE/2;
      OurAtom aBoid1(Element_Bar<OurCoreConfig>::THE_INSTANCE.GetAtom(BAR_SIZE,center));
      OurAtom aBoid2(Element_Mover<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom aDReg(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<OurCoreConfig>::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurCoreConfig>));

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint e1loc(20+2,20+2);
      SPoint e2loc(GRID_WIDTH*realWidth-2-20, GRID_HEIGHT*realWidth-2-20);
      SPoint cloc(GRID_WIDTH*realWidth/2, GRID_HEIGHT*realWidth/2);

      u32 wid = mainGrid.GetWidth()*realWidth;
      u32 hei = mainGrid.GetHeight()*realWidth;
      bool once = true;
      for(u32 x = 7*wid/16; x < 9*wid/16; x+=2) {
        for(u32 y = 7*hei/16; y < 9*hei/16; y+=2) {
          aloc.Set(x,y);
          sloc.Set(x+3,y+3);
          mainGrid.PlaceAtom(aDReg, sloc);
          //

          if (once) {
            mainGrid.PlaceAtom(aBoid1, cloc);
            once = false;
          }
        }
      }

      /*
      mainGrid.PlaceAtom(aBoid2, e1loc);
      mainGrid.PlaceAtom(aBoid2, e2loc);
      mainGrid.PlaceAtom(aBoid2, e1loc+SPoint(1,1));
      mainGrid.PlaceAtom(aBoid2, e2loc+SPoint(1,1));
      */

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
