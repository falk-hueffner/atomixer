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

#include <iostream>

#include "Problem.hh"

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

    IDAStarState(unsigned char atomPositions[NUM_ATOMS]) {
	for (Pos pos = 0; pos != Pos::end(); ++pos) {
	    _fields[pos.fieldNumber()] = EMPTY;
	    if (Problem::isBlock(pos))
		_fields[pos.fieldNumber()] = BLOCK;
	}
	for (int i = 0; i < NUM_ATOMS; ++i) {
	    _positions[i] = atomPositions[i];
	    _fields[atomPositions[i]] = i;
	}

	calcMinMovesLeft();	
    }

    void apply(const IDAStarMove& move) {
	int atomNr = _fields[move.p1.fieldNumber()];
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
    const Pos* positions() const { return _positions; }

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

#endif
