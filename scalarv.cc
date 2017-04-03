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


std::mutex MomIntSq::_mtxarr_[MomIntSq::_width_];
std::unordered_multimap<MomHash,const MomIntSq*> MomIntSq::_maparr_[MomIntSq::_width_];

MomIntSq::MomIntSq(const intptr_t* iarr, MomSize sz, MomHash h)
  : MomAnyVal(MomKind::TagIntSqK, sz, h),
    _ivalarr{}
{
  MOM_ASSERT(sz==0 || iarr != nullptr,
             "MomIntSq::MomIntSq null iarr for sz=" << sz);
  memcpy (const_cast<intptr_t*>(_ivalarr), iarr, sz*sizeof(intptr_t));
}



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



const MomIntSq*
MomIntSq::make_from_array(const intptr_t* iarr, MomSize sz)
{
  MomHash h = compute_hash(iarr, sz);
  MomIntSq* res = nullptr;
  unsigned ix = h % _width_;
  std::lock_guard<std::mutex> _gu(_mtxarr_[ix]);
  constexpr unsigned minbuckcount = 16;
  auto& curmap = _maparr_[ix];
  if (MOM_UNLIKELY(curmap.bucket_count() < minbuckcount))
    curmap.rehash(minbuckcount);
  size_t buckix = curmap.bucket(h);
  auto buckbeg = curmap.begin(buckix);
  auto buckend = curmap.end(buckix);
  for (auto it = buckbeg; it != buckend; it++)
    {
      if (it->first != h)
        continue;
      const MomIntSq*isq = it->second;
      MOM_ASSERT(isq != nullptr, "null isq in buckix=" << buckix);
      if (MOM_UNLIKELY(isq->has_content(iarr, sz)))
        return isq;
    }
  res = new((sz-MOM_FLEXIBLE_DIM)*sizeof(intptr_t)) MomIntSq(iarr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomIntSq::make_from_array

