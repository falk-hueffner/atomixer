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

#ifndef REV_STATE_HH
#define REV_STATE_HH

#include "PackedState.hh"

// A state which exact goal distance was determined by reverse search from the
// goal state.
class RevState : public PackedState {
public:
    RevState() { }
    explicit RevState(const Pos atomPositions[NUM_ATOMS])
	: PackedState(atomPositions) { _goalDist = 0; }
    explicit RevState(const unsigned char atomPositions[NUM_ATOMS])
	: PackedState(atomPositions) { _goalDist = 0;}
    RevState (const RevState& state, const Move& move)
	: PackedState(state, move) { _goalDist = state._goalDist + 1; }

    int goalDist() const { return _goalDist; }
    bool better(const RevState& other) const { return _goalDist < other._goalDist; }
    void update(const RevState& other) { _goalDist = other._goalDist; }
private:
    char _goalDist;
};

#endif
