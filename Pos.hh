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

#ifndef POS_HH
#define POS_HH

#include <iosfwd>

#include "Dir.hh"
#include "Size.hh"

using namespace std;

class Pos {
public:
    Pos() { }		// leave uninitialized
    Pos(int np) : p(np) { }
    Pos(int x, int y) : p(y * XSIZE + x) { }

    int x() const { return p % XSIZE; }
    int y() const { return p / XSIZE; }
    int fieldNumber() const { return p; }

    static Pos null() { return Pos(-1); }
    static Pos end() { return Pos(NUM_FIELDS); }
    bool ok() const { return p != -1; }

    bool operator==(Pos other) const { return p == other.p; }
    bool operator!=(Pos other) const { return p != other.p; }
    bool operator<(Pos other) const { return p < other.p; }
    bool operator>(Pos other) const { return p > other.p; }
    bool operator<=(Pos other) const { return p <= other.p; }
    Pos& operator+=(Dir dir) { p += dir; return *this; }
    Pos& operator++() { ++p; return *this; }
    Pos operator+(Dir dir) const { return Pos(p + dir); }
    Pos operator-(Dir dir) const { return Pos(p - dir); }

private:
    int p;
};

ostream& operator<<(ostream& out, const Pos& pos);

#endif
