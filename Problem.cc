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

#include "Level.hh"
#include "Problem.hh"

bool Problem::myIsBlock[NUM_FIELDS];
vector<Pos> Problem::myStartPositions;
vector<Pos> Problem::myGoalPositions;

void Problem::setProblem(const Level& level, int goalPosNr) {
    Pos d = level.goalPos(goalPosNr);
    int dx = d.x(), dy = d.y();
    for (int x = 0; x < XSIZE; ++x) {
	for (int y = 0; y < YSIZE; ++y) {
	    const Atom& atom = level.startBoard().field(x, y);
	    if (atom.isAtom()) {
		myStartPositions.push_back(Pos(x, y));
		Pos goalPos = level.goal().find(atom);
		Pos realGoalPos = Pos(goalPos.x() + dx, goalPos.y() + dy);
		assert(realGoalPos.ok());
		myGoalPositions.push_back(realGoalPos);
	    }
	    myIsBlock[Pos(x, y).fieldNumber()] = atom.isBlock();
	}
    }
}
