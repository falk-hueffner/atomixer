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

#undef DO_CACHING
//#define DO_CACHING 1

//#undef DO_PARTIAL
#define DO_PARTIAL 1

#include <assert.h>
#include <stdlib.h>

#include <deque>
#include <iostream>
#include <vector>

#include "Dir.hh"
#include "HashTable.hh"
#include "IDAStar.hh"
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
static uint64_t maxBitsSet = (MEMORY * 8) / 8;
static bool doAddBits = true;
#endif

// global variables to describe current search state
static int maxMoves;
static int moves;
bool isBlocking[NUM_FIELDS]; // FIXME try whether char is faster
//static IDAStarState state;
static deque<Move> solution;
static Timer timer;

static bool dfs(Move lastMove, const State& state, int minMovesLeft);
deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    moves = 0;
    ++Statistics::statesGenerated;
    State state = State(Problem::startPositions());
    solution.clear();
    if (state.minMovesLeft() > maxDist)
	return solution;	// saves memory allocation and freeing

    for (Pos pos = 0; pos != Pos::end(); ++pos)
	isBlocking[pos.fieldNumber()] = Problem::isBlock(pos);
    for (int i = 0; i < NUM_ATOMS; ++i)
	isBlocking[state.atomPosition(i)] = true;


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

    Statistics::timer.start();
    dfs(Move(), state, state.minMovesLeft());
    Statistics::timer.stop();

    return solution;
}

#if 0
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

static bool dfs(Move lastMove, const State& state, int minMovesLeft) {
    DEBUG0(spaces(moves) << "dfs: moves =  " << moves << " state = " << state);
    // FIXME do this earlier
    if (minMovesLeft == 0)
	return true;		// not true for all heuristics, but for this one
    if (moves + minMovesLeft > maxMoves)
	return false;

#ifdef DO_CACHING
    // We know HashTable::find() only uses State::operator==(), and nothing
    // from IDAStarCacheState, so this cast works.
    IDAStarCacheState* cachedState =
	cachedStates.find(*reinterpret_cast<const IDAStarCacheState*>(&state));
	//cachedStates.find(IDAStarCacheState(state.positions()));

    if (cachedState != NULL) {
	DEBUG0("found" << state << " in cache");
	if (cachedState->minMovesFromStart <= moves)
	    return false;
	else if (cachedState->minMovesFromStart > moves)
	    cachedState->minMovesFromStart = moves;

	if (moves + cachedState->minMovesLeft > maxMoves)
	    return false;
    }

    if (cachedStates.size() < cachedStates.capacity())
	cachedStates.insert(IDAStarCacheState(state.atomPositions(),
					      moves, minMovesLeft));
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
	     << " moves = " << moves
	     << endl;
	Statistics::print(cout);
    }

    // generate all moves...
    ++Statistics::statesExpanded;

    static const int MAX_BUCKET_SIZE = NUM_ATOMS * 4;
    typedef pair<State, Move> Bucket;
    Bucket bucket0[MAX_BUCKET_SIZE];
    Bucket bucket1[MAX_BUCKET_SIZE];
    Bucket bucket2[MAX_BUCKET_SIZE];
    int bucket0size = 0, bucket1size = 0, bucket2size = 0;

    for (int atomNr = 0; atomNr < NUM_ATOMS; ++atomNr) {
	Pos startPos = state.atomPosition(atomNr);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    DEBUG0(spaces(moves) << "moving " << atomNr << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !isBlocking[pos.fieldNumber()]; pos += dir) { }
	    Pos newPos = pos - dir;
	    if (newPos != startPos) {
		++Statistics::numChildren;
		DEBUG0(spaces(moves) << "moves to " << newPos);
		Move move(atomNr, startPos, newPos, dir);
		if (moves > 0) {
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
		//unsigned char oldatoms[NUM_ATOMS];
		//for (int i = 0; i < NUM_ATOMS; ++i)
		//    oldatoms[i] = state.atomPosition(i);

		//state.apply(move);

		State newState(state, move);
		++Statistics::statesGenerated;
		++Statistics::statesGeneratedAtDepth[maxMoves];

#ifdef DO_PARTIAL
		// ugh... 64 bit modulo is dog slow on i386...
		uint64_t hash = newState.hash64() % stateBits.numBits();
		DEBUG0(newState << " hashes to " << hash);
		if (stateBits.isSet(hash)) {
		    DEBUG0(" bit already set");
		    continue;
		}
		if (doAddBits) {
		    stateBits.set(hash);
		    if (++numBitsSet > maxBitsSet) {
			DEBUG1(" Bit hash table full");
			doAddBits = false;
		    }
		}

	    //assert(stateBits.isSet(hash));
#endif

		int newMinMovesLeft = newState.minMovesLeft();

		switch (newMinMovesLeft - minMovesLeft) {
		case -1:
		    bucket0[bucket0size++] = make_pair(newState, move);
		    break;
		case 0:
		    bucket1[bucket1size++] = make_pair(newState, move);
		    break;
		case 1:
		    bucket2[bucket2size++] = make_pair(newState, move);
		    break;
		default:
		    cerr << "Impossible: old minMovesLeft = " << minMovesLeft
			 << ", new minMovesLeft = " << newMinMovesLeft << endl;
		    abort();
		}

		//state.undo(move, oldatoms);
	    }
	}
    }

    for (int i = 0; i < bucket0size; ++i) {
	++moves;
	const Move& move = bucket0[i].second;
	isBlocking[move.pos1().fieldNumber()] = false;
	isBlocking[move.pos2().fieldNumber()] = true;
	
	if (dfs(move, bucket0[i].first, minMovesLeft - 1)) {
	    solution.push_front(move);
	    return true;
	}
	isBlocking[move.pos1().fieldNumber()] = true;
	isBlocking[move.pos2().fieldNumber()] = false;
	
	--moves;
    }
    for (int i = 0; i < bucket1size; ++i) {
	++moves;
	const Move& move = bucket1[i].second;
	isBlocking[move.pos1().fieldNumber()] = false;
	isBlocking[move.pos2().fieldNumber()] = true;
	
	if (dfs(move, bucket1[i].first, minMovesLeft)) {
	    solution.push_front(move);
	    return true;
	}
	isBlocking[move.pos1().fieldNumber()] = true;
	isBlocking[move.pos2().fieldNumber()] = false;
	
	--moves;
    }
    for (int i = 0; i < bucket2size; ++i) {
	++moves;
	const Move& move = bucket2[i].second;
	isBlocking[move.pos1().fieldNumber()] = false;
	isBlocking[move.pos2().fieldNumber()] = true;
	
	if (dfs(move, bucket2[i].first, minMovesLeft + 1)) {
	    solution.push_front(move);
	    return true;
	}
	isBlocking[move.pos1().fieldNumber()] = true;
	isBlocking[move.pos2().fieldNumber()] = false;
	
	--moves;
    }

#ifdef DO_CACHING
    if (cachedStates.size() < cachedStates.capacity())
	cachedStates.insert(IDAStarCacheState(state.atomPositions(),
					       moves, (maxMoves + 1) - moves));
#endif

    return false;
}
