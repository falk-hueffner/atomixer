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

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
typedef unsigned long long uint64_t;
#endif

#include <iostream>
#include <vector>

#include "Move.hh"
#include "Size.hh"

// A complete representation of a game state.

class State {
    friend std::ostream& operator<<(std::ostream& out, const State& state);
public:
    // leave uninitialized
    State() { }
    // mostly for constructing starting state from the static data in Problem
    inline State(const Pos positions[NUM_ATOMS]);
    // useful for constructing from another State descendant
    inline State(const unsigned char positions[NUM_ATOMS]);
    // apply move
    inline State(const State& state, const Move& move);

    int atomPosition(int atomNr) const { return atomPositions_[atomNr]; }
    const unsigned char* atomPositions() const { return atomPositions_; }
    inline int minMovesLeft() const;
    
    inline bool operator==(const State& other) const;

    inline void apply(const Move& move);
    inline void undo(const Move& move);

    inline std::vector<Move> moves() const;
    inline std::vector<Move> rmoves() const;

    inline size_t hash() const;
    inline uint64_t hash64_1() const;
    inline uint64_t hash64_2() const;

protected:
    unsigned char atomPositions_[NUM_ATOMS];
}
#ifdef HAVE_ATTRIBUTE_PACKED
    __attribute__ ((packed))
#endif
    ;

#include "State.cc"

#endif
