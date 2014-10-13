#include "Mutex.h"
#include "Logger.h"

namespace MFM
{
  void Mutex::ReportMutexStatus(int ilevel)
  {
    Logger::Level level = (Logger::Level) ilevel;
    if (m_locked)
    {
      LOG.Log(level,"    Mutex %p is locked by thread %p",
              (void*) this,
              (void*) m_threadId);
    }
    else
    {
      LOG.Log(level,"    Mutex %p is unlocked",
              (void*) this);
    }

    LOG.Log(level,  "    In VCW: %s", m_inVCW?"true":"false");
  }
}
