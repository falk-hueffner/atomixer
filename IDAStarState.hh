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

#ifndef IDASTARSTATE_HH
#define IDASTARSTATE_HH

#include "Pos.hh"
#include "Problem.hh"
#include "State.hh"

// For an IDAStarState, memory is not a concern; there is only a single
// instance being used in IDAStar. So we can:
// * cache minMovesLeft
// * keep a matrix of blocked fields for faster move generation

class IDAStarState : public State {
public:
    // leave uninitialized
    IDAStarState() { }
    IDAStarState(const State& state)
	: State(state), moves_(0) {
	for (Pos pos = 0; pos != Pos::end(); ++pos)
	    isBlocking_[pos.fieldNumber()] = Problem::isBlock(pos);
	for (int i = 0; i < NUM_ATOMS; ++i)
	    isBlocking_[atomPosition(i)] = true;
	
	calcMinMovesLeft();
    }

    // these override methods from State
    int moves() const { return moves_; }
    int minMovesLeft() const { return minMovesLeft_; }
    int minTotalMoves() const { return moves_ + minMovesLeft_; }

    void apply(const Move& move) {
	State::apply(move);
	isBlocking_[move.pos1().fieldNumber()] = false;
	isBlocking_[move.pos2().fieldNumber()] = true;
	calcMinMovesLeft();
	++moves_;
    }
    void apply(const Move& move, int minMovesLeft) {
	State::apply(move);
	isBlocking_[move.pos1().fieldNumber()] = false;
	isBlocking_[move.pos2().fieldNumber()] = true;
	minMovesLeft_ = minMovesLeft;
	++moves_;
    }
    void undo(const Move& move, int minMovesLeft) {
	State::undo(move);
	isBlocking_[move.pos1().fieldNumber()] = true;
	isBlocking_[move.pos2().fieldNumber()] = false;
	minMovesLeft_ = minMovesLeft;
	--moves_;
    }

    bool isBlocking(Pos pos) const { return isBlocking_[pos.fieldNumber()]; }

private:
    void undo(const Move& move); // shouldn't be used

    void calcMinMovesLeft() { minMovesLeft_ = State::minMovesLeft(); }
    unsigned int moves_;
    unsigned int minMovesLeft_;
    bool isBlocking_[NUM_FIELDS]; // FIXME try whether char is faster
};

#endif
