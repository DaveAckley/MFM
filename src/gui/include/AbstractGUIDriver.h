/*                                              -*- mode:C++ -*-
  AbstractGUIDriver.h Base class for all GUI-based MFM drivers
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file AbstractGUIDriver.h Base class for all GUI-based MFM drivers
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ABSTRACTGUIDRIVER_H
#define ABSTRACTGUIDRIVER_H

#include <sys/stat.h>  /* for mkdir */
#include <sys/types.h> /* for mkdir */
#include <errno.h>     /* for errno */
#include "Utils.h"     /* for GetDateTimeNow */
#include "Logger.h"
#include "AssetManager.h"
#include "AbstractButton.h"
#include "AbstractCheckbox.h"
#include "Tile.h"
#include "GridRenderer.h"
#include "GridPanel.h"
#include "TextPanel.h"
#include "ToolboxPanel.h"
#include "TeeByteSink.h"
#include "StatsRenderer.h"
#include "Element_Empty.h" /* Need common elements */
#include "Element_Dreg.h"
#include "Element_Res.h"
#include "Element_Wall.h"
#include "Element_Consumer.h"
#include "ExternalConfig.h"
#include "FileByteSource.h"
#include "Keyboard.h"
#include "Camera.h"
#include "AbstractDriver.h"
#include "VArguments.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "HelpPanel.h"

namespace MFM
{
#define FRAMES_PER_SECOND 100.0

#define CAMERA_SLOW_SPEED 2
#define CAMERA_FAST_SPEED 50

#define STATS_WINDOW_WIDTH 288

#define STATS_START_WINDOW_WIDTH 280
#define STATS_START_WINDOW_HEIGHT 120

  /* super speedy for now */
#define MINIMAL_START_WINDOW_WIDTH 1
#define MINIMAL_START_WINDOW_HEIGHT 1


#define MAX_PATH_LENGTH 1000
#define MIN_PATH_RESERVED_LENGTH 100

  template<class GC>
  class AbstractGUIDriver : public AbstractDriver<GC>
  {
   private:
    typedef AbstractDriver<GC> Super;

   protected:
    typedef typename Super::OurGrid OurGrid;
    typedef typename Super::CC CC;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};

    bool m_startPaused;

    Fonts m_fonts;

    bool m_keyboardPaused;   // Toggled by keyboard space, ' ', SDLK_SPACE
    bool m_mousePaused;      // Set if any buttons down, clear if all up
    bool m_gridPaused;       // Set if keyboard || mouse paused, checked by RunGrid
    bool m_reinitRequested;
    void RequestReinit()
    {
      m_reinitRequested = true;
    }

    bool m_renderStats;
    u32 m_ticksLastStopped;

    s32 m_recordScreenshotPerAEPS;
    s32 m_maxRecordScreenshotPerAEPS;
    s32 m_countOfScreenshotsAtThisAEPS;
    s32 m_countOfScreenshotsPerRate;
    u32 m_nextEventCountsAEPS;
    u32 m_nextScreenshotAEPS;
    u32 m_nextTimeBasedDataAEPS;

    Keyboard m_keyboard;
    Camera camera;
    SDL_Surface* screen;
    Panel m_rootPanel;
    Drawing m_rootDrawing;

    u32 m_screenWidth;
    u32 m_screenHeight;


    /************************************/
    /*******ABSTRACT BUTTONS*************/
    /************************************/

    class AbstractGridButton : public AbstractButton
    {
     public:
      void SetDriver(AbstractGUIDriver & driver)
      {
        m_driver = &driver;
      }

      void SetGridRenderer(GridRenderer& grend)
      {
        m_grend = &grend;
      }

     protected:
      AbstractGUIDriver * m_driver;

      GridRenderer* m_grend;

      AbstractGridButton(const char* title) :
        AbstractButton(title), m_driver(0),
        m_grend(NULL)
      { }
    };

    class AbstractGridCheckbox : public AbstractCheckbox
    {
     public:
      void SetDriver(AbstractGUIDriver & driver)
      {
        m_driver = &driver;
      }

     protected:
      AbstractGUIDriver * m_driver;

      AbstractGridCheckbox(const char* title) :
        AbstractCheckbox(title),
        m_driver(0)
      { }
    };

    /************************************/
    /******CONCRETE BUTTONS**************/
    /************************************/


