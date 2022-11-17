/****************************************************************************
 *
 * thrust_count.cu - count intersections using the Thrust library
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
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#if (USE_STL_SORT && !__CUDACC__)
#include <parallel/algorithm>
#else
#include <thrust/sort.h>
#endif
#include <thrust/reduce.h>
#include <thrust/transform_scan.h>
#include <thrust/transform_reduce.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "interval.hh"
#include "endpoint.hh"

namespace th = thrust;

/**
 * This unary function takes an interval as input, and produces a
 * pair of endpoints <lower, upper>
 */
typedef typename th::tuple<endpoint, endpoint> Pair_of_endpoints;

struct make_endpoint : public th::unary_function< const interval &, Pair_of_endpoints > /* lower, upper */
{
    endpoint::ep_type ep_type;

    GLOBAL
    make_endpoint(endpoint::ep_type ep) : ep_type(ep) { }

    GLOBAL
    Pair_of_endpoints operator()(const interval &i) const
    {
        return th::make_tuple( endpoint(i.id, i.lower, endpoint::LOWER, ep_type),
                               endpoint(i.id, i.upper, endpoint::UPPER, ep_type) );
    }
};

/* This is a function that maps an endpoint to 1 iff the endpoint is
   of a given type (subscription or update) and of a given extreme
   (lower, upper). */
struct init_count : public th::unary_function<const endpoint &, int>
{
    endpoint::ep_type my_ep_type;
    endpoint::ep_extreme my_ep_extreme;

    GLOBAL
    init_count(endpoint::ep_type t, endpoint::ep_extreme e) :
        my_ep_type(t),
        my_ep_extreme(e)
    { }

    GLOBAL
    int operator()(const endpoint &ep) const
    {
        return (ep.t == my_ep_type && ep.e == my_ep_extreme );
    }
};

template<typename Iter>
struct compute_counts : public th::unary_function<const endpoint &, int > {

    Iter left_begin, right_begin, n_upd_open_begin, n_upd_closed_begin;

    /**
     * The constructor takes the following arguments:
     *
     * - An iterator pointing to the beginning of the integer array
     *   left[i] storing the index of the left endpoint of interval
     *   with id == i;
     *
     * - An iterator pointing to the beginning of the integer array
     *   right[i] storing the index of the right endpoint of interval
     *   with id == i;
     *
     * - An iterator pointing to the beginning of the integer array
     *   n_upd_open[i] that will accumulate the number of left
     *   subscription endpoints up to position i in the sorted
     *   endpoint array;
     *
     * - An iterator pointing to the beginning of the integer array
     *   n_upd_closed[i] that will accumulate the number of right
     *   subscription endpoints up to position i in the sorted
     *   endpoint array;
     */
    GLOBAL
    compute_counts( Iter li, Iter ri, Iter no, Iter nc ) :
        left_begin(li),
        right_begin(ri),
        n_upd_open_begin(no),
        n_upd_closed_begin(nc)
    { };

    GLOBAL
    int operator()(const interval &i) const
    {
        const int idx = i.id;
        const int ll = *(left_begin + idx);
        const int rr = *(right_begin + idx);
        return *(n_upd_open_begin + rr) - *(n_upd_closed_begin + ll);
    }
};

template<typename Iter_ep, typename Iter_idx>
struct init_idx
{
    Iter_idx left_begin, right_begin;
    Iter_ep ep_begin;

    /**
     * - left_begin is the iterator that points to the left[] array,
     *   where left[i] is the position (index) in the sorted array of
     *   the left endpoint of the subscription interval with id==i.
     *
     * - right_begin is the iterator that points to the left[] array,
     *   where right[i] is the position (index) in the sorted array of
     *   the left endpoint of the subscription interval with id==i.
     *
     * - ep_begin is the iterator that points to the beginning of the
     *   sorted endpoint array.
     */
    GLOBAL
    init_idx( Iter_ep e, Iter_idx l, Iter_idx r ):
        left_begin(l),
        right_begin(r),
        ep_begin(e)
    { };

    GLOBAL
    void operator()(int i) const
    {
        const endpoint& ep = *(ep_begin + i);
        if (ep.t == endpoint::SUBSCRIPTION) {
            const int idx = ep.id;
            if (ep.e == endpoint::LOWER)
                *(left_begin + idx) = i;
            else
                *(right_begin + idx) = i;
        }
    }
};


