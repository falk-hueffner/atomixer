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

#include <iostream>
#include <fstream>

#include "Level.hh"
#include "Problem.hh"
#include "State.hh"

using namespace std;

int main(int argc, char* argv[]) {
    /*
    assert(argc >= 2);

    cout << "Level           |#A|#G|1x|2x|3x|4x|5x|\n"
	 << "----------------+--+--+--+--+--+--+--+\n";
    for (int i = 1; i < argc; ++i) {
	cout << argv[i] << '\t';
	ifstream levelStream(argv[i]);
	assert(levelStream);
	Level level(levelStream);
	level.printStats();
	cout << endl;
    }
    */
    assert(argc == 2);

    ifstream levelStream(argv[1]);
    assert(levelStream);
    Level level(levelStream);
    Problem::setProblem(level, 0);
    State state(Problem::startPositions());
    vector<Move> moves = state.moves();

    cout << level.startBoard();
    for (int i = 0; i < moves.size(); ++i)
	cout << moves[i] << endl;
    cout << "state.minMovesLeft(): " << state.minMovesLeft() << endl;

    return 0;
}
