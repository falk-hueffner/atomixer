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

#ifndef PACKED_STATE_HH
#define PACKED_STATE_HH

#include <iosfwd>
#include <vector>

class Problem;

#include "Move.hh"

using namespace std;

class PackedState {
    friend ostream& operator<<(ostream& out, const PackedState& state);

public:
    PackedState() { }		// leave uninitialized
    explicit PackedState(const Pos atomPositions[NUM_ATOMS]);
    explicit PackedState(const unsigned char atomPositions[NUM_ATOMS]);
    PackedState(const PackedState& state, const Move& move);

    bool operator<(const PackedState& other) const; // to put into STL set
    bool operator==(const PackedState& other) const;
    unsigned int hash() const;

    vector<Move> moves() const;
    vector<Move> rmoves() const;
    int minMovesLeft() const;

private:
    unsigned char atomPositions[NUM_ATOMS];
};

#endif
