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

void State::apply(const Move& move) {
    int atomNr = move.atomNr();
    atomPositions_[atomNr] = move.pos2().fieldNumber();
}

void State::undo(const Move& move) {
    int atomNr = move.atomNr();
    atomPositions_[atomNr] = move.pos1().fieldNumber();
}

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

int State::rminMovesLeft() const {
    int minMovesLeft = 0;

    // 1. Unique atoms
    for (int i = 0; i < NUM_UNIQUE; ++i)
	minMovesLeft += Problem::rgoalDist(i, atomPositions_[i]);

    // 2. Atoms with 2 instances
    for (int i = PAIRED_START; i < PAIRED_END; i += 2) {
	int moves1 = Problem::rgoalDist(i, atomPositions_[i])
	    + Problem::rgoalDist(i + 1, atomPositions_[i + 1]);
	int moves2 = Problem::rgoalDist(i, atomPositions_[i + 1])
	    + Problem::rgoalDist(i + 1, atomPositions_[i]);
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
		minMoves += Problem::rgoalDist(i + perm[j],
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

vector<Move> State::moves() const {
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

vector<Move> State::rmoves() const {
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

    for (int i = 0; i < NUM_ATOMS; ++i)
	result = 97 * result + atomPositions_[i];

    return result;
}

// "one-at-a-time hash"
size_t State::hash2() const {
    size_t hash = 0;
    for (int i = 0; i < NUM_ATOMS; ++i) {
	hash += atomPositions_[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

uint64_t State::hash64_1() const {
    /*
    uint64_t perl = 0;
    for (int i = 0; i < NUM_ATOMS; ++i)
	perl = 33 * perl + atomPositions_[i];
    perl += (perl >> 5);
    
    uint64_t python = atomPositions_[0] << 7;
    for (int i = 0; i < NUM_ATOMS; ++i)
	python = (1000003 * python) ^ atomPositions_[i];
    */
    // "one-at-a-time hash"
    uint64_t hash = 0;
    for (int i = 0; i < NUM_ATOMS; ++i) {
	hash += atomPositions_[i];
	hash += (hash << 10);
	hash ^= (hash >> 6);
    }
    
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

uint64_t State::hash64_2() const {
    uint64_t python = atomPositions_[0] << 7;
    for (int i = 0; i < NUM_ATOMS; ++i)
	python = (1000003 * python) ^ atomPositions_[i];

    return python;
}
 
inline std::ostream& operator<<(std::ostream& out, const State& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << Pos(state.atomPositions_[i]) << ' ';

    return out << state.minMovesLeft();
}
