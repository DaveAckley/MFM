#include "main.h"
#include "ParamConfig.h"

namespace MFM {

  typedef ParamConfig<96,4,8,176> OurParamConfig;
  //DEPRECATED: typedef P1Atom<OurParamConfig> OurAtom;
  typedef P3Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig,2,1> OurGridConfig;
  typedef StatsRenderer<OurGridConfig> OurStatsRenderer;
  struct MFMSimDHSDemo : public AbstractGUIDriver<OurGridConfig>
  {
  private: typedef AbstractGUIDriver<OurGridConfig> Super;

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
      NeedElement(&Element_Sorter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Emitter<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Consumer<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Data<OurCoreConfig>::THE_INSTANCE);
      NeedElement(&Element_Wall<OurCoreConfig>::THE_INSTANCE);
    }

    virtual void HandleResize()
    {

    }

    StatsRenderer<OurGridConfig>::ElementDataSlotSum m_sortingSlots[4];

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();

      OurAtom atom(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom sorter(Element_Sorter<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom emtr(Element_Emitter<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom cnsr(Element_Consumer<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

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

      sorter.SetStateField(0,32,DATA_MINVAL + ((DATA_MAXVAL - DATA_MINVAL) / 2));  // Default threshold

      mainGrid.SurroundRectangleWithWall(3, 3, 161, 97, R);

      u32 realWidth = 32;

      SPoint aloc(24, 30);
      SPoint sloc(24, 10);
      SPoint eloc(162, 10);
      SPoint cloc(5, 14);

      for(u32 x = 0; x < 5; x++)
        {
          for(u32 y = 0; y < 3; y++)
            {
              for(u32 z = 0; z < 4; z++)
                {
                  aloc.Set(10 + x * realWidth + z, 10 + y * realWidth);
                  sloc.Set(11 + x * realWidth + z, 11 + y * realWidth);
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

int main(int argc, const char** argv)
{
  MFM::MFMSimDHSDemo sim;

  sim.Init(argc, argv);

  sim.Reinit();

  sim.Run();

  return 0;
}
