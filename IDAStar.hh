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

#ifndef IDASTAR_HH
#define IDASTAR_HH

#include <deque>

#include "Move.hh"

#undef DO_BACKWARD_SEARCH	// normal forward search
//#define DO_BACKWARD_SEARCH 1	// search from goal toward starting position

#undef DO_PREHEATING
//#define DO_PREHEATING 1

//#undef DO_MOVE_PRUNING
#define DO_MOVE_PRUNING 1

#undef DO_MAY_MOVE_PRUNING
//#define DO_MAY_MOVE_PRUNING 1

#undef DO_CACHING
//#define DO_CACHING 1

#undef DO_PARTIAL
//#define DO_PARTIAL 1

//#undef DO_COMPACTION
#define DO_COMPACTION 1

#undef DO_STOCHASTIC_CACHING
//#define DO_STOCHASTIC_CACHING 1

#define CACHE_INSERT_PROBABILITY 0.1

static const char* ALGORITHM_NAME = "idastar"
#ifdef DO_BACKWARD_SEARCH
  "-backward"
#endif
#ifndef DO_PREHEATING
  "-nopreheat"
#endif
#ifndef DO_MOVE_PRUNING
  "-nomoveprune"
#endif
#ifdef DO_MAY_MOVE_PRUNING
  "-maymoveprune"
#endif
#if !defined(DO_CACHING) && !defined(DO_PARTIAL) && !defined(DO_COMPACTION)
  "-nocaching"
#endif
#ifdef DO_PARTIAL
  "-partial"
#endif
#ifdef DO_COMPACTION
  "-compaction"
#endif
#ifdef DO_STOCHASTIC_CACHING
  "-stochastic"
#endif
;

deque<Move> IDAStar(int maxDist);

#endif
