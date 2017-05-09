// file nodev.cc - scalar values

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

std::mutex MomNode::_mtxarr_[MomNode::_swidth_];
std::unordered_multimap<MomHash,const MomNode*> MomNode::_maparr_[MomNode::_swidth_];

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
  MomNode*res = nullptr;
  MomHash h = compute_hash(conn,varr,sz);
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
      const MomNode*ind = it->second;
      MOM_ASSERT(ind != nullptr, "null ind in buckix=" << buckix);
      if (MOM_UNLIKELY(ind->has_content(conn, varr, sz)))
        return ind;
    }
  res = new(mom_newtg, mom_align((sz-MOM_FLEXIBLE_DIM)*sizeof(MomValue))) MomNode(conn,varr,sz,h);
  curmap.insert({h,res});
  if (MOM_UNLIKELY(MomRandom::random_32u() % minbuckcount == 0))
    {
      curmap.reserve(9*curmap.size()/8 + 5);
    }
  return res;
} // end MomNode::make_from_array


void
MomNode::gc_todo_clear_marks(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_marks start");
  for (unsigned ix=0; ix<_swidth_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_slot(thisgc,ix);
    });
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_marks end");
} // end MomNode::gc_todo_clear_marks



void
MomNode::gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_slot invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_slot start slotix=" << slotix);
  std::lock_guard<std::mutex> gu(_mtxarr_[slotix]);
  unsigned chcnt = 0;
  unsigned chunkix=0;
  std::array<MomNode*,_chunklen_> arrptr;
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
      arrptr[chcnt++] = const_cast<MomNode*>(p.second);
    }
  if (chcnt>0)
    {
      gc->add_todo([=](MomGC*thisgc)
      {
        gc_todo_clear_mark_chunk(thisgc,slotix,chunkix,arrptr);
      });
      chunkix++;
    }
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_slot end slotix=" << slotix
               << " last chunkix=" << chunkix);
} // end MomNode::gc_todo_clear_mark_slot

void
MomNode::gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomNode*,_chunklen_> arrptr)
{
  MOM_ASSERT(slotix<_swidth_, "gc_todo_clear_mark_chunk invalid slotix=" << slotix);
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_chunk start slotix=" << slotix
               << " chunkix=" << chunkix);
  /// we don't need to lock any mutex
  for (MomNode*pis : arrptr)
    {
      if (!pis) break;
      pis->gc_set_mark(gc,false);
    }
  MOM_DEBUGLOG(garbcoll, "MomNode::gc_todo_clear_mark_chunk end slotix=" << slotix
               << " chunkix=" << chunkix);
} // end MomNode::gc_todo_clear_mark_chunk



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
  return _mtxarr_+slotindex(hash());
} // end MomNode::valmtx
