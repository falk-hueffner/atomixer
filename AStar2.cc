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
#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
typedef long long int64_t;
#endif

#include <algorithm>
#include <deque>
#include <iostream>
#include <vector>

#include "AStar2.hh"
#include "State2.hh"
#include "IDAStarState.hh"
#include "Timer.hh"

#define DEBUG0(x) do { } while (0)
#define DEBUG1(x) cout << x << endl

using namespace std;

extern int64_t totalNodesGenerated;

static const double LOAD_FACTOR = 1.4;

static const unsigned int MAX_STATES = (unsigned int)
    (MEMORY / (sizeof(int) * LOAD_FACTOR + sizeof(State2)));
static const unsigned int MAX_HASHES = (unsigned int)
    (MAX_STATES * LOAD_FACTOR);

vector<State2> states;
vector<int> hashTable;
deque<Move> solution;

static int maxMoves;
static int minMinTotalMoves;
static int firstOpen;
static int searchIndex;
static int numOpen;
static int64_t nodesGenerated;
static Timer timer;

#ifdef DO_MREC
static int mrecMoves;
static IDAStarState mrecState;
static bool idastar();
#endif

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

deque<Move> aStar2(const State2& start, int nmaxMoves) {
    maxMoves = nmaxMoves;
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
    
    DEBUG1("start state: " << start);
    hashInsert(start);
    ++totalNodesGenerated;

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
	    ++totalNodesGenerated;
	    if ((++nodesGenerated & 0xfffff) == 0) // % is extremely slow on int64_t
		cout << "best: " << states[bestIndex] << endl
		     << " Nodes: " << nodesGenerated
		     << " nodes/second: "
		     << (int64_t) (double(nodesGenerated) / timer.seconds())
		     << "\n  open: " << numOpen
		     << "\nstates: " << states.size()
		     << " \t(" << (states.size() * sizeof(State2)) / 1000000
		     << "M/" << (states.capacity() * sizeof(State2)) / 1000000
		     << "M)\nhashes: " << hashTable.size()
		     << " \t(" << (states.size() * sizeof(int)) / 1000000
		     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
		     << "M)\n";
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
		// This can't happen if the heuristic is monotone
		DEBUG1("minTotalMoves of " << newState2.minTotalMoves()
		       << " smaller than previous minMinTotalMoves of "
		       << minMinTotalMoves);
		assert(0);
	    }

	    if (states.size() == MAX_STATES) {
		DEBUG1("State table full.");
#ifdef DO_MREC
		for (int i = firstOpen; i < states.size(); ++i) {
		    DEBUG1("MREC for " << states[i]);
		    mrecState = states[i].atomPositions;
		    mrecMoves = states[i].numMoves;
		    if (idastar()) {
			// FIXME copy&paste
			cout << "Found solution.\n"
			     << "\nstates: " << states.size()
			     << " \t(" << (states.size() * sizeof(State2)) / 1000000
			     << "M/" << (states.capacity() * sizeof(State2)) / 1000000
			     << "M)\nhashes: " << hashTable.size()
			     << " \t(" << (states.size() * sizeof(int)) / 1000000
			     << "M/" << (hashTable.capacity() * sizeof(int)) / 1000000
			     << "M)\n";
			State2* pNode = &states[i];
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

			return solution;
			//end FIXME
		    }
		}
		DEBUG1("IDA* for all open nodes left didn't find anything.");
		return deque<Move>();
#else
		exit(1);
#endif
	    }

	    hashInsert(newState2);
	    DEBUG0("inserted" << newState2);
	}
    }

    DEBUG1("Queue empty; no solution possible.");

    return deque<Move>();
}

#ifdef DO_MREC
static bool idastar() {
    //static int64_t lastOutput;

    DEBUG0(spaces(mrecMoves) << "dfs: moves =  " << mrecMoves
	   << " state = " << mrecState);
    if (mrecMoves + mrecState.minMovesLeft() > maxMoves)
	return false;

    /*
    if (nodesGenerated - lastOutput > 2000000) {
	lastOutput = nodesGenerated;
	cout << mrecState << endl
	     << " Nodes: " << nodesGenerated
	     << " moves = " << mrecMoves
	     << " nodes/second: "
	     << (int64_t) (double(nodesGenerated) / timer.seconds())
	     << endl;
    }
    */

    // generate all moves...
    for (int atomNo = 0; atomNo < NUM_ATOMS; ++atomNo) {
	Pos startPos = mrecState.position(atomNo);
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    DEBUG0(spaces(mrecMoves) << "moving " << atomNo << " @ " << startPos
		   << ' ' << dir);
	    Pos pos;
	    for (pos = startPos + dir; !mrecState.isBlocking(pos); pos += dir) { }
	    Pos newPos = pos - dir;
	    if (newPos != startPos) {
		DEBUG0(spaces(mrecMoves) << "moves to " << newPos);
		IDAStarMove move(startPos, newPos);
		mrecState.apply(move);
		++mrecMoves;
		++nodesGenerated;
		++totalNodesGenerated;
		if (mrecState.minMovesLeft() == 0 || idastar()) {
		    solution.push_front(Move(atomNo, startPos, newPos, dir));
		    return true;
		}
		--mrecMoves;
		mrecState.undo(move);
	    }
	}
    }

    return false;
}
#endif
