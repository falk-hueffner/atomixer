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

#ifndef STATE2_HH
#define STATE2_HH

#include <iosfwd>
#include <vector>

class Problem;

#include "Move.hh"

//#define DO_REVERSE_SEARCH 1
#undef DO_REVERSE_SEARCH

#ifdef DO_REVERSE_SEARCH
// max. distance from goal for bidirectional search
static const int REV_SEARCH_MAX_GOAL_DIST = 6;
#endif

using namespace std;

class State2 {
    friend ostream& operator<<(ostream& out, const State2& state);

public:
    State2() { }		// leave uninitialized
    explicit State2(const Pos atomPositions[NUM_ATOMS]);
    State2(const State2& state, const Move& move);

    bool operator<(const State2& other) const; // to put into STL set
    bool operator==(const State2& other) const;
    size_t hash() const;

    vector<Move> moves() const;
    vector<Move> rmoves() const;
    void calcMinMovesLeft();
    int minMovesLeft() const { return _minMovesLeft; }
    int minTotalMoves() const { return numMoves + minMovesLeft(); }

    int predecessor;
    unsigned char atomPositions[NUM_ATOMS];
    unsigned char _minMovesLeft;
    unsigned int numMoves : 7;
    bool isOpen		  : 1;
}
#ifdef HAVE_ATTRIBUTE_PACKED
    __attribute__ ((packed))
#endif
    ;

#endif
