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

#ifndef TIMER_HH
#define TIMER_HH

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

class Timer {
public:
    Timer() { reset(); }

    void reset() {
	rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	_start = usage.ru_utime;
    }

    double seconds() const {
	rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	return usage.ru_utime.tv_sec - _start.tv_sec
	    + (usage.ru_utime.tv_usec - _start.tv_usec) / 1000000.0;
    }

private:
    struct timeval _start;
};

#endif