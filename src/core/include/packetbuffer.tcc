
namespace MFM {
template <class T>
PacketBuffer<T>::PacketBuffer()
{
  m_heldPackets = 0;
}

template <class T>
void PacketBuffer<T>::PushPacket(Packet<T>& packet)
{
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
    FAIL(INCOMPLETE_CODE);
  }

  return m_buffer + (--m_heldPackets);
}
} /* namespace MFM */

