/****************************************************************************
 *
 * endpoint.hh - endpoint definition
 *
 * Copyright (C) 2022, 2023 Moreno Marzolla, Giovanni Birolo, Gabriele D'Angeloo, Piero Fariselli
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

#ifndef ENDPOINT_HH
#define ENDPOINT_HH

#include <cassert>

#if __CUDACC__
#define GLOBAL __host__ __device__
#else
#define GLOBAL
#endif

/******************************************************************************
 * Endpoint stored in the endpoint arrays. Each endpoint represents a
 * single scalar value v, that can either correspond to a lower or
 * upper bound of a subscription or update region.
 ******************************************************************************/
struct endpoint {

    enum ep_extreme { LOWER, UPPER, UNDEF };
    enum ep_type { SUBSCRIPTION, UPDATE };

    int id;     // ID of the interval this endpoint belongs to
    int32_t v;  // value of this endpoint
    ep_extreme e; // whether this is a lower or upper endpoint
    ep_type t;  // whether this endpoint belongs to a subscription or update interval

    // Default constructor
    GLOBAL
    endpoint() :
        id(0), v(0), e(UNDEF), t(SUBSCRIPTION)
    { };

    // Constructor
    GLOBAL
    endpoint(int ep_id, int32_t ep_v, ep_extreme ep_e, ep_type ep_t) :
        id( ep_id ), v( ep_v ), e( ep_e ), t( ep_t )
    { };

    // Copy constructor
    GLOBAL
    endpoint(const endpoint& other) :
        id( other.id ), v( other.v ), e( other.e ), t( other.t )
    { };

    /* Endpoints are sorted according to their value. Special care
       must be taken for endpoints having exactly the same value.
       Since we are dealing with closed intervals, a lower
       endpoint must always precede an upper endpoint with the
       same value. If this does not happen, an overlap is missed.
    */

    GLOBAL
    bool operator < (const endpoint& other) const
    {
        assert( UNDEF != e );
        assert( UNDEF != other.e );
        return (v < other.v || (v == other.v && e == LOWER && other.e == UPPER));
    }
};

#endif
