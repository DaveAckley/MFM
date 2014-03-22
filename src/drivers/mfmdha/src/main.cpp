#include "main.h"

namespace MFM {

  struct MFMSimCloudDemo : public AbstractDriver<P1Atom,5,3,4>
  {
    MFMSimCloudDemo(DriverArguments & args) 
      : AbstractDriver(args) 
    {
    }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();

      mainGrid.Needed(Element_Empty<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Dreg<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Boids1<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Boids2<P1Atom, 4>::THE_INSTANCE);
    }

    void ReinitEden() 
    {
      OurGrid & mainGrid = GetGrid();
      StatsRenderer & srend = GetStatsRenderer();

      srend.DisplayStatsForType(Element_Empty<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Dreg<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Res<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Boids1<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Boids2<P1Atom, 4>::TYPE);

      P1Atom aBoid1(Element_Boids1<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
      P1Atom aBoid2(Element_Boids2<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
      P1Atom aDReg(Element_Dreg<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<P1Atom,4>::OWNED_SIDE;

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