    class ClearButton : public AbstractGridButton
    {
     public:
      ClearButton() : AbstractGridButton("Clear Tile")
      {
        AbstractButton::SetName("ClearButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 250));
      }

      virtual void OnClick(u8 button)
      {
        OurGrid & grid = AbstractGridButton::m_driver->GetGrid();
        GridRenderer & grend = AbstractGridButton::m_driver->GetGridRenderer();

        const SPoint selTile = grend.GetSelectedTile();
        if(selTile.GetX() >= 0 && selTile.GetX() < W &&
           selTile.GetY() >= 0 && selTile.GetY() < H)
        {
          grid.EmptyTile(grend.GetSelectedTile());
        }
      }
    } m_clearButton;

    class NukeButton : public AbstractGridButton
    {
     public:
      NukeButton() : AbstractGridButton("Nuke")
      {
        AbstractButton::SetName("NukeButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 100));
      }

      virtual void OnClick(u8 button)
      {
        AbstractGridButton::m_driver->GetGrid().RandomNuke();
      }
    } m_nukeButton;

    struct XRayButton : public AbstractGridButton
    {
      XRayButton() : AbstractGridButton("XRay")
      {
        AbstractButton::SetName("XRayButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 150));
      }

      virtual void OnClick(u8 button)
      {
        AbstractGridButton::m_driver->GetGrid().XRay();
      }
    } m_xrayButton;

    class GridRunCheckbox : public AbstractGridCheckbox
    {
     public:
      GridRunCheckbox() : AbstractGridCheckbox("Pause")
      {
        AbstractButton::SetName("GridRunButton");
        Panel::SetDimensions(200, 25);
        AbstractButton::SetRenderPoint(SPoint(2, 0));
      }

      virtual void OnCheck(bool value)
      { }
    }m_gridRunButton;

    struct GridRenderButton : public AbstractGridCheckbox
    {
      GridRenderButton() : AbstractGridCheckbox("Grid")
      {
        AbstractButton::SetName("GridRenderButton");
        Panel::SetDimensions(200,25);
        AbstractButton::SetRenderPoint(SPoint(2, 25));
      }

      virtual void OnCheck(bool value)
      { }
    } m_gridRenderButton;

    struct HeatmapButton : public AbstractGridCheckbox
    {
      HeatmapButton() : AbstractGridCheckbox("Heatmap")
      {
        AbstractButton::SetName("HeatmapButton");
        Panel::SetDimensions(200,25);
        AbstractButton::SetRenderPoint(SPoint(2, 50));
      }

      virtual void OnCheck(bool value)
      { }
    } m_heatmapButton;

    struct TileViewButton : public AbstractGridButton
    {
      TileViewButton() : AbstractGridButton("Toggle Tile View")
      {
        AbstractButton::SetName("TileViewButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 200));
      }

      virtual void OnClick(u8 button)
      {
        AbstractGridButton::m_driver->GetGridRenderer().ToggleMemDraw();
      }
    } m_tileViewButton;

    struct SaveButton : public AbstractGridButton
    {
      SaveButton() : AbstractGridButton("Save State"),
                     m_saveStateIndex(1)
      {
        AbstractButton::SetName("SaveButton");
        Panel::SetDimensions(200, 40);
        AbstractButton::SetRenderPoint(SPoint(2, 300));
      }

      virtual void OnClick(u8 button)
      {
        const char* filename =
          AbstractGridButton::m_driver->GetSimDirPathTemporary("save/%d.mfs",
                                                               m_saveStateIndex++);

        LOG.Debug("Saving to: %s", filename);
        ExternalConfig<GC> cfg(AbstractGridButton::m_driver->GetGrid());
        FILE* fp = fopen(filename, "w");
        FileByteSink fs(fp);

        cfg.Write(fs);
        fs.Close();
      }

    private:

      u32 m_saveStateIndex;
    }m_saveButton;

    struct QuitButton : public AbstractGridButton
    {
      QuitButton() : AbstractGridButton("Quit")
      {
        AbstractButton::SetName("QuitButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 350));
      }

      virtual void OnClick(u8 button)
      {
        exit(0);
      }
    } m_quitButton;

    struct PauseTileButton : public AbstractGridButton
    {
      PauseTileButton() : AbstractGridButton("Pause Tile")
      {
        AbstractButton::SetName("PauseTileButton");
        Panel::SetDimensions(200, 40);
        AbstractButton::SetRenderPoint(SPoint(2, 400));
      }

