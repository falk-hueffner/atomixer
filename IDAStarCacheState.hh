/*
  atomixer -- Atomix puzzle solver
  Copyright (C) 2000 Falk Hueffner

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 Temple
  Place, Suite 330, Boston, MA 02111-1307 USA

  $Id$
*/

#ifndef IDASTARCACHESTATE_HH
#define IDASTARCACHESTATE_HH

#include "Pos.hh"
#include "State.hh"

// A compact representation for use in transposition tables in IDAStar. This
// could be implemented much nicer with a HashMap mapping from States to a
// pair of ints, but thay would waste a few (2) bytes. Perhaps I should do it
// anyway...

class IDAStarCacheState : public State {
public:
    // leave uninitialized
    IDAStarCacheState() { }
    //IDAStarCacheState(const unsigned char positions[NUM_ATOMS]) : State(positions) { }
    IDAStarCacheState(const unsigned char positions[NUM_ATOMS],
		      int nminMovesFromStart, int nminMovesLeft)
	: State(positions), minMovesFromStart(nminMovesFromStart),
	  minMovesLeft(nminMovesLeft) { }

    // This method will be called when "other" is inserted into the hash
    // table, and represents the same state as "*this".
    void update(const IDAStarCacheState& other) {
	if (other.minMovesFromStart < minMovesFromStart)
	    minMovesFromStart = other.minMovesFromStart;
	if (other.minMovesLeft > minMovesLeft)
	    minMovesLeft = other.minMovesLeft;
    }

    unsigned char minMovesFromStart;
    unsigned char minMovesLeft;
}
#ifdef HAVE_ATTRIBUTE_PACKED
    __attribute__ ((packed))
#endif
    ;

#endif
