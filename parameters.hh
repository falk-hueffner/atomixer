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

// maximum amount of memory to be used
static const unsigned long MEMORY = 7UL * 1024UL * 1024UL * 1024UL;

// define if you're sure your OS returns fresh pages zeroed (like Linux, but
// unlike Solaris)
#undef MY_OS_ZEROES_LARGE_MEMORY_ALLOCATIONS
//#define MY_OS_ZEROES_LARGE_MEMORY_ALLOCATIONS

#endif