      virtual void OnClick(u8 button)
      {
        SPoint selectedTile = AbstractGridButton::m_grend->GetSelectedTile();

        if(selectedTile.GetX() > 0 && selectedTile.GetY() > 0)
        {
          AbstractGridButton::m_driver->GetGrid().SetTileToExecuteOnly(
            selectedTile,
            !AbstractGridButton::m_driver->GetGrid().GetTileExecutionStatus(selectedTile));
        }

      }
    }m_pauseTileButton;

    struct BGRButton : public AbstractGridCheckbox
    {
      BGRButton() : AbstractGridCheckbox("XRay On Write")
      {
        AbstractButton::SetName("BGRButton");
        Panel::SetDimensions(200,40);
        AbstractButton::SetRenderPoint(SPoint(2, 75));
      }

      virtual void OnCheck(bool value)
      {
        AbstractGridCheckbox::m_driver->GetGrid().SetBackgroundRadiation(
          AbstractCheckbox::IsChecked());
      }
    } m_bgrButton;

    HelpPanel m_helpPanel;

  protected: /* Need these for our buttons at driver level */
    GridRenderer m_grend;
    StatsRenderer<GC> m_srend;

    void RegisterSliderConfig(const AbstractSliderConfig<CC>* config)
    {
      m_toolboxPanel.RegisterSliderConfig(config);
    }

    GridRenderer & GetGridRenderer()
    {
      return m_grend;
    }

    virtual void PostUpdate()
    {
      /* Update the stats renderer */
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());
      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
    }

    virtual void OnceOnly(VArguments& args)
    {
      /// Mux our screen logger into the LOGing path, before calling parent!
      {

        ByteSink * old = LOG.SetByteSink(m_logSplitter);
        m_logSplitter.SetSink1(old);
        m_logSplitter.SetSink2(&m_logPanel.GetByteSink());
      }

      // Let the parent 'go first'!
      Super::OnceOnly(args);

      if (m_countOfScreenshotsPerRate > 0) {
        m_maxRecordScreenshotPerAEPS = m_recordScreenshotPerAEPS;
        m_recordScreenshotPerAEPS = 1;
        m_countOfScreenshotsAtThisAEPS = 0;
      }

      if (!getenv("SDL_VIDEO_ALLOW_SCREENSAVER"))          // If user isn't already messing with this
        putenv((char *) "SDL_VIDEO_ALLOW_SCREENSAVER=1");  // Old school sdl 1.2 mechanism

      SDL_Init(SDL_INIT_VIDEO);

      SetScreenSize(m_screenWidth, m_screenHeight);

      m_fonts.Init();

      m_rootPanel.SetName("Root");
      m_gridPanel.SetBorder(Drawing::BLACK);
      m_gridPanel.SetGridRenderer(&m_grend);
      m_gridPanel.SetToolboxPanel(&m_toolboxPanel);
      m_gridPanel.SetGrid(&Super::GetGrid());

      m_statisticsPanel.SetStatsRenderer(&m_srend);
      m_statisticsPanel.SetGrid(&Super::GetGrid());
      m_statisticsPanel.SetAEPS(Super::GetAEPS());
      m_statisticsPanel.SetAER(Super::GetAER());
      m_statisticsPanel.SetAEPSPerFrame(Super::GetAEPSPerFrame());
      m_statisticsPanel.SetOverheadPercent(Super::GetOverheadPercent());
      m_statisticsPanel.SetVisibility(false);

      m_rootPanel.Insert(&m_gridPanel, NULL);
      m_gridPanel.Insert(&m_statisticsPanel, NULL);
      m_statisticsPanel.Insert(&m_buttonPanel, NULL);
      m_buttonPanel.SetVisibility(true);

      m_gridPanel.Insert(&m_logPanel, NULL);
      m_logPanel.SetVisibility(false);
      m_logPanel.SetDimensions(m_screenWidth, 160);
      m_logPanel.SetAnchor(ANCHOR_SOUTH);
      m_logPanel.SetFont(m_fonts.GetDefaultFont(16));

      m_toolboxPanel.SetName("Toolbox");
      m_toolboxPanel.SetVisibility(false);
      m_toolboxPanel.SetBackground(Drawing::GREY60);
      m_toolboxPanel.SetAnchor(ANCHOR_WEST);
      m_gridPanel.Insert(&m_toolboxPanel, NULL);

      m_helpPanel.SetName("Help");
      m_helpPanel.SetVisibility(false);
      m_helpPanel.SetDimensions(m_screenWidth / 3, m_screenHeight);
      m_helpPanel.SetAnchor(ANCHOR_WEST);
      m_gridPanel.Insert(&m_helpPanel, NULL);

      m_rootPanel.Print(STDOUT);

      m_srend.OnceOnly(m_fonts);

      SDL_WM_SetCaption(MFM_VERSION_STRING_LONG, NULL);

      m_ticksLastStopped = 0;

      m_reinitRequested = false;

      OnceOnlyButtons();

      // Again to 'set' stuff?
      SetScreenSize(m_screenWidth, m_screenHeight);
    }

