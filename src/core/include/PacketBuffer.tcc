/* -*- C++ -*- */
namespace MFM
{
  template <class T>
  PacketBuffer<T>::PacketBuffer() :
    m_heldPackets(0),
    m_packetCapacity(PACKETBUFFER_SIZE / sizeof(Packet<T>))
  { }

  template <class T>
  void PacketBuffer<T>::PushPacket(Packet<T>& packet)
  {
    if(m_heldPackets + 1 >= m_packetCapacity)
    {
      FAIL(OUT_OF_ROOM);
    }

    Packet<T>& writePack = m_buffer[m_heldPackets++];

    writePack.SetType(packet.GetType());
    writePack.SetAtom(packet.GetAtom());
    writePack.SetReceivingNeighbor(packet.GetReceivingNeighbor());
    writePack.SetLocation(packet.GetLocation());
  }

  template <class T>
  Packet<T>* PacketBuffer<T>::PopPacket()
  {
    if(!m_heldPackets)
    {
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    }

    return m_buffer + (--m_heldPackets);
  }
} /* namespace MFM */
