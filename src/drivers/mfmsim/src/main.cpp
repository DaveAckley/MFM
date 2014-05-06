#include "main.h"
#include "ParamConfig.h"

namespace MFM {

  typedef ParamConfig<64,4,8,40> OurParamConfig;
  typedef P1Atom<OurParamConfig> OurAtom;
  typedef CoreConfig<OurAtom,OurParamConfig> OurCoreConfig;
  typedef GridConfig<OurCoreConfig,5,3> OurGridConfig;
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

    class AbstractGridButton : public AbstractButton
    {
    protected:
      GridRenderer* m_grend;
      OurGrid* m_grid;

      AbstractGridButton(const char* title) :
        AbstractButton(title) { }

    public:
      void SetGridRenderer(GridRenderer* grend)
      {
	m_grend = grend;
      }

      void SetGrid(OurGrid* grid)
      {
	m_grid = grid;
      }
    };

    struct ClearButton : public AbstractGridButton
    {
      ClearButton() : AbstractGridButton("Clear Tile")
      {
	AbstractButton::SetName("ClearButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 450));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	SPoint& selTile = AbstractGridButton::m_grend->GetSelectedTile();
	if(selTile.GetX() >= 0 && selTile.GetX() < W &&
	   selTile.GetY() >= 0 && selTile.GetY() < H)
	{
	  AbstractGridButton::m_grid->
	    EmptyTile(AbstractGridButton::m_grend->GetSelectedTile());
	}
      }
    } m_clearButton;

    struct PauseButton : public AbstractGridButton
    {
      PauseButton() : AbstractGridButton("Pause Tile")
      {
	AbstractButton::SetName("PauseButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2,500));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	printf("click\n");
	SPoint& selTile = AbstractGridButton::m_grend->GetSelectedTile();
	if(selTile.GetX() >= 0 && selTile.GetX() < W &&
	   selTile.GetY() >= 0 && selTile.GetY() < H)
	{
	  AbstractGridButton::m_grid->
	    SetTileToExecuteOnly(selTile,
				 !AbstractGridButton::m_grid->
				 GetTileExecutionStatus(selTile));
	  printf("pause\n");
	}
      }
    } m_pauseButton;

    struct NukeButton : public AbstractGridButton
    {
      NukeButton() : AbstractGridButton("Nuke")
      {
	AbstractButton::SetName("NukeButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 550));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	m_grid->RandomNuke();
      }
    } m_nukeButton;

    struct XRayButton : public AbstractGridButton
    {
      XRayButton() : AbstractGridButton("XRay")
      {
	AbstractButton::SetName("XRayButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 600));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	m_grid->XRay();
      }
    } m_xrayButton;

    struct GridRenderButton : public AbstractGridButton
    {
      GridRenderButton() : AbstractGridButton("Toggle Grid")
      {
	AbstractButton::SetName("GridRenderButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 650));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	m_grend->ToggleGrid();
      }
    } m_gridRenderButton;

    struct HeatmapButton : public AbstractGridButton
    {
      HeatmapButton() : AbstractGridButton("Toggle Heatmap")
      {
	AbstractButton::SetName("HeatmapButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 700));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	m_grend->ToggleDataHeatmap();
      }
    } m_heatmapButton;

    struct TileViewButton : public AbstractGridButton
    {
      TileViewButton() : AbstractGridButton("Toggle Tile View")
      {
	AbstractButton::SetName("TileViewButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 750));
        AbstractButton::SetForeground(Drawing::WHITE);
        AbstractButton::SetBackground(Drawing::BLACK);
      }

      virtual void OnClick()
      {
	m_grend->ToggleMemDraw();
      }
    } m_tileViewButton;



    virtual void HandleResize()
    {
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

      m_clearButton.SetGrid(&mainGrid);
      m_clearButton.SetGridRenderer(&m_grend);
      m_xrayButton.SetGrid(&mainGrid);
      m_xrayButton.SetGridRenderer(&m_grend);
      m_pauseButton.SetGrid(&mainGrid);
      m_pauseButton.SetGridRenderer(&m_grend);
      m_nukeButton.SetGrid(&mainGrid);
      m_nukeButton.SetGridRenderer(&m_grend);
      m_gridRenderButton.SetGrid(&mainGrid);
      m_gridRenderButton.SetGridRenderer(&m_grend);
      m_heatmapButton.SetGrid(&mainGrid);
      m_heatmapButton.SetGridRenderer(&m_grend);
      m_tileViewButton.SetGrid(&mainGrid);
      m_tileViewButton.SetGridRenderer(&m_grend);

      m_statisticsPanel.Insert(&m_clearButton, NULL);
      m_statisticsPanel.Insert(&m_pauseButton, NULL);
      m_statisticsPanel.Insert(&m_nukeButton, NULL);
      m_statisticsPanel.Insert(&m_xrayButton, NULL);
      m_statisticsPanel.Insert(&m_gridRenderButton, NULL);
      m_statisticsPanel.Insert(&m_heatmapButton, NULL);
      m_statisticsPanel.Insert(&m_tileViewButton, NULL);
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

