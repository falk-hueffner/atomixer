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

#ifndef STATE_HH
#define STATE_HH

#include <iosfwd>
#include <vector>

class Problem;

#include "Move.hh"

using namespace std;

class State {
public:
    State() { }			// leave uninitialized
    explicit State(vector<Pos> atomPositions);
    State(const State& state, const Move& move);

    bool operator<(const State& other) const; // to put into STL set
    bool operator==(const State& other) const;

    vector<Move> moves() const;
    int minMovesLeft() const;

    const vector<Pos>& atomPositions() const { return myAtomPositions; }

private:
    vector<Pos> myAtomPositions;
};

ostream& operator <<(ostream& out, const State& state);

#endif
