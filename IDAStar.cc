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

#undef DO_PREHEATING
//#define DO_PREHEATING 1

//#undef DO_MOVE_PRUNING
#define DO_MOVE_PRUNING 1

//#undef DO_MOVE_ORDERING
//#define DO_MOVE_ORDERING 1

//#undef DO_CACHING
#define DO_CACHING 1

#undef DO_PARTIAL
//#define DO_PARTIAL 1

#undef DO_STOCHASTIC_CACHING
//#define DO_STOCHASTIC_CACHING 1

#define CACHE_INSERT_PROBABILITY 0.1

#include <assert.h>
#include <stdlib.h>

#include <deque>
#include <iostream>
#include <vector>

#include "Dir.hh"
#include "HashTable.hh"
#include "IDAStar.hh"
#include "IDAStarState.hh"
#include "Problem.hh"
#include "State.hh"
#include "Statistics.hh"
#include "Timer.hh"
#include "parameters.hh"

#ifdef DO_CACHING
#include "IDAStarCacheState.hh"
#endif
#ifdef DO_PARTIAL
#include "BitVector.hh"
#endif

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

#ifdef DO_CACHING
static const double LOAD_FACTOR = 1.4;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(IDAStarCacheState)));

static int cacheGoalNr = -1;
static HashTable<IDAStarCacheState> cachedStates;
#endif
#ifdef DO_PARTIAL
static BitVector stateBits;
static uint64_t numBitsSet;
static const uint64_t maxBitsSet = (MEMORY * 8) / 8;
static bool doAddBits = true;
#endif

// global variables to describe current search state
static int maxMoves;
static IDAStarState state;
bool mayMove[NUM_ATOMS][4];
static deque<Move> solution;
static Timer timer;

static bool dfs(const Move& lastMove);

deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    ++Statistics::statesGenerated;
    state = State(Problem::startPositions());
    solution.clear();
    if (state.minMovesLeft() > maxDist)
	return solution;	// saves memory allocation and freeing

#ifdef DO_CACHING
    if (Problem::goalNr != cacheGoalNr) {
	DEBUG1("cache of wrong goal nr. Clearing.");
	cacheGoalNr = Problem::goalNr;
	cachedStates.clear(MAX_STATES, LOAD_FACTOR);
#ifdef DO_PREHEATING
	if (maxDist > 0) {
	    DEBUG1("Pre-heating cache.");
	    for (maxMoves = 0; maxMoves < maxDist; ++maxMoves) {
		DEBUG1("Pre-heating with maxDist = " << maxMoves);
		dfs(Move(), state, state.minMovesLeft());
		assert(solution.empty());
		for (HashTable<IDAStarCacheState>::Iterator it = cachedStates.begin();
		    it != cachedStates.end(); ++it)
		    ++((*it).minMovesFromStart); // to force re-expansion
	    }
	    DEBUG1("Pre-heated cache.");
	}
#endif
    } else {
	for (HashTable<IDAStarCacheState>::Iterator it = cachedStates.begin();
	     it != cachedStates.end(); ++it)
	    ++((*it).minMovesFromStart); // to force re-expansion
    }
#endif

#ifdef DO_PARTIAL
    stateBits.init(MEMORY * 8);
#endif

    maxMoves = maxDist;

    for (int i = 0; i < NUM_ATOMS; ++i)
	for (int j = 0; j < 4; ++j)
	    mayMove[i][j] = true;

    Statistics::timer.start();
    dfs(Move());
    Statistics::timer.stop();

    return solution;
}

#if 1
static string spaces(int n) {
    return string(n, ' ');
}
#endif

static inline bool between(Pos p1, Pos p2, Dir dir, Pos pm) {
    switch(dir) {
    case UP:
	return p1.x() == pm.x() && p2 <= pm && pm <= p1;
    case DOWN:
	return p1.x() == pm.x() && p1 <= pm && pm <= p2;
    case LEFT:
	return p2 <= pm && pm <= p1;
    case RIGHT:
	return p1 <= pm && pm <= p2;
    default:
	abort();
    }
    return false;		// Compaq C++ just doesn't get it...
}

