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

#include <deque>
#include <iostream>
#include <vector>

#include "Dir.hh"
#include "IDAStar.hh"
#include "Problem.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

class IDAStarMove {
    friend class IDAStarState;
public:
    IDAStarMove(Pos np1, Pos np2) : p1(np1), p2(np2) { }

private:
    Pos p1, p2;
};

class IDAStarState {
    friend ostream& operator<<(ostream& out, const IDAStarState& state);
public:
    typedef int Field;
    static const Field EMPTY = -1;
    static const Field BLOCK = NUM_ATOMS;

    IDAStarState() { }		// leave uninitialized
    IDAStarState(int /* dummy */) {
	for (Pos pos = 0; pos != Pos::end(); ++pos) {
	    _fields[pos.fieldNumber()] = EMPTY;
	    if (Problem::isBlock(pos))
		_fields[pos.fieldNumber()] = BLOCK;
	}
	for (int i = 0; i < NUM_ATOMS; ++i) {
	    _positions[i] = Problem::startPosition(i);
	    _fields[Problem::startPosition(i).fieldNumber()] = i;
	}

	calcMinMovesLeft();
    }

    void apply(const IDAStarMove& move) {
	int atomNr = _fields[move.p1.fieldNumber()];
	DEBUG0("applying " << atomNr << ' ' << move.p1 << " -> " << move.p2);
	_fields[move.p2.fieldNumber()] = atomNr;
	_fields[move.p1.fieldNumber()] = EMPTY;
	_positions[atomNr] = move.p2;
	calcMinMovesLeft();
    }

    void undo(const IDAStarMove& move) {
	int atomNr = _fields[move.p2.fieldNumber()];
	_fields[move.p1.fieldNumber()] = atomNr;
	_fields[move.p2.fieldNumber()] = EMPTY;
	_positions[atomNr] = move.p1;
	calcMinMovesLeft();
    }

    Field field(Pos pos) const { return _fields[pos.fieldNumber()]; }
    bool isBlocking(Pos pos) const { return _fields[pos.fieldNumber()] != EMPTY; }
    Pos position(int atomNo) const { return _positions[atomNo]; }
    int minMovesLeft() const { return _minMovesLeft; }

private:
    void calcMinMovesLeft() {
	_minMovesLeft = 0;
	for (int i = 0; i < NUM_UNIQUE; ++i)
	    _minMovesLeft += Problem::goalDist(i, _positions[i]);
	for (int i = PAIRED_START; i < NUM_ATOMS; i += 2) {
	    int moves1 = Problem::goalDist(i, _positions[i])
		+ Problem::goalDist(i + 1, _positions[i + 1]);
	    int moves2 = Problem::goalDist(i, _positions[i + 1])
		+ Problem::goalDist(i + 1, _positions[i]);
	    _minMovesLeft += min(moves1, moves2);
	}
    }

    Field _fields[NUM_FIELDS];
    Pos _positions[NUM_ATOMS];
    int _minMovesLeft;
};

// global variables to describe current search state
static int maxMoves;
static int moves;
static IDAStarState state;
static deque<Move> solution;

static bool dfs();
deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    maxMoves = maxDist;
    moves = 0;
    state = IDAStarState(0);
    solution.clear();
    dfs();
    
    return solution;
}

static string spaces(int n) {
    return string(n, ' ');
}

static bool dfs() {
    DEBUG0(spaces(moves) << "dfs: moves =  " << moves << " state = " << state);
    if (state.minMovesLeft() == 0)
	return true;		// not true for all heuristics, but for this one

    int minTotalMoves = moves + state.minMovesLeft();
    if (minTotalMoves > maxMoves)
	return false;

    // generate all moves...
    for (int atomNo = 0; atomNo < NUM_ATOMS; ++atomNo) {
	Pos startPos = state.position(atomNo);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    DEBUG0(spaces(moves) << "moving " << atomNo << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !state.isBlocking(pos); pos += dir) { }
	    Pos newPos = pos - dir;
	    if (newPos != startPos) {
		DEBUG0(spaces(moves) << "moves to " << newPos);
		IDAStarMove move(startPos, newPos);
		state.apply(move);
		++moves;
		if (dfs()) {
		    solution.push_front(Move(atomNo, startPos, newPos, dir));
		    return true;
		}
		--moves;
		state.undo(move);
	    }
	}
    }

    return false;
}

ostream& operator<<(ostream& out, const IDAStarState& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << state._positions[i] << ' ';
    return out << state._minMovesLeft; 
}
