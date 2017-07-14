/*                                              -*- mode:C++ -*-
  StatisticsPanel.h Info box for the GUI driver
  Copyright (C) 2014-2015 The Regents of the University of New Mexico.  All rights reserved.

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
  \file StatisticsPanel.h Info box for the GUI driver
  \author Trent R. Small.
  \author Dave Small.
  \date (C) 2014-2015 All rights reserved.
  \lgpl
 */
#ifndef STATISTICSPANEL_H
#define STATISTICSPANEL_H

#include "MovablePanel.h"
#include "GUIConstants.h"
#include "Grid.h"
#include "AbstractDriver.h"

namespace MFM
{
  template <class GC>
  class StatisticsPanel : public MovablePanel
  {
    typedef MovablePanel Super;

    typedef AbstractDriver<GC> OurDriver;
    typedef Grid<GC> OurGrid;
    typedef typename GC::EVENT_CONFIG EC;

    OurDriver & m_driver;

  public:
    OurDriver & GetDriver() { return m_driver; }

    StatisticsPanel(AbstractDriver<GC> & ad)
      : MovablePanel()
      , m_driver(ad)
      , m_reportersInUse(0)
      , m_displayElementsInUse(0)
      , m_displayAER(2)
      , m_maxDisplayAER(5)
      , m_screenshotTargetFPS(-1)
        //      , m_registeredButtons(0)
      , m_drawPoint(10,0)
    {
      SetName("StatisticsPanel");
      SetDimensions(STATS_START_WINDOW_WIDTH,
                    SCREEN_INITIAL_HEIGHT);
      SetDesiredSize(STATS_START_WINDOW_WIDTH,
                     U32_MAX);
      SetRenderPoint(SPoint(100000, 0));
      SetForeground(Drawing::WHITE);
      SetBackground(Drawing::DARK_PURPLE);
      SetFont(FONT_ASSET_ELEMENT);
    }

    bool LoadDetails(const char * key, LineCountingByteSource & source)
    {
      if (Super::LoadDetails(key, source)) return true;

      return StatsRendererLoadDetails(key, source);
    }

    void SaveDetails(ByteSink & sink) const
    {
      Super::SaveDetails(sink);

      StatsRendererSaveDetails(sink);
    }

  protected:
    virtual void PaintComponent(Drawing& drawing)
    {
      this->Panel::PaintComponent(drawing);
      this->RenderGridStatistics(drawing);
    }

    virtual void PaintBorder(Drawing & config)
    { /* No border please */ }

    //// StatsRenderer import

   public:
    class DataReporter
    {
    public:
      virtual const char * GetLabel() const = 0;

      /**
       * Report the current associated data value to bs, in whatever
       * form desired.  This is to include everything but the GetLabel
       * label, which is sometimes handled separately (e.g., in data
       * files).
       */
      virtual void GetValue(ByteSink & bs, bool endOfEpoch) const = 0;

      virtual ~DataReporter()
      { }
    };

    class CapturableStatistic : public DataReporter
    {
     public:
      virtual const char * GetLabel() const = 0;

      virtual s32 GetDecimalPlaces() const = 0;

      virtual double GetValue(bool endOfEpoch) const = 0;

      virtual void GetValue(ByteSink & bs, bool endOfEpoch) const
      {
        const u32 FMT_BUFFER_SIZE = 32;
        char fmtBuffer[FMT_BUFFER_SIZE];

        const u32 STR_BUFFER_SIZE = 128;
        char strBuffer[STR_BUFFER_SIZE];

        s32 places = GetDecimalPlaces();
        if (places >= 0)
        {
          snprintf(fmtBuffer, FMT_BUFFER_SIZE, "%%8.%df",places);
          snprintf(strBuffer, STR_BUFFER_SIZE, fmtBuffer, GetValue(endOfEpoch));
        }
        bs.Print(strBuffer);
      }

      virtual ~CapturableStatistic()
      { }
    };

    class ElementCount : public DataReporter
    {
     private:
      const Element<EC> * m_element;
      const Grid<GC> * m_grid;

     public:
      ElementCount() :
        m_element(0),
        m_grid(0)
      { }

      void Set(const Grid<GC> * grid, const Element<EC> * elt)
      {
        m_element = elt;
        m_grid = grid;
      }

      virtual const char * GetLabel() const
      {
        if (m_element)
        {
          //          return m_element->GetUUID().GetLabel();
          return m_element->GetName();
        }
        return "<unset>";
      }

      virtual void GetValue(ByteSink & bs, bool endOfEpoch) const
      {
        if (!m_element || !m_grid)
        {
          return;
        }
        u32 type = m_element->GetType();
        u32 allSites = m_grid->CountActiveSites();
        if (allSites == 0)
        {
          bs.Printf("%d ?%%", 0);
          return;
        }

        u32 count = m_grid->GetAtomCount(type);
        double pct = 100.0 * count / allSites;

        if (pct == 0 || pct >= 1)
        {
          bs.Printf("%d %2d", count, (int) pct);
        }
        else
        {
          bs.Printf("%d .%d", count, (int) (10 * pct));
        }
      }
    };

    class ElementDataSlotSum : public CapturableStatistic
    {
     private:
      Grid<GC> * m_grid;
      const char * m_label;
      u32 m_elementType;
      u32 m_slot;
      u32 m_outOfSlots;
      bool m_resetOnRead;

