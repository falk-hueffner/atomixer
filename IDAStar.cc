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
static const uint64_t maxBitsSet = (MEMORY * 8) / 16;
static bool doAddBits;
#endif
#ifdef DO_COMPACTION
static uint8_t* compactionTable;
static size_t compactionTableCapacity;
static size_t compactionTableEntries;
#endif

// global variables to describe current search state
static int maxMoves;
static IDAStarState state;
#ifdef DO_MAY_MOVE_PRUNING
bool mayMove[NUM_ATOMS][4];
#endif
static deque<Move> solution;
static Timer timer;

static bool dfs(const Move& lastMove);

deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    ++Statistics::statesGenerated;
#ifndef DO_BACKWARD_SEARCH
    state = State(Problem::startPositions());
#else
    state = State(Problem::rstartPositions());
#endif
    solution.clear();
    if (state.minMovesLeft() > maxDist)
	return solution;	// saves memory allocation and freeing

#ifdef DO_CACHING
    if (Problem::goalNr != cacheGoalNr) {
	DEBUG1("cache of wrong goal nr. Clearing.");
	cacheGoalNr = Problem::goalNr;
	DEBUG1("MAX_STATES = " << MAX_STATES);
	cachedStates.clear(MAX_STATES, LOAD_FACTOR);
#ifdef DO_PREHEATING
	if (maxDist > 0) {
	    DEBUG1("Pre-heating cache.");
	    for (maxMoves = 0; maxMoves < maxDist - 3; ++maxMoves) {
		DEBUG1("Pre-heating with maxDist = " << maxMoves);
		//dfs(Move(), state, state.minMovesLeft());
		dfs(Move());
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
    numBitsSet = 0;
    doAddBits = true;
#endif

#ifdef DO_COMPACTION
    delete[] compactionTable;
    compactionTableCapacity = MEMORY;
    compactionTableEntries = 0;
    compactionTable = new uint8_t[compactionTableCapacity];
    size_t* compactionTable2 = static_cast<size_t*>(compactionTable);
    for (size_t i = 0; i < compactionTableCapacity; i += sizeof(size_t))
	// This is supposed to avoid triggering copy-on-write for OS that
	// return the zero page on allocation of huge memory areas. Wastes a
	// lot of time, unfortunately. If you know your OS zeroes memory (like
	// Linux does, but not Solaris), consider commenting out this loop.
        if (compactionTable[i] != 0)
            compactionTable[i] = 0;

#endif

    maxMoves = maxDist;

#ifdef DO_MAY_MOVE_PRUNING
    for (int i = 0; i < NUM_ATOMS; ++i)
	for (int j = 0; j < 4; ++j)
	    mayMove[i][j] = true;
#endif

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

    if (cachedStates.capacityLeft() > 0
#ifdef DO_STOCHASTIC_CACHING
	&& double(rand()) / double(RAND_MAX) <= CACHE_INSERT_PROBABILITY
#endif
	)
	cachedStates.insert(cacheState);
#endif
    
    if ((Statistics::statesExpanded & 0xfffff) == 0) {
	cout << ALGORITHM_NAME << endl << state << " / " << maxMoves << endl
#ifdef DO_CACHING
	     << " cached: " << cachedStates.size() << " ("
	     << double(cachedStates.size()) * 100.0
	        / double(cachedStates.capacityLeft() + cachedStates.size())
	     << "%)"
#endif
#ifdef DO_PARTIAL
	     << " cached: " << numBitsSet
	     << " (" << (double(numBitsSet) * 100.0) / double(maxBitsSet)
	     << "%)"
#endif
#ifdef DO_COMPACTION
	     << " cached: " << compactionTableEntries
	     << " (" << (double(compactionTableEntries) * 100.0)
		/ double(compactionTableCapacity)
	     << "%)"
#endif
	     << " moves = " << state.moves()
	     << endl;
	Statistics::print(cout);
    }

    // generate all moves...
    ++Statistics::statesExpanded;

#ifndef DO_BACKWARD_SEARCH
    static const int MAX_BUCKET_SIZE = NUM_ATOMS * 4;
#else
    // Hmmm... not easy... we don't want to waste space and reduce
    // locality... We'll just guess and assert later :)
    static const int MAX_BUCKET_SIZE = NUM_ATOMS * 8;
#endif    
    int bucketsize[3] = { };
    Move buckets[3][MAX_BUCKET_SIZE];

    for (int atomNr = 0; atomNr < NUM_ATOMS; ++atomNr) {
	Pos startPos = state.atomPosition(atomNr);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];

#ifdef DO_MAY_MOVE_PRUNING
	    if (!mayMove[atomNr][dirNo])
		continue;
#endif
	    
	    DEBUG0(spaces(moves) << "moving " << atomNr << " @ " << startPos
		   << ' ' << dir);
	    Pos newPos;
#ifndef DO_BACKWARD_SEARCH
	    Pos pos;
	    for (pos = startPos + dir; !state.isBlocking(pos); pos += dir) { }
	    newPos = pos - dir;
	    if (newPos == startPos)
		continue;
#else
	    if (!state.isBlocking(startPos - dir))
		continue;
	    // Note different indention would be needed here
	    for (newPos = startPos + dir; !state.isBlocking(newPos);
		 newPos += dir) {
#endif
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
		    // this is only allowed if the two moves are not
		    // independent.
#ifndef DO_BACKWARD_SEARCH
		    if (!(between(lastMove.pos1(), lastMove.pos2(),
				  lastMove.dir(), newPos)
			  || newPos + dir == lastMove.pos2()
			  || between(startPos, newPos, dir, lastMove.pos1())
			  || startPos == lastMove.pos2() + lastMove.dir())) {
			++Statistics::numPruned;
			continue;
#else // defined(DO_BACKWARD_SEARCH)
		    if (!(between(lastMove.pos1(), lastMove.pos2(),
				  lastMove.dir(), newPos)
			  || startPos - dir == lastMove.pos2()
			  || between(startPos, newPos, dir, lastMove.pos1())
			  || startPos == lastMove.pos1() - lastMove.dir())) {
			++Statistics::numPruned;
			continue;
#endif // #ifndef DO_BACKWARD_SEARCH
		    }
		}
	    }
#endif // #ifdef DO_MOVE_PRUNING
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
	    {
		// ugh... 64 bit modulo is dog slow on i386...
		uint64_t hash1 = (state.hash64_1() + state.moves())
				% stateBits.numBits();
		uint64_t hash2 = (state.hash64_2() + state.moves())
				% stateBits.numBits();
		DEBUG0(state << " hashes to " << hash1 << " & " << hash2);
		int bitsSet = stateBits.isSet(hash1) + stateBits.isSet(hash2);
		if (bitsSet == 2) {
		    DEBUG0(" bits already set");
		    goto skip;
		}

		// perhaps it is the table with $g$ less by one?
		uint64_t hash1a
		    = hash1 == 0 ? stateBits.numBits() - 1 : hash1 - 1;
		uint64_t hash2a
		    = hash2 == 0 ? stateBits.numBits() - 1 : hash2 - 1;
		if (stateBits.isSet(hash1a) && stateBits.isSet(hash2a))
		    goto skip;

		if (doAddBits) {
		    stateBits.set(hash1);
		    stateBits.set(hash2);
		    numBitsSet += 2 - bitsSet;
		    if (numBitsSet > maxBitsSet) {
			DEBUG1(" Bit hash table full");
			doAddBits = false;
		    }
		}
	    }
#endif
#ifdef DO_COMPACTION
	    {
		// hash() works much worse than hash2() ???
		//size_t hash  = (state.hash() + state.moves())
		//		% compactionTableCapacity;
		size_t hash  = (state.hash2() + state.moves())
				% compactionTableCapacity;
		size_t signature = state.hash() % 255 + 1;
		if (compactionTable[hash] == signature)
		    goto skip;

		// perhaps it is the table with $g - 1$?
		size_t hashg = hash == 0
		    ? compactionTableCapacity - 1 : hash - 1;
		
		if (compactionTable[hashg] == signature)
		    goto skip;

		// assume state is new.
		if (compactionTable[hash] == 0)
		    ++compactionTableEntries;
		compactionTable[hash] = signature;

		// check if it is already in the table with $g + 1$
		if (++hash == compactionTableCapacity)
		    hash = 0;
		if (compactionTable[hash] == signature) {
		    // make space for more valuable entry (or, in one of 255
		    // cases, kill a random state needlessly)
		    compactionTable[hash] = 0;
		    --compactionTableEntries;
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
	assert(bucketsize[bucketNr] < MAX_BUCKET_SIZE);
	for (int i = 0; i < bucketsize[bucketNr]; ++i) {
	    int oldMinMovesLeft = state.minMovesLeft();
	    const Move& move = buckets[bucketNr][i];

#ifdef DO_MAY_MOVE_PRUNING
	    bool mayMoveBak[NUM_ATOMS][4];
	    for (int i = 0; i < NUM_ATOMS; ++i)
		for (int j = 0; j < 4; ++j)
		    mayMoveBak[i][j] = mayMove[i][j];

	    if (state.moves() > 0) {
		for (int d = 0; d < 4; ++d) {
		    mayMove[move.atomNr()][d]
			= (d != noOfDir(Dir(-move.dir())));
		}
#if 0 // buggy
		if (move.atomNr() != lastMove.atomNr()) {
		    if (!(between(lastMove.pos1(), lastMove.pos2(),
				  lastMove.dir(), move.pos2())
			  || move.pos2() + move.dir() == lastMove.pos2()
			  || between(move.pos1(), move.pos2(), move.dir(),
				     lastMove.pos1())
			  || move.pos1() == lastMove.pos2() + lastMove.dir())){
			for (int d = 0; d < 4; ++d)
			    mayMove[lastMove.atomNr()][d] =  false;
		    }
		}
#endif
	    }
	    // wake up others
	    int moveDirNo = noOfDir(move.dir());
	    int mmoveDirNo = noOfOppositeDir(move.dir());
	    Dir perpDir;
	    int perpDirNr, mperpDirNr;
	    if (move.dir() == UP || move.dir() == DOWN) {
		perpDir = LEFT;
		perpDirNr = 2;
		mperpDirNr = 3;
	    } else {
		perpDir = UP;
		perpDirNr = 0;
		mperpDirNr = 1;
	    }
	    // case 1
	    Pos pp;
	    // moved at least 2 fields?
	    if (move.pos1() + move.dir() != move.pos2()) {
		for (Pos p = move.pos1() + move.dir();
		     //p != move.pos2() - move.dir(); p += move.dir()) {
		     p != move.pos2(); p += move.dir()) {
		    if (1 || state.isBlocking(p - perpDir)) {
			for (pp = p + perpDir; !state.isBlocking(pp);
			     pp += perpDir) { }
			if (state.isAtom(pp))
			    mayMove[state.atomNr(pp)][mperpDirNr] = true;
		    }
		    if (1 || state.isBlocking(p + perpDir)) {
			for (pp = p - perpDir; !state.isBlocking(pp);
			     pp -= perpDir) { }
			if (state.isAtom(pp))
			    mayMove[state.atomNr(pp)][perpDirNr] = true;
		    }
		}
	    }
	    // case 1 special case
	    for (pp = move.pos1() - move.dir(); !state.isBlocking(pp);
		 pp -= move.dir()) { }
	    if (state.isAtom(pp))
		mayMove[state.atomNr(pp)][moveDirNo] = true;

	    // case 2 & 3 
	    for (pp = move.pos1() + perpDir; !state.isBlocking(pp);
		 pp += perpDir) { }
	    if (state.isAtom(pp))
		mayMove[state.atomNr(pp)][mperpDirNr] = true;
	    for (pp = move.pos1() - perpDir; !state.isBlocking(pp);
		 pp -= perpDir) { }
	    if (state.isAtom(pp))
		mayMove[state.atomNr(pp)][perpDirNr] = true;
	    // case 4
	    if (state.isAtom(move.pos2() + move.dir()))
		mayMove[state.atomNr(move.pos2() + move.dir())][mmoveDirNo]
		    = true;
#endif
	    state.apply(move, oldMinMovesLeft + bucketNr - 1);	
	    if (dfs(move)) {
		solution.push_front(move);
		return true;
	    }
	    state.undo(move, oldMinMovesLeft);
#ifdef DO_MAY_MOVE_PRUNING
	    for (int i = 0; i < NUM_ATOMS; ++i)
		for (int j = 0; j < 4; ++j)
		    mayMove[i][j] = mayMoveBak[i][j];
#endif
#ifdef DO_BACKWARD_SEARCH
	}
#endif
	}
    }

#ifdef DO_CACHING
    if (cachedStates.capacityLeft() > 0
#ifdef DO_STOCHASTIC_CACHING
	&& double(rand()) / double(RAND_MAX)  <= CACHE_INSERT_PROBABILITY
#endif
	) {
	cacheState.minMovesLeft = (maxMoves + 1) - state.moves();
	cachedStates.insert(cacheState);
    }
#endif

    return false;
}
