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
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "Level.hh"
#include "Problem.hh"
#include "State2.hh"
#include "AStar2.hh"
//#include "IDAStar.hh"

using namespace std;

string isotime() {
    time_t timet = time(NULL);
    char timestr[256];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M",
	     localtime(&timet));
    return string(timestr);
}

int main(int argc, char* argv[]) {
    assert(argc >= 2);

    if (argc > 2) {		// info
	cout << "Level                   |#A|#G|1x|2x|3x|4x|5x|\n"
	     << "------------------------+--+--+--+--+--+--+--+\n";
	for (int i = 1; i < argc; ++i) {
	    cout << argv[i] << '\t';
	    ifstream levelStream(argv[i]);
	    assert(levelStream);
	    Level level(levelStream);
	    level.printStats();
	    cout << endl;
	}

	return 0;
    }

    ifstream levelStream(argv[1]);
    assert(levelStream);
    Level level(levelStream);
    cout << level.startBoard();
    Problem::setLevel(level);

    string levelName = string(argv[1]);
    while (levelName.find('/') != string::npos)
	levelName = levelName.substr(levelName.find('/') + 1);
    cout << levelName << endl;

    int knownLowerBound = 0;
    
    ifstream statStream("stats");
    assert(statStream);
    string line;
    while (getline(statStream, line)) {
	if (line.find(levelName) == 0) {
	    cout << line << endl;
	    for (int i = 0; i < 8; ++i)
		line = line.substr(line.find('|') + 1);
	    cout << line << endl;
	    line = line.substr(0, line.find('|'));
	    cout << line << endl;
	    knownLowerBound = atoi(line.c_str());
	    if (knownLowerBound != 0)
		cout << "Known lower bound: " << knownLowerBound << endl;
	}
    }

    cout << "sizeof(State2) = " << sizeof(State2) << endl;

    for (int maxMoves = knownLowerBound; ; ++maxMoves) {
	cout << "******************** " << maxMoves << " ********************\n";
	for (int goalNr = 0; goalNr < level.numGoals(); ++goalNr) {
	    cout << "-------------------- "
		 << maxMoves << ": " << level.goalPos(goalNr)
		 << " --------------------\n";
	    Problem::setGoal(level, goalNr);
	    State2 start(Problem::startPositions());
	    deque<Move> moves = aStar2(start, maxMoves);
	    //deque<Move> moves = IDAStar(maxMoves);
	    if (moves.size() > 0) {
		State2 state = start;
		for (deque<Move>::const_iterator m = moves.begin();
		     m != moves.end(); ++m) {
		    state = State2(state, *m);
		    cout << Board(state);
		}
		cout << "Final board:\n";
		cout << Board(state);

		cout << "Solution in " << moves.size() << " moves.\n";
		for (deque<Move>::const_iterator m = moves.begin();
		     m != moves.end(); ++m) {
		    cout << *m << endl;
		}
		ofstream boundStream("bounds", ios::app);
		boundStream << levelName << ": = " << maxMoves << endl;

		ofstream solStream("solutions", ios::app);
		solStream << levelName << ' ' << isotime()
			  << ' ' << moves.size() << " moves: ";
		for (deque<Move>::const_iterator m = moves.begin();
		     m != moves.end(); ++m)
		    solStream << *m << " ";
		solStream << endl;

		return 0;
	    }
	}
	// ok, now we know we need at least maxMoves + 1 moves
	if (maxMoves + 1 > knownLowerBound) {
	    ofstream boundStream("bounds", ios::app);
	    boundStream << levelName << ": >= " << maxMoves + 1 << endl;
	    cout << "New lower bound found for " << levelName
		 << ": " << maxMoves + 1 << endl;
	}
    }
}
