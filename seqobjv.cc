// file seqobjv.cc - sequence of object  values - sets and tuples

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


template<unsigned hinit, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
MomHash
MomAnyObjSeq::compute_hash_seq(MomObject*const* obarr, unsigned sz)
{
  if (MOM_UNLIKELY(sz>0 && obarr==nullptr))
    MOM_FAILURE("MomAnyObjSeq::compute_hash null obarr for sz=" << sz);
  MomHash h1 = hinit;
  MomHash h2 = sz ^ (hinit + k2 + (k2 + k3) / ((sz&0xffff)+2) + k4);
  for (unsigned ix=0; ix<sz; ix++)
    {
      const MomObject* curob = obarr[ix];
      if (MOM_UNLIKELY(curob==nullptr))
        MOM_FAILURE("MomAnyObjSeq::compute_hash null object at ix=" << ix);
      if (MOM_UNLIKELY(curob->kindw() != MomKind::TagObjectK))
        MOM_FAILURE("MomAnyObjSeq::compute_hash bad object at ix=" << ix);
      MomHash hob = curob->hash();
      if (ix % 2 == 0)
        h1 = (k1 * h1) ^ (hob * k2 + ix);
      else
        h2 = (k3 * h2 + 11 * (ix&0xffff)) ^ (hob * k4);
    }
  MomHash h = h1 ^ h2;
  if (MOM_UNLIKELY(mom_hash(h)==0))
    return 3*(h1 & 0xffffff) + 5*(h2 & 0xfffff) + (sz & 0xffff) + hinit/1000 + 8;
  return mom_hash(h);
} // end of MomAnyObjSeq::compute_hash


void
MomAnyObjSeq::scan_gc(MomGC* gc) const
{
  for (auto pob : *this)
    if (pob)
      gc->scan_object(const_cast<MomObject*>(pob));
} // end MomAnyObjSeq::scan_gc


MomAnyObjSeq::MomAnyObjSeq(MomKind kd, MomObject*const* obarr, MomSize sz, MomHash h)
  : MomAnyVal(kd,sz,h),
    _obseq{nullptr}
{
  memcpy(const_cast<MomObject**>(_obseq), obarr, sz*sizeof(MomObject*));
} // end MomAnyObjSeq::MomAnyObjSeq





//////////////// sets

std::mutex MomSet::_mtxarr_[MomSet::_swidth_];
std::unordered_multimap<MomHash,const MomSet*> MomSet::_maparr_[MomSet::_swidth_];

MomHash
MomSet::compute_hash(MomObject*const* obarr, unsigned sz)
{
  return compute_hash_seq<MomSet::hinit,
         MomSet::k1,
         MomSet::k2,
         MomSet::k3,
         MomSet::k4>(obarr, sz);
};


void
MomSet::gc_todo_clear_marks(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_marks start");
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_slot(thisgc,ix);
    });
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_marks end");
} // end MomSet::gc_todo_clear_marks



void
MomSet::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_slot start slotix=" << slotix);
  std::lock_guard<std::mutex> gu(_mtxarr_[slotix]);
  unsigned chcnt = 0;
  unsigned chunkix=0;
  std::array<MomSet*,_chunklen_> arrptr;
  for (auto p : _maparr_[slotix])
    {
      if (chcnt>=_chunklen_)
        {
          gc->add_todo([=](MomGC*thisgc)
          {
            gc_todo_clear_mark_chunk(thisgc,slotix,chunkix,arrptr);
          });
          chunkix++;
          chcnt=0;
        }
      arrptr[chcnt++] = const_cast<MomSet*>(p.second);
    }
  if (chcnt>0)
    {
      gc->add_todo([=](MomGC*thisgc)
      {
        gc_todo_clear_mark_chunk(thisgc,slotix,chunkix,arrptr);
      });
      chunkix++;
    }
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_slot end slotix=" << slotix
               << " last chunkix=" << chunkix);
} // end MomSet::gc_todo_clear_mark_slot

void
MomSet::gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomSet*,_chunklen_> arrptr)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_chunk invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_chunk start slotix=" << slotix
               << " chunkix=" << chunkix);
  /// we don't need to lock any mutex
  for (MomSet*pis : arrptr)
    {
      if (!pis) break;
      pis->gc_set_mark(gc,false);
    }
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_chunk end slotix=" << slotix
               << " chunkix=" << chunkix);
} // end MomSet::gc_todo_clear_mark_chunk

