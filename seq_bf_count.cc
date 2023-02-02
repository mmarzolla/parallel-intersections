/****************************************************************************
 *
 * seq_bf_count.cc - brute-force intersection count
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
#include <vector>
#include <iostream>
#include <numeric>
#include "interval.hh"
#include "seq_bf_count.hh"

/* Return the total number of overlaps */
size_t seq_bf_count( const std::vector<interval> &upd,
                     const std::vector<interval> &sub,
                     std::vector<int> &counts )
{
    const int n = sub.size();
    const int m = upd.size();
    counts.resize(n);

    std::cout << "seq_bf_count: " << std::flush;

    for (int i=0; i<n; i++) {
        for (int j=0; j<m; j++) {
            counts[i] += intersect(sub[i], upd[j]);
        }
    }

    const int n_intersections = std::accumulate(counts.begin(), counts.end(), 0)
    return n_intersections;
}
