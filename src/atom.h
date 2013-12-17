#ifndef ATOM_H
#define ATOM_H

#include "point.h"
#include "eventwindow.h"

class EventWindow;
class Atom
{
protected:
  unsigned char m_type;

public:

  Atom(unsigned char type) {m_type = type;}

  Atom() { }

  ~Atom() { }

  virtual void Update(EventWindow* window) = 0;

};

#endif /*ATOM_H*/
