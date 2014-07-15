#include "Rect.h"

namespace MFM
{

  void Rect::IntersectWith(const Rect & other)
  {
    SPoint startInt = max(this->m_position, other.m_position);

    SPoint endThis = m_position + MakeSigned(m_size);
    SPoint endOthr = other.m_position + MakeSigned(other.m_size);
    SPoint endInt = max(startInt, min(endThis,endOthr));

    m_position = startInt;
    m_size = MakeUnsigned(endInt-startInt);
  }

} /* namespace MFM */
