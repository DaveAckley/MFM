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
#include "StatsRenderer.h"
#include "GUIConstants.h"

namespace MFM
{
  template <class GC>
  class StatisticsPanel : public Panel
  {
    typedef Grid<GC> OurGrid;

    StatsRenderer<GC>* m_srend;
    OurGrid* m_mainGrid;
    double m_AEPS;
    double m_AER;
    double m_overheadPercent;
    u32 m_aepsPerFrame;
    u32 m_currentAEPSPerEpoch;

  public:
    StatisticsPanel() : m_srend(NULL)
    {
      SetName("StatisticsPanel");
      SetDimensions(STATS_START_WINDOW_WIDTH,
                    SCREEN_INITIAL_HEIGHT);
      SetDesiredSize(STATS_START_WINDOW_WIDTH,
                     U32_MAX);
      SetRenderPoint(SPoint(100000, 0));
      SetForeground(Drawing::WHITE);
      SetBackground(Drawing::DARK_PURPLE);
      m_AEPS = m_AER = 0.0;
      m_currentAEPSPerEpoch = 0;
      m_aepsPerFrame = 0;
      m_overheadPercent = 0.0;
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

    void SetCurrentAEPSPerEpoch(u32 aepsPerEpoch)
    {
      m_currentAEPSPerEpoch = aepsPerEpoch;
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

    bool LoadDetails(LineCountingByteSource & source)
    {
      u32 tmp_m_AEPS;
      u32 tmp_m_AER;
      u32 tmp_m_overheadPercent;
      u32 tmp_m_aepsPerFrame;
      u32 tmp_m_currentAEPSPerEpoch;
      if (6 != source.Scanf(",%D%D%D%D%D",
                            &tmp_m_AEPS,
                            &tmp_m_AER,
                            &tmp_m_overheadPercent,
                            &tmp_m_aepsPerFrame,
                            &tmp_m_currentAEPSPerEpoch))
        return false;

      MFM_API_ASSERT_NONNULL(m_srend);
      if (!m_srend->LoadDetails(source)) return false;

      m_AEPS = tmp_m_AEPS / 10.0;
      m_AER = tmp_m_AER / 100.0;
      m_overheadPercent = tmp_m_overheadPercent / 1000.0;
      m_aepsPerFrame = tmp_m_aepsPerFrame;
      m_currentAEPSPerEpoch = tmp_m_currentAEPSPerEpoch;
      return true;
    }

    void SaveDetails(ByteSink & sink) const
    {
      sink.Printf(",%D%D%D%D%D",
                  (u32)(10*m_AEPS),
                  (u32)(100*m_AER),
                  (u32)(1000*m_overheadPercent),
                  m_aepsPerFrame,
                  m_currentAEPSPerEpoch);

      MFM_API_ASSERT_NONNULL(m_srend);
      m_srend->SaveDetails(sink);
    }

  protected:
    virtual void PaintComponent(Drawing& drawing)
    {
      this->Panel::PaintComponent(drawing);
      m_srend->RenderGridStatistics(drawing, *m_mainGrid,
                                    m_AEPS, m_AER, m_aepsPerFrame,
                                    m_overheadPercent, false,
                                    m_currentAEPSPerEpoch);
    }

    virtual void PaintBorder(Drawing & config)
    { /* No border please */ }

    virtual bool HandlePostDrag(MouseButtonEvent& mbe)
    {
      return true;  /* Eat the event to keep the grid from taking it */
    }
  };
} /* namespace MFM */
#endif /*STATISTICSPANEL_H*/
