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

#ifndef BOARD_HH
#define BOARD_HH

#include <iosfwd>
#include <map>
#include <string>

class State;

#include "Atom.hh"
#include "Pos.hh"

using namespace std;

// a complete representation of a board state, useful for I/O
class Board {
public:
    Board() { }			// leave everything blank
    Board(map<string, string> lines, string key, int len);
    Board(const State& state);

    const Atom& field(int x, int y) const { return myFields[x][y]; }
    const Atom& field(Pos p) const { return myFields[p.x()][p.y()]; }
    Pos find(const Atom& atom) const;

private:
    void floodFill(int x, int y);
    
    Atom myFields[XSIZE][YSIZE];
    Atom myGoal[XSIZE][YSIZE];
};

ostream& operator<<(ostream& out, const Board& board);

#endif
