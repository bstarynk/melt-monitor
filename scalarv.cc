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


std::atomic<std::uint64_t> MomAnyVal::_wordalloca;

std::mutex MomIntSq::_mtxarr_[MomIntSq::_swidth_];
std::unordered_multimap<MomHash,const MomIntSq*> MomIntSq::_maparr_[MomIntSq::_swidth_];

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
  if (MOM_UNLIKELY(mom_hash(h)==0))
    h = (h1 & 0xffffff) + (h2 & 0xffffff) + 5*(sz & 0xfff) + 10;
  return mom_hash(h);
} // end MomIntSq::compute_hash



const MomIntSq*
MomIntSq::make_from_array(const intptr_t* iarr, MomSize sz)
{
  MomHash h = compute_hash(iarr, sz);
  MomIntSq* res = nullptr;
  unsigned ix = slotindex(h);
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
  res = new(mom_newtg, mom_align((sz-MOM_FLEXIBLE_DIM)*sizeof(intptr_t))) MomIntSq(iarr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomIntSq::make_from_array

std::mutex*
MomIntSq::valmtx() const
{
  return _mtxarr_+slotindex(hash());
} // end MomIntSq::valmtx

////////////////////////////////////////////////////////////////
std::mutex MomDoubleSq::_mtxarr_[MomDoubleSq::_swidth_];
std::unordered_multimap<MomHash,const MomDoubleSq*> MomDoubleSq::_maparr_[MomDoubleSq::_swidth_];

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
  if (mom_hash(h)==0)
    {
      h = e;
      if (mom_hash(h)==0)
        h = (x > 0.0) ? 1689767 : (x < 0.0) ? 2000281 : 13;
    }
  return mom_hash(h);
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
  if (MOM_UNLIKELY(mom_hash(h)==0))
    h = (h1 & 0xfffff) + 11 * (h2 & 0xfffff) + 10;
  return mom_hash(h);
} // end MomDoubleSq::compute_hash


const MomDoubleSq*
MomDoubleSq::make_from_array(const double* darr, MomSize sz)
{
  MomHash h = compute_hash(darr, sz);
  MomDoubleSq* res = nullptr;
  unsigned ix = slotindex(h);
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
  res = new(mom_newtg, (sz-MOM_FLEXIBLE_DIM)*sizeof(double)) MomDoubleSq(darr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomDoubleSq::make_from_array

std::mutex*
MomDoubleSq::valmtx() const
{
  return _mtxarr_+slotindex(hash());
} // end MomDoubleSq::valmtx


////////////////////////////////////////////////////////////////
std::mutex MomString::_mtxarr_[MomString::_swidth_];
std::unordered_multimap<MomHash,const MomString*> MomString::_maparr_[MomString::_swidth_];

MomHash
MomString::compute_hash_dim(const char*cstr, MomSize*psiz, uint32_t*pbylen)
{
  if (!cstr)
    {
      if (psiz)
        *psiz=0;
      if (pbylen)
        *pbylen=0;
      return 0;
    };
  const gchar* pend = nullptr;
  if (MOM_UNLIKELY(!g_utf8_validate(cstr, -1, &pend) || !pend || *pend))
    MOM_FAILURE("MomString::compute_hash_dim invalid UTF-8 string");
  if (MOM_UNLIKELY(pend - cstr > INT32_MAX))
    MOM_FAILURE("MomString::compute_hash_dim huge size " << (pend - cstr));
  MomSize sz = pend - cstr;
  MomHash h1 = 13*sz + 10;
  MomHash h2 = 34403;
  int cnt = 0;
  for (const char* pc = cstr; *pc; pc = g_utf8_next_char(pc))
    {
      gunichar uc = g_utf8_get_char(pc);
      if (cnt % 2 == 0)
        h1 = (h1*5413 + cnt) ^ (6427 * uc + 10);
      else
        h2 = (h2*9419) ^ (11*cnt + 11437 * uc - 17*cnt);
    }
  MomHash h = h1 ^ h2;
  if (MOM_UNLIKELY(mom_hash(h)==0))
    h = 5*(h1 & 0xfffff) + 13*(h2 & 0xffffff) + 3*(cnt&0xfff) + 9;
  if (psiz)
    *psiz = cnt;
  if (pbylen)
    *pbylen = sz;
  return mom_hash(h);
} // end MomString::compute_hash_dim


MomString::MomString(const char*cstr, MomSize sz, uint32_t bylen, MomHash h)
  : MomAnyVal(MomKind::TagStringK, sz, h),
    _bylen(bylen),
    _bstr{}
{
  MOM_ASSERT(sz==0 || cstr != nullptr,
             "MomString::MomString null cstr for sz=" << sz);
  memcpy(const_cast<char*>(_bstr), cstr, bylen);
} // end MomString::MomString


const MomString*
MomString::make_from_cstr(const char*cstr)
{
  if (MOM_UNLIKELY(cstr==nullptr)) return nullptr;
  MomSize sz = 0;
  uint32_t bylen = 0;
  MomHash h = compute_hash_dim(cstr, &sz, &bylen);
  MomString* res = nullptr;
  unsigned ix = slotindex(h);
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
      const MomString*strv = it->second;
      MOM_ASSERT(strv != nullptr, "null strv in buckix=" << buckix);
      if (MOM_UNLIKELY(strv->has_cstr_content(cstr, bylen)))
        return strv;
    }
  res = new (mom_newtg, ((bylen+MOM_FLEXIBLE_DIM+1)|7)+1) MomString(cstr,sz,bylen,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomString::make_from_cstr


const MomString*
MomString::make_sprintf(const char*fmt, ...)
{
  constexpr int tinylen = 64;
  va_list args;
  int l = 0;
  if (fmt==nullptr) return nullptr;
  if (strlen(fmt)<tinylen)
    {
      char buf[5*tinylen+40];
      memset(buf, 0, sizeof(buf));
      va_start(args, fmt);
      l = vsnprintf(buf, sizeof(buf), fmt, args);
      va_end(args);
      if (l < (int)sizeof(buf))
        return  make_from_cstr(buf);
    }
  char *pbuf = nullptr;
  va_start(args, fmt);
  l = vasprintf(&pbuf, fmt, args);
  va_end(args);
  if (MOM_UNLIKELY(l<0 || pbuf==nullptr))
    MOM_FATALOG("MomString::make_sprintf out of memory fmt=" << fmt);
  auto res = make_from_cstr(pbuf);
  free (pbuf);
  return res;
} // end of MomString::make_sprintf

std::mutex*
MomString::valmtx() const
{
  return _mtxarr_+slotindex(hash());
} // end MomString::valmtx
