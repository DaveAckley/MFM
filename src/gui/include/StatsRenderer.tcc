/* -*- C++ -*- */
#include "Drawing.h"
#include <stdlib.h>
#include <string.h>

namespace MFM {

  template <class GC>
  void StatsRenderer::RenderGridStatistics(Grid<GC>& grid, double aeps, double aer, u32 AEPSperFrame, double overhead)
  {
    // Extract short names for parameter types
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

    Drawing::FillRect(m_dest, m_drawPoint.GetX(), m_drawPoint.GetY(),
                      m_dimensions.GetX(), m_dimensions.GetY(),
                      0xff400040);

    const u32 STR_BUFFER_SIZE = 128;
    char strBuffer[STR_BUFFER_SIZE];

    const u32 ROW_HEIGHT = 30;
    u32 baseY = 0;

    sprintf(strBuffer, "%8.3f kAEPS", aeps/1000.0);

    Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                      Point<u32>(m_dimensions.GetX(), ROW_HEIGHT), 0xffffffff);
    baseY += ROW_HEIGHT;

    if (m_displayAER) {
      sprintf(strBuffer, "%8d/frame", AEPSperFrame);
      Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                        Point<u32>(m_dimensions.GetX(), ROW_HEIGHT), 0xffffffff);
      baseY += ROW_HEIGHT;

      sprintf(strBuffer, "%8.3f AER", aer);
      Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                        Point<u32>(m_dimensions.GetX(), ROW_HEIGHT), 0xffffffff);
      baseY += ROW_HEIGHT;

      sprintf(strBuffer, "%8.3f %%ovrhd", overhead);
      Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                        Point<u32>(m_dimensions.GetX(), ROW_HEIGHT), 0xffffffff);
      baseY += ROW_HEIGHT;

    }

    baseY += ROW_HEIGHT; // skip a line
    for (u32 i = 0; i < m_displayTypesInUse; ++i) {
      u32 type = m_displayTypes[i];
      const Element<CC> * elt = grid.GetTile(SPoint(0,0)).GetElementTable().Lookup(type);
      if (elt == 0) continue;
      u32 typeCount = grid.GetAtomCount(type);

      snprintf(strBuffer, STR_BUFFER_SIZE, "%8d %s", typeCount, elt->GetName());

      Drawing::BlitText(m_dest, m_drawFont, strBuffer, Point<u32>(m_drawPoint.GetX(), baseY),
                        Point<u32>(m_dimensions.GetX(), ROW_HEIGHT), 0xffffffff);
      baseY += ROW_HEIGHT;
    }
  }

  template <class GC>
  void StatsRenderer::WriteRegisteredCounts(FILE * fp, bool writeHeader, Grid<GC>& grid,
                                           double aeps, double aer, u32 AEPSperFrame,
                                           double overhead)
  {
    // Extract short names for parameter types
    typedef typename GC::CORE_CONFIG CC;
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { W = GC::GRID_WIDTH};
    enum { H = GC::GRID_HEIGHT};
    enum { R = P::EVENT_WINDOW_RADIUS};

    if (writeHeader) {
      fprintf(fp,"# kAEPS AEPSperFrame AER pctOvrhd");
      for (u32 i = 0; i < m_displayTypesInUse; ++i) {
        u32 type = m_displayTypes[i];
        const Element<CC> * elt = grid.GetTile(SPoint(0,0)).GetElementTable().Lookup(type);
        if (elt == 0) continue;
        fprintf(fp," #%s", elt->GetName());
      }
      fprintf(fp,"\n");
    }

    fprintf(fp,"%8.3f", aeps/1000.0);
    fprintf(fp," %d", AEPSperFrame);
    fprintf(fp, "%8.3f", aer);
    fprintf(fp, "%8.3f", overhead);

    for (u32 i = 0; i < m_displayTypesInUse; ++i) {
      u32 type = m_displayTypes[i];
      const Element<CC> * elt = grid.GetTile(SPoint(0,0)).GetElementTable().Lookup(type);
      if (elt == 0) continue;
      u32 typeCount = grid.GetAtomCount(type);

      fprintf(fp," %8d", typeCount);
    }
    fprintf(fp,"\n");
  }

} /* namespace MFM */

