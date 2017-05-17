// file nodev.cc - node values

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


MomNode::MomPtrBag<MomNode> MomNode::_bagarr_[MomNode::_swidth_];
std::atomic<unsigned> MomNode::_nbclearedbags_;

MomHash
MomNode::compute_hash(const MomObject*conn, const MomValue*arr, MomSize sz)
{
  if (!conn || conn->vkind() != MomKind::TagObjectK)
    MOM_FAILURE("MomNode::compute_hash missing connective");
  MomHash h1 = (73453*conn->hash()) ^ (13477*sz +9);
  if (MOM_UNLIKELY(mom_hash(h1)==0))
    h1 = ((367*sz) & 0xfffff) + 140;
  MomHash h2 = (379*sz) + 101;
  if (sz==0) return mom_hash(h1);
  if (MOM_UNLIKELY(arr==nullptr))
    MOM_FAILURE("MomNode::compute_hash null arr with sz=" << sz);
  for (unsigned ix=0; ix<sz; ix++)
    {
      auto curv = arr[ix];
      if (curv.is_empty())
        MOM_FAILURE("MomNode::compute_hash no curv @ix=" << ix);
      MomHash curh = curv.hash();
      if (ix % 2 == 0)
        {
          if (MOM_UNLIKELY(curv.is_transient()))
            curh += 5347*ix + 10;
          h1 = (5659*h1+11*ix) ^ (373*curh);
        }
      else
        {
          if (MOM_UNLIKELY(curv.is_transient()))
            curh += 5667*ix + 13;
          h2 = (5651*h2) ^ (277*curh + 13*ix);
        }
    }
  MomHash h = h1 ^ h2;
  if (MOM_UNLIKELY(mom_hash(h)==0))
    h = 11*(h1 & 0xffffff) + 5*(h2 & 0xffffff) + 1000;
  return mom_hash(h);
} // end MomNode::compute_hash

const MomNode*
MomNode::make_from_array(const MomObject*conn, const MomValue*varr, MomSize sz)
{
  MomHash h = compute_hash(conn,varr,sz);
  unsigned slotix = slotindex(h);
  auto& curbag = _bagarr_[slotix];
  return curbag.unsync_bag_make_from_hash
         (h,
          mom_align((sz-MOM_FLEXIBLE_DIM)*sizeof(MomValue*)),
          conn, varr, sz);
} // end MomNode::make_from_array



void
MomNode::gc_todo_clear_marks(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_marks start");
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
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_marks end");
} // end MomNode::gc_todo_clear_marks



void
MomNode::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_slot start slotix=" << slotix);
  auto& curbag = _bagarr_[slotix];
  std::lock_guard<std::mutex> gu(curbag._bag_mtx);
  curbag.unsync_bag_gc_clear_marks(gc);
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_slot end slotix=" << slotix);
} // end MomNode::gc_todo_clear_mark_slot


void
MomNode::scan_gc(MomGC*gc)const
{
  gc->scan_object(const_cast<MomObject*>(_nod_conn));
  for (auto vcomp : *this)
    gc->scan_value(vcomp);
} // end MomNode::scan_gc



std::mutex*
MomNode::valmtx() const
{
  return &_bagarr_[slotindex(hash())]._bag_mtx;
} // end MomNode::valmtx
