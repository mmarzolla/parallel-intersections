/****************************************************************************
 *
 * utils.hh - Misc utilities
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

#ifndef UTILS_HH
#define UTILS_HH

/**
 * Returns the number of nanoseconds since the epoch
 */
double now( void );

/**
 * Returns a random integer in [a, b]
 */
int randab(int a, int b);

#endif
