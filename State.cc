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

#include <algorithm>

#include "parameters.hh"
#include "Problem.hh"
#include "State.hh"

State::State(const Pos positions[NUM_ATOMS]) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions_[i] = positions[i].fieldNumber();
}

State::State(const unsigned char positions[NUM_ATOMS]) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions_[i] = positions[i];
}

State::State(const State& state, const Move& move) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	atomPositions_[i] = state.atomPositions_[i];

    apply(move);
}

void State::apply(const Move& move) {
    int atomNr = move.atomNr();
    atomPositions_[atomNr] = move.pos2().fieldNumber();

    canonicallify(atomNr);
}

void State::undo(const Move& move) {
    int atomNr = move.atomNr();
    atomPositions_[atomNr] = move.pos1().fieldNumber();

    canonicallify(atomNr);
}

int State::minMovesLeft() const {
    int minMovesLeft = 0;

    // 1. Unique atoms
    for (int i = 0; i < NUM_UNIQUE; ++i)
	minMovesLeft += Problem::goalDist(i, atomPositions_[i]);

    // 2. Atoms with 2 instances
    for (int i = PAIRED_START; i < PAIRED_END; i += 2) {
	int moves1 = Problem::goalDist(i, atomPositions_[i])
	    + Problem::goalDist(i + 1, atomPositions_[i + 1]);
	int moves2 = Problem::goalDist(i, atomPositions_[i + 1])
	    + Problem::goalDist(i + 1, atomPositions_[i]);
	minMovesLeft += min(moves1, moves2);
    }

    //3. Atoms with n, n>2 instances
    for (int i = MULTI_START; i < NUM_ATOMS; i += Problem::numIdentical(i)) {
	// More than 16 equal atoms would be too slow anyway.
	int perm[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
	int minMinMoves = 1000000;
	do {
	    int minMoves = 0;
	    for (int j = 0; j < Problem::numIdentical(i); ++j) {
		minMoves += Problem::goalDist(i + perm[j],
					      atomPositions_[i + j]);
	    }
	    if (minMoves < minMinMoves)
		minMinMoves = minMoves;
	} while (next_permutation(perm, perm + Problem::numIdentical(i)));
	minMovesLeft += minMinMoves;
    }

    return minMovesLeft;
}

bool State::operator==(const State& other) const {
    for (int i = 0; i < NUM_ATOMS; ++i)
	if (atomPositions_[i] != other.atomPositions_[i])
	    return false;
    return true;
}

#ifdef DO_BACKWARD_SEARCH
vector<Move> State::rmoves() const {
#else
vector<Move> State::moves() const {
#endif
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];

    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[atomPositions_[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = DIRS[dirNr];
	    
	    Pos pos = atomPositions_[i], tmppos = pos;
	    while (!isBlock[(tmppos += dir).fieldNumber()]) { }
	    Pos newpos = tmppos - dir;
	    if (newpos != pos)
		moves.push_back(Move(i, pos, newpos, dir));
	}
    }

    return moves;
}

#ifdef DO_BACKWARD_SEARCH
vector<Move> State::moves() const {
#else
vector<Move> State::rmoves() const {
#endif
    vector<Move> moves;
    moves.reserve(NUM_ATOMS * 3);

    bool isBlock[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; ++i)
	isBlock[i] = Problem::isBlock(Pos(i));
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlock[atomPositions_[i]] = true;

    for (int i = 0; i < NUM_ATOMS; ++i) {
	for (int dirNr = 0; dirNr < 4; ++dirNr) {
	    Dir dir = DIRS[dirNr];
	    if (!isBlock[atomPositions_[i] - dir])
		continue;

	    Pos pos = atomPositions_[i];
	    while (!isBlock[(pos += dir).fieldNumber()])
		moves.push_back(Move(i, atomPositions_[i], pos, dir));
	}
    }

    return moves;
}

size_t State::hash() const {
    size_t result = 0;

    // Perl uses 33; try that eventually, might be more magic.
    for (int i = 0; i < NUM_ATOMS; ++i)
	result = 97 * result + atomPositions_[i];

    return result;
}

// canonicallify pairs: the first one should always have the lower
// position. This avoids storing logically identical states twice in the
// hash table.
void State::canonicallify(int atomNr) {
    if (atomNr >= PAIRED_START && atomNr < PAIRED_END) {
	if ((atomNr - PAIRED_START) % 2 == 0) {
	    if (atomPositions_[atomNr + 1] < atomPositions_[atomNr])
		swap(atomPositions_[atomNr + 1], atomPositions_[atomNr]);
	} else {
	    if (atomPositions_[atomNr - 1] > atomPositions_[atomNr])
		swap(atomPositions_[atomNr - 1], atomPositions_[atomNr]);
	}
    } else if (atomNr >= MULTI_START) {
	// Bubble sort the changed element to the correct position.
	// Slightly inefficient, swapping all the time. But easy to read, and
	// numIdentical is usually small, like 3.
	for (int i = atomNr - 1;
	     i >= Problem::firstIdentical(atomNr)
		 && atomPositions_[i] > atomPositions_[i + 1]; --i)
	    swap(atomPositions_[i + 1], atomPositions_[i]);
	for (int i = atomNr + 1;
	     i < Problem::firstIdentical(atomNr) + Problem::numIdentical(atomNr)
		 && atomPositions_[i - 1] > atomPositions_[i]; ++i)
	    swap(atomPositions_[i - 1], atomPositions_[i]);
    }
}

std::ostream& operator<<(std::ostream& out, const State& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << Pos(state.atomPositions_[i]) << ' ';

    return out << state.minMovesLeft();
}
