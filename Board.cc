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

#include <assert.h>

#include <iomanip>
#include <iostream>
#include <map>
#include <strstream>
#include <string>

#include "Board.hh"

using namespace std;

Board::Board(map<string, string> lines, string key, int len) {
    for (int y = 0; y < YSIZE; ++y) {
	strstream keyStream;
	keyStream << key << '_' << setfill('0') << setw(len) << y;
	string line = lines[keyStream.str()];
	    
	assert (line.length() <= XSIZE);
	for (int x = 0; x < line.length(); ++x) {
	    if (line[x] == '.')
		continue;
	    if (line[x] == '#') {
		myFields[x][y] = Atom("#");
	    } else {
		string atom = lines[string("atom_") + line[x]];
		if (atom == "")
		    cerr << "Warning: unrecognized field\n";
		else
		    myFields[x][y] = Atom(atom);
	    }
	}
    }
}

ostream& operator<<(ostream& out, const Board& board) {
        out << "  ";
    for (int i = 0; i < Board::XSIZE; ++i)
	out << "   " << char(i + 'A');
    out << "\n   +";
    for (int i = 0; i < Board::XSIZE; ++i)
	out << "---+";
    out << endl;
    for (int y = 0; y < Board::YSIZE; ++y) {
	string sublines[3];
	bool allEmpty = true;
	for (int x = 0; x < Board::XSIZE; ++x) {
	    if (!board.field(x, y).isEmpty())
		allEmpty = false;
	    vector<string> atomLines = board.field(x, y).toAscii();
	    for (int i = 0; i < 3; ++i) 
		sublines[i] += atomLines[i] + '|';
	}
	if (!allEmpty) {
	    for (int i = 0; i < 3; ++i) {
		if (i == 1)
		    out << ' ' << y << (y >= 10 ? "|" : " |");
		else
		    out << "   |";
		out << sublines[i] << endl;
	    }

	    out << "   +";
	    for (int i = 0; i < Board::XSIZE; ++i)
		out << "---+";
	    out << endl;
	}
    }

    return out;
}
