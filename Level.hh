/*
  atomixer -- Atoimx puzzle solver
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

#ifndef LEVEL_HH
#define LEVEL_HH

#include <iosfwd>
#include <vector>

#include "Board.hh"
#include "Pos.hh"

using namespace std;

class Level {
public:
    Level(istream& in);

    const Board& startBoard() const { return myStartBoard; }
    const Board& goal() const { return myGoal; }

    void printStats() const;
    Pos goalPos(int goalPosNr) const { return myGoalPositions[goalPosNr]; }
    int numGoals() const { return myGoalPositions.size(); }

private:
    void findGoalPositions();

    vector<Pos> myGoalPositions;
    Board myStartBoard, myGoal;
};

ostream& operator<<(ostream& out, const Level& level);

#endif
