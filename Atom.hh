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

#ifndef ATOM_HH
#define ATOM_HH

#include <string>
#include <vector>

using namespace std;

class Atom {
public:
    Atom() : myID(0) { }
    Atom(const string& s);

    bool isEmpty() const { return myID == 0; }
    bool isBlock() const { return myID == 1; }

    vector<string> toAscii() const;

private:
    enum Connections { C_N  = (1 << 0), C_NE = (1 << 1),  C_E  = (1 << 2),
		       C_SE = (1 << 3), C_S  = (1 << 4),  C_SW = (1 << 5),
                       C_W  = (1 << 6), C_NW = (1 << 7),  C_N2 = (1 << 8),
                       C_E2 = (1 << 9), C_S2 = (1 << 10), C_W2 = (1 << 11) };

    static const char chars[];
    static const int coords[][2];
    static const char bindings[][4];
    static const int NUM_DIRS = 8;

    int myID;
    int myNumConnections[NUM_DIRS];
};

#endif
