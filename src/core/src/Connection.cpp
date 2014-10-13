#include "Connection.h"

namespace MFM
{
  void Connection::ReportConnectionStatus(Logger::Level level, bool owned)
  {
    LOG.Log(level,"   =Connection %p (%s)=", (void*) this, owned?"owned":"unowned");
    LOG.Log(level,"    Connected: %s", m_connected?"true":"false");
    LOG.Log(level,"    Locked by another: %s", IsLockedByAnother()?"true":"false");
    LOG.Log(level,"    Input buffer count: %d", InputByteCount());
    LOG.Log(level,"    Output buffer count: %d", OutputByteCount());
    m_lock.ReportMutexStatus(level);

  }

}
