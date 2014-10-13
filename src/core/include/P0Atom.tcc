/* -*- C++ -*- */

#include "MDist.h"
#include <stdio.h>

namespace MFM {

  template<class PC>
  P0Atom<PC>& P0Atom<PC>::operator=(const P0Atom<PC> & rhs)
  {
    if (this == &rhs) return *this;

    this->m_bits = rhs.m_bits;

    return *this;
  }
} /* namespace MFM */

