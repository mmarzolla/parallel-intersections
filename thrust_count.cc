/****************************************************************************
 *
 * thrust_count.cc - count intersections using the Thrust library
 *
 * Copyright (C) 2022, 2023, 2024
 * Moreno Marzolla, Giovanni Birolo, Gabriele D'Angelo, Piero Fariselli
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
#include <thrust/sort.h>
#include <thrust/reduce.h>
#include <thrust/transform_scan.h>
#include <thrust/transform_reduce.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include "interval.hh"
#include "endpoint.hh"
#include "utils.hh"

namespace th = thrust;

/**
 * This unary function takes an interval as input, and produces a pair
 * of (left, right) endpoints
 */
typedef typename th::tuple<endpoint, endpoint> pair_of_endpoints;

struct make_endpoint : public th::unary_function< const interval &, pair_of_endpoints >
{
    endpoint::ep_type ep_type;

    GLOBAL
    make_endpoint(endpoint::ep_type ep) : ep_type(ep) { }

    GLOBAL
    pair_of_endpoints operator()(const interval &i) const
    {
        return th::make_tuple( endpoint(i.id, i.left, endpoint::LEFT, ep_type),
                               endpoint(i.id, i.right, endpoint::RIGHT, ep_type) );
    }
};

/* This is a function that maps an endpoint to 1 iff the endpoint is
   of a given type (SET_A or SET_B) and of a given extreme (left,
   right). */
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

    Iter left_begin, right_begin, nleft_begin, nright_begin;

    GLOBAL
    compute_counts( Iter li, Iter ri, Iter no, Iter nc ) :
        left_begin(li),
        right_begin(ri),
        nleft_begin(no),
        nright_begin(nc)
    { };

    /**
     * Returns nleft[right[i.id]] - nright[left[i.id]]
     */
    GLOBAL
    int operator()(const interval &i) const
    {
        const int idx = i.id;
        const int ll = *(left_begin + idx);
        const int rr = *(right_begin + idx);
        return *(nleft_begin + rr) - *(nright_begin + ll);
    }
};

template<typename Iter_ep, typename Iter_idx>
struct init_idx
{
    Iter_idx left_begin, right_begin;
    Iter_ep ep_begin;

    /**
     * - left_begin is the iterator that points to the left[] array.
     *
     * - right_begin is the iterator that points to the right[] array.
     *
     * - ep_begin is the iterator that points to the beginning of the
     *   sorted array of endpoints.
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
        if (ep.t == endpoint::SET_A) {
            const int idx = ep.id;
            if (ep.e == endpoint::LEFT)
                *(left_begin + idx) = i;
            else
                *(right_begin + idx) = i;
        }
    }
};

/**
 * Count how many intervals in `B` overlap each interval in `A`.  The
 * result is stored in the array `counts`.
 */
size_t count_intersections(const std::vector<interval> &A,
                           const std::vector<interval> &B,
                           std::vector<int> &counts )
{
    const size_t n = A.size();
    const size_t m = B.size();
    const size_t n_endpoints = 2*(n+m);
    counts.resize(n);
#if THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_OMP
    std::cout << "Thrust/OpenMP... " << std::flush;
#elif THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_CPP
    std::cout << "Thrust/serial... " << std::flush;
#elif THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_CUDA
    std::cout << "Thrust/CUDA... " << std::flush;
#else
    #error Unknown value for THRUST_DEVICE_SYSTEM
#endif

    // Array of all endpoints: there are exactly 2*(n+m) pf them
    th::device_vector<endpoint> d_endpoints(n_endpoints);

    th::device_vector<interval> d_A(A);
    th::device_vector<interval> d_B(B);

    // Initialize the array of endpoints
    th::transform(d_A.begin(), d_A.end(),
                  th::make_zip_iterator(d_endpoints.begin(), d_endpoints.begin() + n),
                  make_endpoint(endpoint::SET_A));
    th::transform(d_B.begin(), d_B.end(),
                  th::make_zip_iterator(d_endpoints.begin() + 2*n, d_endpoints.begin() + 2*n + m),
                  make_endpoint(endpoint::SET_B));

    th::sort(d_endpoints.begin(), d_endpoints.end());

    /* left_idx[i] is the position (index) in the sorted endpoint
       array of the left endpoint of the interval in A with id==i;

       right_idx[i] is the position (index) in the sorted endpoint
       array of the right endpoint of the interval in A with id==i; */
    th::device_vector<int> left_idx(n), right_idx(n);

    th::for_each( th::make_counting_iterator<int>(0),
                  th::make_counting_iterator<int>(n_endpoints),
                  init_idx<th::device_vector<endpoint>::const_iterator, th::device_vector<int>::iterator>(d_endpoints.begin(), left_idx.begin(), right_idx.begin()) );

    /* nleft[i] is the number of left endpoints in B up to and
       including position i in the array of sorted endpoints */
    th::device_vector<int> nleft(n_endpoints);
    /* nright[i] is the number of right endpoints in B up to and
       including position i in the array of sorted endpoints */
    th::device_vector<int> nright(n_endpoints);

    th::transform_inclusive_scan( d_endpoints.begin(), d_endpoints.end(),
                                  nleft.begin(),
                                  init_count(endpoint::SET_B, endpoint::LEFT),
                                  th::plus<int>() );
    th::transform_inclusive_scan( d_endpoints.begin(), d_endpoints.end(),
                                  nright.begin(),
                                  init_count(endpoint::SET_B, endpoint::RIGHT),
                                  th::plus<int>() );

    th::device_vector<int> d_counts(n);

    th::transform( d_A.begin(), d_A.end(),
                   d_counts.begin(),
                   compute_counts<th::device_vector<int>::const_iterator>(left_idx.begin(), right_idx.begin(), nleft.begin(), nright.begin() ) );

    th::copy(d_counts.begin(), d_counts.end(), counts.begin());

    const int n_intersections = th::reduce(d_counts.begin(), d_counts.end(), 0);
    return n_intersections;
}
