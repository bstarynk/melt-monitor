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



////////////////////////////////////////////////////////////////
std::mutex MomDoubleSq::_mtxarr_[MomDoubleSq::_width_];
std::unordered_multimap<MomHash,const MomDoubleSq*> MomDoubleSq::_maparr_[MomDoubleSq::_width_];

MomDoubleSq::MomDoubleSq(const double* darr, MomSize sz, MomHash h)
  : MomAnyVal(MomKind::TagDoubleSqK, sz, h),
    _dvalarr{}
{
  MOM_ASSERT(sz==0 || darr != nullptr,
             "MomDoubleSq::MomDoubleSq null darr for sz=" << sz);
  memcpy (const_cast<double*>(_dvalarr), darr, sz*sizeof(double));
}


MomHash
MomDoubleSq::hash_double(double d)
{
  int e = 0;
  if (MOM_UNLIKELY(isnan(d)))
    MOM_FAILURE("MomDoubleSq::hash_double got NaN");
  if (MOM_UNLIKELY(isinf(d)))
    {
      if (d>0.0) return 9270666;
      else return 3356443;
    };
  double x = frexp (d, &e);
  MomHash h = ((MomHash) (x / (M_PI * M_LN2 * DBL_EPSILON))) ^ e;
  if (!h)
    {
      h = e;
      if (!h)
        h = (x > 0.0) ? 1689767 : (x < 0.0) ? 2000281 : 13;
    }
  return h;
} // end MomDoubleSq::hash_double


MomHash
MomDoubleSq::compute_hash(const double* darr, MomSize sz)
{
  constexpr MomHash inith = 51413;
  if (sz == 0) return inith;
  if (MOM_UNLIKELY(darr==nullptr))
    MOM_FAILURE("MomDoubleSq::compute_hash null darr");
  if (MOM_UNLIKELY(sz>=_max_size))
    MOM_FAILURE("MomDoubleSq::compute_hash huge sz " <<sz);
  MomHash h1 = 5*sz + 10;
  MomHash h2 = 54413;
  for (unsigned ix=0; ix<sz; ix++)
    {
      double x = darr[ix];
      if (MOM_UNLIKELY(isnan(x)))
        MOM_FAILURE("MomDoubleSq::compute_hash NaN at index " << ix);
      MomHash hx = MomDoubleSq::hash_double(x);
      if (ix % 2 == 0)
        h1 = (h1 * 145069) ^ (hx * 17 + ix);
      else
        h2 = (h2 * 15193 - 7 * ix) ^ (hx * 1409 + 10 - (hx >> 20));
    }
  MomHash h = h1 ^ h2;
  if (MOM_UNLIKELY(h==0))
    h = (h1 & 0xfffff) + 11 * (h2 & 0xfffff) + 10;
  return h;
} // end MomDoubleSq::compute_hash


const MomDoubleSq*
MomDoubleSq::make_from_array(const double* darr, MomSize sz)
{
  MomHash h = compute_hash(darr, sz);
  MomDoubleSq* res = nullptr;
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
      const MomDoubleSq*dsq = it->second;
      MOM_ASSERT(dsq != nullptr, "null dsq in buckix=" << buckix);
      if (MOM_UNLIKELY(dsq->has_content(darr, sz)))
        return dsq;
    }
  res = new((sz-MOM_FLEXIBLE_DIM)*sizeof(double)) MomDoubleSq(darr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomDoubleSq::make_from_array