  private:

    void OnceOnlyButtons()
    {
      m_statisticsPanel.SetAnchor(ANCHOR_EAST);

      m_buttonPanel.InsertCheckbox(&m_heatmapButton);
      m_buttonPanel.InsertCheckbox(&m_gridRenderButton);
      m_buttonPanel.InsertCheckbox(&m_gridRunButton);

      m_buttonPanel.InsertCheckbox(&m_bgrButton);
      m_buttonPanel.InsertButton(&m_clearButton);
      m_buttonPanel.InsertButton(&m_xrayButton);
      m_buttonPanel.InsertButton(&m_nukeButton);
      m_buttonPanel.InsertButton(&m_tileViewButton);
      m_buttonPanel.InsertButton(&m_saveButton);
      m_buttonPanel.InsertButton(&m_quitButton);
      m_buttonPanel.InsertButton(&m_pauseTileButton);

      m_pauseTileButton.SetGridRenderer(m_grend);

      m_gridRenderButton.SetExternalValue(m_grend.GetGridEnabledPointer());
      m_heatmapButton.SetExternalValue(m_grend.GetDrawDataHeatPointer());
      m_bgrButton.SetExternalValue(AbstractDriver<GC>::GetGrid().
                                   GetBackgroundRadiationEnabledPointer());
      m_gridRunButton.SetExternalValue(&m_keyboardPaused);

      m_buttonPanel.SetButtonDrivers(*this);
      m_buttonPanel.InsertButtons();

      m_buttonPanel.SetAnchor(ANCHOR_SOUTH);
    }

    void Update(OurGrid& grid)
    {
      KeyboardUpdate(grid);
      m_gridPaused = m_keyboardPaused || m_mousePaused;
      if (!m_gridPaused)
      {
        Super::RunGrid(grid);
      }
    }

    inline void ToggleStatsView()
    {
      m_statisticsPanel.ToggleVisibility();
      m_grend.SetDimensions(Point<u32>(m_screenWidth - (m_renderStats ? STATS_WINDOW_WIDTH : 0)
                                       , m_screenHeight));
    }

    inline void ToggleLogView()
    {
      m_logPanel.ToggleVisibility();
    }

    inline void ToggleToolbox()
    {
      m_toolboxPanel.ToggleVisibility();
      m_gridPanel.SetPaintingEnabled(m_toolboxPanel.IsVisible());
    }

