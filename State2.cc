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
#include <algorithm>

#include "Dir.hh"
#include "Problem.hh"
#include "State2.hh"

using namespace std;

State2::State2(const Pos natomPositions[NUM_ATOMS]) {
    numMoves = 0;
    isOpen = true;
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions[i] = natomPositions[i].fieldNumber();
    calcMinMovesLeft();
}

State2::State2(const State2& state, const Move& move) {
    numMoves = state.numMoves + 1;
    isOpen = true;
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions[i] = state.atomPositions[i];
    atomPositions[move.atomNr()] = move.pos2().fieldNumber();
    calcMinMovesLeft();
}

vector<Move> State2::moves() const {
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[atomPositions[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = dirs[dirNr];
	    
	    Pos pos = atomPositions[i], tmppos = pos;
	    while (!isBlock[(tmppos += dir).fieldNumber()]) { }
	    Pos newpos = tmppos - dir;
	    if (newpos != pos)
		moves.push_back(Move(i, pos, newpos, dir));
	}
    }

    return moves;
}

void State2::calcMinMovesLeft() {
    _minMovesLeft = 0;
    for (int i = 0; i < NUM_UNIQUE; ++i)
	_minMovesLeft += Problem::goalDist(i, atomPositions[i]);
    for (int i = PAIRED_START; i < NUM_ATOMS; i += 2) {
	int moves1 = Problem::goalDist(i, atomPositions[i])
	    + Problem::goalDist(i + 1, atomPositions[i + 1]);
	int moves2 = Problem::goalDist(i, atomPositions[i + 1])
	    + Problem::goalDist(i + 1, atomPositions[i]);
	_minMovesLeft += min(moves1, moves2);
    }
}

bool State2::operator<(const State2& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (atomPositions[i] != other.atomPositions[i])
	    return atomPositions[i] < other.atomPositions[i];
    return false;
}
    
bool State2::operator==(const State2& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (atomPositions[i] != other.atomPositions[i])
	    return false;
    return true;
}

ostream& operator<<(ostream& out, const State2& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << Pos(state.atomPositions[i]) << ' ';
    out << '(' << state.numMoves
	<< '/' << state.numMoves + state.minMovesLeft() << ')';
    if (state.isOpen)
	out << " *";

    return out;
}

static const int NUM_WORDS = (NUM_ATOMS / sizeof(size_t));// * sizeof(size_t);
static const size_t REST_MASK
	= (((size_t) 1) << ((NUM_ATOMS - (NUM_WORDS * sizeof(size_t))) * 8)) - 1;

/*
size_t State2::hash() const {
    size_t result = 0;
    size_t* words = (size_t*) &atomPositions;

    for (int i = 0; i < NUM_WORDS; ++i)
	result ^= words[i];

    if (REST_MASK != 0)
	result ^= words[i] & REST_MASK;

    if (result == 0) {
	return 1;
    } else {
	return result;
    }
}
*/

size_t State2::hash() const {
    size_t result = 0;

    for (int i = 0; i < NUM_ATOMS; ++i)
	result = 97 * result + atomPositions[i];

    if (result == 0) {
	return 1;
    } else {
	return result;
    }
}
