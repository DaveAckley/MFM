/*                                              -*- mode:C++ -*-
  WindowScanner.h Search tool which wraps EventWindows
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

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
  \file WindowScanner.h Search tool which wraps EventWindows
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef WINDOWSCANNER_H
#define WINDOWSCANNER_H

#include "Dirs.h"
#include "EventWindow.h"

namespace MFM
{
  /**
   * A wrapper for the EventWindow class which allows for EventWindow
   * searching and other powerful EventWindow modifications.
   */
  template <class CC>
  class WindowScanner
  {
   private:
    /* Extract short names for parameter types */
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };
    enum { SITES = EVENT_WINDOW_SITES(R) };
    enum { W = P::TILE_WIDTH };
    enum { B = P::ELEMENT_TABLE_BITS };

    /**
     * A reference to the EventWindow which an instance of this class
     * may read or write to.
     */
    EventWindow<CC>& m_win;

    /**
     * The Random reference held by \c m_win , pulled out for
     * convenience.
     */
    Random& m_rand;

    const u32 EMPTY_TYPE;

    enum
    {
      MOORE_NEIGHBORHOOD_SIZE = 8,
      VON_NEUMANN_NEIGHBORHOOD_SIZE = 4
    }NeighborhoodSize;

   public:
    /**
     * Constructs a new WindowScanner that is ready to be used on a
     * given EventWindow.
     *
     * @param window A reference to the EventWindow which the new
     *               instance of this class may read or write to.
     */
    WindowScanner(EventWindow<CC>& window);

    /**
     * Deconstructs this WindowScanner .
     */
    ~WindowScanner()
    { }

    /**
     * Replaces the Atom at a specified relative location from the
     * center Atom of the held EventWindow with another specified atom
     * if the existing atom is of a particular type.
     *
     * @param relative The location, relative to the center of the
     *                 held EventWindow, of the Atom to inspect and
     *                 perhaps replace.
     *
     * @param type The type of the Atom supposed to be in the \c
     *             relative location. If the Atom in \c relative is
     *             this \c type, it will be replaced with the specified Atom.
     *
     * @param atom The Atom to write to \c relative if the existing
     *             Atom is type \c type .
     *
     * @returns \c true if the atom at \c relative was replaced by \c
     *          atom , else \c false.
     */
    bool FillIfType(const SPoint& relative, const u32 type, const T& atom);

    /**
     * Replaces the Atom at a specified relative location from the
     * center Atom of the held EventWindow with another specified atom
     * if the existing atom is not of a particular type.
     *
     * @param relative The location, relative to the center of the
     *                 held EventWindow, of the Atom to inspect and
     *                 perhaps replace.
     *
     * @param type The type of the Atom supposed to be in the \c
     *             relative location. If the Atom in \c relative is
     *             not this \c type, it will be replaced with the
     *             specified Atom.
     *
     * @param atom The Atom to write to \c relative if the existing
     *             Atom is not type \c type .
     *
     * @returns \c true if the atom at \c relative was replaced by \c
     *          atom , else \c false.
     */
    bool FillIfNotType(const SPoint& relative, const u32 type, const T& atom);

    /**
     * Examines the held EventWindow at a given point and fills the
     * atom there with a specified Atom if it is occupied by an
     * Atom representing Element_Empty.
     *
     * @param relative The loaction of the held EventWindow to
     *                 examine.
     *
     * @param atom The Atom to place in \c relative if it is occupied
     *             by an Element_Empty Atom.
     *
     * @returns \c true if the Atom at the \c relative location
     *          represents Element_Empty and therefore will be
     *          replaced by \c atom , else \c false .
     */
    bool FillIfEmpty(const SPoint& relative, const T& atom);

    /**
     * Scans the held EventWindow up to a specified radius (not
     * including the center Atom) and sees whether or not an Atom of a
     * specified type exists.
     *
     * @param type The type of the Atom being scanned for.
     *
     * @param radius The maximum radius to expand this search to. This
     *               FAILs with ILLEGAL_ARGUMENT if this argument is
     *               either \c 0 or greater than the EventWindow
     *               radius.
     *
     * @returns \c true if an Atom of type \c type is found within \c
     *          radius of the center atom (discluding the center
     *          atom), else \c false .
     */
    bool CanSeeAtomOfType(const u32 type, const u32 radius) const;

    /**
     * Scans the held EventWindow up to a specified radius (not
     * including the center Atom) and counts the Atoms which are of a
     * specified type.
     *
     * @param type The type of the Atom being scanned for.
     *
     * @param radius The maximum radius to expand this search to. This
     *               FAILs with ILLEGAL_ARGUMENT if this argument is
     *               either \c 0 or greater than the EventWindow
     *               radius.
     *
     * @returns The number of Atoms of type \c type within \radius of
     *          the center atom (not including the center atom) in the
     *          held EventWindow.
     */
    u32 CountAtomsOfType(const u32 type, const u32 radius) const;

    /**
     * Scans the held EventWindow up to a specified radius (not
     * including the center Atom) and counts the Atoms which represent
     * Element_Empty
     *
     * @param radius The maximum radius to expand this search to. This
     *               FAILs with ILLEGAL_ARGUMENT if this argument is
     *               either \c 0 or greater than the EventWindow
     *               radius.
     *
     * @returns The number of Element_Empty Atoms within \radius of
     *          the center atom (not including the center atom) in the
     *          held EventWindow.
     */
    u32 CountEmptyAtoms(const u32 radius) const;

    /**
     * Scans a specified neighborhood consisting of Dirs directly
     * surrounding the center atom for an Atom of a specified type.
     *
     * @param type The type of the Atom to search for.
     *
     * @param neighborhood An array of Dirs representing the
     *                     neighborhood around the central Atom.
     *
     * @param dirCount The number of elements inside the \c
     *                 neighborhood array.
     *
     * @returns \c true if an Atom of type \c type is within the
     *          specified neighborhood of the center Atom of this
     *          EventWindow, else \c false .
     *
     * @remarks This is similar to \c CountInNeighborhood() \c > \c 0
     *          except that it returns more quickly if an Atom is
     *          found.
     */
    bool IsBorderingNeighborhood(const u32 type,
                                 const Dir* neighborhood,
                                 const u32 dirCount) const;

    /**
     * Scans the moore neighborhood (N, NE, E, SE, S, SW, W, NW)
     * directly surrounding the center atom for an Atom of a specified
     * type.
     *
     * @param type The type of the Atom to search for.
     *
     * @returns \c true if an Atom of type \c type is within the moore
     *          neighborhood of the center Atom of the held EventWindow,
     *          else \c false .
     */
    bool IsBorderingMoore(const u32 type) const;

    /**
     * Scans the Von Neumann neighborhood (N, E, S, W) directly
     * surrounding the center atom for an Atom of a specified type.
     *
     * @param type The type of the Atom to search for.
     *
     * @returns \c true if an Atom of type \c type is within the Von Neumann
     *          neighborhood of the center Atom of the held EventWindow,
     *          else \c false .
     */
    bool IsBorderingVonNeumann(const u32 type) const;


    /**
     * Scans a specified neighborhood consisting of Dirs directly
     * surrounding the center Atom and counts the Atoms of a specified
     * type.
     *
     * @param type The type of the Atoms to search for.
     *
     * @param neighborhood An array of Dirs representing the
     *                     neighborhood around the central Atom.
     *
     * @param dirCount The number of elements inside the \c
     *                 neighborhood array.
     *
     * @returns The number of Atoms in the specified \c neighborhood
     *          which are of type \c type.
     */
    u32 CountInNeighborhood(const u32 type,
                            const Dir* neighborhood,
                            const u32 dirCount) const;

    /**
     * Scans the moore neighborhood (N, NE, E, SE, S, SW, W, NW)
     * directly surrounding the center Atom and counts the Atoms of a
     * specified type.
     *
     * @param type The type of the Atom to search for.
     *
     * @returns The number of Atoms which are of type \c type in the
     *          moore neighborhood of the central atom.
     */
    u32 CountMooreNeighbors(const u32 type) const;

    /**
     * Scans the Von Neumann neighborhood (N, E, S, W) directly
     * surrounding the center Atom and counts the Atoms of a specified
     * type.
     *
     * @param type The type of the Atom to search for.
     *
     * @returns The number of Atoms which are of type \c type in the
     *          Von Neumann neighborhood of the central atom.
     */
    u32 CountVonNeumannNeighbors(const u32 type) const;

    /**
     * Scans the held EventWindow (discluding the center atom) for all
     * atoms of a specified type, filling a point with the location of
     * a single randomly chosen one.
     *
     * @param type The type of the atoms to search for and potentially
     *             randomly pick.
     *
     * @param outPoint An output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of the correct type is located. If this
     *                 method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms found in the held EventWindow of
     *          type \c type .
     */
    u32 FindRandomLocationOfType(const u32 type, SPoint& outPoint) const;

    /**
     * Scans a subset of the held EventWindow, up to a specified
     * radius, discluding the center Atom, for all Atoms of a
     * specified type, filling a given point with the location of a
     * single randomly chosen Atom of the specified type.
     *
     * @param type The type of the atoms to search for and potentially
     *             randomly pick.
     *
     * @param radius The maximum radius to search for atoms at. This
     *               must be greater than 0 and less than the radius
     *               of the held EventWindow, or this will FAIL with
     *               ILLEGAL_ARGUMENT .
     *
     * @param outPoint An output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of the correct type is located. If this
     *                 method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms found in the search radius of
     *          type \c type .
     */
    u32 FindRandomLocationOfType(const u32 type, const u32 radius, SPoint& outPoint) const;

    /**
     * Scans a subset of the held EventWindow, specified by a given
     * set of relative Points, for an Atom of a particular type,
     * placing the location of a randomly chosen Atom in an output Point.
     *
     * @param type The type of the Atom to search for in this subset.
     *
     * @param subWindow The array of relative Points to search through.
     *
     * @param subCount The length of the \c subWindow array.
     *
     * @param outPoint An output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of the correct type is located. If this
     *                 method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms found in the subWindow of type \c
     *          type .
     */
    u32 FindRandomInSubWindow(const u32 type, const SPoint* subWindow,
                              const u32 subCount, SPoint& outPoint) const;

    /**
     * Scans a subset of the held EventWindow, specified by a given
     * set of relative Points, for an Atom representing Element_Empty,
     * placing the location of a randomly chosen Atom in an output Point.
     *
     * @param subWindow The array of relative Points to search through.
     *
     * @param subCount The length of the \c subWindow array.
     *
     * @param outPoint An output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of the correct type is located. If this
     *                 method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms found in the subWindow which
     *          represent Element_Empty .
     */
    u32 FindEmptyInSubWindow(const SPoint* subWindow,
                             const u32 subCount, SPoint& outPoint) const;

    /**
     * Searches a specified neighborhood around the center atom of the
     * held EventWindow for an Atom of a specified type, giving its
     * location if found.
     *
     * @param type The type of the Atom to search for.
     *
     * @param dirs An array of Dirs specifying the neighborhood to
     *             search around the center Atom.
     *
     * @param dirCount The number of Dir elements inside of \c dirs .
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of \c type type is located. If this method
     *                 returns \c false , the contents of this SPoint
     *                 are undefined.
     *
     * @returns The number of Atoms in the center Atom's specified
     *          neighborhood of type \c type.
     */
    u32 FindRandomInNeighborhood(const u32 type,
                                 const Dir* dirs,
                                 const u32 dirCount,
                                 SPoint& outPoint) const;

    /**
     * Searches a specified neighborhood around the center atom of the
     * held EventWindow for an Atom representing Element_Empty ,
     * giving its location if found.
     *
     * @param dirs An array of Dirs specifying the neighborhood to
     *             search around the center Atom.
     *
     * @param dirCount The number of Dir elements inside of \c dirs .
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one representing Element_Empty is located. If
     *                 this method returns \c false , the contents of
     *                 this SPoint are undefined.
     *
     * @returns The number of Atoms in the center Atom's specified
     *          neighborhood representing Element_Empty
     */
    u32 FindEmptyInNeighborhood(const Dir* dirs,
                                const u32 dirCount,
                                SPoint& outPoint) const;

    /**
     * Searches the Moore neighborhood around the center atom of the
     * held EventWindow for an Atom of a specified type, giving its
     * location if found.
     *
     * @param type The type of the Atom to search for.
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of \c type type is located. If this method
     *                 returns \c 0 , the contents of this SPoint are
     *                 undefined.
     *
     * @returns The number of Atoms in the center Atom's Moore
     *          neighborhood of type \c type.
     */
    u32 FindRandomInMoore(const u32 type, SPoint& outPoint) const;

    /**
     * Searches the Von Neumann neighborhood around the center atom of
     * the held EventWindow for an Atom of a specified type, giving its
     * location if found.
     *
     * @param type The type of the Atom to search for.
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one of \c type type is located. If this method
     *                 returns \c 0 , the contents of this SPoint are
     *                 undefined.
     *
     * @returns The number of Atoms in the center Atom's Von Neumann
     *          neighborhood of type \c type.
     */
    u32 FindRandomInVonNeumann(const u32 type, SPoint& outPoint) const;

    /**
     * Searches the Moore neighborhood around the center atom of the
     * held EventWindow for an Atom representing Element_Empty ,
     * giving its location if found.
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one representing Element_Empty is located. If
     *                 this method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms in the center Atom's Moore
     *          neighborhood representing Element_Empty.
     */
    u32 FindEmptyInMoore(SPoint& outPoint) const;

    /**
     * Searches the Von Neumann neighborhood around the center atom of
     * the held EventWindow for an Atom representing Element_Empty ,
     * giving its location if found.
     *
     * @param outPoint an output parameter; The SPoint to fill with
     *                 the coordinates of a randomly chosen atom if
     *                 one representing Element_Empty is located. If
     *                 this method returns \c 0 , the contents of this
     *                 SPoint are undefined.
     *
     * @returns The number of Atoms in the center Atom's Von Neumann
     *          neighborhood representing Element_Empty.
     */
    u32 FindEmptyInVonNeumann(SPoint& outPoint) const;

    /**
     * Searches the held EventWindow under a specified radius for
     * different types of Atoms, giving the number of Atoms found and
     * the location of a randomly chosen one for each being searched
     * for.
     *
     * @param radius The radius of the search over the held
     *               EventWindow in manhattan distance. The center
     *               Atom is never included in this search.
     *
     * @param count The number of Atom - Type pairs being searched
     *              for.
     *
     * @param ... A variadic number of triplets specifying this
     *            particular search. Each triplet consists of three parts:
     *
     *            - SPoint* Output parameter, the location of a randomly chosen
     *                      Atom of its paired type if one is found.
     *
     *            - u32 The Type of the Atom to search for.
     *
     *            - u32* Output parameter, the number of Atoms of the paired type
     *                   encountered during the search.
     *
     */
    void FindRandomAtoms(const u32 radius, const u32 count, ...) const;

    /**
     * Searches the held EventWindow for different types of Atoms,
     * giving the number of Atoms found and the location of a randomly
     * chosen one for each being searched for.
     *
     * @param count The number of Atom - Type pairs being searched
     *              for.
     *
     * @param ... A variadic number of triplets specifying this
     *            particular search. Each triplet consists of three parts:
     *
     *            - SPoint* Output parameter, the location of a randomly chosen
     *                      Atom of its paired type if one is found.
     *
     *            - u32 The Type of the Atom to search for.
     *
     *            - u32* Output parameter, the number of Atoms of the paired type
     *                   encountered during the search.
     *
     */
    void FindRandomAtoms(const u32 count, ...) const;

    bool FindRandomAtomsInSubWindows(const u32 count, ...) const;

   private:

    void FindRandomAtoms(const u32 radius, const u32 count, va_list& list) const;
  };

  const Dir MooreNeighborhood[8] =
  {
    Dirs::NORTH, Dirs::NORTHEAST, Dirs::EAST, Dirs::SOUTHEAST,
    Dirs::SOUTH, Dirs::SOUTHWEST, Dirs::WEST, Dirs::NORTHWEST
  };

  const Dir VonNeumannNeighborhood[4] =
  {
    Dirs::NORTH, Dirs::EAST, Dirs::SOUTH, Dirs::WEST
  };
}

#include "WindowScanner.tcc"

#endif /* WINDOWSCANNER_H */
