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
#include <iomanip>
#include <map>

#include "Level.hh"

Level::Level(istream& in) {
    map<string, string> lines;
    string line;

    while (getline(in, line)) {
	string::size_type equalPos = line.find('=');
	if (equalPos == string::npos)
	    continue;
	string key   = line.substr(0, equalPos);
	string value = line.substr(equalPos + 1);

	lines[key] = value;
    }

    myStartBoard = Board(lines, "feld", 2);
    myGoal       = Board(lines, "mole", 1);
}

void Level::printStats() const {
    map<Atom, int> atomCounts;
    int numAtoms = 0;
    for (int y = 0; y < Board::YSIZE; ++y) {
	for (int x = 0; x < Board::XSIZE; ++x) {
	    if (startBoard().field(x, y).isAtom()) {
		++atomCounts[startBoard().field(x, y)];
		++numAtoms;
	    }
	}
    }
    map<int, int> bucketSizes;
    cout << "Atoms: " << setw(2) << numAtoms <<  '|';
    for (map<Atom, int>::const_iterator p = atomCounts.begin();
	 p != atomCounts.end(); ++p) {
	++bucketSizes[p->second];
    }
    //for (map<int, int>::const_iterator p = bucketSizes.begin();
    //p != bucketSizes.end(); ++p) {
    for (int i = 1; i <= 6; ++i)
	if (bucketSizes[i] == 0)
	    cout <<  "  |";
	else
	    cout << setw(2) << bucketSizes[i] << '|';
    /*
	if (p->first == 1)
	    cout << "\tunique:\t";
	else if (p->first == 2)
	    cout << "\ttwice:\t";
	else if (p->first == 3)
	    cout << "\tthrice:\t";
	else
	    cout << '\t' << p->first << " times:\t";
	cout << -> second;
    */
}

ostream& operator<<(ostream& out, const Level& level) {
    return out << level.goal() << level.startBoard();
}
