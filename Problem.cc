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

#include <iostream>
#include <queue>
#include <set>

#include "parameters.hh"
#include "Dir.hh"
#include "Level.hh"
#include "Problem.hh"

using namespace std;

bool Problem::myIsBlock[NUM_FIELDS];
Pos Problem::myStartPositions[NUM_ATOMS];
Pos Problem::myGoalPositions[NUM_ATOMS];
int Problem::myNumIdentical[NUM_ATOMS];
int Problem::myFirstIdentical[NUM_ATOMS];
int Problem::goalDists[NUM_ATOMS][NUM_FIELDS];
int Problem::goalNr;
Atom Problem::atoms[NUM_ATOMS];
#ifdef DO_REVERSE_SEARCH
HashTable<RevState> Problem::_revStates;
#endif

void Problem::setLevel(const Level& level) {
    typedef multimap<Atom, Pos> AtomMap;
    AtomMap startAtoms;
    int numFields = 0;

    for (Pos pos = 0; pos != Pos::end(); ++pos) {
	const Atom& atom = level.startBoard().field(pos);
	if (atom.isAtom())
	    startAtoms.insert(make_pair(atom, pos));

	myIsBlock[pos.fieldNumber()] = atom.isBlock();
	if (!atom.isBlock())
	    ++numFields;
    }

    assert(startAtoms.size() == NUM_ATOMS);

    int numUnique = 0, numPaired = 0, numMulti = 0;
    for (AtomMap::const_iterator pf = startAtoms.begin();
	 pf != startAtoms.end(); ++pf) {
	const Atom& atom = pf->first;
	Pos pos = pf->second;
	switch (startAtoms.count(atom)) {
	case 1:
	    atoms[numUnique] = atom;
	    myStartPositions[numUnique] = pos;

	    ++numUnique;
	    break;
	case 2: {
	    ++pf;
	    assert(pf->first == atom);
	    Pos pos2 = pf->second;
	    atoms[PAIRED_START + numPaired    ] = atom;
	    atoms[PAIRED_START + numPaired + 1] = atom;
	    myStartPositions[PAIRED_START + numPaired    ] = pos;
	    myStartPositions[PAIRED_START + numPaired + 1] = pos2;

	    numPaired += 2;
	    break;
	}
	default:
	    cout << "num: " << startAtoms.count(atom) << endl;
	    for (int nr = MULTI_START + numMulti;
		 nr < MULTI_START + numMulti + startAtoms.count(atom); ++nr) {
		if (nr != MULTI_START + numMulti)
		    ++pf;
		assert(pf->first == atom);
		atoms[nr] = atom;
		myStartPositions[nr] = pf->second;
		myNumIdentical[nr] = startAtoms.count(atom);
		myFirstIdentical[nr] = MULTI_START + numMulti;
	    }
	    numMulti += startAtoms.count(atom);
	}
    }
    cout << "unique: " << numUnique << "; paired: " << numPaired
	 << "; multi: " << numMulti << endl;
    cout << "Fields: " << numFields << endl;
    assert(numUnique == NUM_UNIQUE);
    assert(numPaired == NUM_PAIRED);
    assert(numMulti == NUM_MULTI);
    cout << "returning from " << __PRETTY_FUNCTION__ << endl;
}

