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
static const unsigned int MEMORY = 900 * 1024 * 1024;
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
    //DEBUG1(cachedStates.capacity() << " 1 " << MAX_STATES);
    //if (cachedStates.capacity() != MAX_STATES)
    //cachedStates = HashTable<IDAStarPackedState>(MAX_STATES, LOAD_FACTOR);
    //DEBUG1(cachedStates.capacity() << " 2 " << MAX_STATES);

    timer.reset();
    dfs(IDAStarMove());
    totalNodesGenerated += nodesGenerated;

    return solution;
}

static string spaces(int n) {
    return string(n, ' ');
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
		    if (atomNo < lastMove.atomNo) {
			// this is only allowed if the two moves are not independent.
			// case 1
			for (Pos p = lastMove.p1; p != lastMove.p2; p += lastMove.dir)
			    if (p == newPos)
				goto dependent;
			// case 2
			if (newPos + dir == lastMove.p2)
			    goto dependent;
			// case 3
			for (Pos p = startPos; p != newPos; p += dir)
			    if (p == lastMove.p1)
				goto dependent;
			// case 4
			if (newPos == lastMove.p2 + lastMove.dir)
			    goto dependent;
				
			// all test failed: moves are independent. Prune this
			// ordering.
			continue;
		    dependent:
			;		// moves are dependent; no ordering possible
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
		if (cachedStates.size() < cachedStates.capacity())
		    cachedStates.insert(IDAStarPackedState(state.positions(),
							   //moves, (maxMoves + 1) - moves));
							   moves, maxMoves - moves));
	    }
	}
    }

    return false;
}

ostream& operator<<(ostream& out, const IDAStarState& state) {
    for (int i = 0; i < NUM_ATOMS; ++i)
	out << state._positions[i] << ' ';
    return out << state._minMovesLeft; 
}
