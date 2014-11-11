/* -*- C++ -*- */

#include "CacheProcessor.h"
#include "CharBufferByteSource.h"

namespace MFM
{
  template <class CC>
  bool PacketIO::SendUpdateBegin(CacheProcessor<CC> & cxn, const SPoint & localCenter)
  {
    SPoint center = cxn.LocalToRemote(localCenter);
    m_buffer.Reset();
    m_buffer.Printf("%c%h%h", PacketType::UPDATE_BEGIN, center.GetX(), center.GetY());
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class CC>
  bool PacketIO::ReceiveUpdateBegin(CacheProcessor<CC> & cxn, ByteSource & bs)
  {
    u8 ptype;
    s16 cx, cy;
    if (bs.Scanf("%c%h%h", &ptype, &cx, &cy) != 3 || ptype != PacketType::UPDATE_BEGIN)
    {
      return false;
    }

    cxn.BeginUpdate(SPoint(cx, cy));
    return true;
  }

  template <class CC>
  bool PacketIO::SendUpdateEnd(CacheProcessor<CC> & cxn)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c", PacketType::UPDATE_END);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class CC>
  bool PacketIO::ReceiveUpdateEnd(CacheProcessor<CC> & cxn, ByteSource & bs)
  {
    cxn.ReceiveUpdateEnd();
    return true;
  }


  template <class CC>
  bool PacketIO::SendAtom(PacketTypeCode ptype, CacheProcessor<CC> & cxn, u16 siteNumber, const typename CC::ATOM_TYPE & atom)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c%c",ptype,siteNumber);
    Element<CC>::GetBits(atom).PrintBytes(m_buffer);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class CC>
  bool PacketIO::ReceiveAtom(CacheProcessor<CC> & cxn, ByteSource & bs)
  {
    u8 ptype;
    u8 site;
    if (bs.Scanf("%c%c", &ptype, &site) != 2)
    {
      return false;
    }
    if (ptype != PacketType::UPDATE && ptype != PacketType::CHECK)
    {
      return false;
    }

    typename CC::ATOM_TYPE atom;
    if (!Element<CC>::GetBits(atom).ReadBytes(bs))
    {
      return false;
    }

    // OK, need EOF now
    if (bs.Read() >= 0)
    {
      return false;
    }

    cxn.ReceiveAtom(ptype==PacketType::UPDATE, site, atom);
    return true;
  }

  template <class CC>
  bool PacketIO::SendReply(u8 consistentCount, CacheProcessor<CC> & cxn)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c%c", PacketType::UPDATE_ACK, consistentCount);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class CC>
  bool PacketIO::ReceiveReply(CacheProcessor<CC> & cxn, ByteSource & bs)
  {
    u8 ptype;
    u8 consistentCount;
    if (bs.Scanf("%c%c", &ptype, &consistentCount) != 2 || ptype != PacketType::UPDATE_ACK)
    {
      return false;
    }

    cxn.ReceiveReply(consistentCount);
    return true;
  }

  template <class CC>
  bool PacketIO::HandlePacket(CacheProcessor<CC> & cxn, PacketBuffer & buf)
  {
    CharBufferByteSource cbs(buf.GetBuffer(), buf.GetLength());
    switch (cbs.Peek())
    {
    case PacketType::UPDATE_BEGIN:
      return ReceiveUpdateBegin(cxn, cbs);

    case PacketType::UPDATE:
    case PacketType::CHECK:
      return ReceiveAtom(cxn, cbs);

    case PacketType::UPDATE_END:
      return ReceiveUpdateEnd(cxn, cbs);

    case PacketType::UPDATE_ACK:
      return ReceiveReply(cxn, cbs);

    default:
      FAIL(ILLEGAL_STATE);
    }

    FAIL(INCOMPLETE_CODE);

    // Eventually arriving at:
    cxn.SetIdle();
    return true;
  }

}
