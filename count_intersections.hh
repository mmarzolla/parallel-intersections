/****************************************************************************
 *
 * count_intersections.hh
 *
 * Copyright (C) 2022--2024 Moreno Marzolla, Giovanni Birolo, Gabriele D'Angelo, Piero Fariselli
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

#ifndef COUNT_INTERSECTIONS_HH
#define COUNT_INTERSECTIONS_HH

#include <vector>
#include "interval.hh"

/**
 * Count how many intervals in `upd` overlap each interval in `sub`.
 * The result is stored in the array `counts`.
 */
size_t count_intersections( const std::vector<interval> &upd,
                            const std::vector<interval> &sub,
                            std::vector<int> &counts );

#endif /* COUNT_INTERSECTIONS_HH */
