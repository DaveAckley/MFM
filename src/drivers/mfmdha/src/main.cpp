#include "main.h"
#include "Logger.h"
#include "Element_QBar.h"
#include "Element_SBar.h"
#include "Element_DBar.h"
#include "Element_Mover.h"
#include "StdEventConfig.h"
#include "StdElements.h"
#include <dlfcn.h>          /* For dlopen etc */

extern "C" typedef void * (*MFM_Element_Plugin_Get_Static_Pointer)();

namespace MFM {

  typedef StdAtom OurAtom;
  typedef StdEventConfig OurEventConfig;
  typedef GridConfig<OurEventConfig,50,3,2> OurSmallGridConfig;
  typedef GridConfig<OurEventConfig,50,5,3> OurBigGridConfig;
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
      StdElements<OurEventConfig> se;
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
      NeedElement(&Element_Empty<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Dreg<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Wall<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Data<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Emitter<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Consumer<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Sorter<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Block<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb1<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb2<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_ForkBomb3<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_AntiForkBomb<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Collector<OurEventConfig>::THE_INSTANCE);
#if 0
      if (!m_qbarInstance)
      {
        FAIL(ILLEGAL_STATE);
      }
      NeedElement(m_qbarInstance);
#endif
      NeedElement(&Element_Mover<OurEventConfig>::THE_INSTANCE);
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
                            Element_Emitter<OurEventConfig>::THE_INSTANCE.GetType(),
                            Element_Emitter<OurEventConfig>::DATUMS_EMITTED_SLOT,
                            Element_Emitter<OurEventConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[1].Set(mainGrid, "Overflow",
                            Element_Emitter<OurEventConfig>::THE_INSTANCE.GetType(),
                            Element_Emitter<OurEventConfig>::DATUMS_REJECTED_SLOT,
                            Element_Emitter<OurEventConfig>::DATA_SLOT_COUNT,
                            true);

      m_sortingSlots[2].Set(mainGrid, "Data out",
                            Element_Consumer<OurEventConfig>::THE_INSTANCE.GetType(),
                            Element_Consumer<OurEventConfig>::DATUMS_CONSUMED_SLOT,
                            Element_Consumer<OurEventConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[3].Set(mainGrid, "Sort error",
                            Element_Consumer<OurEventConfig>::THE_INSTANCE.GetType(),
                            Element_Consumer<OurEventConfig>::TOTAL_BUCKET_ERROR_SLOT,
                            Element_Consumer<OurEventConfig>::DATA_SLOT_COUNT,
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
      //      OurAtom aBoid1(((Element_QBar<OurEventConfig>*) m_qbarInstance)->GetAtom(QBAR_SIZE,center));
      OurAtom aBoid1(m_qbarInstance->GetDefaultAtom());
      //      ((Element_QBar<OurEventConfig>*) m_qbarInstance)->SetSymI(aBoid1, 0);

      OurAtom aDReg(Element_Dreg<OurEventConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = Tile<OurEventConfig>::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurEventConfig>));

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
            if (Element_Empty<OurEventConfig>::THE_INSTANCE.IsType(
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
              T mover = Element_Mover<OurEventConfig>::THE_INSTANCE.GetDefaultAtom();
              mainGrid.PlaceAtom(mover, e2loc);
            }
            once = false;
          }
        }
      }
#endif
    }

#if 0
    Element<OurEventConfig> * m_qbarInstance;

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
      m_qbarInstance = (Element<OurEventConfig> *) result;
      if (!m_qbarInstance)
      {
        LOG.Error("Accessor failed");
        FAIL(IO_ERROR);
      }
    }
#endif
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
      NeedElement(&Element_Empty<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Dreg<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Res<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_SBar<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_DBar<OurEventConfig>::THE_INSTANCE);
      NeedElement(&Element_Mover<OurEventConfig>::THE_INSTANCE);
    }

    virtual void HandleResize()
    {

    }

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();

      const SPoint SD_BAR_SIZE(21,55);
      const SPoint center = SD_BAR_SIZE/4;
      OurAtom aBoid1(Element_SBar<OurEventConfig>::THE_INSTANCE.GetAtom(SD_BAR_SIZE,center));
      Element_SBar<OurEventConfig>::THE_INSTANCE.SetSymI(aBoid1, 3);

      OurAtom aBoid2(Element_DBar<OurEventConfig>::THE_INSTANCE.GetAtom(SD_BAR_SIZE,center));
      Element_DBar<OurEventConfig>::THE_INSTANCE.SetSymI(aBoid2, PSYM_DEG000L);

      OurAtom aDReg(Element_Dreg<OurEventConfig>::THE_INSTANCE.GetDefaultAtom());

      u32 realWidth = OurGrid::OWNED_SIDE;

      // printf("atom size %ld\n",8*sizeof(MFM::Atom<OurEventConfig>));

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

    sim.ProcessArguments(argc, argv);
    sim.AddInternalLogging();
    //    sim.LoadPlugin();
    sim.Init();

    sim.Run();
    break;
  }
  case 2: {
    MFM::MFMSimSBarDemo sim;

    sim.ProcessArguments(argc, argv);
    sim.AddInternalLogging();
    sim.Init();

    sim.Run();
    break;
  }
  }
  return 0;
}
