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

#include "PackedState.hh"
#include "Problem.hh"

using namespace std;

PackedState::PackedState(const Pos natomPositions[NUM_ATOMS]) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions[i] = natomPositions[i].fieldNumber();
}

PackedState::PackedState(const unsigned char natomPositions[NUM_ATOMS]) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions[i] = natomPositions[i];
}

PackedState::PackedState(const PackedState& state, const Move& move) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions[i] = state.atomPositions[i];
    atomPositions[move.atomNr()] = move.pos2().fieldNumber();
}

vector<Move> PackedState::moves() const {
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[atomPositions[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = DIRS[dirNr];
	    
	    Pos pos = atomPositions[i], tmppos = pos;
	    while (!isBlock[(tmppos += dir).fieldNumber()]) { }
	    Pos newpos = tmppos - dir;
	    if (newpos != pos)
		moves.push_back(Move(i, pos, newpos, dir));
	}
    }

    return moves;
}

vector<Move> PackedState::rmoves() const {
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[atomPositions[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = DIRS[dirNr];
	    if (!isBlock[atomPositions[i] - dir])
		continue;

	    Pos pos = atomPositions[i];
	    while (!isBlock[(pos += dir).fieldNumber()])
		moves.push_back(Move(i, atomPositions[i], pos, dir));
	}
    }

    return moves;
}

int PackedState::minMovesLeft() const {
    int minMovesLeft = 0;
    for (int i = 0; i < NUM_UNIQUE; ++i)
	minMovesLeft += Problem::goalDist(i, atomPositions[i]);
    for (int i = PAIRED_START; i < NUM_ATOMS; i += 2) {
	int moves1 = Problem::goalDist(i, atomPositions[i])
	    + Problem::goalDist(i + 1, atomPositions[i + 1]);
	int moves2 = Problem::goalDist(i, atomPositions[i + 1])
	    + Problem::goalDist(i + 1, atomPositions[i]);
	minMovesLeft += min(moves1, moves2);
    }

    return minMovesLeft;
}

bool PackedState::operator<(const PackedState& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (atomPositions[i] != other.atomPositions[i])
	    return atomPositions[i] < other.atomPositions[i];
    return false;
}
    
bool PackedState::operator==(const PackedState& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (atomPositions[i] != other.atomPositions[i])
	    return false;
    return true;
}

ostream& operator<<(ostream& out, const PackedState& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << Pos(state.atomPositions[i]) << ' ';

    return out;
}

// Seems to work ok, but needs more testing.
unsigned int PackedState::hash() const {
    unsigned int result = 0;

    for (int i = 0; i < NUM_ATOMS; ++i)
	result = 97 * result + atomPositions[i];

    return result;
}
