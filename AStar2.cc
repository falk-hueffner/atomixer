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

#include <stdlib.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <fstream>
#include <vector>

#include "AStar2.hh"
#include "AStarState.hh"
#include "State.hh"
#include "Statistics.hh"
#include "parameters.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

// the hash table is considered to be full if it has less than LOAD_FACTOR *
// #states entries
static const double LOAD_FACTOR = 1.4;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(AStarState)));
static const unsigned int MAX_HASHES = (unsigned int)
    (MAX_STATES * LOAD_FACTOR);

vector<AStarState> states;
vector<int> hashTable;
deque<Move> solution;

static int maxMoves;		// cutoff
static int minMinTotalMoves;	// currently lowest f-value of an open state
static int firstOpen;
static int searchIndex;
static int numOpen;

void hashInsert(const AStarState& state) {
    unsigned int hash = state.hash() % hashTable.size();
    while (true) {
	if (hashTable[hash] == 0) { // found empty place?
	    states.push_back(state);
	    ++numOpen;
	    hashTable[hash] = states.size() - 1;

	    return;
	} else {
	    AStarState& oldState = states[hashTable[hash]];
	    if (oldState == state) { // already known?
		if (state.numMoves < oldState.numMoves) {
		    // An important property of A* is that states that have
		    // already been expanded will never be re-expanded.
		    assert(oldState.isOpen);
		    oldState.numMoves = state.numMoves;
		    oldState.predecessor = state.predecessor;
		    if (hashTable[hash] < firstOpen)
			firstOpen = hashTable[hash];
		    return;
		} else {
		    // we already know a better way
		    return;
		}
	    }
	    // hash collision; probe linearly for first free space.
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }
}

int findBest(int maxMoves) {
    ++searchIndex;
    while (true) {
	bool alreadyRestarted = false;
	while (true) {
	    for (; searchIndex < states.size(); ++searchIndex) {
		if (states[searchIndex].isOpen
		    && (states[searchIndex].minTotalMoves()
			<= minMinTotalMoves)) {
		    assert(states[searchIndex].minTotalMoves()
			   == minMinTotalMoves);
		    return searchIndex;
		}
	    }
	    // we fell off the end
	    if (alreadyRestarted) // already twice?
		break;
	    // find first open state
	    for (; firstOpen < states.size() && !states[firstOpen].isOpen;
		 ++firstOpen) { }
	    if (firstOpen == states.size()) {
		// no open nodes left at all...
		return 0;
	    }
	    searchIndex = firstOpen;
	    alreadyRestarted = true;
	}
	// We fell off the end twice. No open states with f = minMinTotalMoves
	// left.
	if (++minMinTotalMoves > maxMoves)
	    return 0;

	searchIndex = firstOpen;
    }
}

deque<Move> aStar2(const State& startState, int nmaxMoves) {
    AStarState start = startState;
    maxMoves = nmaxMoves;
    if (start.minTotalMoves() > maxMoves)
	return deque<Move>();	// saves the allocations which can take quite
				// some time

    states.clear();
    states.push_back(AStarState()); // 0 reserved for 'empty'
    states.reserve(MAX_STATES);
    hashTable.clear();
    hashTable.resize(MAX_HASHES);

    minMinTotalMoves = 0;
    firstOpen = 1;		// 0 reserved for 'empty'
    searchIndex = firstOpen;
    numOpen = 0;

    DEBUG1("start state: " << start);
    hashInsert(start);
    ++Statistics::statesGenerated;

    Statistics::timer.start();
    while (true) {
	int bestIndex = findBest(maxMoves);
	if (bestIndex == 0)	// no open state left
	    break;
	states[bestIndex].isOpen = false;
	--numOpen;

	vector<Move> moves = states[bestIndex].moves();
	++Statistics::statesExpanded;
	Statistics::numChildren += moves.size();
	for (vector<Move>::const_iterator m = moves.begin();
	     m != moves.end(); ++m) {
	    ++Statistics::statesGenerated;
	    // % is extremely slow on int64_t...
	    if ((Statistics::statesGenerated & 0xffffff) == 0) {
		cout << "\nbest: " << states[bestIndex]
		     << "\n  open: " << numOpen
		     << "\nstates: " << states.size()
		     << " \t(" << (states.size() * sizeof(AStarState)) / 1000000
		     << "M/" << (states.capacity() * sizeof(AStarState)) / 1000000
		     << "M)\nhashes: " << hashTable.size()
		     << " \t(" << (states.size() * sizeof(int)) / 1000000
		     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
		     << ")\n";
		Statistics::print(cout);
	    }
	    AStarState newState(states[bestIndex], *m);
	    newState.predecessor = bestIndex;

	    int minMovesLeft = newState.minMovesLeft();
	    if (minMovesLeft == 0) { // special property of our heuristic...
		Statistics::timer.stop();
		cout << "Found solution.\n"
		     << "\nstates: " << states.size()
		     << " \t(" << (states.size() * sizeof(AStarState)) / 1000000
		     << "M/" << (states.capacity() * sizeof(AStarState)) / 1000000
		     << "M)\nhashes: " << hashTable.size()
		     << " \t(" << (states.size() * sizeof(int)) / 1000000
		     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
		     << "M)\n";
		Statistics::print(cout);
		AStarState* pNode = &states[bestIndex];
		AStarState* pNextNode;

		bool done = false;
		while (!done) {
		    if (pNode->predecessor == 1) // start node
			done = true;
		    pNextNode = pNode;
		    DEBUG0("predecessor of " << *pNode
			   << " is " << pNode->predecessor);
		    pNode = &states[pNode->predecessor];

		    vector<Move> moves = pNode->moves();

		    for (vector<Move>::const_iterator m = moves.begin();
			 m != moves.end(); ++m) {
			if (AStarState(*pNode, *m) == *pNextNode) {
			    solution.push_front(*m);
			    break;
			}
		    }
		}
		solution.push_back(*m);
		return solution;
	    }
	    if (newState.minTotalMoves() > maxMoves) {
		DEBUG0("State exceeds move limit.");
		continue;
	    }

	    // we have a monotone heuristic
	    assert(newState.minTotalMoves() >= minMinTotalMoves);

	    if (states.size() == MAX_STATES) {
		DEBUG1("State table full.");
		exit(1);
	    }

	    hashInsert(newState);
	    DEBUG0("inserted" << newState);
	}
    }

    DEBUG1("Queue empty; no solution possible.");

    Statistics::timer.stop();
    return deque<Move>();
}

//deque<Move> buildSolution()
