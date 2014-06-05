#include "main.h"
#include "P3Atom.h"
#include "Element_SBar.h"
#include "Element_DBar.h"
#include "Element_QBar.h"
#include "Element_Mover.h"

namespace MFM {

  //  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef ParamConfig<96,4,8,40> OurParamConfig;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;

  typedef GridConfig<OurCoreConfig,3,2> OurSmallGridConfig;
  typedef GridConfig<OurCoreConfig,8,5> OurBigGridConfig;
  //  typedef GridConfig<OurCoreConfig,5,3> OurBigGridConfig;
  typedef OurBigGridConfig OurGridConfig;

  typedef StatsRenderer<OurGridConfig> OurStatsRenderer;

  struct MFMSimQBarDemo : public AbstractGUIDriver<OurGridConfig>
  {
    int m_whichSim;

    MFMSimQBarDemo(u32 argc, const char** argv, int whichSim)
      : AbstractGUIDriver(argc, argv), m_whichSim(whichSim)
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

    virtual void HandleResize()
    {

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

      const SPoint QBAR_SIZE(27,3*27);
      SPoint center = QBAR_SIZE/4;
      // Ensure we start with even-even
      if (center.GetX()&1) center += SPoint(1,0);
      if (center.GetY()&1) center += SPoint(0,1);
      OurAtom aBoid1(Element_QBar<OurCoreConfig>::THE_INSTANCE.GetAtom(QBAR_SIZE,center));
      Element_QBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid1, 0);

      OurAtom aDReg(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<OurCoreConfig>::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurCoreConfig>));

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint e1loc(20+2,20+2);
      SPoint e2loc(GRID_WIDTH*realWidth-2-20, GRID_HEIGHT*realWidth-2-20);
      SPoint cloc(GRID_WIDTH*realWidth, GRID_HEIGHT*realWidth/2);
      SPoint seedAtomPlace(3*GRID_WIDTH*realWidth/4,QBAR_SIZE.GetY()/4*3/2+15);

      u32 wid = mainGrid.GetWidth()*realWidth;
      u32 hei = mainGrid.GetHeight()*realWidth;
      bool once = true;
      for(u32 x = 0; x < wid; x+=3) {
        for(u32 y = 0; y < hei; y+=3) {
          aloc.Set(x,y);
          SPoint tloc(aloc);
          tloc.Subtract(seedAtomPlace);
          if (tloc.GetMaximumLength() < 12)
            if (Element_Empty<OurCoreConfig>::IsType(mainGrid.GetAtom(aloc)->GetType()))
              mainGrid.PlaceAtom(aDReg, aloc);

          if (once) {
            mainGrid.PlaceAtom(aBoid1, seedAtomPlace);
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

  struct MFMSimSBarDemo : public AbstractGUIDriver<OurGridConfig>
  {
    typedef StatsRenderer<OurGridConfig> OurStatsRenderer;

    MFMSimSBarDemo(u32 argc, const char** argv)
      : AbstractGUIDriver(argc, argv)
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

    virtual void HandleResize()
    {

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

      const SPoint SD_BAR_SIZE(21,55);
      const SPoint center = SD_BAR_SIZE/4;
      OurAtom aBoid1(Element_SBar<OurCoreConfig>::THE_INSTANCE.GetAtom(SD_BAR_SIZE,center));
      Element_SBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid1, 3);

      OurAtom aBoid2(Element_DBar<OurCoreConfig>::THE_INSTANCE.GetAtom(SD_BAR_SIZE,center));
      Element_DBar<OurCoreConfig>::THE_INSTANCE.SetSymI(aBoid2, PSYM_DEG000L);

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

          if (once) {
          mainGrid.PlaceAtom(aDReg, aloc);
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

int main(int argc, const char** argv)
{
  int whichSim = 0;
  if (argc > 0)
    whichSim = atoi(argv[0]);

  switch (whichSim) {
  default:
  case 0: {
    MFM::MFMSimQBarDemo sim((MFM::u32)argc, argv, whichSim);
    sim.Reinit();
    sim.Run();
    break;
  }
  case 2: {
    MFM::MFMSimSBarDemo sim((MFM::u32)argc, argv);
    sim.Reinit();
    sim.Run();
    break;
  }
  }
  return 0;
}