     public:
      ElementDataSlotSum() :
        m_grid(0),
        m_label(0),
        m_elementType(0),
        m_slot(0),
        m_outOfSlots(0),
        m_resetOnRead(false)
      { }

      void Set(Grid<GC> & grid, const char * label, u32 elementType,
               u32 slot, u32 outOfSlots, bool resetOnRead)
      {
        m_grid = &grid;
        m_label = label;
        m_elementType = elementType;
        m_slot = slot;
        m_outOfSlots = outOfSlots;
        m_resetOnRead = resetOnRead;
      }

      virtual const char * GetLabel() const
      {
        if (m_label)
        {
          return m_label;
        }
        return "<unset>";
      }

      virtual s32 GetDecimalPlaces() const
      {
        return 0;
      }
      virtual double GetValue(bool endOfEpoch) const
      {
        if (!m_grid)
        {
          return -1;
        }
        if (m_slot >= m_outOfSlots)
        {
          return -1;
        }

        u64 sum = 0;
        for (typename Grid<GC>::iterator_type i = m_grid->begin(); i != m_grid->end(); ++i)
        {
          u64 * eds = 0;

          if (!eds)
          {
            continue;
          }
          sum += eds[m_slot];
          if (endOfEpoch && m_resetOnRead)
          {
            eds[m_slot] = 0;
          }
        }

        return (double) sum;
      }
    };

    bool StatsRendererLoadDetails(const char * key, LineCountingByteSource & source) ;
    void StatsRendererSaveDetails(ByteSink & sink) const ;

  private:

    static const u32 MAX_TYPES = 100;
    const DataReporter *(m_reporters[MAX_TYPES]);
    u32 m_reportersInUse;

    ElementCount m_displayElements[MAX_TYPES];
    u32 m_displayElementsInUse;

    u32 m_displayAER;
    u32 m_maxDisplayAER;

    OString64 m_runLabel;

    s32 m_screenshotTargetFPS;

#if 0 // Mon Jun 29 12:11:56 2015  WTF?  Prehistory?
    static const u32 MAX_BUTTONS = 16;
    AbstractButton* m_buttons[MAX_BUTTONS];
    u32 m_registeredButtons;
#endif

    UPoint m_drawPoint;

  public:

    enum {
      LINE_HEIGHT_PIXELS = 32,
      DETAIL_LINE_HEIGHT_PIXELS = 32
    };

#if 0
    void ClearButtons()
    {
      m_registeredButtons = 0;
    }

    enum {
      BUTTON_HEIGHT_PIXELS = 20,
      LINE_HEIGHT_PIXELS = 32,
      DETAIL_LINE_HEIGHT_PIXELS = 32
    };
    void ReassignButtonLocations()
    {
      SPoint loc(4, m_reportersInUse * BUTTON_HEIGHT_PIXELS);
      loc.SetY(loc.GetY() + m_displayAER*LINE_HEIGHT_PIXELS);
      for(u32 i = 0; i < m_registeredButtons; i++)
      {
        m_buttons[i]->SetLocation(loc);
        loc.SetY(loc.GetY() + BUTTON_HEIGHT_PIXELS);
      }
    }

    void AddButton(AbstractButton* b)
    {
      if (m_registeredButtons >= MAX_BUTTONS)
        FAIL(OUT_OF_ROOM);

      SPoint dimensions(268, BUTTON_HEIGHT_PIXELS - 2);
      SPoint location(4, (m_reportersInUse + m_registeredButtons) * BUTTON_HEIGHT_PIXELS);
      b->SetDimensions(dimensions);
      b->SetLocation(location);

      m_buttons[m_registeredButtons++] = b;
    }
#endif

    u32 GetDisplayAER() const
    {
      return m_displayAER;
    }

    u32 GetMaxDisplayAER() const
    {
      return m_maxDisplayAER;
    }

    void SetScreenshotTargetFPS(s32 fps)
    {
      m_screenshotTargetFPS = fps;
    }

    void SetDisplayAER(u32 displayAER)
    {
      m_displayAER = displayAER % (m_maxDisplayAER + 1);
      //      ReassignButtonLocations();
    }

    const char * GetRunLabel() const
    {
      return m_runLabel.GetZString();
    }

    void SetRunLabel(const char * label)
    {
      MFM_API_ASSERT_NONNULL(label);
      m_runLabel.Reset();
      m_runLabel.Printf("%s",label);
    }

    bool DisplayDataReporter(const DataReporter * cs)
    {
      if (m_reportersInUse >= MAX_TYPES)
      {
        return false;
      }
      u32 index = m_reportersInUse++;
      m_reporters[index] = cs;
      return true;
    }

    bool DisplayStatsForElement(const Grid<GC>  & grd, const Element<EC> & elt)
    {
      if (m_displayElementsInUse >= MAX_TYPES)
      {
        return false;
      }
      u32 index = m_displayElementsInUse++;
      m_displayElements[index].Set(&grd,&elt);
      return DisplayDataReporter(&m_displayElements[index]);
    }

    void RenderGridStatistics(Drawing & drawing);

  };
} /* namespace MFM */

#include "StatisticsPanel.tcc"

#endif /*STATISTICSPANEL_H*/
