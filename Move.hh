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

#ifndef MOVE_HH
#define MOVE_HH

#include <iosfwd>

#include "Dir.hh"
#include "Pos.hh"

using namespace std;

class Move {
public:
    Move() { }			// leave uninitialized
    Move(int natomNr, Pos np1, Pos np2, Dir ndir)
	: myAtomNr(natomNr), myPos1(np1), myPos2(np2), myDir(ndir) { }

    int atomNr() const { return myAtomNr; }
    Pos pos1() const { return myPos1; }
    Pos pos2() const { return myPos2; }
    Dir dir() const { return myDir; }

private:
    int myAtomNr;
    Pos myPos1, myPos2;
    Dir myDir;
};

ostream& operator <<(ostream& out, const Move& move);

#endif
