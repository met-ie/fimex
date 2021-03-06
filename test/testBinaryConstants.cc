/*
 * Fimex, testBinaryConstants.cc
 *
 * (C) Copyright 2009, met.no
 *
 * Project Info:  https://wiki.met.no/fimex/start
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 *  Created on: Sep 10, 2009
 *      Author: Heiko Klein
 */

#include "testinghelpers.h"
#include "fimex/binaryConstants.h"

TEST4FIMEX_TEST_CASE(test_binaryConstants)
{
    TEST4FIMEX_CHECK_EQ(0, binary<0>::value);
    TEST4FIMEX_CHECK_EQ(1, binary<01>::value);
    TEST4FIMEX_CHECK_EQ(64, binary<01000000>::value);
    TEST4FIMEX_CHECK(64 != binary<1000000>::value);                          // remember to take octal values
    TEST4FIMEX_CHECK_EQ(2097152, binary<01000000000000000000000ULL>::value); // large value
}
