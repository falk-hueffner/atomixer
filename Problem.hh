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

class Level;
class Board;

#include "parameters.hh"
#include "Atom.hh"
#include "Pos.hh"
#include "Size.hh"
#ifdef DO_REVERSE_SEARCH
#include "RevState.hh"
#include "HashTable.hh"
#endif

using namespace std;

class Problem {
public:
    static void setLevel(const Level& level);
    static void setGoal(const Level& level, int goalPosNr);

    static bool isBlock(Pos p) { return myIsBlock[p.fieldNumber()]; }
#ifdef DO_BACKWARD_SEARCH
    static const Pos* startPositions() { return myGoalPositions; }
    static Pos startPosition(int nr) { return myGoalPositions[nr]; }
    static Pos goalPosition(int nr) { return myStartPositions[nr]; }
#else
    static const Pos* startPositions() { return myStartPositions; }
    static Pos startPosition(int nr) { return myStartPositions[nr]; }
    static Pos goalPosition(int nr) { return myGoalPositions[nr]; }
#endif
    static int numIdentical(int nr) { return myNumIdentical[nr]; }
    static int firstIdentical(int nr) { return myFirstIdentical[nr]; }
    static int goalDist(int atomNr, Pos pos) {
	return goalDists[atomNr][pos.fieldNumber()];
    }
    static Atom atom(int nr) { return atoms[nr]; }
#ifdef DO_REVERSE_SEARCH
    static const HashTable<RevState>& revStates() { return _revStates; }
#endif

    static int goalNr;

private:
    static void calcDists(int dists[NUM_FIELDS], Pos goal);
#ifdef DO_REVERSE_SEARCH
    static void calcCloseStates();
#endif

    static bool myIsBlock[NUM_FIELDS];
    static Pos myStartPositions[NUM_ATOMS];
    static Pos myGoalPositions[NUM_ATOMS];
    static int myNumIdentical[NUM_ATOMS];
    static int myFirstIdentical[NUM_ATOMS];
    static int goalDists[NUM_ATOMS][NUM_FIELDS];
    static Atom atoms[NUM_ATOMS];
#ifdef DO_REVERSE_SEARCH
    static HashTable<RevState> _revStates;
#endif
};

#endif
