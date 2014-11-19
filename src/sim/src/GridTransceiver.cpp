#include "GridTransceiver.h"
#include "Util.h"  // For MIN

namespace MFM
{
  GridTransceiver::GridTransceiver()
    : m_enabled(false)
    , m_bytesPerSecond(500000) // default ~500KBps == ~4Mbps
    , m_maxBytesInFlight(1)
    , m_excessNanoseconds(0)
  {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    m_lastAdvanced = now;
  }

  bool GridTransceiver::AdvanceToTime(const timespec & now)
  {
    const u32 ONE_BILLION = 1000*1000*1000;

    // We keep max time lapse under 2 secs
    u32 secs = MIN(1, (s32) (now.tv_sec - m_lastAdvanced.tv_sec));
    u32 nanoselapsed = (u32) (((s32) secs*ONE_BILLION) + (now.tv_nsec - m_lastAdvanced.tv_nsec));
    m_lastAdvanced = now;
    return Advance(nanoselapsed);
  }

  bool GridTransceiver::Advance(u32 nanoseconds)
  {
    Mutex::ScopeLock lock(m_access);

    const u64 ONE_BILLION = 1000000000;
    nanoseconds += m_excessNanoseconds;  // include any leftovers
    u32 bytes = (u32) (((u64) m_bytesPerSecond) * nanoseconds / ONE_BILLION);
    u32 nanosUsed = (u32) (bytes * ONE_BILLION / m_bytesPerSecond);
    m_excessNanoseconds = nanoseconds - nanosUsed;
    return Transceive(bytes, m_maxBytesInFlight);
  }

  bool GridTransceiver::ByteChannel::Transceive(u32 maxBytes, u32 maxInFlight)
  {
    u32 rcvable = MIN(CanRcv(), maxBytes);
    u32 sndable = MIN(CanXmit(), maxBytes);
    Increment(m_rcvIndex, rcvable);     // First pull up to maxBytes out of the air
    Increment(m_xmitIndex, sndable);    // Now push up to maxBytes into the air

    // Now if more than maxInFlight are in the air, clear the excess
    if (CanRcv() > maxInFlight)
    {
      Increment(m_rcvIndex, CanRcv() - maxInFlight);
    }

    return rcvable > 0 || sndable > 0;
  }

  u32 GridTransceiver::ByteChannel::Write(const u8 * data, u32 length)
  {
    const u32 count = MIN(CanWrite(), length);
    for (u32 i = 0; i < count; ++i)
    {
      m_data[m_writeIndex] = data[i];
      Increment(m_writeIndex);
    }
    return count;
  }

  u32 GridTransceiver::ByteChannel::Read(u8 * data, u32 length)
  {
    const u32 count = MIN(CanRead(), length);
    for (u32 i = 0; i < count; ++i)
    {
      data[i] = m_data[m_readIndex];
      Increment(m_readIndex);
    }
    return count;
  }

}
