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

using namespace std;

int main(int argc, char* argv[]) {
    assert(argc >= 2);

    cout << "Level           |#A|#G|1×|2×|3×|4×|5×|\n"
	 << "----------------+--+--+--+--+--+--+--+\n";
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
