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

#include "Statistics.hh"

uint64_t Statistics::statesGenerated;
uint64_t Statistics::statesExpanded;
uint64_t Statistics::numChildren;
uint64_t Statistics::numPruned;

int Statistics::lowerBound;
int Statistics::upperBound;
int Statistics::solutionLength;

Timer    Statistics::timer;

void Statistics::print(std::ostream& out) {
    out << " Time:             " << timer
	<< "\n States generated: " << statesGenerated
	<< "\n States/second     " << uint64_t(double(statesGenerated) /
					       (timer.seconds() + 1e-6))
	<< "\n States expanded:  " << statesExpanded
	<< "\n Branching factor: " << double(numChildren)
					/ (double(statesExpanded) + 1e-6)
	<< "\n  effective:       " << double(numChildren - numPruned)
					/ (double(statesExpanded) +  1e-6)
	<< std::endl;
}
