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
    Atom();
    Atom(const string& s);

    bool isEmpty() const { return myID == ' '; }
    bool isBlock() const { return myID == '#'; }
    bool isAtom()  const { return !isEmpty() && !isBlock(); }

    vector<string> toAscii() const;

    bool operator<(const Atom& other) const;
    bool operator==(const Atom& other) const;

private:
    static const char idMap[][2];
    static const int coords[][2];
    static const char bindings[][4];
    static const int NUM_DIRS = 8;

    char myID;
    int myNumConnections[NUM_DIRS];
};

#endif
