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

#include <iostream>

#include "Atom.hh"

using namespace std;

const char Atom::idMap[][2] = {{'.', ' '},
			       {'#', '#'},
			       {'1', 'H'},
			       {'2', 'C'},
			       {'3', 'O'},
			       {'4', 'N'},
			       {'5', 'S'},
			       {'6', 'F'},
			       {'7', 'L'}, // Cl actually
			       {'o', 'o'}, // "crystal"
			       {'A', '-'},
			       {'C', '|'},
			       {  0,   0}};

const int Atom::coords[][2] = {{0, 1}, {0, 2}, {1, 2}, {2, 2},
			       {2, 1}, {2, 0}, {1, 0}, {0, 0}};

const char Atom::bindings[][4] = {{' ', '|',  '"', '3'},
				  {' ', '/'           },
				  {' ', '-',  '=', '3'},
				  {' ', '\\'          },
				  {' ', '|',  '"', '3'},
				  {' ', '/'           },
				  {' ', '-',  '=', '3'},
				  {' ', '\\'          }};

/*
  1  2  3  4  5  6  7  o        A
  H  C  O  N  ?  F  Cl crystal  horizontal floating connection

  Katomic atom description:
  h a b    A      E
   \|/     "      3
  g-o-c  D=o=B  H3o3F
   /|\     "      3
  f e d    C      G

*/

Atom::Atom() : myID(' ') {
    for (int i = 0; i < NUM_DIRS; ++i)
	myNumConnections[i] = 0;
}

Atom::Atom(const string& s) {
    for (int i = 0; i < NUM_DIRS; ++i)
	myNumConnections[i] = 0;

    myID = 0;
    for (int i = 0; idMap[i][0] != 0; ++i) {
	if (s[0] == idMap[i][0]) {
	    myID = idMap[i][1];
	    break;
	}
    }
    if (myID == 0) {
	cerr << "Warning: unrecognized atom type\n";
	myID = '#';
    }

    for (int i = 2; i < s.length(); ++i) {
	if ((s[i] >= 'a' && s[i] <= 'h'))
	    myNumConnections[s[i] - 'a'] = 1;
	else if (s[i] >= 'A' && s[i] <= 'D')
	    myNumConnections[(s[i] - 'A') * 2] = 2;
	else if (s[i] >= 'E' && s[i] <= 'H')
	    myNumConnections[(s[i] - 'E') * 2] = 3;
	else
	    cerr << "Warning: unrecognized connection letter\n";
    }
}

bool Atom::operator<(const Atom& other) const {
    if (myID != other.myID)
	return myID < other.myID;
    for (int i = 0; i < NUM_DIRS; ++i)
	if (myNumConnections[i] != other.myNumConnections[i])
	    return myNumConnections[i] < other.myNumConnections[i];

    return false;
}

bool Atom::operator==(const Atom& other) const {
    if (myID != other.myID)
	return false;
    for (int i = 0; i < NUM_DIRS; ++i)
	if (myNumConnections[i] != other.myNumConnections[i])
	    return false;

    return true;
}

vector<string> Atom::toAscii() const {
    if (isBlock())
	return vector<string>(3, "###");

    vector<string> ascii(3, "   ");

    if (isEmpty())
	return ascii;

    for (int i = 0; i < NUM_DIRS; ++i)
	ascii[coords[i][0]][coords[i][1]] = bindings[i][myNumConnections[i]];

    ascii[1][1] = myID;

    return ascii;
}