const MomSet*
MomSet::make_from_ascending_array(MomObject*const* obarr, MomSize sz)
{
  MomHash h = compute_hash(obarr, sz);
  MomSet* res = nullptr;
  for (unsigned ix=1; ix<sz; ix++)
    {
      const MomObject*curob = obarr[ix];
      const MomObject*prevob = obarr[ix-1];
      if (MOM_UNLIKELY(!prevob->less(curob)))
        MOM_FAILURE("MomSet::make_from_ascending_array bad order at ix="
                    << ix);
    }
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
      const MomSet*iset = it->second;
      MOM_ASSERT(iset != nullptr, "null iset in buckix=" << buckix);
      if (MOM_UNLIKELY(iset->has_content(obarr, sz)))
        {
          MomGC::the_garbcoll.scan_anyval(const_cast<MomSet*>(iset));
          return iset;
        }
    }
  res = new(mom_newtg, (sz-MOM_FLEXIBLE_DIM)*sizeof(MomObject*)) MomSet(obarr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomSet::make_from_ascending_array

const MomSet*
MomSet::make_from_objptr_set(const MomObjptrSet&oset)
{
  MomObjptrVector vec;
  vec.reserve(oset.size());
  for (MomObject* pob : oset)
    {
      if (MOM_LIKELY(pob != nullptr))
        vec.push_back(pob);
    }
  return make_from_ascending_objptr_vector(vec);
} // end MomSet::make_from_objptr_set

std::mutex*
MomSet::valmtx() const
{
  return _mtxarr_+slotindex(hash());
} // end MomSet::valmtx

//////////////// tuples

std::mutex MomTuple::_mtxarr_[MomTuple::_swidth_];
std::unordered_multimap<MomHash,const MomTuple*> MomTuple::_maparr_[MomTuple::_swidth_];


MomHash
MomTuple::compute_hash(MomObject*const* obarr, unsigned sz)
{
  return  compute_hash_seq<MomTuple::hinit,
          MomTuple::k1,
          MomTuple::k2,
          MomTuple::k3,
          MomTuple::k4>(obarr, sz);
};


const MomTuple*
MomTuple::make_from_array(MomObject*const* obarr, MomSize sz)
{
  MomHash h = compute_hash(obarr, sz);
  MomTuple* res = nullptr;
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
      const MomTuple*ituple = it->second;
      MOM_ASSERT(ituple != nullptr, "null ituple in buckix=" << buckix);
      if (MOM_UNLIKELY(ituple->has_content(obarr, sz)))
        {
          MomGC::the_garbcoll.scan_anyval(const_cast<MomTuple*>(ituple));
          return ituple;
        }
    }
  res = new(mom_newtg, (sz-MOM_FLEXIBLE_DIM)*sizeof(MomObject*)) MomTuple(obarr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomTuple::make_from_array

void
MomTuple::gc_todo_clear_marks(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_marks start");
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_slot(thisgc,ix);
    });
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_marks end");
} // end MomTuple::gc_todo_clear_marks



void
MomTuple::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_slot start slotix=" << slotix);
  std::lock_guard<std::mutex> gu(_mtxarr_[slotix]);
  unsigned chcnt = 0;
  unsigned chunkix=0;
  std::array<MomTuple*,_chunklen_> arrptr;
  for (auto p : _maparr_[slotix])
    {
      if (chcnt>=_chunklen_)
        {
          gc->add_todo([=](MomGC*thisgc)
          {
            gc_todo_clear_mark_chunk(thisgc,slotix,chunkix,arrptr);
          });
          chunkix++;
          chcnt=0;
        }
      arrptr[chcnt++] = const_cast<MomTuple*>(p.second);
    }
  if (chcnt>0)
    {
      gc->add_todo([=](MomGC*thisgc)
      {
        gc_todo_clear_mark_chunk(thisgc,slotix,chunkix,arrptr);
      });
      chunkix++;
    }
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_slot end slotix=" << slotix
               << " last chunkix=" << chunkix);
} // end MomTuple::gc_todo_clear_mark_slot

void
MomTuple::gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomTuple*,_chunklen_> arrptr)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_chunk invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_chunk start slotix=" << slotix
               << " chunkix=" << chunkix);
  /// we don't need to lock any mutex
  for (MomTuple*pis : arrptr)
    {
      if (!pis) break;
      pis->gc_set_mark(gc,false);
    }
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_chunk end slotix=" << slotix
               << " chunkix=" << chunkix);
} // end MomTuple::gc_todo_clear_mark_chunk


std::mutex*
MomTuple::valmtx() const
{
  return _mtxarr_+slotindex(hash());
} // end MomTuple::valmtx
