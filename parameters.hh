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

#ifndef PARAMETERS_HH
#define PARAMETERS_HH

//#undef DO_BACKWARD_SEARCH	// normal forward search
#define DO_BACKWARD_SEARCH 1	// search from goal toward starting position

//#define USE_IDASTAR 1		// IDA*
#undef USE_IDASTAR		// A*

// maximum amount of memory to be used
static const unsigned int MEMORY = 800 * 1024 * 1024;

#define ALGORITHM_NAME "astar-backward"

#endif
