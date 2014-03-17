#include "main.h"

namespace MFM {

  class MFMSimDHSDemo : public AbstractDriver<P1Atom,5,3,4>
  {
    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();

      mainGrid.Needed(Element_Dreg<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Sorter<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Emitter<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Consumer<P1Atom, 4>::THE_INSTANCE);
      mainGrid.Needed(Element_Data<P1Atom, 4>::THE_INSTANCE);
    }

    void ReinitEden() 
    {
      OurGrid & mainGrid = GetGrid();
      StatsRenderer & srend = GetStatsRenderer();

      P1Atom atom(Element_Dreg<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
      P1Atom sorter(Element_Sorter<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
      P1Atom emtr(Element_Emitter<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());
      P1Atom cnsr(Element_Consumer<P1Atom,4>::THE_INSTANCE.GetDefaultAtom());

      srend.DisplayStatsForType(Element_Empty<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Dreg<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Res<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Sorter<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Emitter<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Consumer<P1Atom, 4>::TYPE);
      srend.DisplayStatsForType(Element_Data<P1Atom, 4>::TYPE);

      emtr.SetStateField(0,10,10);  // What is this for??
      cnsr.SetStateField(0,10,10);  // What is this for??

      sorter.SetStateField(0,32,50);  // Default threshold

      u32 realWidth = TILE_WIDTH - EVENT_WINDOW_RADIUS * 2;

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint eloc(GRID_WIDTH*realWidth-2, 10);
      SPoint cloc(1, 10);

      for(u32 x = 0; x < mainGrid.GetWidth(); x++)
        {
          for(u32 y = 0; y < mainGrid.GetHeight(); y++)
            {
              for(u32 z = 0; z < 4; z++)
                {
                  aloc.Set(20 + x * realWidth + z, 20 + y * realWidth);
                  sloc.Set(21 + x * realWidth + z, 21 + y * realWidth);
                  mainGrid.PlaceAtom(sorter, sloc);
                  mainGrid.PlaceAtom(atom, aloc);
                }
            }
        }

      mainGrid.PlaceAtom(emtr, eloc);
      mainGrid.PlaceAtom(cnsr, cloc);
      mainGrid.PlaceAtom(cnsr, cloc+SPoint(1,1));  // More consumers than emitters!
    }

  };
}

int main(int argc, char** argv)
{
  MFM::DriverArguments args(argc,argv);

  MFM::MFMSimDHSDemo sim;

  sim.SetSeed(args.GetSeed());
  sim.Reinit();

  sim.Run();

  return 0;
}

