
namespace MFM {
template <class T>
Packet<T>::Packet(PacketType type)
{
  m_type = type;
}

template <class T>
void Packet<T>::SetAtom(T& atom)
{
  m_atom = atom;
}

template <class T>
T& Packet<T>::GetAtom()
{
  return m_atom;
}

template <class T>
void Packet<T>::SetLocation(const SPoint& fromPt)
{
  m_edgeLoc = fromPt;
}

template <class T>
const SPoint& Packet<T>::GetLocation() const
{
  return m_edgeLoc;
}
} /* namespace MFM */

