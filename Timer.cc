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
#include <iomanip>

#include "Timer.hh"

void Timer::reset() {
    time_.tv_sec = 0;
    time_.tv_usec = 0;
    running_ = false;
}

void Timer::start() {
    if (!running_) {
	rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	start_ = usage.ru_utime;
    }
    running_ = true;
}

void Timer::stop() {
    if (running_) {
	rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	time_.tv_sec += (usage.ru_utime.tv_sec - start_.tv_sec);
	time_.tv_usec += (usage.ru_utime.tv_usec - start_.tv_usec);
	// could normalize but won't overflow that soon...
    }
    running_ = false;
}

double Timer::seconds() const {
    if (running_) {
	rusage usage;
	getrusage(RUSAGE_SELF, &usage);
	return time_.tv_sec + (usage.ru_utime.tv_sec - start_.tv_sec)
	    + (time_.tv_usec + (usage.ru_utime.tv_usec - start_.tv_usec))
	    / 1000000.0;
    } else {
	return time_.tv_sec + time_.tv_usec / 1000000.0;
    }
}

std::ostream& operator<<(std::ostream& out, const Timer& timer) {
    int seconds = int(timer.seconds());
    int days = seconds / (60*60*24);
    seconds -= days * (60*60*24);
    int hours = seconds / (60*60);
    seconds -= hours / (60*60);
    int minutes = seconds / 60;
    seconds -= minutes * 60;

    if (days > 0)
	out << days << "d ";
    if (hours > 0)
	out << std::setw(2) << std::setfill('0') << hours << ':';
    return out << std::setw(2) << std::setfill('0') << minutes << ':'
	       << std::setw(2) << std::setfill('0') << seconds; 
}
