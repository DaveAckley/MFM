#ifndef ATOM_H
#define ATOM_H

#include "bitfield.h"
#include "point.h"
#include "eventwindow.h"

#define ATOM_SIZE 64

class EventWindow;
class Atom
{
protected:
  BitField<ATOM_SIZE> m_bits; 

public:

  Atom() { }

  ~Atom() { }

  void Update(EventWindow* window);

};

#endif /*ATOM_H*/
