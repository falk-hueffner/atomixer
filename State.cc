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

#include <iostream>

#include "Dir.hh"
#include "Problem.hh"
#include "State.hh"

using namespace std;

State::State(const Pos atomPositions[NUM_ATOMS]) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	myAtomPositions[i] = atomPositions[i].fieldNumber();
}

State::State(const State& state, const Move& move) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	myAtomPositions[i] = state.myAtomPositions[i];
    myAtomPositions[move.atomNr()] = move.pos2().fieldNumber();
}

vector<Move> State::moves() const {
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[myAtomPositions[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = dirs[dirNr];
	    
	    Pos pos = myAtomPositions[i], tmppos = pos;
	    while (!isBlock[(tmppos += dir).fieldNumber()]) { }
	    Pos newpos = tmppos - dir;
	    if (newpos != pos)
		moves.push_back(Move(i, pos, newpos, dir));
	}
    }

    return moves;
}

int State::minMovesLeft() const {
    int minMovesLeft = 0;
    for (int i = 0; i < NUM_ATOMS; ++i)
	minMovesLeft += Problem::goalDist(i, myAtomPositions[i]);

    return minMovesLeft;
}

bool State::operator<(const State& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (myAtomPositions[i] != other.myAtomPositions[i])
	    return myAtomPositions[i] < other.myAtomPositions[i];
    return false;
}
    
bool State::operator==(const State& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (myAtomPositions[i] != other.myAtomPositions[i])
	    return false;
    return true;
}

ostream& operator<<(ostream& out, const State& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << Pos(state.myAtomPositions[i]) << ' ';
    return out;
}
