/****************************************************************************
 *
 * interval.hh
 *
 * Count intersections
 *
 * Copyright (C) 2013, 2016, 2021, 2022 Moreno Marzolla, Gabriele D'Angelo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/

#ifndef INTERVAL_HH
#define INTERVAL_HH

#include <cstdint>

/**
 * This struct represents the closed intnerval [lower, upper]
 */
struct interval {
    int id;             /* unique identifier (0, ... n_intervals - 1) */
    int32_t lower;	/* lower bound */
    int32_t upper;	/* upper bound */
    void* payload;      /* user-defined payload associated with this interval */
};

/**
 * Return true if intervals |x| and |y| intersect, 0 otherwise.
 */
bool intersect( const interval &x, const interval &y );

#endif
