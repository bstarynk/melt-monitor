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
    _gc_timestart(0.0), _gc_allocstart(0), _gc_cycle(0),
    _gc_active(false),
    _gc_valque(), _gc_objque(), _gc_todoque()
{
  MOM_ASSERT(this == &MomGC::the_garbcoll, "non singleton MomGC");
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
MomGC::set_end_notify(std::function<void(MomGC*)> fun)
{
  std::lock_guard<std::mutex>  gu(_gc_mtx);
  unsync_set_end_notify(fun);
} // end MomGC::set_end_notify


void
MomGC::unsync_start_gc_cycle(void)
{
  MOM_ASSERT(_gc_todoque.empty(), "unsync_start_gc_cycle nonempty todoque");
  if (MOM_UNLIKELY(!_gc_active.exchange(true)))
    MOM_FATAPRINTF("unsync_start_gc_cycle still active GC");
  _gc_timestart.store(mom_clock_time (CLOCK_MONOTONIC));
  _gc_allocstart.store(MomAnyVal::allocation_word_count());
  // we zero all the clear counts, to be sure that the
  // gc_all_bags_cleared methods works well afterwards...
  MomIntSq::gc_zero_clear_count(this);
  MomDoubleSq::gc_zero_clear_count(this);
  MomString::gc_zero_clear_count(this);
  MomSet::gc_zero_clear_count(this);
  MomTuple::gc_zero_clear_count(this);
  MomNode::gc_zero_clear_count(this);
  MomObject::gc_zero_clear_count(this);
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
  const char* reason = nullptr;
  if (false) {}
  else if (!MomIntSq::gc_all_bags_cleared(this))
    reason = "ints";
  else if (!MomDoubleSq::gc_all_bags_cleared(this))
    reason = "doubles";
  else if (!MomString::gc_all_bags_cleared(this))
    reason = "strings";
  else if (!MomSet::gc_all_bags_cleared(this))
    reason = "sets";
  else if (!MomTuple::gc_all_bags_cleared(this))
    reason = "tuples";
  else if (!MomNode::gc_all_bags_cleared(this))
    reason = "nodes";
  else if (!MomObject::gc_all_buckets_cleared(this))
    reason = "objects";
  else
    {
      MOM_DEBUGLOG(garbcoll, "maybe_start_scan is starting scanning");
      add_todo([=](MomGC*thisgc)
      {
        thisgc->initialize_scan();
      });
      return;
    }
  {
    MOM_DEBUGLOG(garbcoll, "maybe_start_scan dont start scanning,"
                 << " should still clear " << reason);
  }
} // end MomGC::maybe_start_scan




void
MomGC::initialize_scan(void)
{
  MOM_DEBUGLOG(garbcoll, "MomGC::initialize_scan start");
  // scan the current predefined
  MomObject::do_each_predefined([=](const MomObject*pob)
  {
    scan_object(const_cast<MomObject*>(pob));
    return false;
  });
  // scan the current globdata
  MomRegisterGlobData::every_globdata
  ([=](const std::string&nam, std::atomic<MomObject*>*pdata)
  {
    MOM_ASSERT(!nam.empty(), "empty globdata name");
    MomObject*pob = pdata->load();
    if (pob == nullptr) return false;
    scan_object(pob);
    return false;
  });
  // collect and run the scan functions
  std::vector<std::function<void(MomGC*)>> scanfunvec;
  {
    std::lock_guard<std::mutex>  gu(_gc_mtx);
    scanfunvec.reserve(_gc_scanfunmap.size());
    for (auto it: _gc_scanfunmap)
      scanfunvec.push_back(it.second);
  }
  for (auto fun: scanfunvec)
    {
      fun(this);
    }
  scanfunvec.clear();
  MOM_BACKTRACELOG("incomplete MomGC::initialize_scan");
#warning incomplete MomGC::initialize_scan
  // should todo several scan object contents
  MOM_DEBUGLOG(garbcoll, "MomGC::initialize_scan end");
} // end MomGC::initialize_scan

unsigned
MomGC::add_scan_function(std::function<void(MomGC*)> fun)
{
  MOM_ASSERT(fun, "empty function to MomGC::add_scan_function");
  std::lock_guard<std::mutex>  gu(_gc_mtx);
  unsigned nbh = 1+_gc_scanfunmap.size();
  _gc_scanfunmap.insert({nbh,fun});
  return nbh;
} // end MomGC::add_scan_function

void
MomGC::remove_scan_handle(unsigned rk)
{
  std::lock_guard<std::mutex>  gu(_gc_mtx);
  _gc_scanfunmap.erase(rk);
} // end MomGC::remove_scan_handle

void
MomGC::todo_some_scan(void)
{
  constexpr unsigned chkvalsiz = 4;
  constexpr unsigned chkobjsiz = 3;
  bool donescanning = false;
  unsigned nbscanvaltodos = 2 + MomRandom::random_32u() % (5 * mom_nb_jobs / 4 + 2);
  unsigned nbval = chkvalsiz * nbscanvaltodos;
  std::vector<MomAnyVal*> vecval;
  vecval.reserve(nbval);
  std::vector<MomObject*> vecobj;
  unsigned nbscanobjtodos = 2 + MomRandom::random_32u() % (5 * mom_nb_jobs / 4 + 3);
  unsigned nbobj = chkobjsiz * nbscanobjtodos;
  vecobj.reserve(nbobj);
  MOM_DEBUGLOG(garbcoll, "MomGC::todo_some_scan nbscanvaltodos=" << nbscanvaltodos
               << " nbval=" << nbval
               << " nbscanobjtodos=" << nbscanobjtodos
               << " nbobj=" << nbobj);
  {
    std::lock_guard<std::mutex>  gu(_gc_mtx);
    if (_gc_valque.empty() && _gc_objque.empty())
      donescanning = true;
    else
      {
        while (vecval.size() < nbval && !_gc_valque.empty())
          {
            MomAnyVal* curval =_gc_valque.front();
            _gc_valque.pop_front();
            if (curval)
              vecval.push_back(curval);
          }
        while (vecobj.size() < nbobj && !_gc_objque.empty())
          {
            MomObject* curobj = _gc_objque.front();
            _gc_objque.pop_front();
            if (curobj)
              vecobj.push_back(curobj);
          }
      }
  }
  MOM_DEBUGLOG(garbcoll, "MomGC::todo_some_scan scanning "
               << (donescanning?"done":"incomplete"));
  if (donescanning)
    {
      MomIntSq::gc_zero_sweep_count(this);
      MomDoubleSq::gc_zero_sweep_count(this);
      MomString::gc_zero_sweep_count(this);
      MomSet::gc_zero_sweep_count(this);
      MomTuple::gc_zero_sweep_count(this);
      MomNode::gc_zero_sweep_count(this);
      MomObject::gc_zero_sweep_count(this);
      add_todo([=](MomGC*thisgc)
      {
        MomIntSq::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomDoubleSq::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomString::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomSet::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomTuple::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomNode::gc_todo_destroy_dead(thisgc);
      });
      add_todo([=](MomGC*thisgc)
      {
        MomObject::gc_todo_destroy_dead(thisgc);
      });
    }
  else
    {
      unsigned ixval = 0;
      unsigned sizval = vecval.size();
      while (ixval<sizval)
        {
          std::array<MomAnyVal*,chkvalsiz> chkval;
          for (unsigned j=0; j<chkvalsiz; j++)
            if (ixval<sizval) chkval[j]=vecval[ixval++];
          add_todo([=](MomGC*thisgc)
          {
            for (MomAnyVal* curval: chkval)
              {
                if (curval == nullptr)
                  continue;
                curval->gc_set_mark(thisgc, true);
                curval->scan_gc(thisgc);
              }
          });
        }
      unsigned ixobj = 0;
      unsigned sizobj = vecobj.size();
      while (ixobj<sizobj)
        {
          std::array<MomObject*,chkobjsiz> chkobj;
          for (unsigned j=0; j<chkobjsiz; j++)
            if (ixobj<sizobj) chkobj[j]=vecobj[ixobj++];
          add_todo([=](MomGC*thisgc)
          {
            for (MomObject* curpob: chkobj)
              {
                if (curpob == nullptr)
                  continue;
                curpob->gc_set_mark(thisgc, true);
                curpob->scan_gc(thisgc);
              }
          });
        }
      add_todo([=](MomGC*thisgc)
      {
        thisgc->todo_some_scan();
      });
    }
} // end MomGC::todo_some_scan


void
MomGC::maybe_done_sweep(void)
{
  MOM_DEBUGLOG(garbcoll, "MomGC::maybe_done_sweep start");
  const char* reason = nullptr;
  if (false) {}
  else if (!MomIntSq::gc_all_bags_sweeped(this))
    reason = "ints";
  else if (!MomDoubleSq::gc_all_bags_sweeped(this))
    reason = "doubles";
  else if (!MomString::gc_all_bags_sweeped(this))
    reason = "strings";
  else if (!MomSet::gc_all_bags_sweeped(this))
    reason = "sets";
  else if (!MomTuple::gc_all_bags_sweeped(this))
    reason = "tuples";
  else if (!MomNode::gc_all_bags_sweeped(this))
    reason = "nodes";
  else if (!MomObject::gc_all_buckets_sweeped(this))
    reason = "objects";
  else
    {
      std::function<void(MomGC*)> endfun;
      {

        std::lock_guard<std::mutex>  gu(_gc_mtx);
        endfun = _gc_notifyendcycle;
      }
      _gc_active.store(false);
      if (endfun)
        endfun(this);
#warning maybe_done_sweep completed GC cycle
      MOM_BACKTRACELOG("MomGC::maybe_done_sweep completed GC cycle");
      return;
    }
  {
    MOM_DEBUGLOG(garbcoll, "maybe_done_sweep still sweeping "
                 << reason);
  }
  MOM_DEBUGLOG(garbcoll, "MomGC::maybe_done_sweep end");
} // end MomGC::maybe_done_sweep
