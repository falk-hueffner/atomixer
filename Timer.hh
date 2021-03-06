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

#include <iosfwd>

class Timer {
public:
    Timer() { reset(); }

    void reset();
    void start();
    void stop();
    double seconds() const;

private:
    bool running_;
    struct timeval time_;
    struct timeval start_;
};

std::ostream& operator<<(std::ostream& out, const Timer& timer);

#endif
