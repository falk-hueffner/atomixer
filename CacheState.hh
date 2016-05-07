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

#ifndef CACHESTATE_HH
#define CACHESTATE_HH

#include "Pos.hh"
#include "Problem.hh"
#include "State.hh"

// A CacheState is useful for storing in caches, because it is always
// canonicallified.

class CacheState : public State {
private:
    // canonicallify the state: identical atoms are sorted by number. This
    // avoids storing logically identical states twice in the hash table.
    void canonicallify() {
	for (int atomNr = PAIRED_START; atomNr < PAIRED_END; atomNr += 2)
	    if (atomPositions_[atomNr + 1] < atomPositions_[atomNr])
		swap(atomPositions_[atomNr + 1], atomPositions_[atomNr]);
	for (int atomNr = MULTI_START; atomNr < NUM_ATOMS;
	     atomNr += Problem::numIdentical(atomNr)) {
	    sort(atomPositions_ + atomNr,
		 atomPositions_ + atomNr + Problem::numIdentical(atomNr));
	}
    }
    // if only one atom changed, we can do it faster.
    void canonicallify(int atomNr) {
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

public:
    // leave uninitialized
    CacheState() { }
    CacheState(const State& state) : State(state) { canonicallify(); }
    CacheState(const Pos positions[NUM_ATOMS]) : State(positions) { canonicallify(); }
    CacheState(const ShortPos positions[NUM_ATOMS])
	: State(positions) { canonicallify(); }
    CacheState(const State& state, const Move& move)
	: State(state, move)  { canonicallify(move.atomNr()); }
    void apply(const Move& move) { State::apply(move); canonicallify(move.atomNr()); }

private:
    void undo(const Move& move); // not easily implementable
} __attribute__ ((packed));

#endif
