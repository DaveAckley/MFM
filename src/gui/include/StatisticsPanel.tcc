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
    sink.Printf(" PP(daer=%d)\n",m_displayAER);
    sink.Printf(" PP(dvnl=%d)\n",m_displayVersionLine);
    sink.Printf(" PP(dtsl=%d)\n",m_displayTimestampLine);
    sink.Printf(" PP(daep=%d)\n",m_displayAEPS);
  }

  template <class GC>
  bool StatisticsPanel<GC>::StatsRendererLoadDetails(const char * key, LineCountingByteSource & source)
  {
    if (!strcmp("daer",key)) return 1 == source.Scanf("%?d", sizeof m_displayAER, &m_displayAER);
    if (!strcmp("dvnl",key)) return 1 == source.Scanf("%?d", sizeof m_displayVersionLine, &m_displayVersionLine);
    if (!strcmp("dtsl",key)) return 1 == source.Scanf("%?d", sizeof m_displayTimestampLine, &m_displayTimestampLine);
    if (!strcmp("daep",key)) return 1 == source.Scanf("%?d", sizeof m_displayAEPS, &m_displayAEPS);
    return false;
  }

  template <class GC>
  void StatisticsPanel<GC>::RenderGridStatistics(Drawing & drawing)
  {
    OurDriver & ad = this->GetDriver();
    OurGrid& grid = ad.GetGrid();
    double aeps = ad.GetAEPS();
    double aer = ad.GetRecentAER();
    u32 AEPSperFrame = ad.GetAEPSPerFrame();
    double overhead = ad.GetOverheadPercent();
    bool endOfEpoch = false;
    u32 aepsInCurrentEpoch = ad.GetAEPSPerEpoch();

    // Extract short names for parameter types
    typedef typename GC::EVENT_CONFIG EC;
    enum { R = EC::EVENT_WINDOW_RADIUS};

    const u32 BUFSIZE = 128;
    char strBuffer[BUFSIZE];

    FontAsset font = drawing.GetFont();
    const u32 ROW_HEIGHT = AssetManager::GetFontLineSkip(font);
    const u32 DETAIL_ROW_HEIGHT = ROW_HEIGHT;
    u32 baseY = 0;

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
      SPoint loc(MAX(0, ((s32) dims.GetX())-size.GetX())/2, baseY);

      if (m_displayVersionLine)
      {
        drawing.BlitText(runLabel,
                         loc,
                         UPoint(dims.GetX(), ROW_HEIGHT));
        baseY += DETAIL_ROW_HEIGHT;
      }
      
      if (m_displayAER < 2)
      {
        break;
      }

      if (m_displayTimestampLine)
      {

        u64 now = Utils::GetDateTimeNow();
        snprintf(strBuffer,BUFSIZE,"%d %06d",
                  Utils::GetDateFromDateTime(now),
                  Utils::GetTimeFromDateTime(now)
                  );
        size = drawing.GetTextSize(strBuffer);
        loc = SPoint(MAX(0, ((s32) dims.GetX())-size.GetX())/2, baseY);
        drawing.BlitText(strBuffer,
                         loc,
                         UPoint(dims.GetX(), ROW_HEIGHT));
        baseY += DETAIL_ROW_HEIGHT;
      }
      
      if (m_displayAER < 3)
      {
        break;
      }

      OString128 ob;

      if (m_screenshotTargetFPS < 0)
      {
        sprintf(strBuffer, "%0.3f AER", aer);
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
      loc = SPoint(MAX(0, ((s32) dims.GetX())-size.GetX())/2, baseY);
      drawing.BlitText(str,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 4)
      {
        break;
      }

      sprintf(strBuffer, "%0.3f %%ov", overhead);
      size = drawing.GetTextSize(strBuffer);
      loc = SPoint(MAX(0, ((s32) dims.GetX())-size.GetX())/2, baseY);
      drawing.BlitText(strBuffer,
                       loc,
                       UPoint(dims.GetX(), ROW_HEIGHT));
      baseY += DETAIL_ROW_HEIGHT;

      if (m_displayAER < 5)
      {
        break;
      }

      sprintf(strBuffer, "%d/frame", AEPSperFrame);
      size = drawing.GetTextSize(strBuffer);
      loc = SPoint(MAX(0, ((s32) dims.GetX())-size.GetX())/2, baseY);
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
      baseY += ROW_HEIGHT/3;
    }

    drawing.SetForeground(Drawing::WHITE);

    if (m_displayAEPS)
    {

      sprintf(strBuffer, "%8.3f kAEPS", aeps/1000.0);

      drawing.SetForeground(Drawing::WHITE);
      drawing.BlitText(strBuffer,
                       SPoint(m_drawPoint.GetX(), baseY),
                       UPoint(dims.GetX(), ROW_HEIGHT));
      
      baseY += ROW_HEIGHT;
      baseY += ROW_HEIGHT;
    }

    sprintf(strBuffer, "%8.3f %%full", grid.GetFullSitePercentage() * 100);
    drawing.BlitText(strBuffer,
                     SPoint(m_drawPoint.GetX(), baseY),
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

        drawing.SetForeground(0xffffffff);
        drawing.BlitText(output.GetZString(),
                         SPoint(m_drawPoint.GetX(), baseY),
                         UPoint(dims.GetX(), ROW_HEIGHT));
        baseY += ROW_HEIGHT;
      }
    }
  }

} /* namespace MFM */
