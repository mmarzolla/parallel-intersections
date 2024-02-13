/****************************************************************************
 *
 * utils.cc - Utilities
 *
 * Copyright (C) 2022, 2023, 2024 Moreno Marzolla, Giovanni Birolo, Gabriele D'Angelo, Piero Fariselli
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
#include <cstdlib>
#include <ctime>
#include "utils.hh"

double now( void )
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (double)ts.tv_nsec / 1.0e9;
}

int randab(int a, int b)
{
    return (a + rand() % (b-a+1));
}
