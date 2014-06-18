#include "main.h"
#include "Logger.h"
#include "Element_QBar.h"
#include "Element_SBar.h"
#include "Element_DBar.h"
#include "Element_Mover.h"
#include "Element_Wall.h"
#include "LocalConfig.h"
#include <dlfcn.h>          /* For dlopen etc */

extern "C" typedef void * (*MFM_Element_Plugin_Get_Static_Pointer)();

namespace MFM {

  typedef GridConfig<OurCoreConfig,3,2> OurSmallGridConfig;
  typedef GridConfig<OurCoreConfig,4,3> OurBigGridConfig;
  //  typedef GridConfig<OurCoreConfig,5,3> OurBigGridConfig;
  typedef OurBigGridConfig OurGridConfig;

  typedef StatsRenderer<OurGridConfig> OurStatsRenderer;

  struct MFMSimQBDemo : public AbstractGUIDriver<OurGridConfig>
  {
  private: typedef AbstractGUIDriver<OurGridConfig> Super;

  public:
    int m_whichSim;

    MFMSimQBDemo(int whichSim)
      : m_whichSim(whichSim)
    {
    }

    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();
      bool addMover = m_whichSim==1;

      mainGrid.Needed(Element_Empty<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Wall<OurCoreConfig>::THE_INSTANCE);
      if (!m_qbarInstance)
        FAIL(ILLEGAL_STATE);
      mainGrid.Needed(*m_qbarInstance);
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
      srend.DisplayStatsForElement(mainGrid,Element_Wall<OurCoreConfig>::THE_INSTANCE);
      if (!m_qbarInstance)
        FAIL(ILLEGAL_STATE);
      srend.DisplayStatsForElement(mainGrid,*m_qbarInstance);
      if (addMover)
        srend.DisplayStatsForElement(mainGrid,Element_Mover<OurCoreConfig>::THE_INSTANCE);

      AbstractGUIDriver::RegisterToolboxElement(&Element_Empty<OurCoreConfig>::THE_INSTANCE);
      AbstractGUIDriver::RegisterToolboxElement(&Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      AbstractGUIDriver::RegisterToolboxElement(&Element_Res<OurCoreConfig>::THE_INSTANCE);
      AbstractGUIDriver::RegisterToolboxElement(&Element_SBar<OurCoreConfig>::THE_INSTANCE);
      AbstractGUIDriver::RegisterToolboxElement(&Element_Wall<OurCoreConfig>::THE_INSTANCE);
      AbstractGUIDriver::RegisterToolboxElement(m_qbarInstance);

      const SPoint QBAR_SIZE(27,3*27);
      SPoint center = QBAR_SIZE/4;
      // Ensure we start with even-even
      if (center.GetX()&1) center += SPoint(1,0);
      if (center.GetY()&1) center += SPoint(0,1);
      if (!m_qbarInstance)
        FAIL(ILLEGAL_STATE);
      //      OurAtom aBoid1(((Element_QBar<OurCoreConfig>*) m_qbarInstance)->GetAtom(QBAR_SIZE,center));
      OurAtom aBoid1(m_qbarInstance->GetDefaultAtom());
      //      ((Element_QBar<OurCoreConfig>*) m_qbarInstance)->SetSymI(aBoid1, 0);

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
          if (tloc.GetMaximumLength() < 12 && tloc.GetMaximumLength() > 4)
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
    }

    Element<OurCoreConfig> * m_qbarInstance;

    void LoadPlugin() {
      const char * path = "./bin/Element_MQBar-Plugin.so";

      // Load lib
      LOG.Debug("Calling dlopen");
      void* dllib = dlopen(path, RTLD_LAZY);
      if (!dllib) {
        LOG.Error("Cannot load library: %s", dlerror());
        FAIL(IO_ERROR);
      }
      LOG.Debug("Back from dlopen");

      // get accessor
      typedef void* (*FuncPtr)();
      typedef FuncPtr FuncPtrArray[1];

      LOG.Debug("Calling dlsym");
      FuncPtrArray * symptr = (FuncPtrArray*) dlsym(dllib, "get_static_element_pointer");
      LOG.Debug("Back from dlsym");

      const char* dlsym_error = dlerror();
      if (dlsym_error) {
        LOG.Error("Cannot find accessor in library: %s", dlsym_error);
        FAIL(IO_ERROR);
      }

       // get ptr
      FuncPtrArray & ary = * (FuncPtrArray*) symptr;
      FuncPtr fp = ary[0];
      void * result = fp();
      m_qbarInstance = (Element<OurCoreConfig> *) result;
      if (!m_qbarInstance) {
        LOG.Error("Accessor failed");
        FAIL(IO_ERROR);
      }
    }
  };

  struct MFMSimSBarDemo : public AbstractGUIDriver<OurGridConfig>
  {
  private: typedef AbstractGUIDriver<OurGridConfig> Super;

  public:

    typedef StatsRenderer<OurGridConfig> OurStatsRenderer;

    MFMSimSBarDemo()
    { }

    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
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
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.ALL);

  int whichSim = 0;
  if (argc > 0)
    whichSim = atoi(argv[0]);

  switch (whichSim) {
  default:
  case 0: {
    MFM::MFMSimQBDemo sim(whichSim);
    sim.LoadPlugin();
    sim.Init(argc, argv);
    sim.Reinit();
    sim.Run();
    break;
  }
  case 2: {
    MFM::MFMSimSBarDemo sim;
    sim.Init(argc, argv);
    sim.Reinit();
    sim.Run();
    break;
  }
  }
  return 0;
}
