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

#ifndef SIZE_HH
#define SIZE_HH

// these are hardcoded for speed and simplicity
const int XSIZE = 16, YSIZE = 15, NUM_FIELDS = XSIZE * YSIZE;

const int NUM_UNIQUE = 3;
const int NUM_PAIRED = 6;	// note *paired* not *pairs*
const int NUM_ATOMS = NUM_UNIQUE + NUM_PAIRED;

const int PAIRED_START = NUM_UNIQUE;

#endif
