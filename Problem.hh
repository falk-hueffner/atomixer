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

#ifndef PROBLEM_HH
#define PROBLEM_HH

#include <vector>

class Level;
class Board;

#include "Pos.hh"
#include "Size.hh"

using namespace std;

class Problem {
public:
    static void setLevel(const Level& level);
    static void setGoal(const Level& level, int goalPosNr);

    static bool isBlock(Pos p) { return myIsBlock[p.fieldNumber()]; }
    static const Pos* startPositions() { return myStartPositions; }
    static Pos startPosition(int nr) { return myStartPositions[nr]; }
    static Pos goalPosition(int nr) { return myGoalPositions[nr]; }
    static int goalDist(int atomNr, int fieldNr) {
	return goalDists[atomNr][fieldNr];
    }

private:
    static void calcDists(int dists[NUM_FIELDS], Pos goal);

    static bool myIsBlock[NUM_FIELDS];
    static Pos myStartPositions[NUM_ATOMS];
    static Pos myGoalPositions[NUM_ATOMS];
    static int goalDists[NUM_ATOMS][NUM_FIELDS];
};

#endif
