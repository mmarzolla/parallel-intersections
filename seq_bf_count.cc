/****************************************************************************
 *
 * seq_bf_count.cc - sequential brute-force counting
 *
 * Copyright (C) 2022 Moreno Marzolla
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

/* Return true iff there are two overlapping intervals in v */
size_t seq_bf_count( const std::vector<interval> &upd,
                     const std::vector<interval> &sub )
{
    const int n = sub.size();
    const int m = upd.size();
    std::vector<int> counts(n); // FIXME: array of size_t

    std::cout << "seq_bf_count: " << std::flush;

    for (int i=0; i<n; i++) {
        for (int j=0; j<m; j++) {
            counts[i] += intersect(sub[i], upd[j]);
        }
    }

    const int n_intersections = std::accumulate(counts.begin(), counts.end(), 0);
    std::cout << n_intersections << " intersections" << std::endl;
    return n_intersections;
}
