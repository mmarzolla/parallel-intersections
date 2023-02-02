/****************************************************************************
 *
 * omp_bf_count.cc - OpenMP brute-force counting
 *
 * Copyright (C) 2022, 2023 Moreno Marzolla
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

#include <iostream>
#include <vector>
#include <numeric>
#include "interval.hh"
#include "omp_bf_count.hh"

/* Return the total number of overlaps */
size_t omp_bf_count( const std::vector<interval> &upd,
                     const std::vector<interval> &sub,
                     std::vector<int> &counts )
{
    const int n = sub.size();
    const int m = upd.size();
    counts.resize(n);

    std::cout << "omp_bf_count: " << std::flush;

    for (int i=0; i<n; i++) {
        int c = 0;
#if __GNUC__ < 9
#pragma omp parallel for default(none) shared(i, sub, upd) reduction(+:c)
#else
#pragma omp parallel for default(none) shared(i, m, sub, upd) reduction(+:c)
#endif
        for (int j=0; j<m; j++) {
            c += intersect(sub[i], upd[j]);
        }
        counts[i] = c;
    }

    const int n_intersections = std::accumulate(counts.begin(), counts.end(), 0);
    return n_intersections;
}