    void KeyboardUpdate(OurGrid& grid)
    {
      u8 speed = m_keyboard.ShiftHeld() ?
        CAMERA_FAST_SPEED : CAMERA_SLOW_SPEED;

      if(m_keyboard.IsDown(SDLK_q) && (m_keyboard.IsDown(SDLK_LCTRL) || m_keyboard.IsDown(SDLK_RCTRL)))
      {
        exit(0);
      }

      /* View Control */
      if(m_keyboard.SemiAuto(SDLK_a))
      {
        m_srend.SetDisplayAER(!m_srend.GetDisplayAER());
      }
      if(m_keyboard.SemiAuto(SDLK_i))
      {
        ToggleStatsView();
      }
      if(m_keyboard.SemiAuto(SDLK_g))
      {
        m_grend.ToggleGrid();
      }
      if(m_keyboard.SemiAuto(SDLK_m))
      {
        m_grend.ToggleMemDraw();
      }
      if(m_keyboard.SemiAuto(SDLK_k))
      {
        m_grend.ToggleDataHeatmap();
      }
      if(m_keyboard.SemiAuto(SDLK_h))
      {
        m_helpPanel.ToggleVisibility();
      }
      if(m_keyboard.SemiAuto(SDLK_l))
      {
        ToggleLogView();
      }
      if(m_keyboard.SemiAuto(SDLK_p))
      {
        m_grend.ToggleTileSeparation();
      }

      if(m_keyboard.SemiAuto(SDLK_ESCAPE))
      {
        m_grend.DeselectTile();
      }

      /* Camera Recording */
      if(m_keyboard.SemiAuto(SDLK_r))
      {
        camera.ToggleRecord();
      }

      if(m_keyboard.SemiAuto(SDLK_t))
      {
        ToggleToolbox();
      }

      /* Camera Movement*/
      if(m_keyboard.IsDown(SDLK_LEFT))
      {
        m_grend.MoveLeft(speed);
      }
      if(m_keyboard.IsDown(SDLK_DOWN))
      {
        m_grend.MoveDown(speed);
      }
      if(m_keyboard.IsDown(SDLK_UP))
      {
        m_grend.MoveUp(speed);
      }
      if(m_keyboard.IsDown(SDLK_RIGHT))
      {
        m_grend.MoveRight(speed);
      }

      /* Speed Control */
      if(m_keyboard.SemiAuto(SDLK_SPACE))
      {
        m_keyboardPaused = !m_keyboardPaused;
      }
      if(m_keyboard.IsDown(SDLK_COMMA))
      {
        Super::DecrementAEPSPerFrame();
      }
      if(m_keyboard.IsDown(SDLK_PERIOD))
      {
        Super::IncrementAEPSPerFrame();
      }

      m_keyboard.Flip();
    }


#if 0
    void ExportTimeBasedData(OurGrid& grid)
    {
      /* Current header : */
      /* # AEPS activesites empty dreg res wall sort-hits sort-misses sort-total sort-hit-pctg sort-bucket-miss-average*/

      if(m_recordTimeBasedDataPerAEPS > 0)
      {
        if(m_AEPS > m_nextTimeBasedDataAEPS)
        {
          const char* path = GetSimDirPathTemporary("tbd/tbd.txt", m_nextEventCountsAEPS);
          FILE* fp = fopen(path, "a");

          u64 consumed = 0, totalError = 0;
          for (OurGrid::iterator_type i = grid.begin(); i != grid.end(); ++i) {
            Tile<CC> * t = *i;
t            consumed += Element_Consumer<CC>::THE_INSTANCE.GetAndResetDatumsConsumed(*t);
            totalError += Element_Consumer<CC>::THE_INSTANCE.GetAndResetBucketError(*t);
          }

          fprintf(fp, "%g %d %d %d %d %d %ld %ld\n",
                  m_AEPS,
                  grid.CountActiveSites(),
                  grid.GetAtomCount(Element_Empty<CC>::TYPE),
                  grid.GetAtomCount(Element_Dreg<CC>::TYPE),
                  grid.GetAtomCount(Element_Res<CC>::TYPE),
                  grid.GetAtomCount(Element_Wall<CC>::TYPE),
                  consumed, totalError);

          fclose(fp);
          m_nextTimeBasedDataAEPS += m_recordTimeBasedDataPerAEPS;
        }
      }
    }
#endif

  public:

    AbstractGUIDriver() :
      m_startPaused(true),
      m_renderStats(false),
      m_screenWidth(SCREEN_INITIAL_WIDTH),
      m_screenHeight(SCREEN_INITIAL_HEIGHT),
      m_selectedTool(TOOL_SELECTOR),
      m_toolboxPanel(&m_selectedTool),
      m_buttonPanel(m_fonts)
    { }

    ~AbstractGUIDriver()
    {
      AssetManager::Destroy();
    }

    virtual void ReinitUs()
    {
      m_nextEventCountsAEPS = 0;
      m_nextScreenshotAEPS = 0;
      m_nextTimeBasedDataAEPS = 0;
    }

    virtual void PostReinit(VArguments& args)
    {
      HandleResize();

      m_renderStats = false;

      m_toolboxPanel.AddButtons();
    }

    virtual void PostReinitPhysics()
    {
      for(u32 i = 0; i < AbstractDriver<GC>::m_neededElementCount; i++)
      {
        GetStatsRenderer().DisplayStatsForElement(AbstractDriver<GC>::GetGrid(),
                                                  *(AbstractDriver<GC>::m_neededElements[i]));
        this->RegisterToolboxElement(AbstractDriver<GC>::m_neededElements[i]);
      }
    }

    virtual void HandleResize()
    { }

    void ToggleTileView()
    {
      m_grend.ToggleMemDraw();
    }

