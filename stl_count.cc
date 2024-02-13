/****************************************************************************
 *
 * stl_count.cc - count intersections using the Standard Template Library
 *
 * Copyright (C) 2024
 * Moreno Marzolla
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

struct make_left_endpoint :
    public std::unary_function< const interval &, endpoint >
{
    endpoint::ep_type t;

    make_left_endpoint(endpoint::ep_type t) : t(t) { }

    endpoint operator()(const interval &i) const
    {
        return endpoint(i.id, i.left, endpoint::LEFT, t);
    }
};

struct make_right_endpoint :
    public std::unary_function< const interval &, endpoint >
{
    endpoint::ep_type t;

    make_right_endpoint(endpoint::ep_type t) : t(t) { }

    endpoint operator()(const interval &i) const
    {
        return endpoint(i.id, i.right, endpoint::RIGHT, t);
    }
};

/**
 * Count how many intervals in `B` overlap each interval in `A`.
 * The result is stored in the array `counts`.
 */
size_t count_intersections(const std::vector<interval> &A,
                           const std::vector<interval> &B,
                           std::vector<int> &counts )
{
    const size_t n = A.size();
    const size_t m = B.size();
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
            const endpoint ep_left(sub[i].id, sub[i].lower, endpoint::LEFT, endpoint::SET_A);
            const endpoint ep_right(sub[i].id, sub[i].upper, endpoint::RIGHT, endpoint::SET_A);
            endpoints[i  ] = ep_left;
            endpoints[i+n] = ep_right;
        }
#pragma omp for
        for (size_t i=0; i<m; i++) {
            const endpoint ep_left(upd[i].id, upd[i].lower, endpoint::LEFT, endpoint::SET_B);
            const endpoint ep_right(upd[i].id, upd[i].upper, endpoint::RIGHT, endpoint::SET_B);
            endpoints[2*n + i  ] = ep_left;
            endpoints[2*n + i+m] = ep_right;
        }
    }
#else
    std::transform(std::execution::par,
                   A.begin(), A.end(),
                   endpoints.begin(),
                   make_left_endpoint(endpoint::SET_A));
    std::transform(std::execution::par,
                   A.begin(), A.end(),
                   endpoints.begin() + n,
                   make_right_endpoint(endpoint::SET_A));
    std::transform(std::execution::par,
                   B.begin(), B.end(),
                   endpoints.begin() + 2*n,
                   make_left_endpoint(endpoint::SET_B));
    std::transform(std::execution::par,
                   B.begin(), B.end(),
                   endpoints.begin() + 2*n + m,
                   make_right_endpoint(endpoint::SET_B));
#endif

    std::sort(std::execution::par, endpoints.begin(), endpoints.end());

    std::vector<int> nleft(n_endpoints);
    std::vector<int> nright(n_endpoints);

    /* C++-17 does not provide counted iterators, so there is no
       STL-compliant way to parallelize the following loop without
       using an explicit "parallel for" directive */
#pragma omp parallel for
    for (size_t i=0; i<n_endpoints; i++) {
        nleft[i] = nright[i] = 0;
        if (endpoints[i].t == endpoint::SET_B) {
            if (endpoints[i].e == endpoint::LEFT)
                nleft[i] = 1;
            else
                nright[i] = 1;
        }
    }

    std::inclusive_scan(std::execution::par, nleft.cbegin(), nleft.cend(), nleft.begin(), std::plus<int>());
    std::inclusive_scan(std::execution::par, nright.cbegin(), nright.cend(), nright.begin(), std::plus<int>());

    std::vector<int> left_idx(n);
    std::vector<int> right_idx(n);

#pragma omp parallel
    {
#pragma omp for
        for (size_t i=0; i<n_endpoints; i++) {
            if (endpoints[i].t == endpoint::SET_A) {
                if (endpoints[i].e == endpoint::LEFT)
                    left_idx[endpoints[i].id] = i;
                else
                    right_idx[endpoints[i].id] = i;
            }
        }
#pragma omp for
        for (size_t i=0; i<n; i++) {
            counts[i] = nleft[right_idx[i]] - nright[left_idx[i]];
        }
    }

    const int n_intersections = std::reduce(std::execution::par, counts.begin(), counts.end(), 0, std::plus<int>());
    return n_intersections;
}
