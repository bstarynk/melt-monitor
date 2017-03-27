// file scalarv.cc - scalar values

/**   Copyright (C)  2017  Basile Starynkevitch and later the FSF
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.

    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "meltmoni.hh"

MomHash
MomIntSq::compute_hash(const intptr_t* iarr, MomSize sz)
{
  constexpr MomHash inith = 60037;
  if (sz == 0) return inith;
  if (MOM_UNLIKELY(iarr==nullptr))
    MOM_FAILURE("MomIntSq::compute_hash null iarr");
  if (MOM_UNLIKELY(sz>=_max_size))
    MOM_FAILURE("MomIntSq::compute_hash huge sz " <<sz);
  MomHash h1 = sz;
  MomHash h2 = 60091;
  for (unsigned ix=0; ix<sz; ix++)
    {
      if (ix % 2 == 0)
        h1 = (12149 * h1 + 5 * ix) ^ MomHash(21157 * iarr[ix]);
      else
        h2 = (31153 * h2 - 13 * ix) ^ MomHash(45121 * iarr[ix]);
    }
  MomHash h = (17*h1) ^ (457*h2);
  if (MOM_UNLIKELY(h==0))
    h = (h1 & 0xffffff) + (h2 & 0xffffff) + 5*(sz & 0xfff) + 10;
  return h;
} // end MomIntSq::compute_hash
