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

#ifndef DIR_HH
#define DIR_HH

#include <stdlib.h>

#include <iosfwd>

#include "Size.hh"

using namespace std;

enum Dir { NONE = 0, UP = -XSIZE, DOWN = XSIZE, LEFT = -1, RIGHT = 1 };

ostream& operator <<(ostream& out, Dir dir);

static const Dir DIRS[4] = { UP, DOWN, LEFT, RIGHT };

inline int noOfDir(Dir dir) {
    switch(dir) {
    case UP: return 0;
    case DOWN: return 1;
    case LEFT: return 2;
    case RIGHT: return 3;
    }
    abort();
    return 0;			// avoid stupid cxx warning
}

inline int noOfOppositeDir(Dir dir) {
    switch(dir) {
    case UP: return 1;
    case DOWN: return 0;
    case LEFT: return 3;
    case RIGHT: return 2;
    }
    abort();
    return 0;
}

#endif
