#include "main.h"
#include "ParamConfig.h"

namespace MFM {

  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P1Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig,3,2> OurGridConfig;
  typedef StatsRenderer<OurGridConfig> OurStatsRenderer;
  struct MFMSimDHSDemo : public AbstractDriver<OurGridConfig>
  {
    MFMSimDHSDemo(DriverArguments& args) : AbstractDriver(args) { }

    virtual void ReinitPhysics() {
      OurGrid & mainGrid = GetGrid();

      mainGrid.Needed(Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Res<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Sorter<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Emitter<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Consumer<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Data<OurCoreConfig>::THE_INSTANCE);
      mainGrid.Needed(Element_Wall<OurCoreConfig>::THE_INSTANCE);
    }

    StatsRenderer<OurGridConfig>::ElementDataSlotSum m_sortingSlots[4];

    void ReinitEden()
    {
      OurGrid & mainGrid = GetGrid();
      OurStatsRenderer & srend = GetStatsRenderer();

      OurAtom atom(Element_Dreg<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom sorter(Element_Sorter<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom emtr(Element_Emitter<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());
      OurAtom cnsr(Element_Consumer<OurCoreConfig>::THE_INSTANCE.GetDefaultAtom());

      srend.DisplayStatsForElement(mainGrid, Element_Empty<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Dreg<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Res<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Sorter<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Emitter<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Consumer<OurCoreConfig>::THE_INSTANCE);
      srend.DisplayStatsForElement(mainGrid, Element_Data<OurCoreConfig>::THE_INSTANCE);

      srend.AddButton(BUTTONFUNC_RANDOM_NUKE, "Nuke");
      srend.AddButton(BUTTONFUNC_XRAY, "XRay");
      srend.AddButton(BUTTONFUNC_EMPTY_TILE, "Clear Tile");
      srend.AddButton(BUTTONFUNC_TOGGLE_EXECUTION, "Pause Tile");
      srend.AddButton(BUTTONFUNC_TOGGLE_HEATMAP, "Toggle Heatmap");
      srend.AddButton(BUTTONFUNC_TOGGLE_GRID, "Toggle Grid");
      srend.AddButton(BUTTONFUNC_TOGGLE_TILEVIEW, "Toggle Tile View");

      m_sortingSlots[0].Set(mainGrid, "Data in",
                            Element_Emitter<OurCoreConfig>::TYPE,
                            Element_Emitter<OurCoreConfig>::DATUMS_EMITTED_SLOT,
                            Element_Emitter<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[1].Set(mainGrid, "Overflow",
                            Element_Emitter<OurCoreConfig>::TYPE,
                            Element_Emitter<OurCoreConfig>::DATUMS_REJECTED_SLOT,
                            Element_Emitter<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);

      m_sortingSlots[2].Set(mainGrid, "Data out",
                            Element_Consumer<OurCoreConfig>::TYPE,
                            Element_Consumer<OurCoreConfig>::DATUMS_CONSUMED_SLOT,
                            Element_Consumer<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);
      m_sortingSlots[3].Set(mainGrid, "Sort error",
                            Element_Consumer<OurCoreConfig>::TYPE,
                            Element_Consumer<OurCoreConfig>::TOTAL_BUCKET_ERROR_SLOT,
                            Element_Consumer<OurCoreConfig>::DATA_SLOT_COUNT,
                            true);

      for (u32 i = 0; i < 4; ++i)
        srend.DisplayCapturableStats(&m_sortingSlots[i]);

      sorter.SetStateField(0,32,DATA_MINVAL + ((DATA_MAXVAL - DATA_MINVAL) / 2));  // Default threshold

      u32 realWidth = P::TILE_WIDTH - P::EVENT_WINDOW_RADIUS * 2;

      SPoint aloc(20, 30);
      SPoint sloc(20, 10);
      SPoint eloc(GRID_WIDTH*realWidth-2, GRID_HEIGHT*realWidth/2);
      SPoint cloc(0, GRID_HEIGHT*realWidth/2);

      for(u32 x = 0; x < mainGrid.GetWidth(); x++)
        {
          for(u32 y = 0; y < mainGrid.GetHeight(); y++)
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
    }

  };
}

int main(int argc, char** argv)
{
  MFM::DriverArguments args(argc,argv);

  MFM::MFMSimDHSDemo sim(args);

  sim.Reinit();

  sim.Run();

  return 0;
}

