/*
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pawel@foremski.pl>
 *
 * libpjf is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libpjf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MATH_H_
#define _MATH_H_

#define ABS(x) (((x) > 0.0) ? (x) : -(x))

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(a, min, max) (MIN(MAX((a), (min)), (max)))

/* moving averages */
#define MA(avg, new, i) (avg += ((new) - (avg)) / (i))

#define EWMA_ALPHA(N) (2.0 / (MAX(1.0, (N)) + 1.0))
#define EWMA(ewma, val, N) (EWMA_ALPHA(N) * (val) + (1.0-EWMA_ALPHA(N)) * (ewma))

#endif /* _MATH_H_ */