/**
 * Count how many intervals in `upd` overlap each interval in `sub`.  The
 * result is stored in the array `counts`, but is not currently
 * returned to the caller.
 *
 * This function should work correctly regardless whether intervals in
 * `upd` (or `sub`) self-intersect.
 */
size_t thrust_count(const std::vector<interval> &upd,
                    const std::vector<interval> &sub)
{
    const size_t n = sub.size();
    const size_t m = upd.size();
    const size_t n_endpoints = 2*(n+m);
#if THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_OMP
    std::cout << "thrust_count (OpenMP)... " << std::flush;
#elif THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_CPP
    std::cout << "thrust_count (serial)... " << std::flush;
#elif THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_CUDA
    std::cout << "thrust_count (CUDA)... " << std::flush;
#else
    #error Unknown value for THRUST_DEVICE_SYSTEM
#endif

    // Array of all endpoints: there are exactly 2*(n+m) pf them
    th::device_vector<endpoint> d_endpoints(n_endpoints);

    th::device_vector<interval> d_sub(sub);
    th::device_vector<interval> d_upd(upd);

    // Initialize the array of endpoints
    th::transform(d_sub.begin(), d_sub.end(),
                  th::make_zip_iterator(d_endpoints.begin(), d_endpoints.begin() + n),
                  make_endpoint(endpoint::SUBSCRIPTION));
    th::transform(d_upd.begin(), d_upd.end(),
                  th::make_zip_iterator(d_endpoints.begin() + 2*n, d_endpoints.begin() + 2*n + m),
                  make_endpoint(endpoint::UPDATE));

#if (USE_STL_SORT && !__CUDACC__)
#error std::sort is not compatible with th:device_vector
    // __gnu_parallel::sort(d_endpoints.begin(), d_endpoints.end());
#else
    th::sort(d_endpoints.begin(), d_endpoints.end());
#endif

    /* left_idx[i] is the position (index) in the sorted endpoint
       array of the left endpoint of subscription interval with id==i;

       right_idx[i] is the position (index) in the sorted endpoint
       array of the right endpoint of subscription interval with
       id==i; */
    th::device_vector<int> left_idx(n), right_idx(n);

    th::for_each( th::make_counting_iterator<int>(0),
                  th::make_counting_iterator<int>(n_endpoints),
                  init_idx<th::device_vector<endpoint>::const_iterator, th::device_vector<int>::iterator>(d_endpoints.begin(), left_idx.begin(), right_idx.begin()) );

    /* n_upd_open[i] is the number of open update endpoints up to and
       including position i in the array */
    th::device_vector<int> open_count(n_endpoints);
    /* n_upd_closed[i] is the number of closed update endpoints up to
       and including position i in the array */
    th::device_vector<int> closed_count(n_endpoints);

    th::transform_inclusive_scan( d_endpoints.begin(), d_endpoints.end(),
                                  open_count.begin(),
                                  init_count(endpoint::UPDATE, endpoint::LOWER),
                                  th::plus<int>() );
    th::transform_inclusive_scan( d_endpoints.begin(), d_endpoints.end(),
                                  closed_count.begin(),
                                  init_count(endpoint::UPDATE, endpoint::UPPER),
                                  th::plus<int>() );

    th::device_vector<int> d_counts(n);

    th::transform( d_sub.begin(), d_sub.end(),
                   d_counts.begin(),
                   compute_counts<th::device_vector<int>::const_iterator>(left_idx.begin(), right_idx.begin(), open_count.begin(), closed_count.begin() ) );

    // The result is stored here. `counts[i]` is the number of
    // intervals in `upd` that overlap with `upd[i]`
    th::host_vector<int> counts(d_counts);

#if 0
    // Compute total number of intersections. This is not useful if
    // we are only interested in per-interval intersection counts, so
    // it is commented out.
    const int n_intersections = th::reduce(d_counts.begin(), d_counts.end(), 0);
    std::cout << n_intersections << " intersections" << std::endl;
    return n_intersections;
#else
    return 0;
#endif
}
