/* -*- C++ -*- */

#include "CacheProcessor.h"
#include "CharBufferByteSource.h"

namespace MFM
{
  template <class EC>
  bool PacketIO::SendUpdateBegin(CacheProcessor<EC> & cxn, const SPoint & localCenter)
  {
    SPoint center = cxn.LocalToRemote(localCenter);
    m_buffer.Reset();
    m_buffer.Printf("%c%h%h", PacketType::UPDATE_BEGIN, center.GetX(), center.GetY());
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class EC>
  bool PacketIO::ReceiveUpdateBegin(CacheProcessor<EC> & cxn, ByteSource & bs)
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

  template <class EC>
  bool PacketIO::SendUpdateEnd(CacheProcessor<EC> & cxn)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c", PacketType::UPDATE_END);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class EC>
  bool PacketIO::ReceiveUpdateEnd(CacheProcessor<EC> & cxn, ByteSource & bs)
  {
    cxn.ReceiveUpdateEnd();
    return true;
  }


  template <class EC>
  bool PacketIO::SendAtom(PacketTypeCode ptype,
                          CacheProcessor<EC> & cxn,
                          u16 siteNumber,
                          const typename EC::ATOM_CONFIG::ATOM_TYPE & atom)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c%c",ptype,siteNumber);
    Element<EC>::GetBits(atom).PrintBytes(m_buffer);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class EC>
  bool PacketIO::ReceiveAtom(CacheProcessor<EC> & cxn, ByteSource & bs)
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

    typename EC::ATOM_CONFIG::ATOM_TYPE atom;
    if (!Element<EC>::GetBits(atom).ReadBytes(bs))
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

  template <class EC>
  bool PacketIO::SendReply(u8 consistentCount, CacheProcessor<EC> & cxn)
  {
    m_buffer.Reset();
    m_buffer.Printf("%c%c", PacketType::UPDATE_ACK, consistentCount);
    return cxn.ShipBufferAsPacket(m_buffer);
  }

  template <class EC>
  bool PacketIO::ReceiveReply(CacheProcessor<EC> & cxn, ByteSource & bs)
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

  template <class EC>
  bool PacketIO::HandlePacket(CacheProcessor<EC> & cxn, PacketBuffer & buf)
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
