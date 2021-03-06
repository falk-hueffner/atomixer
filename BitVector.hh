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

#ifndef BITVECTOR_HH
#define BITVECTOR_HH

#include "stdint.h"
#include <assert.h>
#include <string.h>

// unfortunately, vector<bool> has no operator[](uint64_t), so it is limited
// to half a gig of memory. So we have to do it ourselves for 32-bit
// machines...

class BitVector {
public:
    BitVector(uint64_t numBits = 0) {
	bits = NULL;
	init(numBits);
    }

    void init(uint64_t numBits) {
	delete[] bits;
	numBits_ = numBits;
	numLimbs = (numBits_ + BITS_PER_ULONG - 1) / BITS_PER_ULONG;
	bits = new unsigned long[numLimbs];
#ifdef MY_OS_ZEROES_LARGE_MEMORY_ALLOCATIONS
	if (numBits / 8 < 65536)
#endif
	    memset(bits, 0, numBits / 8);
    }

    ~BitVector() { delete[] bits; }

    uint64_t numBits() const { return numBits_; }

    bool isSet(uint64_t index) const {
	return bits[index / BITS_PER_ULONG] & (1UL << (index % BITS_PER_ULONG));
    }

    void set(uint64_t index) {
	bits[index / BITS_PER_ULONG] |= 1UL << (index % BITS_PER_ULONG);
    }

private:
    BitVector(const BitVector&); // copying not allowed

    static const int BITS_PER_ULONG = sizeof(unsigned long) * 8;

    size_t numLimbs;
    uint64_t numBits_;
    unsigned long* bits;
};

#endif
