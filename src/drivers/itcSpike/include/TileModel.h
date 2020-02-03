/*  -*- mode:C++ -*- */
/*
  TileModel.h Tile spike for ITC level sequencing
  Copyright (C) 2020 The T2 Tile Project.  All rights reserved.

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
  \file TileModel.h Tile spike for ITC level sequencing
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \lgpl
 */
#ifndef TILEMODEL_H
#define TILEMODEL_H

#include "itype.h"
#include "Random.h"
#include "Fail.h"
#include "Logger.h"
#include "OverflowableCharBufferByteSink.h" // For OString128

namespace MFM
{
  /**
   * TileModel manages and provides access to the T2 locking hardware
   */
  class TileModel
  {
  private:
    Random& mRandom;
    u32 mLevel;
    u32 mGeneration;
    OString128 mMFZVersionString;
    
  public:

    TileModel(Random& random) ;

    ~TileModel() ;

    void update() ;

    u32 getLevel() const ;

    u32 getGeneration() const ;

    const char * getMFZVersionString() const ;

  };

}

#endif /* TILEMODEL_H */
