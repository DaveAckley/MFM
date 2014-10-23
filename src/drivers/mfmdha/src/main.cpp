#include "main.h"
#include "Logger.h"
#include "Element_QBar.h"
#include "Element_SBar.h"
#include "Element_DBar.h"
#include "Element_Mover.h"
#include "LocalConfig.h"
#include "StdElements.h"
#include <dlfcn.h>          /* For dlopen etc */

extern "C" typedef void * (*MFM_Element_Plugin_Get_Static_Pointer)();

namespace MFM {

  typedef GridConfig<OurCoreConfig,3,2> OurSmallGridConfig;
  typedef GridConfig<OurCoreConfig,5,3> OurBigGridConfig;
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
      StdElements<OurCoreConfig> se;
      LOG.Debug("Standard elements %d", se.GetStdElementCount());
    }

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
      NeedElement(&Element_Empty<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Wall<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Data<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Emitter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Consumer<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Sorter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Block<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb1<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb2<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb3<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_AntiForkBomb<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Collector<OurCoreConfig>::THE_INSTANCE);
      if (!m_qbarInstance)
      {
        FAIL(ILLEGAL_STATE);
      }
      NeedElement(m_qbarInstance);
      NeedElement(&Element_Mover<OurCoreConfig>::THE_INSTANCE);
    }

    virtual void HandleResize()
    {

    }

    StatsRenderer<OurGridConfig>::ElementDataSlotSum m_sortingSlots[4];

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();
      OurStatsRenderer & srend = GetStatsRenderer();

      m_sortingSlots[0].Set(mainGrid, "Data in",
                            Element_Emitter<OurCoreConfig>::THE_INSTANCE.GetType(),
                            Element_Emitter<OurCoreConfig>::DATUMS_EMITTED_SLOT,
                            Element_Emitter<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[1].Set(mainGrid, "Overflow",
                            Element_Emitter<OurCoreConfig>::THE_INSTANCE.GetType(),
                            Element_Emitter<OurCoreConfig>::DATUMS_REJECTED_SLOT,
                            Element_Emitter<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);

      m_sortingSlots[2].Set(mainGrid, "Data out",
                            Element_Consumer<OurCoreConfig>::THE_INSTANCE.GetType(),
                            Element_Consumer<OurCoreConfig>::DATUMS_CONSUMED_SLOT,
                            Element_Consumer<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[3].Set(mainGrid, "Sort error",
                            Element_Consumer<OurCoreConfig>::THE_INSTANCE.GetType(),
                            Element_Consumer<OurCoreConfig>::TOTAL_BUCKET_ERROR_SLOT,
                            Element_Consumer<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);

      for (u32 i = 0; i < 4; ++i)
        srend.DisplayDataReporter(&m_sortingSlots[i]);

#if 0 /* Start with empty world! */
      OurGrid & mainGrid = GetGrid();
      bool addMover = m_whichSim==1;

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
      for(u32 x = 0; x < wid; x+=3)
      {
        for(u32 y = 0; y < hei; y+=3)
        {
          aloc.Set(x,y);
          SPoint tloc(aloc);
          tloc.Subtract(seedAtomPlace);
          if (tloc.GetMaximumLength() < 12 && tloc.GetMaximumLength() > 4)
          {
            if (Element_Empty<OurCoreConfig>::THE_INSTANCE.IsType(
                  mainGrid.GetAtom(aloc)->GetType()))
            {
              mainGrid.PlaceAtom(aDReg, aloc);
            }
          }

          if (once)
          {
            mainGrid.PlaceAtom(aBoid1, seedAtomPlace);
            if (addMover)
            {
              T mover = Element_Mover<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom();
              mainGrid.PlaceAtom(mover, e2loc);
            }
            once = false;
          }
        }
      }
#endif
    }

    Element<OurCoreConfig> * m_qbarInstance;

    void LoadPlugin()
    {
      const char * path = "./bin/Element_MQBar-Plugin.so";

      // Load lib
      LOG.Debug("Calling dlopen");
      void* dllib = dlopen(path, RTLD_LAZY);
      if (!dllib)
      {
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
      if (dlsym_error)
      {
        LOG.Error("Cannot find accessor in library: %s", dlsym_error);
        FAIL(IO_ERROR);
      }

       // get ptr
      FuncPtrArray & ary = * (FuncPtrArray*) symptr;
      FuncPtr fp = ary[0];
      void * result = fp();
      m_qbarInstance = (Element<OurCoreConfig> *) result;
      if (!m_qbarInstance)
      {
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

    virtual void DefineNeededElements()
    {
      NeedElement(&Element_Empty<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_SBar<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_DBar<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Mover<OurCoreConfig>::THE_INSTANCE);
    }

    virtual void HandleResize()
    {

    }

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();

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
      for(u32 x = 0; x < wid; x+=5)
      {
        for(u32 y = 0; y < hei; y+=5)
        {
          aloc.Set(x,y);

          if (once)
          {
            mainGrid.PlaceAtom(aDReg, aloc);
            mainGrid.PlaceAtom(aBoid1, cloc);

            once = false;
          }
        }
      }

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
