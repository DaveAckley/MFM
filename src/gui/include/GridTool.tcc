/* -*- C++ -*- */
#include "GridPanel.h"
#include "ToolboxPanel.h"
#include "Fail.h"

namespace MFM {
  template<class GC>
  void GridToolShapeUpdater<GC>::SaveDetails(ByteSink & sink) const
  {
    const char * tag = this->GetTag();
    sink.Printf(" PP(%s_trad=%d)\n", tag, m_toolRadius);
    sink.Printf(" PP(%s_tshp=%d)\n", tag, m_toolShape);
  }

  template<class GC>
  bool GridToolShapeUpdater<GC>::LoadDetails(const char * key, LineCountingByteSource & source)
  {
    const char * tag = this->GetTag();
    CharBufferByteSource cbbs(key, strlen(key));

    cbbs.Reset();
    if (6 == cbbs.Scanf("%z_trad",tag))
      return 1 == source.Scanf("%?d", sizeof m_toolRadius, &m_toolRadius);

    cbbs.Reset();
    if (6 == cbbs.Scanf("%z_tshp",tag))
      return 1 == source.Scanf("%?d", sizeof m_toolShape, &m_toolShape);

    return false;
  }

}