void Problem::setGoal(const Level& level, int goalPosNr) {
    cout << "Problem::setGoal\n";
    goalNr = goalPosNr;
    Pos d = level.goalPos(goalPosNr);
    int dx = d.x(), dy = d.y();
    typedef multimap<Atom, Pos> AtomMap;
    AtomMap goalAtoms;

    for (Pos pos = 0; pos != Pos::end(); ++pos) {
	const Atom& atom = level.goal().field(pos);
	if (atom.isAtom()) {
	    Pos goalPos(pos.x() + dx, pos.y() + dy);
	    goalAtoms.insert(make_pair(atom, goalPos));
	}
    }
    // FIXME add asserts
    for (int i = 0; i < NUM_UNIQUE; ++i) {
	const Atom& atom = level.startBoard().field(myStartPositions[i]);
	myGoalPositions[i] = goalAtoms.find(atom)->second;
    }
    for (int i = PAIRED_START; i < PAIRED_END; i += 2) {
	const Atom& atom = level.startBoard().field(myStartPositions[i]);
	AtomMap::const_iterator p = goalAtoms.find(atom);
	myGoalPositions[i] = p->second;
	++p;
	myGoalPositions[i + 1] = p->second;
    }
    cout << "MULTI_START = " << MULTI_START << endl;
    for (int i = MULTI_START; i < NUM_ATOMS; i += Problem::numIdentical(i)) {
	const Atom& atom = level.startBoard().field(myStartPositions[i]);
	AtomMap::const_iterator p = goalAtoms.find(atom);
	for (int j = 0; j < Problem::numIdentical(i); ++j, ++p)
	    myGoalPositions[i + j] = p->second;
	cout << "Atom Nr." << i << ": " << Problem::numIdentical(i)
	     << " times, starting positions";
	for (int t = i; t < i + Problem::numIdentical(i); ++t)
	    cout << ' ' << myStartPositions[t];
	cout << "; goal positions";
	for (int t = i; t < i + Problem::numIdentical(i); ++t)
	    cout << ' ' << myGoalPositions[t];
	cout << endl;
    }

    for (int i = 0; i < NUM_ATOMS; ++i)
#ifdef DO_BACKWARD_SEARCH
	calcDists(goalDists[i], myStartPositions[i]);
#else
	calcDists(goalDists[i], myGoalPositions[i]);
#endif

#ifdef DO_REVERSE_SEARCH
    calcCloseStates();
#endif
}

#ifdef DO_REVERSE_SEARCH
void Problem::calcCloseStates() {
    HashTable<RevState> states[REV_SEARCH_MAX_GOAL_DIST + 1];
    states[0].insertNew(RevState(myGoalPositions));

    for (int dist = 1; dist <= REV_SEARCH_MAX_GOAL_DIST; ++dist) {
	for (HashTable<RevState>::Iterator poldState = states[dist - 1].begin();
	     poldState != states[dist - 1].end(); ++poldState) {
	    const RevState& oldState = *poldState;
	    vector<Move> moves = oldState.rmoves();
	    //cout << moves.size() << " rmoves.\n";
	    for (int m = 0; m < moves.size(); ++m) {
		RevState newState(oldState, moves[m]);
		for (int dist2 = 0; dist2 < dist; ++dist2)
		    if (states[dist2].find(newState) != states[dist2].end())
			goto next;
		states[dist].insert(newState);
		next:
		;
	    }
	}
	cout << states[dist].size() << " states of dist " << dist << endl;
    }

    _revStates.clear();
    for (int dist = 0; dist <= REV_SEARCH_MAX_GOAL_DIST; ++dist)
	for (HashTable<RevState>::Iterator pState = states[dist].begin();
	     pState != states[dist].end(); ++pState)
	    _revStates.insertIfBetter(*pState);
    cout << _revStates.size() << " reverse states" << endl;
}
#endif

// store in dist[p] the minimum move distance to goal
void Problem::calcDists(int dists[NUM_FIELDS], Pos goal) {
    for (int i = 0; i < NUM_FIELDS; ++i)
	dists[i] = 100000;

    dists[goal.fieldNumber()] = 0;

    queue<Pos> q;

    q.push(goal);

    while (!q.empty()) {
	Pos p = q.front();
	int dist = dists[p.fieldNumber()];
	for (int dirNo = 0; dirNo < 4; ++dirNo) {
	    Dir dir = DIRS[dirNo];
	    for (Pos tp = p + dir; !myIsBlock[tp.fieldNumber()]; tp += dir) {
		if (dists[tp.fieldNumber()] > dist + 1) {
		    dists[tp.fieldNumber()] = dist + 1;
		    q.push(tp);
		}
	    }
	}
	q.pop();
    }

    /*
    cout << "Distances to " << goal << ":\n";
    for (int y = 0; y < YSIZE; ++y) {
	for (int x = 0; x < XSIZE; ++x) {
	    //cout << dists[Pos(x, y).fieldNumber()] << ' ';
	    int d = dists[Pos(x, y).fieldNumber()];
	    if (d < 100000)
		cout << d << ' ';
	    else
		cout << '.' << ' ';
	}
	cout << endl;
    }
    */
}