    void RegisterToolboxElement(Element<CC>* element)
    {
      m_toolboxPanel.RegisterElement(element);
    }

    /*********************************
     * GUI SPECIFIC DRIVER ARGUMENTS *
     *********************************/

    static void ConfigStatsOnlyView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;


      driver->m_screenWidth = STATS_START_WINDOW_WIDTH;
      driver->m_screenHeight = STATS_START_WINDOW_HEIGHT;
      driver->ToggleStatsView();
      driver->m_srend.SetDisplayAER(!driver->m_srend.GetDisplayAER());
    }

    static void ConfigMinimalView(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_screenWidth = MINIMAL_START_WINDOW_HEIGHT;
      driver->m_screenHeight = MINIMAL_START_WINDOW_WIDTH;
    }

    static void SetRecordScreenshotPerAEPSFromArgs(const char* aeps, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_recordScreenshotPerAEPS = atoi(aeps);
    }

    static void SetStartPausedFromArgs(const char* not_used, void* driverptr)
    {
      AbstractGUIDriver& driver = *((AbstractGUIDriver*)driverptr);

      driver.m_startPaused = false;
    }

    static void SetPicturesPerRateFromArgs(const char* aeps, void* driverptr)
    {
      AbstractGUIDriver* driver = (AbstractGUIDriver<GC>*)driverptr;

      driver->m_countOfScreenshotsPerRate = atoi(aeps);
    }

    void AddDriverArguments()
    {
      Super::AddDriverArguments();

      this->RegisterSection("Display-specific switches");

      this->RegisterArgument("Start with only the statistics view on the screen.",
                             "--startwithoutgrid", &ConfigStatsOnlyView, this, false);

      this->RegisterArgument("Start with a minimal-sized window.",
                             "--startminimal", &ConfigMinimalView, this, false);

      this->RegisterArgument("Record screenshots every ARG aeps",
                             "-p|--pictures", &SetRecordScreenshotPerAEPSFromArgs, this, true);

      this->RegisterArgument("Take ARG shots per speed from 1 up to -p value",
                             "--picturesPerRate",
                             &SetPicturesPerRateFromArgs, this, true);

      this->RegisterArgument("Simulation begins upon program startup.",
                             "--run", &SetStartPausedFromArgs, this, false);
    }

    EditingTool m_selectedTool;

    GridPanel<GC> m_gridPanel;

    ToolboxPanel<CC> m_toolboxPanel;

    class StatisticsPanel : public Panel
    {
      StatsRenderer<GC>* m_srend;
      OurGrid* m_mainGrid;
      double m_AEPS;
      double m_AER;
      double m_overheadPercent;
      u32 m_aepsPerFrame;

    public:
      StatisticsPanel() : m_srend(NULL)
      {
        SetName("Statistics Panel");
        SetDimensions(STATS_START_WINDOW_WIDTH,
                      SCREEN_INITIAL_HEIGHT);
        SetRenderPoint(SPoint(100000, 0));
        SetForeground(Drawing::WHITE);
        SetBackground(Drawing::DARK_PURPLE);
        m_AEPS = m_AER = 0.0;
        m_aepsPerFrame = 0;
      }

      void SetStatsRenderer(StatsRenderer<GC>* srend)
      {
        m_srend = srend;
      }

      void SetGrid(OurGrid* mainGrid)
      {
        m_mainGrid = mainGrid;
      }

      void SetAEPS(double aeps)
      {
        m_AEPS = aeps;
      }

      void SetAER(double aer)
      {
        m_AER = aer;
      }

      void SetOverheadPercent(double overheadPercent)
      {
        m_overheadPercent = overheadPercent;
      }

      void SetAEPSPerFrame(u32 apf)
      {
        m_aepsPerFrame = apf;
      }

    protected:
      virtual void PaintComponent(Drawing& drawing)
      {
        this->Panel::PaintComponent(drawing);
        m_srend->RenderGridStatistics(drawing, *m_mainGrid,
                                      m_AEPS, m_AER, m_aepsPerFrame,
                                      m_overheadPercent, false);
      }

      virtual void PaintBorder(Drawing & config)
      { /* No border please */ }

      virtual bool Handle(MouseButtonEvent& mbe)
      {
        return true;  /* Eat the event to keep the grid from taking it */
      }
    }m_statisticsPanel;

