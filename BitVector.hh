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

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
typedef unsigned long long uint64_t;
#endif

#include <assert.h>

// unfortunately, vector<bool> has no operator[](uint64_t), so it is limited
// to half a gig of memory. So we have to do it ourselves for 32-bit
// machines...

class BitVector {
public:
    BitVector(uint64_t numBits = 0) {
	numBits_ = numBits;
	numLimbs = (numBits_ + BITS_PER_ULONG - 1) / BITS_PER_ULONG;
	bits = new unsigned long[numLimbs];
    }

    void init(uint64_t numBits) {
	delete[] bits;
	numBits_ = numBits;
	numLimbs = (numBits_ + BITS_PER_ULONG - 1) / BITS_PER_ULONG;
	bits = new unsigned long[numLimbs];
    }

    ~BitVector() { delete[] bits; }

    uint64_t numBits() const { return numBits_; }

    bool isSet(uint64_t index) const {
	assert(index / BITS_PER_ULONG < numLimbs);
	return bits[index / BITS_PER_ULONG] & (1UL << (index % BITS_PER_ULONG));
    }

    void set(uint64_t index) {
	assert(index / BITS_PER_ULONG < numLimbs);
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
