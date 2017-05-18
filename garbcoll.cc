// file garbcoll.cc - garbage collector

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

// see http://wiki.luajit.org/New-Garbage-Collector
std::atomic<bool> MomGC::_forbid_allocation_;
thread_local bool MomAnyVal::_allocok;

MomGC MomGC::the_garbcoll;

MomGC::MomGC()
  : _gc_thrid(std::this_thread::get_id()), _gc_mtx(), _gc_changecond(),
    _gc_valque(), _gc_objque(), _gc_todoque()
{
  MOM_DEBUGLOG(garbcoll, "MomGC created thrid=" << _gc_thrid);
} // end MomGC::MomGC

MomGC::~MomGC()
{
  MOM_DEBUGLOG(garbcoll, "MomGC destroyed thrid=" << _gc_thrid);
} // end MomGC::~MomGC


void
MomGC::scan_value(const MomValue v)
{
  const MomAnyVal* av = v.to_val();
  if (!av) return;
  if (av->vkind() == MomKind::TagObjectK)
    scan_object(reinterpret_cast<MomObject*>(const_cast<MomAnyVal*>(av)));
  else
    scan_anyval(av);
} // end MomGC::scan_value


void
MomGC::scan_object(MomObject*pob)
{
  if (!pob) return;
  MOM_ASSERT(pob->vkind() == MomKind::TagObjectK, "scan_object bad pob");
  bool oldgray = pob->gc_set_grey(this,true);
  if (!oldgray)
    {
      std::lock_guard<std::mutex> gu(_gc_mtx);
      _gc_objque.push_back(pob);
    }
} // end MomGC::scan_object

void
MomGC::scan_anyval(const MomAnyVal*av)
{
  if (!av) return;
  MomAnyVal* aval = const_cast<MomAnyVal*>(av);
  bool oldgray = aval->gc_set_grey(this,true);
  if (!oldgray)
    {
      std::lock_guard<std::mutex>  gu(_gc_mtx);
      _gc_valque.push_back(aval);
    }
} // end MomGC::scan_anyval

void
MomGC::add_todo(std::function<void(MomGC*)> fun)
{
  MOM_ASSERT(fun, "empty fun for GC add_todo");
  std::lock_guard<std::mutex>  gu(_gc_mtx);
  unsync_add_todo(fun);
} // end MomGC::add_todo


void
MomGC::unsync_start_gc_cycle(void)
{
  MOM_ASSERT(_gc_todoque.empty(), "unsync_start_gc_cycle nonempty todoque");
  unsync_add_todo([=](MomGC*gc)
  {
    MomIntSq::gc_todo_clear_marks(gc);
  });
  unsync_add_todo([=](MomGC*gc)
  {
    MomDoubleSq::gc_todo_clear_marks(gc);
  });
  unsync_add_todo([=](MomGC*gc)
  {
    MomString::gc_todo_clear_marks(gc);
  });
  unsync_add_todo([=](MomGC*gc)
  {
    MomSet::gc_todo_clear_marks(gc);
  });
  unsync_add_todo([=](MomGC*gc)
  {
    MomTuple::gc_todo_clear_marks(gc);
  });
  unsync_add_todo([=](MomGC*gc)
  {
    MomNode::gc_todo_clear_marks(gc);
  });
} // end MomGC::unsync_start_gc_cycle

void
MomGC::maybe_start_scan(void)
{
  if (true
      && MomIntSq::gc_all_bags_cleared(this)
      && MomDoubleSq::gc_all_bags_cleared(this)
      && MomString::gc_all_bags_cleared(this)
      && MomSet::gc_all_bags_cleared(this)
      && MomTuple::gc_all_bags_cleared(this)
      && MomNode::gc_all_bags_cleared(this)
      && MomObject::gc_all_buckets_cleared(this)
     )
    {
      MOM_DEBUGLOG(garbcoll, "maybe_start_scan is starting scanning");
      add_todo([=](MomGC*thisgc)
      {
        thisgc->initialize_scan();
      });
    }
  else
    {
      MOM_DEBUGLOG(garbcoll, "maybe_start_scan dont start scanning");
    }
} // end MomGC::maybe_start_scan

void
MomGC::initialize_scan(void)
{
  MOM_DEBUGLOG(garbcoll, "MomGC::initialize_scan start");
  // scan the predefined
  MomObject::do_each_predefined([=](MomObject*pob)
  {
    scan_object(pob);
    return true;
  });
  MOM_FATAPRINTF("unimplemented MomGC::initialize_scan");
#warning  unimplemented MomGC::initialize_scan
  // should scan the globals and the local of every non-worker threads
  MOM_DEBUGLOG(garbcoll, "MomGC::initialize_scan end");
} // end MomGC::initialize_scan
