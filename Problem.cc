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

#include <queue>

#include "Dir.hh"
#include "Level.hh"
#include "Problem.hh"

using namespace std;

bool Problem::myIsBlock[NUM_FIELDS];
Pos Problem::myStartPositions[NUM_ATOMS];
Pos Problem::myGoalPositions[NUM_ATOMS];
int Problem::goalDists[NUM_ATOMS][NUM_FIELDS];

void Problem::setLevel(const Level& level) {
    int numAtoms = 0;
    for (int y = 0; y < YSIZE; ++y) {
	for (int x = 0; x < XSIZE; ++x) {
	    const Atom& atom = level.startBoard().field(x, y);
	    if (atom.isAtom())
		myStartPositions[numAtoms++] = Pos(x, y);
	    assert(numAtoms <= NUM_ATOMS);
	    myIsBlock[Pos(x, y).fieldNumber()] = atom.isBlock();
	}
    }
    assert(numAtoms == NUM_ATOMS);
}

void Problem::setGoal(const Level& level, int goalPosNr) {
    int atomNo = 0;
    Pos d = level.goalPos(goalPosNr);
    int dx = d.x(), dy = d.y();
    for (int i = 0; i < NUM_ATOMS; ++i) {
	const Atom& atom = level.startBoard().field(myStartPositions[i].fieldNumber());
	Pos goalPos = level.goal().find(atom);
	Pos realGoalPos = Pos(goalPos.x() + dx, goalPos.y() + dy);
	assert(realGoalPos.ok());
	myGoalPositions[atomNo++] = realGoalPos;
    }

    for (int i = 0; i < NUM_ATOMS; ++i)
	calcDists(goalDists[i], myGoalPositions[i]);
}

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
	    Dir dir = dirs[dirNo];
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
