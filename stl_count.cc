/****************************************************************************
 *
 * stl_count.cc - count intersections using the Standard Template Library
 *
 * Copyright (C) 2024 Moreno Marzolla
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
#include <string>
#include <vector>
#include <cassert>
#include <functional>
#include <iterator>
#include <numeric>
#include <algorithm>
#include <execution>
#include "interval.hh"
#include "endpoint.hh"
#include "utils.hh"
#include "count_intersections.hh"

struct make_lower_endpoint :
    public std::unary_function< const interval &, endpoint >
{
    endpoint::ep_type t;

    make_lower_endpoint(endpoint::ep_type t) : t(t) { }

    endpoint operator()(const interval &i) const
    {
        return endpoint(i.id, i.lower, endpoint::LOWER, t);
    }
};

struct make_upper_endpoint :
    public std::unary_function< const interval &, endpoint >
{
    endpoint::ep_type t;

    make_upper_endpoint(endpoint::ep_type t) : t(t) { }

    endpoint operator()(const interval &i) const
    {
        return endpoint(i.id, i.upper, endpoint::UPPER, t);
    }
};

/**
 * Count how many intervals in `upd` overlap each interval in `sub`.
 * The result is stored in the array `counts`.
 *
 * This function should work correctly regardless whether intervals in
 * `upd` (or `sub`) self-intersect.
 */
size_t count_intersections(const std::vector<interval> &sub, // A
                           const std::vector<interval> &upd, // B
                           std::vector<int> &counts )
{
    const size_t n = sub.size();
    const size_t m = upd.size();
    const size_t n_endpoints = 2*(n+m);
    counts.resize(n);
    std::cout << "stl_count... " << std::flush;

    // Array of all endpoints
    std::vector<endpoint> endpoints(n_endpoints);
#if 0
#pragma omp parallel
    {
#pragma omp for
        for (size_t i=0; i<n; i++) {
            const endpoint ep_left(sub[i].id, sub[i].lower, endpoint::LOWER, endpoint::SUBSCRIPTION);
            const endpoint ep_right(sub[i].id, sub[i].upper, endpoint::UPPER, endpoint::SUBSCRIPTION);
            endpoints[i  ] = ep_left;
            endpoints[i+n] = ep_right;
        }
#pragma omp for
        for (size_t i=0; i<m; i++) {
            const endpoint ep_left(upd[i].id, upd[i].lower, endpoint::LOWER, endpoint::UPDATE);
            const endpoint ep_right(upd[i].id, upd[i].upper, endpoint::UPPER, endpoint::UPDATE);
            endpoints[2*n + i  ] = ep_left;
            endpoints[2*n + i+m] = ep_right;
        }
    }
#else
    std::transform(std::execution::par,
                   sub.begin(), sub.end(),
                   endpoints.begin(),
                   make_lower_endpoint(endpoint::SUBSCRIPTION));
    std::transform(std::execution::par,
                   sub.begin(), sub.end(),
                   endpoints.begin() + n,
                   make_upper_endpoint(endpoint::SUBSCRIPTION));
    std::transform(std::execution::par,
                   upd.begin(), upd.end(),
                   endpoints.begin() + 2*n,
                   make_lower_endpoint(endpoint::UPDATE));
    std::transform(std::execution::par,
                   upd.begin(), upd.end(),
                   endpoints.begin() + 2*n + m,
                   make_upper_endpoint(endpoint::UPDATE));
#endif

    std::sort(std::execution::par, endpoints.begin(), endpoints.end());

    std::vector<int> left(n_endpoints);
    std::vector<int> right(n_endpoints);

    /* C++-17 does not provide counted iterators, so there is no
       STL-compliant way to parallelize the following loop without
       using an explicit "parallel for" directive */
#pragma omp parallel for
    for (size_t i=0; i<n_endpoints; i++) {
        left[i] = right[i] = 0;
        if (endpoints[i].t == endpoint::UPDATE) {
            if (endpoints[i].e == endpoint::LOWER)
                left[i] = 1;
            else
                right[i] = 1;
        }
    }

    std::inclusive_scan(std::execution::par, left.cbegin(), left.cend(), left.begin(), std::plus<int>());
    std::inclusive_scan(std::execution::par, right.cbegin(), right.cend(), right.begin(), std::plus<int>());

    std::vector<int> left_idx(n);
    std::vector<int> right_idx(n);

#pragma omp parallel
    {
#pragma omp for
        for (size_t i=0; i<n_endpoints; i++) {
            if (endpoints[i].t == endpoint::SUBSCRIPTION) {
                if (endpoints[i].e == endpoint::LOWER)
                    left_idx[endpoints[i].id] = i;
                else
                    right_idx[endpoints[i].id] = i;
            }
        }
#pragma omp for
        for (size_t i=0; i<n; i++) {
            counts[i] = left[right_idx[i]] - right[left_idx[i]];
        }
    }

    const int n_intersections = std::reduce(std::execution::par, counts.begin(), counts.end(), 0, std::plus<int>());
    return n_intersections;
}
