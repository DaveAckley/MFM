#include "main.h"
#include "P1Atom.h"
#include "Element_SBar.h"
#include "Element_DBar.h"
#include "Element_QBar.h"
#include "Element_Mover.h"

namespace MFM {

  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P1Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;

  typedef GridConfig<OurCoreConfig,3,2> OurSmallGridConfig;
  typedef GridConfig<OurCoreConfig,6,4> OurBigGridConfig;

  struct MFMSimQBarDemo : public AbstractDriver<OurBigGridConfig>
  {
    typedef StatsRenderer<OurBigGridConfig> OurStatsRenderer;

    int m_whichSim;

    MFMSimQBarDemo(DriverArguments & args, int whichSim)
      : AbstractDriver(args), m_whichSim(whichSim)
    {
    }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();
      bool addMover = m_whichSim==1;

      mainGrid.Needed(Element_Empty<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_QBar<OurCoreConfig>::THE_INSTANCE);
      if (addMover)
        mainGrid.Needed(Element_Mover<OurCoreConfig>::THE_INSTANCE);
    }

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();
      OurStatsRenderer & srend = GetStatsRenderer();
      bool addMover = m_whichSim==1;

      srend.DisplayStatsForElement(mainGrid,Element_Empty<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Res<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_QBar<OurCoreConfig>::THE_INSTANCE);
      if (addMover)
        srend.DisplayStatsForElement(mainGrid,Element_Mover<OurCoreConfig>::THE_INSTANCE);

      const SPoint BAR_SIZE(17,49);
      const SPoint center = BAR_SIZE/4;
      OurAtom aBoid1(Element_QBar<OurCoreConfig>::THE_INSTANCE.GetAtom(BAR_SIZE,center));
      Element_QBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid1, 0);

      OurAtom aDReg(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<OurCoreConfig>::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurCoreConfig>));

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint e1loc(20+2,20+2);
      SPoint e2loc(GRID_WIDTH*realWidth-2-20, GRID_HEIGHT*realWidth-2-20);
      SPoint cloc(GRID_WIDTH*realWidth, GRID_HEIGHT*realWidth/2);

      u32 wid = mainGrid.GetWidth()*realWidth;
      u32 hei = mainGrid.GetHeight()*realWidth;
      bool once = true;
      for(u32 x = 0; x < wid; x+=5) {
        for(u32 y = 0; y < hei; y+=5) {
          aloc.Set(x,y);
          mainGrid.PlaceAtom(aDReg, aloc);

          if (once) {
            mainGrid.PlaceAtom(aBoid1, cloc*2/5);
            if (addMover) {
              T mover = Element_Mover<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom();
              mainGrid.PlaceAtom(mover, e2loc);
            }
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

  struct MFMSimSBarDemo : public AbstractDriver<OurBigGridConfig>
  {
    typedef StatsRenderer<OurBigGridConfig> OurStatsRenderer;

    MFMSimSBarDemo(DriverArguments & args)
      : AbstractDriver(args)
    {
    }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();

      mainGrid.Needed(Element_Empty<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_SBar<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_DBar<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Mover<OurCoreConfig>::THE_INSTANCE);
    }

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();
      OurStatsRenderer & srend = GetStatsRenderer();

      srend.DisplayStatsForElement(mainGrid,Element_Empty<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_Res<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_SBar<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid,Element_DBar<OurCoreConfig>::THE_INSTANCE);
      //      srend.DisplayStatsForType(Element_Mover<OurCoreConfig>::TYPE);

      const SPoint BAR_SIZE(11,25);
      const SPoint center = BAR_SIZE/4;
      OurAtom aBoid1(Element_SBar<OurCoreConfig>::THE_INSTANCE.GetAtom(BAR_SIZE,center));
      Element_SBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid1, 3);

      OurAtom aBoid2(Element_DBar<OurCoreConfig>::THE_INSTANCE.GetAtom(BAR_SIZE,center));
      Element_DBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid2, DEG000L);

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
      for(u32 x = 0; x < wid; x+=5) {
        for(u32 y = 0; y < hei; y+=5) {
          aloc.Set(x,y);
          mainGrid.PlaceAtom(aDReg, aloc);

          if (once) {
            mainGrid.PlaceAtom(aBoid1, cloc);
            //            mainGrid.PlaceAtom(aBoid2, cloc/2);
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

  int whichSim = 0;
  if (argc > 0)
    whichSim = atoi(argv[0]);

  switch (whichSim) {
  default:
  case 0: {
    MFM::MFMSimQBarDemo sim(args,whichSim);
    sim.Reinit();
    sim.Run();
    break;
  }
  case 2: {
    MFM::MFMSimSBarDemo sim(args);
    sim.Reinit();
    sim.Run();
    break;
  }
  }
  return 0;
}