    struct ButtonPanel : public Panel
    {
      static const u32 MAX_BUTTONS = 16;
      static const u32 CHECKBOX_SPACING_HEIGHT = 30;
      static const u32 BUTTON_SPACING_HEIGHT = 40;
      static const u32 BUTTON_HEIGHT = 30;
      static const u32 BUTTON_WIDTH = 200;

      virtual void PaintBorder(Drawing & config)
      { /* No border please */ }

      ButtonPanel(Fonts & fonts) :
        m_checkboxCount(0),
        m_buttonCount(0)
      {
        SetName("ButtonPanel");
        SetDimensions(STATS_START_WINDOW_WIDTH,
                      SCREEN_INITIAL_HEIGHT / 2);
        SetRenderPoint(SPoint(0,
                              SCREEN_INITIAL_HEIGHT / 2));
        SetForeground(Drawing::WHITE);
        SetBackground(Drawing::DARK_PURPLE);
        SetFont(fonts.GetDefaultFont(20));
      }

      void InsertCheckbox(AbstractGridCheckbox* checkbox)
      {
        if(m_checkboxCount >= MAX_BUTTONS)
        {
          FAIL(OUT_OF_ROOM);
        }

        m_checkboxes[m_checkboxCount++] = checkbox;

        Pack();
      }

      void InsertButton(AbstractGridButton* button)
      {
        if(m_buttonCount >= MAX_BUTTONS)
        {
          FAIL(OUT_OF_ROOM);
        }

        m_buttons[m_buttonCount++] = button;

        Pack();
      }

      void SetButtonDrivers(AbstractGUIDriver& driver)
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          m_checkboxes[i]->SetDriver(driver);
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          m_buttons[i]->SetDriver(driver);
        }
      }

      void InsertButtons()
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          Panel::Insert(m_checkboxes[i], NULL);
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          Panel::Insert(m_buttons[i], NULL);
          m_buttons[i]->SetDimensions(SPoint(BUTTON_WIDTH, BUTTON_HEIGHT));
        }
      }

     private:
      AbstractGridCheckbox* m_checkboxes[MAX_BUTTONS];
      AbstractGridButton*   m_buttons[MAX_BUTTONS];

      u32 m_checkboxCount;
      u32 m_buttonCount;

