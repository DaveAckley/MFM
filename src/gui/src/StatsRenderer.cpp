#include "StatsRenderer.h"

namespace MFM {
  bool StatsRenderer::DisplayStatsForType(u32 type) {
    if (m_displayTypesInUse >= MAX_TYPES) return false;
    m_displayTypes[m_displayTypesInUse++] = type;
    return true;
  }

}
