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
MomSet::MomPtrBag<MomSet> MomSet::_bagarr_[MomSet::_swidth_];
std::atomic<unsigned> MomSet::_nbclearedbags_;
std::atomic<unsigned> MomSet::_nbsweepedbags_;

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
  _nbclearedbags_.store(0);
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_slot(thisgc,ix);
      if (1+_nbclearedbags_.fetch_add(1) >= _swidth_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_start_scan();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_marks end");
} // end MomSet::gc_todo_clear_marks



void
MomSet::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_slot start slotix=" << slotix);
  auto& curbag = _bagarr_[slotix];
  std::lock_guard<std::mutex> gu(curbag._bag_mtx);
  curbag.unsync_bag_gc_clear_marks(gc);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_clear_mark_slot end slotix=" << slotix);
} // end MomSet::gc_todo_clear_mark_slot

void
MomSet::gc_todo_destroy_dead(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_destroy_dead start");
  _nbsweepedbags_.store(0);
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_sweep_destroy_slot(thisgc,ix);
      if (1+_nbsweepedbags_.fetch_add(1) >= _swidth_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_done_sweep();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_destroy_dead end");
} // end MomSet::gc_todo_destroy_dead


void
MomSet::gc_todo_sweep_destroy_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_sweep_destroy_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_sweep_destroy_slot start slotix=" << slotix);
  auto& curbag = _bagarr_[slotix];
  std::lock_guard<std::mutex> gu(curbag._bag_mtx);
  curbag.unsync_bag_gc_delete_unmarked_values(gc);
  MOM_DEBUGLOG(garbcoll, "MomSet::gc_todo_sweep_destroy_slot end slotix=" << slotix);
} // end MomSet::gc_todo_clear_mark_slot


const MomSet*
MomSet::make_from_ascending_array(const MomObject*const* obarr, MomSize sz)
{
  MomHash h = compute_hash(const_cast<MomObject*const*>(obarr), sz);
  unsigned slotix = slotindex(h);
  auto& curbag = _bagarr_[slotix];
  return curbag.unsync_bag_make_from_hash
         (h,
          mom_align((sz-MOM_FLEXIBLE_DIM)*sizeof(MomObject*)),
          const_cast<MomObject**>(obarr), sz);
} // end MomSet::make_from_ascending_array

const MomSet*
MomSet::make_from_objptr_set(const MomObjptrSet&oset)
{
  MomObjptrVector vec;
  vec.reserve(oset.size());
  for (const MomObject* pob : oset)
    {
      if (MOM_LIKELY(pob != nullptr))
        vec.push_back(pob);
    }
  return make_from_ascending_objptr_vector(vec);
} // end MomSet::make_from_objptr_set

std::mutex*
MomSet::valmtx() const
{
  return &_bagarr_[slotindex(hash())]._bag_mtx;
} // end MomSet::valmtx




//////////////// tuples

MomTuple::MomPtrBag<MomTuple> MomTuple::_bagarr_[MomTuple::_swidth_];
std::atomic<unsigned> MomTuple::_nbclearedbags_;
std::atomic<unsigned> MomTuple::_nbsweepedbags_;

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
  unsigned slotix = slotindex(h);
  auto& curbag = _bagarr_[slotix];
  return curbag.unsync_bag_make_from_hash
         (h,
          mom_align((sz-MOM_FLEXIBLE_DIM)*sizeof(MomObject*)),
          obarr, sz);
} // end MomTuple::make_from_array

void
MomTuple::gc_todo_clear_marks(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_marks start");
  _nbclearedbags_.store(0);
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_slot(thisgc,ix);
      if (1+_nbclearedbags_.fetch_add(1) >= _swidth_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_start_scan();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_marks end");
} // end MomTuple::gc_todo_clear_marks




void
MomTuple::gc_todo_destroy_dead(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_destroy_dead start");
  _nbsweepedbags_.store(0);
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_sweep_destroy_slot(thisgc,ix);
      if (1+_nbsweepedbags_.fetch_add(1) >= _swidth_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_done_sweep();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_destroy_dead end");
} // end MomTuple::gc_todo_destroy_dead

void
MomTuple::gc_todo_sweep_destroy_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_sweep_destroy_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_sweep_destroy_slot start slotix=" << slotix);
  auto& curbag = _bagarr_[slotix];
  std::lock_guard<std::mutex> gu(curbag._bag_mtx);
  curbag.unsync_bag_gc_delete_unmarked_values(gc);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_sweep_destroy_slot end slotix=" << slotix);
} // end MomTuple::gc_todo_clear_mark_slot


void
MomTuple::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_slot start slotix=" << slotix);
  auto& curbag = _bagarr_[slotix];
  std::lock_guard<std::mutex> gu(curbag._bag_mtx);
  curbag.unsync_bag_gc_clear_marks(gc);
  MOM_DEBUGLOG(garbcoll, "MomTuple::gc_todo_clear_mark_slot end slotix=" << slotix);
} // end MomTuple::gc_todo_clear_mark_slot


std::mutex*
MomTuple::valmtx() const
{
  return &_bagarr_[slotindex(hash())]._bag_mtx;
} // end MomTuple::valmtx
