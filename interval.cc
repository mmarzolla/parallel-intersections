/****************************************************************************
 *
 * interval.cc - operations on intervals
 *
 * Copyright (C) 2022, 2023 Moreno Marzolla, Giovanni Birolo, Gabriele D'Angelo, Piero Fariselli
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

#include "interval.hh"

/**
 * `x` and `y` are considered closed intervals; therefore, they
 * intersect if they overlap also with the corners.
 */
bool intersect( const interval &x, const interval &y )
{
    //return ( x.lower < y.upper && y.lower < x.upper );
    return ( x.lower <= y.upper && y.lower <= x.upper );
}
