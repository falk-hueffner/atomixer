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

#ifdef HAVE_SSTREAM
# include <sstream>
#else
# include <strstream>
#endif
#include <string>

#include "Board.hh"
#include "State.hh"
#include "State2.hh"
#include "Problem.hh"

using namespace std;

Board::Board(map<string, string> lines, string key, int len) {
    for (int y = 0; y < YSIZE; ++y) {
#ifdef HAVE_SSTREAM
	ostringstream keyStream;
#else
	ostrstream keyStream;
#endif
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
    for (int x = 0; x < XSIZE; ++x) {
	floodFill(x, 0);
	floodFill(x, YSIZE - 1);
    }
    for (int y = 0; y < YSIZE; ++y) {
	floodFill(0, y);
	floodFill(XSIZE - 1, y);
    }
}

Board::Board(const State& state) {
    for (Pos pos = 0; pos != Pos::end(); ++pos)
	if (Problem::isBlock(pos))
	    myFields[pos.x()][pos.y()] = Atom("#");
	else
	    myFields[pos.x()][pos.y()] = Atom(".");
    for (int i = 0; i < NUM_ATOMS; ++i) {
	Pos pos = state.atomPositions()[i];
	myFields[pos.x()][pos.y()] = Problem::atom(i);
    }

}

Board::Board(const State2& state) {
    for (Pos pos = 0; pos != Pos::end(); ++pos)
	if (Problem::isBlock(pos))
	    myFields[pos.x()][pos.y()] = Atom("#");
	else
	    myFields[pos.x()][pos.y()] = Atom(".");
    for (int i = 0; i < NUM_ATOMS; ++i) {
	Pos pos = state.atomPositions[i];
	myFields[pos.x()][pos.y()] = Problem::atom(i);
    }

}

Pos Board::find(const Atom& atom) const {
    for (int x = 0; x < XSIZE; ++x) {
	for (int y = 0; y < YSIZE; ++y) {
	    if (field(x, y) == atom)
		return Pos(x, y);
	}
    }

    return Pos::null();
}

void Board::floodFill(int x, int y) {
    if (!field(x, y).isEmpty())
	return;
    myFields[x][y] = Atom("#");
    if (x > 0)
	floodFill(x - 1, y);
    if (y > 0)
	floodFill(x, y - 1);
    if (x < XSIZE - 1)
	floodFill(x + 1, y);
    if (y < YSIZE - 1)
	floodFill(x, y + 1);
}

ostream& operator<<(ostream& out, const Board& board) {
        out << "  ";
    for (int i = 0; i < XSIZE; ++i)
	out << "   " << char(i + 'A');
    out << "\n   +";
    for (int i = 0; i < XSIZE; ++i)
	out << "---+";
    out << endl;
    for (int y = 0; y < YSIZE; ++y) {
	string sublines[3];
	bool allBlock = true;
	for (int x = 0; x < XSIZE; ++x) {
	    if (!board.field(x, y).isBlock())
		allBlock = false;
	    vector<string> atomLines = board.field(x, y).toAscii();
	    for (int i = 0; i < 3; ++i) 
		sublines[i] += atomLines[i] + '|';
	}
	if (!allBlock) {
	    for (int i = 0; i < 3; ++i) {
		if (i == 1)
		    out << ' ' << y << (y >= 10 ? "|" : " |");
		else
		    out << "   |";
		out << sublines[i] << endl;
	    }

	    out << "   +";
	    for (int i = 0; i < XSIZE; ++i)
		out << "---+";
	    out << endl;
	}
    }

    return out;
}
