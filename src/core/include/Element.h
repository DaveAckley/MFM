/*                                              -*- mode:C++ -*-
  Element.h Base of all MFM elemental types
  Copyright (C) 2014,2016 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element.h Base of all MFM elemental types
  \author Trent R. Small.
  \author David H. Ackley.
  \date (C) 2014, 2016 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_H
#define ELEMENT_H

#include "Atom.h"
#include "Site.h"
#include "Parameter.h"
#include "itype.h"
#include "ElementTypeNumberMap.h"
#include "UUID.h"
#include "Dirs.h"
#include "Logger.h"
#include "BitField.h"

namespace MFM
{

  template <class EC> class EventWindow; // FORWARD
  template <class EC> class ElementTable; // FORWARD
  template <class EC> class UlamElement; // FORWARD

  /**
   * A standard basis for specifying degrees of diffusability.
   */
  static const u32 COMPLETE_DIFFUSABILITY = 1000;

  /**
   * An Element describes how a given type of Atom behaves.
   */
  template <class EC>
  class Element
  {
   private:
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { R = EC::EVENT_WINDOW_RADIUS };
    enum { BPA = AC::BITS_PER_ATOM };
    enum { ELEMENT_EMPTY_TYPE = T::ATOM_EMPTY_TYPE };

    /**
     * The configurable element parameters of this Element . Element
     * parameters apply globally to all atoms of a given Element.
     */
    ElementParameters<EC> m_elementParameters;

    /**
     * The configurable atomic parameters of this Element . Atomic
     * parameters apply individually to each atom of a given Element.
     */
    AtomicParameters<EC> m_atomicParameters;

    /**
     * The unique UUID of this Element . Each Element must be given
     * one upon construction.
     */
    const UUID m_UUID;

    /**
     * The type shared by every Atom of this Element.
     */
    u32 m_type;

    /**
     * A flag indicating whether or not the type of this Atom (held by
     * \c m_type ) is a valid type.
     */
    bool m_hasType;

    /**
     * A flag used when rendering every Atom of this Element which
     * determines which color to use during rendering.
     */
    bool m_renderLowlight;

    /**
     * The basic, most generic Atom of this Element to be used when
     * placing a new Atom.
     */
    T m_defaultAtom;

    /**
     * The Atomic Symbol of this Element, i.e. a One or Two character
     * string representing a very short name for this Element.
     */
    const char* m_atomicSymbol;

    /**
     * The English name of this Element.
     */
    const char* m_name;

   public:

    /**
     * Get the smallest event window radius that this element does \e
     * NOT need to access during an event.  The default implementation
     * returns R + 1, indicating the full window from 0..R may be
     * accessed.  However, elements can override this method to
     * declare a smaller boundary if they wish, which will allow them
     * to execute closer to tile edges without locking -- and thus
     * tend to increase their effective event rate.  In addition,
     * declaring a smaller event boundary reduces overall event
     * processing costs and may, down the road, be granted some
     * particular advantage (such as increased write or hold power) in
     * exchange.  Any attempt to access the event window at the
     * boundary distance or greater will fail and cause the offending
     * atom to be erased.
     *
     * The Empty element, uniquely, declares a boundary of 0, meaning
     * it doesn't even need to access itself.  Utterly passive
     * elements like Wall can declare a boundary of 1, allowing access
     * to \e only themselves.  Similarly, von Neumann neighborhood
     * elements can declare a boundary of 2, while Moore neighborhood
     * elements must declare a boundary of at least 3.
     *
     * @returns The smallest event window radius this Element will
     * never access.  Return values larger than R + 1 are treated as R
     * + 1.  
     */
    virtual u32 GetEventWindowBoundary() const
    {
      return R + 1;
    }

    /**
     * Describes how the default Atom of this Element should be
     * constructed. The default behavior of this method builds an atom
     * with the type of this Element and otherwise empty fields.
     *
     * @returns The default Atom of this Element, to be stored in \c m_defaultAtom .
     */
    virtual T BuildDefaultAtom() const
    {
      T defaultAtom(this->GetType(), 0, 0, 0);
      return defaultAtom;
    }

    /**
     * Retrieves the read-only BitVector held inside a specified Atom .
     *
     * @param atom The Atom to retrieve the held BitVector from.
     *
     * @returns The BitVector held inside \c atom .
     */
    static const BitVector<BPA> & GetBits(const T & atom)
    {
      return atom.m_bits;
    }

    /**
     * Retrieves the writable BitVector held inside a specified Atom .
     *
     * @param atom The Atom to retrieve the held BitVector from.
     *
     * @returns The BitVector held inside \c atom .
     */
    static BitVector<BPA> & GetBits(T & atom)
    {
      return atom.m_bits;
    }

    /**
     * Checks to see if a specified Atomic Symbol is a valid symbol. A
     * valid symbol must be either a single capital letter, or a
     * capital letter followed by a single lowercase letter.
     *
     * @param symbol The string to check symbolic validity of.
     *
     * @returns \c true If \c symbol is a valid atomic symbol, else \c
     *          false .
     */
    bool IsValidAtomicSymbol(const char* symbol)
    {
      MFM_API_ASSERT_NONNULL(symbol);

      switch (strlen(symbol))
      {
      case 2:
        if (!(symbol[1] >= 'a' && symbol[1] <= 'z')
            && !(symbol[1] >= 'A' && symbol[1] <= 'Z')  // v2.1: allow two uppercase
            && !(symbol[1] >= '0' && symbol[1] <= '9')) // v2.1: and one digit
        {
          return false;
        }
        // FALL THROUGH

      case 1:
        if (symbol[0] >= 'A' && symbol[0] <= 'Z')
        {
          return true;
        }
        // FALL THROUGH

      default:
        return false;
      }
    }

  protected:
    /**
     * Sets the Atomic Symbol of this Element, which is mainly used
     * during the rendering of this Element .
     *
     * @param symbol The string representing the Atomic Symbol of this
     *               Element. If this symbol is not valid (in terms of
     *               the \c IsValidAtomicSymbol() method), this will
     *               FAIL with ILLEGAL_ARGUMENT .
     */
    void SetAtomicSymbol(const char* symbol)
    {
      /* Single or double letters only, like the periodic table. */
      MFM_API_ASSERT_ARG(IsValidAtomicSymbol(symbol));
      m_atomicSymbol = symbol;
    }

    /**
     * Sets the English name of this Element which is used for
     * inspection purposes.
     *
     * @param name The new English name of this Element .
     */
    void SetName(const char* name)
    {
      m_name = name;
    }

   public:

    /**
     * Constructs a new Element with a given UUID .
     *
     * @param uuid The UUID of this Element, used for various
     *              operations. It is vital that this UUID be unique,
     *              but this constructor does not check for that.
     */
    Element(const UUID & uuid) : m_UUID(uuid), m_type(0),
                                 m_hasType(false),
                                 m_renderLowlight(false),
                                 m_atomicSymbol("!!"),
                                 m_name("UNNAMED")
    {
      LOG.Debug("Constructed %@ at %p", &m_UUID, this);
    }

    void AllocateTypeForTesting(ElementTypeNumberMap<EC>& etnm)
    {
      m_hasType = false;
      AllocateType(etnm);
    }

    /**
     * Assigns the type of this Element using the supplied
     * ElementTypeNumberMap . This type is only assigned if it has not
     * been assigned already. Once this type has been allocated, the
     * default Atom of this Element is constructed and an Atom of this
     * Element may be placed.
     *
     */
    void AllocateType(ElementTypeNumberMap<EC>& etnm)
    {
      if (!m_hasType)
      {
        m_type = etnm.AllocateType(m_UUID);
        m_hasType = true;
        m_defaultAtom = BuildDefaultAtom();
      }
    }

    /**
     * Declare that this element is the Empty element, and assign it
     * the specific type reserved for that element.
     *
     */
    void AllocateEmptyType()
    {
      if (!m_hasType)
      {
        m_type = ELEMENT_EMPTY_TYPE;
        m_hasType = true;
        m_defaultAtom = BuildDefaultAtom();
      }
    }

    /**
     * Gets the unique type of this Element . If the type has not been
     * assigned yet (by using \c AllocateType() ), this will FAIL with
     * ILLEGAL_STATE .
     *
     * @returns The unique type of this Element .
     */
    u32 GetType() const
    {
      MFM_API_ASSERT_STATE(m_hasType);
      return m_type;
    }

    void MakeAlternateNameForTestingInternal(const Element & asThis)
    {
      MFM_API_ASSERT_STATE(m_hasType);
      MFM_API_ASSERT_ARG(asThis.m_hasType);
      MFM_API_ASSERT_STATE(GetUUID() == asThis.GetUUID());
      m_type = asThis.GetType();
      m_hasType = true;
      m_defaultAtom = BuildDefaultAtom();
    }

    /**
     * Gets the Atomic Symbol of this Element . If this has not been
     * set, the default Atomic Symbol is the invalid symbol "!!" .
     *
     * @returns The Atomic Symbol of this Element .
     */
    const char* GetAtomicSymbol() const
    {
      return m_atomicSymbol;
    }

    /**
     * Gets the English name of this Element , which is normally used
     * for display purposes. The default is the string "UNNAMED" .
     *
     * @returns The English name of this Element .
     */
    const char* GetName() const
    {
      return m_name;
    }

    /**
     * Appends a short description of the data held by an Atom of this
     * Element.
     *
     * @param atom A pointer to an Atom (guaranteed to be an instance
     *             of this Element ) to append a description of.
     *
     * @param desc The ByteSink to append the description to.
     */
    virtual void AppendDescription(const T& atom, ByteSink& desc) const
    {
      /* Default to no description */
    }

    /**
     * Checks to see if this Element is of a specified type.
     *
     * @param type The \c type of which to check against they type of
     *             this Element.
     *
     * @returns \c true if this Element is of \c type type, else \c
     *          false.
     */
    bool IsType(u32 type) const
    {
      return GetType() == type;
    }

    /**
     * Gets the UUID of this Element .
     *
     * @returns The UUID of this Element.
     */
    const UUID & GetUUID() const
    {
      return m_UUID;
    }

    /**
     * Describes the behavior of this Element. This is invoked when an
     * Atom of this Element is chosen as the center of an EventWindow
     * for an Event, which is given as a parameter.
     *
     * @param window The EventWindow describing the Event which is
     * currently being executed.
     */
    virtual void Behavior(EventWindow<EC>& window) const = 0;

    /**
       Downcast an Element pointer to an UlamElement pointer, if
       possible.
     */
    virtual const UlamElement<EC> * AsUlamElement() const
    {
      return 0;
    }

    virtual UlamElement<EC> * AsUlamElement()
    {
      return 0;
    }

    /**
     * Gets the default Atom of this Element . If this Element has not
     * been assigned a type through \c AllocateType() , this will FAIL
     * with ILLEGAL_STATE.  NOTE: The defaultAtom returned here will
     * have been constructed by Element::BuildDefaultAtom, which
     * leaves all state bits zero.  If this is not desirable, element
     * subclasses should override this to provide their own default
     * atom initialization.
     *
     * @returns The default Atom of this Element.
     * @sa BuildDefaultAtom
     */
    virtual const T & GetDefaultAtom() const
    {
      MFM_API_ASSERT_STATE(m_hasType);
      return m_defaultAtom;
    }

    /**
     * Gets the 32-bit ARGB color that represents this Element,
     * independent of any atomic details, possibly modified by
     * user-requested lowlighting etc.
     *
     */
    u32 GetStaticColor() const
    {
      u32 baseColor = this->GetElementColor();

      if(m_renderLowlight)
      {
        baseColor = DarkenColor(baseColor, 50);
      }

      return baseColor;
    }


    /**
     * Gets a string of a short description of the behavior of this
     * Element . This will be displayed in some way when a user needs
     * help with this Element . This string must not include newlines;
     * they will be added when they are needed.
     *
     * @returns A helpful description string briefly describing the
     *          behavior of this Element.
     */
    virtual const char* GetDescription() const
    {
      return "NO DESCRIPTION";
    }

    /**
     * Gets the 32-bit ARGB formatted color represents this element,
     * independent of any atomic details.
     *
     * @returns The 32-bit ARGB formatted color that represents this
     * element.
     */
    virtual u32 GetElementColor() const = 0;

    /**
     * Toggles the lowlight / normal rendering of all Atoms of this
     * Element.
     */
    void ToggleLowlightPhysicsColor()
    {
      m_renderLowlight = !m_renderLowlight;
    }

    /**
     * For use during rendering, return the color to render a specific
     * Atom of this Element.  The default base class implementation
     * returns GetElementColor() for all atoms.  This should be
     * overridden if wanting to use a gradient or some other variable
     * color based on the body of the specified Atom .
     *
     * @param atom The atom of this Element, of which its current
     *             color is to be found.
     *
     * @param selector An additional argument which may be used to
     *                 determine the color of which to render \c atom.
     *                 Values in the range of 1..3 are defined; a
     *                 value of 0 should be treated as requesting the
     *                 GetElementColor() .
     *
     * @returns The 32-bit ARGB color of which to render \c atom with.
     */
    virtual u32 GetAtomColor(const ElementTable<EC> & et, const T& atom, u32 selector) const
    {
      return GetElementColor();
    }

    /**
     * Gets the 32-bit ARGB color that represents a specific atom of
     * this Element, possibly computed on the fly, and possibly
     * modified by user-requested lowlighting etc.
     *
     */
    u32 GetDynamicColor(const ElementTable<EC> & et, const T& atom, u32 selector) const
    {
      u32 baseColor = this->GetAtomColor(et, atom, selector);

      if(m_renderLowlight)
      {
        baseColor = DarkenColor(baseColor, 50);
      }

      return baseColor;
    }

    /**
     * On entry, the Atom at \a nowAt will be an instance of the type
     * of this Element.  How much does that atom like the idea that it
     * should be moved to (or remain at; the two SPoints might be
     * equal) location \a maybeAt?  Return 0 to flat-out veto the
     * idea, or from 0 up to COMPLETE_DIFFUSABILITY to request that
     * fraction of diffusability.  (Values larger than
     * COMPLETE_DIFFUSABILITY are treated as equal to it.)  Note that
     * \a nowAt and \a maybeAt are both relative to the EventWindow \a
     * ew, but neither is necessarily the center atom!
     *
     * By default all atoms are considered diffusable, and they return
     * COMPLETE_DIFFUSABILITY chances for every suggested move
     */
    virtual u32 Diffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return COMPLETE_DIFFUSABILITY;
    }

    /**
       A utility method available to subclasses that specifies no
       diffusion is allowed, by specifying a 0 value for the
       Diffusability of every position except when \a nowAt == \a
       maybeAt.
    */
    u32 NoDiffusability(EventWindow<EC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return (nowAt == maybeAt)? COMPLETE_DIFFUSABILITY : 0;
    }

    const ElementParameters<EC> & GetElementParameters() const
    {
      return m_elementParameters;
    }

    ElementParameters<EC> & GetElementParameters()
    {
      return m_elementParameters;
    }

    const AtomicParameters<EC> & GetAtomicParameters() const
    {
      return m_atomicParameters;
    }

    AtomicParameters<EC> & GetAtomicParameters()
    {
      return m_atomicParameters;
    }
  };
}

#include "Element.tcc"

#endif /* ELEMENT_H */