static bool dfs(const Move& lastMove) {
    DEBUG0(spaces(state.moves()) << "dfs: moves =  "
	   << state.moves() << " state = " << state);

#ifdef DO_CACHING
    IDAStarCacheState cacheState(state);
    IDAStarCacheState* cachedState = cachedStates.find(cacheState);
    if (cachedState != NULL) {
	DEBUG0("found" << state << " in cache");
	if (cachedState->minMovesFromStart <= state.moves())
	    return false;
	else if (cachedState->minMovesFromStart > state.moves())
	    cachedState->minMovesFromStart = state.moves();

	if (state.moves() + cachedState->minMovesLeft > maxMoves)
	    return false;
    }

    if (cachedStates.size() < cachedStates.capacity()
#ifdef DO_STOCHASTIC_CACHING
	&& double(rand()) / double(RAND_MAX) <= CACHE_INSERT_PROBABILITY
#endif
	)
	cachedStates.insert(cacheState);
#endif

    if ((Statistics::statesExpanded & 0xfffff) == 0) {
	cout << state << " / " << maxMoves << endl
#ifdef DO_CACHING
	     << " cached: " << cachedStates.size()
	     << " / " << cachedStates.capacity()
#endif
#ifdef DO_PARTIAL
	     << " cached: " << numBitsSet
	     << " / " << maxBitsSet
#endif
	     << " moves = " << state.moves()
	     << endl;
	Statistics::print(cout);
    }

    // generate all moves...
    ++Statistics::statesExpanded;

    static const int MAX_BUCKET_SIZE = NUM_ATOMS * 4;
    //typedef pair<State, Move> Bucket;
    //Move bucket0[MAX_BUCKET_SIZE];
    //Move bucket1[MAX_BUCKET_SIZE];
    //Move bucket2[MAX_BUCKET_SIZE];
    Move buckets[3][MAX_BUCKET_SIZE];
    int bucketsize[3] = { };

    for (int atomNr = 0; atomNr < NUM_ATOMS; ++atomNr) {
	Pos startPos = state.atomPosition(atomNr);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];

	    if (!mayMove[atomNr][dirNo])
		continue;
	    
	    DEBUG0(spaces(moves) << "moving " << atomNr << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !state.isBlocking(pos); pos += dir) { }
	    Pos newPos = pos - dir;
	    if (newPos == startPos)
		continue;
	    
	    ++Statistics::numChildren;
	    DEBUG0(spaces(moves) << "moves to " << newPos);
	    Move move(atomNr, startPos, newPos, dir);
#ifdef DO_MOVE_PRUNING
	    if (state.moves() > 0) {
		if (atomNr == lastMove.atomNr() && dir == -lastMove.dir()) {
		    ++Statistics::numPruned;
		    continue;
		}
		if (atomNr < lastMove.atomNr()) {
		    // this is only allowed if the two moves are not independent.
		    if (!(between(lastMove.pos1(), lastMove.pos2(),
				  lastMove.dir(), newPos)
			  || newPos + dir == lastMove.pos2()
			  || between(startPos, newPos, dir, lastMove.pos1())
			  || startPos == lastMove.pos2() + lastMove.dir())) {
			++Statistics::numPruned;
			continue;
		    }
		}
	    }
#endif
	    int oldMinMovesLeft = state.minMovesLeft();
	    int bucketNr;
	    state.apply(move);
	    ++Statistics::statesGenerated;
	    ++Statistics::statesGeneratedAtDepth[maxMoves];

	    if (state.minMovesLeft() == 0) {
		solution.push_front(move);	    
		return true; // not true for all heuristics, but for this one
	    }
	    if (state.minTotalMoves() > maxMoves)
		goto skip;

#ifdef DO_PARTIAL
	    // ugh... 64 bit modulo is dog slow on i386...
	    uint64_t hash1 = newState.hash64_1() % stateBits.numBits();
	    uint64_t hash2 = newState.hash64_2() % stateBits.numBits();
	    DEBUG0(newState << " hashes to " << hash1 << " & " << hash2);
	    int bitsSet = stateBits.isSet(hash1) + stateBits.isSet(hash2);
	    if (bitsSet == 2) {
		DEBUG0(" bits already set");
		goto skip;
	    }
	    if (doAddBits) {
		stateBits.set(hash1);
		stateBits.set(hash2);
		numBitsSet += 2 - bitsSet;
		if (numBitsSet > maxBitsSet) {
		    DEBUG1(" Bit hash table full");
		    doAddBits = false;
		}
	    }
#endif
	    bucketNr = state.minMovesLeft() - oldMinMovesLeft + 1;
	    if (bucketNr < 0 || bucketNr > 2) {
		    cerr << "Impossible: old minMovesLeft = " << oldMinMovesLeft
			 << ", new minMovesLeft = " << state.minMovesLeft() << endl;
		    abort();		
	    }
	    
	    buckets[bucketNr][bucketsize[bucketNr]++] = move;

	skip:
	    state.undo(move, oldMinMovesLeft);
	}
    }

    for (int bucketNr = 0; bucketNr < 3; ++bucketNr) {
	for (int i = 0; i < bucketsize[bucketNr]; ++i) {
	    int oldMinMovesLeft = state.minMovesLeft();
	    const Move& move = buckets[bucketNr][i];
	    state.apply(move, oldMinMovesLeft + bucketNr - 1);	
	    if (dfs(move)) {
		solution.push_front(move);
		return true;
	    }
	    state.undo(move, oldMinMovesLeft);
	}
    }

#ifdef DO_CACHING
    if (cachedStates.size() < cachedStates.capacity()
#ifdef DO_STOCHASTIC_CACHING
	&& double(rand()) / double(RAND_MAX)  <= CACHE_INSERT_PROBABILITY
#endif
	)
	cacheState.minMovesLeft = (maxMoves + 1) - state.moves();
	cachedStates.insert(cacheState);
#endif

    return false;
}
