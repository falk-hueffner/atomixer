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

#ifndef ASTARSTATE_HH
#define ASTARSTATE_HH

class Move;

#include "CacheState.hh"

class AStarState : public CacheState {
    friend std::ostream& operator<<(std::ostream& out, const AStarState& state);
public:
    AStarState() { }		// leave uninitialized
    AStarState(const State& state);
    AStarState(const AStarState& state, const Move& move);

    // overrides State::minMovesLeft()!
    int minMovesLeft() const { return minMovesLeft_; }
    // overrides State::minTotalMoves()!
    int minTotalMoves() const { return numMoves + minMovesLeft_; }

    int predecessor;

private:
    void calcMinMovesLeft() { minMovesLeft_ = State::minMovesLeft(); }
    unsigned char minMovesLeft_;

public:				// for simplicity
    unsigned int numMoves	: 7;
    bool isOpen			: 1;
}
#ifdef HAVE_ATTRIBUTE_PACKED
    __attribute__ ((packed))
#endif
    ;

#endif
