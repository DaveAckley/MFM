/* -*- C++ -*- */
#include "Drawing.h"
#include <stdlib.h>
#include <ctype.h>    /* For isspace */
#include <string.h>
#include "OverflowableCharBufferByteSink.h"
#include "Version.h"

namespace MFM {

  template <class GC>
  void StatisticsPanel<GC>::StatsRendererSaveDetails(ByteSink & sink) const
  {
    sink.Printf(",%d",m_displayAER);
  }

  template <class GC>
  bool StatisticsPanel<GC>::StatsRendererLoadDetails(LineCountingByteSource & source)
  {
    u32 daer;
    if (2!=source.Scanf(",%d",&daer))
      return false;

    m_displayAER = daer;
    return true;
  }

  template <class GC>
  void StatisticsPanel<GC>::RenderGridStatistics(Drawing & drawing, Grid<GC>& grid, double aeps, double aer, u32 AEPSperFrame, double overhead, bool endOfEpoch, u32 aepsInCurrentEpoch)
  {

    // Extract short names for parameter types
    typedef typename GC::EVENT_CONFIG EC;
    enum { R = EC::EVENT_WINDOW_RADIUS};

    const u32 BUFSIZE = 128;
    char strBuffer[BUFSIZE];

    const u32 ROW_HEIGHT = LINE_HEIGHT_PIXELS;
    const u32 DETAIL_ROW_HEIGHT = DETAIL_LINE_HEIGHT_PIXELS;
    u32 baseY = 0;

    drawing.SetFont(FONT_ASSET_ELEMENT);
    drawing.SetForeground(Drawing::GREY80);

    UPoint dims = this->GetDimensions();

    do  // So we can break when done
    {
      if (m_displayAER < 1)
      {
        break;
      }

      const char * runLabel = m_runLabel.GetZString();
      if (strlen(runLabel) == 0)
        runLabel = MFM_VERSION_STRING_SHORT;

      SPoint size = drawing.GetTextSize(runLabel);
      UPoint loc(MAX(0, ((s32) dims.GetX())-size.GetX()), baseY);

      drawing.BlitText(runLabel,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 2)
      {
        break;
      }

      u64 now = Utils::GetDateTimeNow();
      snprintf(strBuffer,BUFSIZE," %d %06d",
               Utils::GetDateFromDateTime(now),
               Utils::GetTimeFromDateTime(now)
               );
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) dims.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 3)
      {
        break;
      }

      OString128 ob;
      if (m_screenshotTargetFPS < 0)
      {
        sprintf(strBuffer, "%8.3f AER", aer);
        ob.Printf("%s_",strBuffer);
        u64 sites = grid.GetTotalSites();
        ob.PrintAbbreviatedNumber(sites);
      }
      else
      {
        u64 displayedAER = aepsInCurrentEpoch*m_screenshotTargetFPS;
        ob.PrintAbbreviatedNumber(displayedAER);
        ob.Printf(" AER_%dfps", m_screenshotTargetFPS);
      }

      const char * str = ob.GetZString();
      size = drawing.GetTextSize(str);
      loc = UPoint(MAX(0, ((s32) dims.GetX())-size.GetX()), baseY);
      drawing.BlitText(str,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 4)
      {
        break;
      }

      sprintf(strBuffer, "%8.3f %%ov", overhead);
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) dims.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 5)
      {
        break;
      }

      sprintf(strBuffer, "%8d/frame", AEPSperFrame);
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) dims.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 6)
      {
        break;
      }

    } while (0);

    if (m_displayAER > 0)
    {
      baseY += ROW_HEIGHT/2;
    }

    drawing.SetFont(FONT_ASSET_ELEMENT);
    drawing.SetForeground(Drawing::WHITE);

    sprintf(strBuffer, "%8.3f kAEPS", aeps/1000.0);

    drawing.SetFont(FONT_ASSET_ELEMENT);
    drawing.SetForeground(Drawing::WHITE);
    drawing.BlitText(strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                     Point<u32>(dims.GetX(), ROW_HEIGHT));

    baseY += ROW_HEIGHT;
    baseY += ROW_HEIGHT;

    sprintf(strBuffer, "%8.3f %%full", grid.GetFullSitePercentage() * 100);
    drawing.BlitText(strBuffer, UPoint(m_drawPoint.GetX(), baseY),
                     UPoint(dims.GetX(), ROW_HEIGHT));

    baseY += ROW_HEIGHT; // skip a line
    for (u32 i = 0; i < m_reportersInUse; ++i)
    {
      const u32 VALUE_WIDTH = 8;
      const DataReporter * cs = m_reporters[i];
      OString32 datavalue;
      cs->GetValue(datavalue, endOfEpoch);
      OString32 output;

      /* Only report values if they aren't 0 */
      if(strcmp(datavalue.GetZString(), "0  0"))
      {
        for (u32 vlen = datavalue.GetLength(); vlen < VALUE_WIDTH; ++vlen)
        {
          output.Print(" ");
        }

        output.Printf("%2s %s",datavalue.GetZString(), cs->GetLabel());

        drawing.SetFont(FONT_ASSET_ELEMENT);
        drawing.SetForeground(0xffffffff);
        drawing.BlitText(output.GetZString(), Point<u32>(m_drawPoint.GetX(), baseY),
                         Point<u32>(dims.GetX(), ROW_HEIGHT));
        baseY += ROW_HEIGHT;
      }
    }
  }

  template <class GC>
  void StatisticsPanel<GC>::WriteRegisteredCounts(ByteSink & fp, bool writeHeader, Grid<GC>& grid,
                                                double aeps, double aer, u32 AEPSperFrame,
                                                double overhead, bool endOfEpoch)
  {
    FAIL(DEPRECATED);
  }

} /* namespace MFM */
