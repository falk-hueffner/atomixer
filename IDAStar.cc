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
#include "IDAStarCacheState.hh"
#include "IDAStarState.hh"
#include "Problem.hh"
#include "Statistics.hh"
#include "Timer.hh"
#include "parameters.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

//#undef DO_MOVE_PRUNING
#define DO_MOVE_PRUNING 1

#undef DO_MOVE_ORDERING
//#define DO_MOVE_ORDERING 1

static const double LOAD_FACTOR = 1.4;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(IDAStarCacheState)));

// global variables to describe current search state
static int maxMoves;
static int moves;
static IDAStarState state;
static deque<Move> solution;
static Timer timer;
static int cacheGoalNr = -1;
//static HashTable<IDAStarCacheState> cachedStates(MAX_STATES, LOAD_FACTOR);
static HashTable<IDAStarCacheState> cachedStates;

static bool dfs(Move lastMove);
deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    moves = 0;
    ++Statistics::statesGenerated;
    state = IDAStarState(Problem::startPositions());
    solution.clear();
    if (state.minMovesLeft() > maxDist)
	return solution;	// saves memory allocation and freeing

    //DEBUG1(cachedStates.capacity() << " 1 " << MAX_STATES);
    //if (cachedStates.capacity() != MAX_STATES)
    //cachedStates = HashTable<IDAStarCacheState>(MAX_STATES, LOAD_FACTOR);
    //DEBUG1(cachedStates.capacity() << " 2 " << MAX_STATES);
    if (Problem::goalNr != cacheGoalNr) {
	DEBUG1("cache of wrong goal nr. Clearing.");
	cacheGoalNr = Problem::goalNr;
	cachedStates.clear(MAX_STATES, LOAD_FACTOR);
	if (maxDist > 0) {
	    DEBUG1("Pre-heating cache.");
	    for (maxMoves = 0; maxMoves < maxDist; ++maxMoves) {
		DEBUG1("Pre-heating with maxDist = " << maxMoves);
		dfs(Move());
		assert(solution.empty());
		for (HashTable<IDAStarCacheState>::Iterator it = cachedStates.begin();
		    it != cachedStates.end(); ++it)
		    ++((*it).minMovesFromStart); // to force re-expansion
	    }
	    DEBUG1("Pre-heated cache.");
	}
    } else {
	for (HashTable<IDAStarCacheState>::Iterator it = cachedStates.begin();
	     it != cachedStates.end(); ++it)
	    ++((*it).minMovesFromStart); // to force re-expansion
    }
    maxMoves = maxDist;

    Statistics::timer.start();
    dfs(Move());
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

static bool dfs(Move lastMove) {
    DEBUG0(spaces(moves) << "dfs: moves =  " << moves << " state = " << state);
    if (state.minMovesLeft() == 0)
	return true;		// not true for all heuristics, but for this one
    if (moves + state.minMovesLeft() > maxMoves)
	return false;

    // We know HashTable::find() only uses State::operator==(), and nothing
    // from IDAStarCacheState, so this cast works.
    IDAStarCacheState* cachedState =
	cachedStates.find(*reinterpret_cast<IDAStarCacheState*>(&state));
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
					      moves, state.minMovesLeft()));

    if ((Statistics::statesExpanded & 0xfffff) == 0) {
	cout << state << " / " << maxMoves << endl
	     << " cached: " << cachedStates.size()
	     << " / " << cachedStates.capacity()
	     << " moves = " << moves
	     << endl;
	Statistics::print(cout);
    }

    // generate all moves...
    ++Statistics::statesExpanded;
#ifndef DO_MOVE_ORDERING
    for (int atomNr = 0; atomNr < NUM_ATOMS; ++atomNr) {
#else
    for (int i = 0; i < NUM_ATOMS; ++i) {
        int atomNr;
        if (moves > 0) {
            if (i == 0)
                atomNr = lastMove.atomNr();
            else if (i == lastMove.atomNr())
                atomNr = 0;
            else
                atomNr = i;
        } else {
            atomNr = i;
        }
#endif
	Pos startPos = state.atomPosition(atomNr);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    DEBUG0(spaces(moves) << "moving " << atomNr << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !state.isBlocking(pos); pos += dir) { }
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
		unsigned char oldatoms[NUM_ATOMS];
		for (int i = 0; i < NUM_ATOMS; ++i)
		    oldatoms[i] = state.atomPosition(i);

		state.apply(move);
		++moves;
		++Statistics::statesGenerated;
		if (dfs(move)) {
		    solution.push_front(Move(atomNr, startPos, newPos, dir));
		    return true;
		}
		--moves;
		state.undo(move, oldatoms);
	    }
	}
    }
    if (cachedStates.size() < cachedStates.capacity())
	cachedStates.insert(IDAStarCacheState(state.atomPositions(),
					       moves, (maxMoves + 1) - moves));
    // FIXME what is the correct value??
    return false;
}
