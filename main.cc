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
#include <signal.h>
#include <stdlib.h>

#include <iostream>
#include <fstream>

#include "Level.hh"
#include "Problem.hh"
#include "State.hh"
#include "Statistics.hh"
#include "parameters.hh"

#define USE_IDASTAR 1		// IDA*
//#undef USE_IDASTAR		// A*

#ifdef USE_IDASTAR
# include "IDAStar.hh"
#else
# include "AStar2.hh"
#endif

using namespace std;

static string levelName;

string isotime() {
    time_t timet = time(NULL);
    char timestr[256];
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M",
	     localtime(&timet));
    return string(timestr);
}

extern "C" {
    void writestats(void) {
	ofstream statsStream(ALGORITHM_NAME, ios::app);
	statsStream << levelName
		    << " with " << ALGORITHM_NAME << " final statistics:\n";
	Statistics::print(statsStream);
	if (Statistics::solutionLength != 0) {
	    statsStream << " Solution length:  " << Statistics::solutionLength << endl;
	} else {
	    if (Statistics::lowerBound != 0)
		statsStream << " Lower bound:      " << Statistics::lowerBound << endl;
	    if (Statistics::upperBound != 0)
		statsStream << " Upper bound:      " << Statistics::upperBound << endl;
	}
    }
    void signalhandler(int) {
	exit(1);
    }
}

void usage() {
    cout << "Usage: atomixer levelfile           solve level" << endl
	 << "       atomixer --stats levelfile   print statistics" << endl;
}

int main(int argc, char* argv[]) {
    try {
    if (argc < 2 || argc > 3) {
	usage();
	return 1;
    }

    if (argc == 3) {
	if (string(argv[1]) == "--stats") {
	    ifstream levelStream(argv[2]);
	    assert(levelStream);
	    Level level(levelStream);
	    level.printStats();
	} else if (string(argv[1]) == "--show") {
	    ifstream levelStream(argv[2]);
	    assert(levelStream);
	    Level level(levelStream);
	    cout << level.startBoard() << "Goal:\n" << level.goal();
	} else {
	    usage();
	    return 1;
	}

	return 0;
    }

    atexit(writestats);
    signal(SIGTERM, signalhandler);

    ifstream levelStream(argv[1]);
    assert(levelStream);
    Level level(levelStream);
    cout << level.startBoard();
    Problem::setLevel(level);

    levelName = string(argv[1]);
    while (levelName.find('/') != string::npos)
	levelName = levelName.substr(levelName.find('/') + 1);
    cout << "Solving " << levelName << "...\n";

    int knownLowerBound = 0;

    for (int maxMoves = knownLowerBound; ; ++maxMoves) {
	cout << "******************** " << maxMoves << " ********************\n";
	for (int goalNr = 0; goalNr < level.numGoals(); ++goalNr) {
	    cout << "-------------------- "
		 << maxMoves << ": " << level.goalPos(goalNr)
		 << " --------------------\n";
	    Problem::setGoal(level, goalNr);
	    State start(Problem::startPositions());
#ifdef USE_IDASTAR
	    deque<Move> moves = IDAStar(maxMoves);
#else
	    deque<Move> moves = aStar2(start, maxMoves);
#endif
	    if (moves.size() > 0) {
		State state = start;
#ifndef DO_BACKWARD_SEARCH
		for (deque<Move>::const_iterator m = moves.begin();
		     m != moves.end(); ++m) {
		    state = State(state, *m);
		    //cout << Board(state);
		}
#else
		cout << "Found solution.\n"
		     << Board(state);
		// Bug in g++: const_reverse_iterator doesn't work
		for (deque<Move>::reverse_iterator m = moves.rbegin();
		     m != moves.rend(); ++m) {
		    cout << *m << endl;
		    state.undo(*m);
		    cout << Board(state);
		}
#endif
		cout << "Final board:\n"
		     << Board(state)
		     << "Solution in " << moves.size() << " moves.\n";
		for (deque<Move>::const_iterator m = moves.begin();
		     m != moves.end(); ++m) {
		    cout << *m << endl;
		}

		Statistics::solutionLength = moves.size();
		
		cout << levelName << " with " << ALGORITHM_NAME
		     << " final statistics:\n";
		Statistics::print(cout);


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
	Statistics::lowerBound = maxMoves + 1;
	if (maxMoves + 1 > knownLowerBound) {
	    ofstream boundStream("bounds", ios::app);
	    boundStream << levelName << ": >= " << maxMoves + 1 << endl;
	    cout << "New lower bound found for " << levelName
		 << ": " << maxMoves + 1 << endl;
	}
    }
    } catch (const std::exception& e) {
	cout << "Caught exception: " << e.what() << endl;
    }
}
