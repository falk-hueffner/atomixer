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

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
typedef long long int64_t;
#endif

#include <deque>
#include <iostream>
#include <vector>

#include "Dir.hh"
#include "IDAStar.hh"
#include "IDAStarState.hh"
#include "IDAStarPackedState.hh"
#include "Problem.hh"
#include "Timer.hh"
#include "HashTable.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

extern int64_t totalNodesGenerated;

// maximum amount of memory to be used
static const unsigned int MEMORY = 300 * 1024 * 1024;
static const double LOAD_FACTOR = 1.4;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(IDAStarPackedState)));

// global variables to describe current search state
static int maxMoves;
static int moves;
static IDAStarState state;
static deque<Move> solution;
static int64_t nodesGenerated, lastOutput;
static Timer timer;
static int cacheGoalNr = -1;
static HashTable<IDAStarPackedState> cachedStates(MAX_STATES, LOAD_FACTOR);

static bool dfs(IDAStarMove lastMove);
deque<Move> IDAStar(int maxDist) {
    DEBUG0("IDAStar" << maxDist);
    maxMoves = maxDist;
    moves = 0;
    nodesGenerated = 1;
    lastOutput = 0;
    state = IDAStarState(0);
    solution.clear();

    if (Problem::goalNr != cacheGoalNr) {
	DEBUG1("cache of wrong goal nr. Clearing.");
	cacheGoalNr = Problem::goalNr;
	cachedStates = HashTable<IDAStarPackedState>(MAX_STATES, LOAD_FACTOR);
	if (maxDist > 0) {
	    DEBUG1("Pre-heating cache.");
	    for (int maxDist2 = 0; maxDist2 < maxDist - 1; ++maxDist2) {
		DEBUG1("Pre-heating with maxDist = " << maxDist2);
		deque<Move> moves = IDAStar(maxDist2);
		assert(moves.empty());
	    }
	    DEBUG1("Pre-heated cache.");
	}
    }

    timer.reset();
    dfs(IDAStarMove());
    totalNodesGenerated += nodesGenerated;

    return solution;
}

static string spaces(int n) {
    return string(n, ' ');
}

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
    }
    assert(false);
}

static bool dfs(IDAStarMove lastMove) {
    DEBUG0(spaces(moves) << "dfs: moves =  " << moves << " state = " << state);
    if (state.minMovesLeft() == 0)
	return true;		// not true for all heuristics, but for this one
    if (moves + state.minMovesLeft() > maxMoves)
	return false;

    IDAStarPackedState* cachedState =
	cachedStates.find(IDAStarPackedState(state.positions()));

    if (cachedState != NULL) {
	DEBUG0("found" << state << " in cache");
	if (cachedState->minMovesFromStart < moves)
	    return false;
	else if (cachedState->minMovesFromStart > moves)
	    cachedState->minMovesFromStart = moves;

	if (moves + cachedState->minMovesLeft > maxMoves)
	    return false;
    }

    if (nodesGenerated - lastOutput > 2000000) {
	lastOutput = nodesGenerated;
	cout << state << endl
	     << " Nodes: " << nodesGenerated
	     << " cached: " << cachedStates.size()
	     << " / " << cachedStates.capacity()
	     << " moves = " << moves
	     << " nodes/second: "
	     << (int64_t) (double(nodesGenerated) / timer.seconds())
	     << endl;
    }

    // generate all moves...
    for (int atomNo = 0; atomNo < NUM_ATOMS; ++atomNo) {
	Pos startPos = state.position(atomNo);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    DEBUG0(spaces(moves) << "moving " << atomNo << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !state.isBlocking(pos); pos += dir) { }
	    Pos newPos = pos - dir;
	    if (newPos != startPos) {
		DEBUG0(spaces(moves) << "moves to " << newPos);
		IDAStarMove move(atomNo, dir, startPos, newPos);
		if (moves > 0) {
		    if (atomNo == lastMove.atomNo && dir == -lastMove.dir)
			continue;
		    if (atomNo < lastMove.atomNo) {
			// this is only allowed if the two moves are not independent.
			if (!(between(lastMove.p1, lastMove.p2, lastMove.dir, newPos)
			      || newPos + dir == lastMove.p2
			      || between(startPos, newPos, dir, lastMove.p1)
			      || newPos == lastMove.p2 + lastMove.dir))
			    continue;
		    }
		}
		state.apply(move);
		++moves;
		++nodesGenerated;
		if (dfs(move)) {
		    solution.push_front(Move(atomNo, startPos, newPos, dir));
		    return true;
		}
		--moves;
		state.undo(move);
	    }
	}
    }
    if (cachedStates.size() < cachedStates.capacity())
	cachedStates.insert(IDAStarPackedState(state.positions(),
					       //moves, (maxMoves + 1) - moves));
					       moves, maxMoves - moves));
    return false;
}

ostream& operator<<(ostream& out, const IDAStarState& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << state._positions[i] << ' ';
    return out << state._minMovesLeft; 
}
