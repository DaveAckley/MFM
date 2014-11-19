/* -*- C++ -*- */
#include "Drawing.h"
#include <stdlib.h>
#include <ctype.h>    /* For isspace */
#include <string.h>
#include "OverflowableCharBufferByteSink.h"
#include "Version.h"

namespace MFM {

  template <class GC>
  void StatsRenderer<GC>::RenderGridStatistics(Drawing & drawing, Grid<GC>& grid, double aeps, double aer, u32 AEPSperFrame, double overhead, bool endOfEpoch)
  {
    // Extract short names for parameter types
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

    const u32 STR_BUFFER_SIZE = 128;
    char strBuffer[STR_BUFFER_SIZE];

    const u32 ROW_HEIGHT = LINE_HEIGHT_PIXELS;
    const u32 DETAIL_ROW_HEIGHT = DETAIL_LINE_HEIGHT_PIXELS;
    u32 baseY = 0;

    drawing.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
    drawing.SetForeground(Drawing::GREY80);

    do  // So we can break when done
    {
      if (m_displayAER < 1)
      {
        break;
      }

      SPoint size = drawing.GetTextSize(MFM_VERSION_STRING_SHORT);
      UPoint loc(MAX(0, ((s32) m_dimensions.GetX())-size.GetX()), baseY);

      drawing.BlitText(MFM_VERSION_STRING_SHORT,
                       loc,
                       UPoint(m_dimensions.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 2)
      {
        break;
      }

      u64 now = Utils::GetDateTimeNow();
      sprintf(strBuffer, " %d %06d",
              Utils::GetDateFromDateTime(now),
              Utils::GetTimeFromDateTime(now)
              );

      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) m_dimensions.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(m_dimensions.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 3)
      {
        break;
      }

      sprintf(strBuffer, "%8.3f AER", aer);
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) m_dimensions.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(m_dimensions.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 4)
      {
        break;
      }

      sprintf(strBuffer, "%8.3f %%ov", overhead);
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) m_dimensions.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(m_dimensions.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 5)
      {
        break;
      }

      sprintf(strBuffer, "%8d/frame", AEPSperFrame);
      size = drawing.GetTextSize(strBuffer);
      loc = UPoint(MAX(0, ((s32) m_dimensions.GetX())-size.GetX()), baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(m_dimensions.GetX(), ROW_HEIGHT));
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

    drawing.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
    drawing.SetForeground(Drawing::WHITE);

    sprintf(strBuffer, "%8.3f kAEPS", aeps/1000.0);

    drawing.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
    drawing.SetForeground(Drawing::WHITE);
    drawing.BlitText(strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                     Point<u32>(m_dimensions.GetX(), ROW_HEIGHT));

    baseY += ROW_HEIGHT;
    baseY += ROW_HEIGHT;

    sprintf(strBuffer, "%8.3f %%full", grid.GetEmptySitePercentage() * 100);
    drawing.BlitText(strBuffer, UPoint(m_drawPoint.GetX(), baseY),
                     UPoint(m_dimensions.GetX(), ROW_HEIGHT));

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

        drawing.SetFont(AssetManager::Get(FONT_ASSET_ELEMENT));
        drawing.SetForeground(0xffffffff);
        drawing.BlitText(output.GetZString(), Point<u32>(m_drawPoint.GetX(), baseY),
                         Point<u32>(m_dimensions.GetX(), ROW_HEIGHT));
        baseY += ROW_HEIGHT;
      }
    }
  }

  template <class GC>
  void StatsRenderer<GC>::WriteRegisteredCounts(ByteSink & fp, bool writeHeader, Grid<GC>& grid,
                                                double aeps, double aer, u32 AEPSperFrame,
                                                double overhead, bool endOfEpoch)
  {
    FAIL(DEPRECATED);
    /* Use AbstractDriver::WriteTimeBasedData instead. */

    /*
    // Extract short names for parameter types
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::PARAM_CONFIG P;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

    if (writeHeader)
    {
      fp.Printf("# AEPS AEPSperFrame AER100 pctOvrhd100");
      for (u32 i = 0; i < m_reportersInUse; ++i)
      {
        const DataReporter * cs = m_reporters[i];
        // What's this for?        if (cs->GetDecimalPlaces() < 0) continue;

        // Try to ensure splitting on spaces will get the right number of names
        fp.WriteByte(' ');
        for (const char * p = cs->GetLabel(); *p; ++p)
        {
          if (isspace(*p))
          {
            fp.WriteByte('_');
          }
          else
          {
            fp.WriteByte(*p);
          }
        }
      }
      fp.Println();
    }

    //fprintf(fp,"%8.3f", aeps/1000.0);
    fp.Print((u64) aeps);
    fp.Print(" ");
    fp.Print(AEPSperFrame);
    fp.Print(" ");
    fp.Print((u64)(100.0 * aer));
    fp.Print(" ");
    fp.Print((u64)(100.0 * overhead));

    for (u32 i = 0; i < m_reportersInUse; ++i)
    {
      const DataReporter * cs = m_reporters[i];
      //if (cs->GetDecimalPlaces() < 0) continue;
      fp.Print(" ");
      cs->GetValue(fp, endOfEpoch);
    }
    fp.Println();
    */
  }

} /* namespace MFM */