      void Pack()
      {
        for(u32 i = 0; i < m_checkboxCount; i++)
        {
          m_checkboxes[i]->SetRenderPoint(SPoint(2, i * CHECKBOX_SPACING_HEIGHT));
        }

        for(u32 i = 0; i < m_buttonCount; i++)
        {
          m_buttons[i]->SetRenderPoint(SPoint(2, m_checkboxCount * CHECKBOX_SPACING_HEIGHT +
                                                 i * BUTTON_SPACING_HEIGHT));
        }

        Panel::SetDimensions(STATS_WINDOW_WIDTH,
                             m_checkboxCount * CHECKBOX_SPACING_HEIGHT +
                             m_buttonCount * BUTTON_SPACING_HEIGHT);
      }

    }m_buttonPanel;

    TextPanel<100,100> m_logPanel;
    TeeByteSink m_logSplitter;

    void SetScreenSize(u32 width, u32 height)
    {
      m_screenWidth = width;
      m_screenHeight = height;
      screen = SDL_SetVideoMode(m_screenWidth, m_screenHeight, 32,
                                SDL_SWSURFACE | SDL_RESIZABLE);

      AssetManager::Initialize();

      if (screen == 0)
      {
        FAIL(ILLEGAL_STATE);
      }

      UPoint newDimensions(width, height);

      m_rootPanel.SetDimensions(m_screenWidth, m_screenHeight);
      m_gridPanel.SetDimensions(m_screenWidth, m_screenHeight);

      m_rootPanel.SetRenderPoint(SPoint(0, 0));
      m_rootPanel.SetForeground(Drawing::BLUE);
      m_rootPanel.SetBackground(Drawing::RED);
      m_rootPanel.HandleResize(newDimensions);

      m_rootDrawing.Reset(screen, m_fonts.GetDefaultFont());

      if(m_renderStats)
      {
        m_grend.SetDimensions(UPoint(m_screenWidth - STATS_WINDOW_WIDTH,m_screenHeight));
      }
      else
      {
        m_grend.SetDimensions(UPoint(m_screenWidth,m_screenHeight));
      }

      m_srend.SetDrawPoint(SPoint(0,0));//m_screenWidth-STATS_WINDOW_WIDTH, 0));
      m_srend.SetDimensions(UPoint(STATS_WINDOW_WIDTH, m_screenHeight));

      printf("Screen resize: %d x %d\n", width, height);
      HandleResize();
    }

    StatsRenderer<GC> & GetStatsRenderer()
    {
      return m_srend;
    }

    void RunHelper()
    {
      m_keyboardPaused = m_startPaused;

      bool running = true;

      SDL_Event event;
      s32 lastFrame = SDL_GetTicks();

      u32 mouseButtonsDown = 0;
      ButtonPositionArray dragStartPositions;

      while(running)
      {
        while(SDL_PollEvent(&event))
        {
          switch(event.type)
          {
          case SDL_VIDEORESIZE:
            SetScreenSize(event.resize.w, event.resize.h);
            break;

          case SDL_QUIT:
            running = false;
            break;

          case SDL_MOUSEBUTTONUP:
            mouseButtonsDown &= ~(1<<(event.button.button));
            dragStartPositions[event.button.button].Set(-1,-1);
            goto mousebuttondispatch;

          case SDL_MOUSEBUTTONDOWN:
            mouseButtonsDown |= 1<<(event.button.button);
            dragStartPositions[event.button.button].Set(event.button.x,event.button.y);
            // FALL THROUGH

          mousebuttondispatch:
            {
              MouseButtonEvent mbe(m_keyboard, event, m_selectedTool);
              m_rootPanel.Dispatch(mbe,
                                   Rect(SPoint(),
                                        UPoint(m_screenWidth,m_screenHeight)));
            }
            break;

          case SDL_MOUSEMOTION:
          {
            MouseMotionEvent mme(m_keyboard, event,
                                 mouseButtonsDown, dragStartPositions, m_selectedTool);
            m_rootPanel.Dispatch(mme,
                                 Rect(SPoint(),
                                      UPoint(m_screenWidth,m_screenHeight)));
          }
          break;

          case SDL_KEYDOWN:
          case SDL_KEYUP:
            m_keyboard.HandleEvent(&event.key);
            break;

          }
        }

        m_mousePaused = mouseButtonsDown != 0;

        /* Limit framerate */
        s32 sleepMS = (s32)
          ((1000.0 / FRAMES_PER_SECOND) -
           (SDL_GetTicks() - lastFrame));
        if(sleepMS > 0)
        {
          SDL_Delay(sleepMS);
        }
        lastFrame = SDL_GetTicks();


        Update(Super::GetGrid());

        m_rootDrawing.Clear();

        m_rootPanel.Paint(m_rootDrawing);

        if (m_recordScreenshotPerAEPS > 0)
        {
          if (!m_gridPaused && Super::GetAEPS() >= m_nextScreenshotAEPS)
          {
            const char * path = Super::GetSimDirPathTemporary("vid/%010d.png",
                                                              m_nextScreenshotAEPS);

            camera.DrawSurface(screen,path);
            {
              const char * path = Super::GetSimDirPathTemporary("tbd/data.dat");
              bool exists = true;
              {
                FILE* fp = fopen(path, "r");
                if (!fp) exists = false;
                else fclose(fp);
              }
              FILE* fp = fopen(path, "a");
              FileByteSink fbs(fp);
              m_srend.WriteRegisteredCounts(fbs, !exists, Super::GetGrid(),
                                            Super::GetAEPS(),
                                            Super::GetAER(),
                                            Super::GetAEPSPerFrame(),
                                            Super::GetOverheadPercent(), true);
              fclose(fp);
            }
            // Are we accelerating and not yet up to cruising speed?
            if (m_countOfScreenshotsPerRate > 0 &&
                m_recordScreenshotPerAEPS < m_maxRecordScreenshotPerAEPS)
            {

              // Time to step on it?
              if (++m_countOfScreenshotsAtThisAEPS > m_countOfScreenshotsPerRate)
              {
                ++m_recordScreenshotPerAEPS;
                m_countOfScreenshotsAtThisAEPS = 0;
              }
            }

            m_nextScreenshotAEPS += m_recordScreenshotPerAEPS;
          }
        }

        if(Super::GetHaltAfterAEPS() > 0 &&
           Super::GetAEPS() > Super::GetHaltAfterAEPS())
        {
          running = false;
        }

        SDL_Flip(screen);
      }

      SDL_FreeSurface(screen);
      SDL_Quit();
    }
  };
} /* namespace MFM */

#endif /* ABSTRACTGUIDRIVER_H */
