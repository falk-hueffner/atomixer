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
#include <vector>

#include "State2.hh"
#include "Timer.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

static const double LOAD_FACTOR = 1.4;

static const unsigned int MEMORY = 320 * 1024 * 1024;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(State2)));
static const unsigned int MAX_HASHES = (unsigned int)
    (MAX_STATES * LOAD_FACTOR);

vector<State2> states;
vector<int> hashTable;

static int minMinTotalMoves;
static int firstOpen;
static int searchIndex;
static int numOpen;
static long long nodesGenerated;
static Timer timer;

void hashInsert(const State2& state) {
    DEBUG0("inserting " << state);
    unsigned int hash = state.hash() % hashTable.size();
    DEBUG0(" hash = " << hash);
    while (true) {
	if (hashTable[hash] == 0) {
	    DEBUG0(" hash = " << hash << " empty. Putting it there.");
	    states.push_back(state);
	    ++numOpen;
	    DEBUG0(" pushed " << state);
	    hashTable[hash] = states.size() - 1;

	    return;
	} else {
	    State2& oldState2 = states[hashTable[hash]];
	    DEBUG0(" hash = " << hash << ": " << oldState2 << " already there.");
	    if (oldState2 == state) {
		DEBUG0(" hash = " << hash << ": "
		       << oldState2 << " equals new state.");
		if (state.numMoves < oldState2.numMoves) {
		    DEBUG0(" New way to this state is shorter.\n");
		    oldState2.numMoves = state.numMoves;
		    oldState2.isOpen = true;
		    ++numOpen;
		    if (hashTable[hash] < firstOpen) {
			firstOpen = hashTable[hash];
		    }
		    return;
		} else {
		    DEBUG0(" We already know a better way.");
		    return;
		}
	    }
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }
}

int findBest(int maxMoves) {
    DEBUG0("--- findBest ---");
    int numNodes = 0;
    ++searchIndex;
    while (true) {
	DEBUG0("Looking for states with max. " << minMinTotalMoves
	       << " minTotalMoves, starting from " << searchIndex
	       << " (" << states.size() << " states).");

	bool alreadyRestarted = false;
	while (true) {
	    for (; searchIndex < states.size(); ++searchIndex) {
		if (states[searchIndex].isOpen) {
		    if (states[searchIndex].minTotalMoves() <= minMinTotalMoves) {
			if (states[searchIndex].minTotalMoves() != minMinTotalMoves)
			    DEBUG0("*** WEIRD ***");
			DEBUG0("Best at " << searchIndex
			       << ". Looked at " << numNodes << " states.");
			return searchIndex;
		    }
		}
		++numNodes;
	    }
	    DEBUG0("Arrived at end of states.");
	    if (alreadyRestarted)
		break;
	    DEBUG0("Finding first open state.");
	    for (; firstOpen < states.size() && !states[firstOpen].isOpen;
		 ++firstOpen) { }
	    if (firstOpen == states.size()) {
		DEBUG0("No open node left at all!");
		return 0;
	    }
	    DEBUG0("First open: " << firstOpen << '.');
	    searchIndex = firstOpen;
	    alreadyRestarted = true;
	}
	DEBUG0("No states with max. " << minMinTotalMoves << " minTotalMoves.");
	if (++minMinTotalMoves > maxMoves) {
	    DEBUG0("No states within limit of " << maxMoves << " moves.");
	    return 0;
	}
	searchIndex = firstOpen;
    }
}

deque<Move> aStar2(const State2& start, int maxMoves) {
    nodesGenerated = 0;
    if (start.minTotalMoves() > maxMoves)
	return deque<Move>();	// saves the allocations which can take quite some time

    DEBUG0("MAX_STATES = " << MAX_STATES);
    DEBUG0("MAX_HASHES = " << MAX_HASHES);
    states.clear();
    states.push_back(State2());	// 0 reserved for 'empty'
    states.reserve(MAX_STATES);
    hashTable.clear();
    hashTable.resize(MAX_HASHES);

    minMinTotalMoves = 0;
    firstOpen = 1;
    searchIndex = firstOpen;
    numOpen = 0;
    
    static long long totalNodes = 0;
    
    DEBUG1("start state: " << start);
    hashInsert(start);
    ++totalNodes;

    timer.reset();
    while (true) {
	int bestIndex = findBest(maxMoves);
	DEBUG0("bestIndex: " << bestIndex);
	if (bestIndex == 0)
	    break;
	DEBUG0("best: " << states[bestIndex]);
	states[bestIndex].isOpen = false;
	--numOpen;

	vector<Move> moves = states[bestIndex].moves();
	for (vector<Move>::const_iterator m = moves.begin();
	     m != moves.end(); ++m) {
	    DEBUG0("moving " << states[bestIndex] << ' ' << *m);
	    State2 newState2(states[bestIndex], *m);
	    newState2.predecessor = bestIndex;
	    DEBUG0("New state: " << newState2);

	    int minMovesLeft = newState2.minMovesLeft();
	    if (minMovesLeft == 0) { // special property of our heuristic...
		cout << "Found solution.\n"
		     << "\nstates: " << states.size()
		     << " \t(" << (states.size() * sizeof(State2)) / 1000000
		     << "M/" << (states.capacity() * sizeof(State2)) / 1000000
		     << "M)\nhashes: " << hashTable.size()
		     << " \t(" << (states.size() * sizeof(int)) / 1000000
		     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
		     << "M)\n";
		deque<Move> solution;
		State2* pNode = &states[bestIndex];
		State2* pNextNode;

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

			if (State2(*pNode, *m) == *pNextNode) {
			    solution.push_front(*m);
			    break;
			}
		    }
		}
		solution.push_back(*m);
		return solution;
	    }
	    if (newState2.minTotalMoves() > maxMoves) {
		DEBUG0("State2 exceeds move limit.");
		continue;
	    }
	    if (newState2.minTotalMoves() < minMinTotalMoves) {
		// This can't happen if the heuristic is admissible
		DEBUG1("minTotalMoves of " << newState2.minTotalMoves()
		       << " smaller than previous minMinTotalMoves of "
		       << minMinTotalMoves);
		assert(0);
	    }

	    if (states.size() == MAX_STATES) {
		DEBUG1("State table full; giving up.");
		exit(1);
	    }

	    hashInsert(newState2);
	    DEBUG0("inserted" << newState2);
	    ++totalNodes;
	    if ((++nodesGenerated & 0xfffff) == 0)
		cout << "best: " << states[bestIndex] << endl
		     << " Nodes: " << nodesGenerated
		     << " nodes/second: "
		     << (long long) (double(nodesGenerated) / timer.seconds())
		     << "\n  open: " << numOpen
		     << "\nstates: " << states.size()
		     << " \t(" << (states.size() * sizeof(State2)) / 1000000
		     << "M/" << (states.capacity() * sizeof(State2)) / 1000000
		     << "M)\nhashes: " << hashTable.size()
		     << " \t(" << (states.size() * sizeof(int)) / 1000000
		     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
		     << "M)\n";
	}
    }

    DEBUG1("Queue empty; no solution possible.");

    return deque<Move>();
}

