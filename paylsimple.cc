// file paylsimple.cc - simple payloads

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


class MomPaylNamed : public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
  friend void mom_register_unsync_named(MomObject*obj, const char*name);
  friend void mom_forget_unsync_named_object(MomObject*obj);
  friend MomObject*mom_find_named(const char*name);
  friend const char* mom_get_unsync_name(const MomObject*obj);
  friend const std::string mom_get_unsync_string_name(const MomObject*obj);
  friend void mom_forget_name(const char*name);
  friend MomObject*mom_unsync_named_object_proxy(MomObject*objn);
  friend void mom_unsync_named_object_set_proxy(MomObject*objn, MomObject*obproxy);
  friend void mom_each_name_prefixed(const char*prefix,
                                     std::function<bool(const std::string&,MomObject*)> fun);
  friend long mom_nb_named(void);
  typedef std::string stringty;
private:
  const stringty _nam_str;
  static std::mutex _nam_mtx_;
  static std::map<std::string,MomObject*> _nam_dict_;
  MomPaylNamed(MomObject*own, const char*name)
    : MomPayload(&MOM_PAYLOADVTBL(named), own), _nam_str(name) {};
  ~MomPaylNamed()
  {
    (const_cast<stringty*>(&_nam_str))->clear();
  };
public:
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
}; // end class MomPaylNamed

std::mutex MomPaylNamed::_nam_mtx_;
std::map<std::string,MomObject*> MomPaylNamed::_nam_dict_;

void
mom_register_unsync_named(MomObject*obj, const char*name)
{
  if (!obj || obj->vkind() != MomKind::TagObjectK) return;
  if (!mom_valid_name_radix_len(name,-1)) return;
  std::lock_guard<std::mutex> gu(MomPaylNamed::_nam_mtx_);
  auto py = obj->unsync_make_payload<MomPaylNamed>(name);
  MomPaylNamed::_nam_dict_.insert({py->_nam_str,obj});
} // end mom_register_unsync_named

void
mom_forget_unsync_named_object(MomObject*obj)
{
  if (!obj) return;
  auto py = static_cast<MomPaylNamed*>(obj->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return;
  std::lock_guard<std::mutex> gu(MomPaylNamed::_nam_mtx_);
  MomPaylNamed::_nam_dict_.erase(py->_nam_str);
  obj->unsync_clear_payload();
} // end  mom_forget_unsync_named_object

void
mom_unsync_named_object_set_proxy(MomObject*objn, MomObject*obproxy)
{
  if (!objn) return;
  auto py = static_cast<MomPaylNamed*>(objn->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return;
  if (obproxy)
    {
      if (obproxy->vkind() != MomKind::TagObjectK)
        MOM_FAILURE("mom_unsync_named_object_set_proxy objn=" << objn << " bad proxy");
    }
  py->set_proxy(obproxy);
} // end mom_unsync_named_object_set_proxy

MomObject*
mom_unsync_named_object_proxy(MomObject*objn)
{
  if (!objn) return nullptr;
  auto py = static_cast<MomPaylNamed*>(objn->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return nullptr;
  return py->proxy();
} // end mom_unsync_named_object_proxy

MomObject*
mom_find_named(const char*name)
{
  if (!mom_valid_name_radix_len(name,-1))
    return nullptr;
  std::lock_guard<std::mutex> gu(MomPaylNamed::_nam_mtx_);
  auto it = MomPaylNamed::_nam_dict_.find(std::string{name});
  if (it != MomPaylNamed::_nam_dict_.end())
    return it->second;
  return nullptr;
} // end mom_find_named

long mom_nb_named(void)
{
  std::lock_guard<std::mutex> gu(MomPaylNamed::_nam_mtx_);
  return MomPaylNamed::_nam_dict_.size();
} // end mom_nb_named

void
mom_each_name_prefixed(const char*prefix,
                       std::function<bool(const std::string&,MomObject*)> fun)
{
  if (!prefix) prefix="";
  if (prefix[0] && !mom_valid_name_radix_len(prefix,-1))
    return;
  size_t prefixlen = strlen(prefix);
  std::lock_guard<std::mutex> gu(MomPaylNamed::_nam_mtx_);
  auto it = MomPaylNamed::_nam_dict_.lower_bound(std::string{prefix});
  while (it != MomPaylNamed::_nam_dict_.end())
    {
      std::string curnam = it->first;
      MomObject* curobj = it->second;
      if (strncmp(curnam.c_str(),prefix,prefixlen))
        break;
      if (fun(curnam,curobj))
        break;
      it++;
    }
} // end mom_each_name_prefixed


const char*
mom_get_unsync_name(const MomObject*obj)
{
  auto py = static_cast<MomPaylNamed*>(obj->unsync_payload());
  if (!py || py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return nullptr;
  return py->_nam_str.c_str();
} // end mom_get_unsync_name


const std::string
mom_get_unsync_string_name(const MomObject*obj)
{
  auto py = static_cast<MomPaylNamed*>(obj->unsync_payload());
  if (!py || py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named))
    return std::string{};
  return py->_nam_str;
} // end mom_get_unsync_string_name


const struct MomVtablePayload_st MOM_PAYLOADVTBL(named) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylNamed),
  /**   .pyv_name=       */       "named",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylNamed::Destroy,
  /**   .pyv_scangc=     */       MomPaylNamed::Scangc,
  /**   .pyv_scandump=   */       MomPaylNamed::Scandump,
  /**   .pyv_emitdump=   */       MomPaylNamed::Emitdump,
  /**   .pyv_initload=   */       MomPaylNamed::Initload,
  /**   .pyv_loadfill=   */       MomPaylNamed::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylNamed::Getmagic,
  /**   .pyv_fetch=      */       nullptr,
  /**   .pyv_updated=     */       nullptr,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_named(MOM_PAYLOADVTBL(named));

void
MomPaylNamed::Destroy (struct MomPayload*payl,MomObject*own)
{
  auto py = static_cast<MomPaylNamed*>(payl);
  std::lock_guard<std::mutex> gu(_nam_mtx_);
  auto it = _nam_dict_.find(py->_nam_str);
  MOM_ASSERT(it != _nam_dict_.end() && it->second == own, "corrupted _nam_dict_ for own=" << own);
  _nam_dict_.erase(it);
  delete py;
} // end MomPaylNamed::Destroy


void
MomPaylNamed::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "invalid named payload for own=" << own);
} // end MomPaylNamed::Scangc


void
MomPaylNamed::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylNamed::Scandump own=" << own << " name=" << py->_nam_str
               << " proxy=" << py->proxy());
} // end MomPaylNamed::Scandump


void
MomPaylNamed::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "invalid named payload for own=" << own);
  MOM_DEBUGLOG(dump, "PaylNamed::Emitdump own=" << own << " name=" << py->_nam_str
               << " proxy=" << py->proxy());
  mom_dump_named_update_defer(du, own, py->_nam_str);
  empaylinit->out() << py->_nam_str;
} // end MomPaylNamed::Emitdump


MomPayload*
MomPaylNamed::Initload(MomObject*own,MomLoader*,const char*inits)
{
  MOM_DEBUGLOG(load,"PaylNamed::Initload own=" << own << " inits='" << inits << "'");
  mom_register_unsync_named(own,inits);
  return own->unsync_payload();
} // end MomPaylNamed::Initload



void
MomPaylNamed::Loadfill(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills)
{
  auto py = static_cast< MomPaylNamed*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "PaylNamed::Loadfill invalid named payload for own=" << own);
  MOM_DEBUGLOG(load,"PaylNamed::Loadfill own=" << own
               << " named:" <<  py->_nam_str
               << " fills='" << fills << "'");
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Named fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  if (fillpars.eof())
    return;
} // end MomPaylNamed::Loadfill


MomValue
MomPaylNamed::Getmagic (const struct MomPayload*payl,const MomObject*targetob,const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  if (depth >= MomPayload::_py_max_proxdepth_)
    MOM_FAILURE("too deep named getmagic targetob=" << targetob << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
                << " owner=" << py->owner());
  MomObject*proxob = nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "MomPaylNamed::Getmagic invalid named payload for own=" << payl->owner());
  if (attrob == MOMP_name)
    {
      if (pgotit)
        *pgotit = true;
      return MomString::make_from_string(py->_nam_str);
    }
  else if (attrob == MOMP_proxy)
    {
      if (pgotit)
        *pgotit = true;
      return py->proxy();
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
} // end   MomPaylNamed::Getmagic




////////////////////////////////////////////////////////////////

class MomPaylSet: public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
  friend MomObject*mom_unsync_pset_object_proxy(MomObject*objn);
  friend void mom_unsync_pset_object_set_proxy(MomObject*objn, MomObject*obproxy);
private:
  std::set<const MomObject*,MomObjptrLess> _pset_set;
  MomPaylSet(MomObject*own)
    : MomPayload(&MOM_PAYLOADVTBL(set), own), _pset_set() {};
  ~MomPaylSet()
  {
    _pset_set.clear();
  };
public:
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
  static MomPyv_fetch_sig Fetch;
  static MomPyv_updated_sig Updated;
}; // end class MomPaylSet



MomObject*
mom_unsync_pset_object_proxy(MomObject*objs)
{
  if (!objs) return nullptr;
  auto py = static_cast<MomPaylSet*>(objs->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(set)) return nullptr;
  return py->proxy();
} // end mom_unsync_pset_object_proxy

void mom_unsync_pset_object_set_proxy(MomObject*objn, MomObject*obproxy)
{
  if (!objn) return;
  auto py = static_cast<MomPaylSet*>(objn->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(set)) return;
  py->set_proxy(obproxy);
}

const struct MomVtablePayload_st MOM_PAYLOADVTBL(set) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylSet),
  /**   .pyv_name=       */       "set",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylSet::Destroy,
  /**   .pyv_scangc=     */       MomPaylSet::Scangc,
  /**   .pyv_scandump=   */       MomPaylSet::Scandump,
  /**   .pyv_emitdump=   */       MomPaylSet::Emitdump,
  /**   .pyv_initload=   */       MomPaylSet::Initload,
  /**   .pyv_loadfill=   */       MomPaylSet::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylSet::Getmagic,
  /**   .pyv_fetch=      */       MomPaylSet::Fetch,
  /**   .pyv_updated=    */       MomPaylSet::Updated,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_set(MOM_PAYLOADVTBL(set));

void
MomPaylSet::Destroy (struct MomPayload*payl, MomObject*own)
{
  auto py = static_cast<MomPaylSet*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "invalid set payload for own=" << own);
  delete py;
} // end MomPaylSet::Destroy

void
MomPaylSet::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "invalid set payload for own=" << own);
  for (const MomObject* pob : py->_pset_set)
    gc->scan_object(const_cast<MomObject*>(pob));
} // end MomPaylSet::Scangc

void
MomPaylSet::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylSet::Scandump own=" << own
               << " proxy=" << py->proxy());
  for (const MomObject* pob : py->_pset_set)
    pob->scan_dump(du);
} // end MomPaylSet::Scandump

void
MomPaylSet::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "invalid pset payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylSet::Emitdump own=" << own
               << " proxy=" << py->proxy());
  empaylcont->out() << "@SET: ";
  for (const MomObject* pob : py->_pset_set)
    {
      if (!mom_dump_is_dumpable_object(du, const_cast<MomObject*>(pob)))
        continue;
      empaylcont->emit_space(1);
      empaylcont->emit_objptr(pob);
    }
} // end MomPaylSet::Emitdump

MomPayload*
MomPaylSet::Initload(MomObject*own,MomLoader*,const char*inits)
{
  MOM_DEBUGLOG(load,"MomPaylSet::Initload own=" << own << " inits='" << inits << "'");
  auto py = own->unsync_make_payload<MomPaylSet>();
  return py;
} // end MomPaylNamed::Initload


void
MomPaylSet::Loadfill(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills)
{
  auto py = static_cast< MomPaylSet*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Loadfill invalid set payload for own=" << own);
  MOM_DEBUGLOG(load,"MomPaylSet::Loadfill own=" << own
               << " fills::" << fills);
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Set fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  if (fillpars.got_cstring("@SET:"))
    {
      MomObject* pob = nullptr;
      bool gotpob = false;
      while ((pob = fillpars.parse_objptr(&gotpob)), gotpob)
        {
          if (pob)
            py->_pset_set.insert(pob);
        }
    };
} // end MomPaylSet::Loadfill




MomValue
MomPaylSet::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Getmagic invalid set payload for own=" << own);
  if (attrob == MOMP_size)
    {
      if (pgotit)
        *pgotit = true;
      return MomValue{(intptr_t)(py->_pset_set.size())};
    }
  else if (attrob == MOMP_set)
    {
      if (pgotit)
        *pgotit = true;
      return MomSet::make_from_objptr_set(py->_pset_set);
    }
  else if (attrob == MOMP_proxy)
    {
      if (pgotit)
        *pgotit = true;
      return py->proxy();
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
} // end   MomPaylSet::Getmagic


MomValue
MomPaylSet::Fetch(const struct MomPayload*payl,const MomObject*targpob,const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Fetch invalid set payload for targpob=" << targpob << " own=" << py->owner());
  MOM_ASSERT(pgotit, "MomPaylSet::Fetch no pgotit");
  if (attrob == MOMP_get)
    {
      auto elemval = vecarr[0].to_val();
      MomObject*elemob =
        elemval
        ?const_cast<MomObject*>(elemval->to_object())
        :nullptr;
      *pgotit = true;
      if (elemob && py->_pset_set.find(elemob) != py->_pset_set.end())
        return elemob;
      else return nullptr;
    };
  *pgotit = false;
  return nullptr;
} // end MomPaylSet::Fetch



bool
MomPaylSet::Updated(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth)
{
  auto py = static_cast< MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Updated invalid set payload for own=" << own);
  if (attrob == MOMP_put)
    {
      for (unsigned ix=0; ix<veclen; ix++)
        {
          auto elemval = vecarr[ix].to_val();
          auto elemob = elemval?const_cast<MomObject*>(elemval->to_object()):nullptr;
          if (elemob)
            py->_pset_set.insert(elemob);
        };
      return true;
    }
  return false;
} // end MomPaylSet::Updated

////////////////////////////////////////////////////////////////

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(strobuf);

class MomPaylStrobuf: public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
  static constexpr unsigned _max_strobuf_ = 1<<22;
  static constexpr unsigned _max_depth_ = 80;

private:
  std::ostringstream _pstrobuf_out;
  MomObject* _pstrobuf_starter;
  MomPaylStrobuf(MomObject*own)
    : MomPayload(&MOM_PAYLOADVTBL(strobuf), own), _pstrobuf_out(),
      _pstrobuf_starter(nullptr) {};
  ~MomPaylStrobuf()
  {
  };
public:
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
  static MomPyv_fetch_sig Fetch;
  static MomPyv_updated_sig Updated;
  void output_value_to_buffer(MomObject*forpob, const MomValue v, MomObject*ctxob=nullptr, int depth=0);
  void unsync_output_all_to_buffer(MomObject*forpob);
  std::string buffer_string()
  {
    _pstrobuf_out.flush();
    return _pstrobuf_out.str();
  }
}; // end class MomPaylStrobuf


const struct MomVtablePayload_st MOM_PAYLOADVTBL(strobuf) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylStrobuf),
  /**   .pyv_name=       */       "strobuf",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylStrobuf::Destroy,
  /**   .pyv_scangc=     */       MomPaylStrobuf::Scangc,
  /**   .pyv_scandump=   */       MomPaylStrobuf::Scandump,
  /**   .pyv_emitdump=   */       MomPaylStrobuf::Emitdump,
  /**   .pyv_initload=   */       MomPaylStrobuf::Initload,
  /**   .pyv_loadfill=   */       MomPaylStrobuf::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylStrobuf::Getmagic,
  /**   .pyv_fetch=      */       MomPaylStrobuf::Fetch,
  /**   .pyv_update=     */       MomPaylStrobuf::Updated,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_strobuf(MOM_PAYLOADVTBL(strobuf));

void
MomPaylStrobuf::Destroy (struct MomPayload*payl,MomObject*own)
{
  auto py = static_cast<MomPaylStrobuf*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "invalid strobuf payload for own=" << own);
  delete py;
} // end MomPaylStrobuf::Destroy

void
MomPaylStrobuf::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "invalid strobuf payload for own=" << own);
} // end MomPaylStrobuf::Scangc

void
MomPaylStrobuf::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylStrobuf::Scandump own=" << own
               << " proxy=" << py->proxy());
} // end MomPaylStrobuf::Scandump

void
MomPaylStrobuf::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "invalid strobuf payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylStrobuf::Emitdump own=" << own
               << " proxy=" << py->proxy());
  const_cast<std::ostringstream&>(py->_pstrobuf_out).flush();
  auto strb = py->_pstrobuf_out.str();
  empaylcont->emit_newline(0);
  empaylcont->out() << "@STROBUFSTR: ";
  {
    size_t eol = 0;
    size_t beglin = 0;
    while ((eol = strb.find('\n', beglin)) != std::string::npos)
      {
        while (eol+1 < strb.size() && strb[eol] == '\n') eol++;
        if (eol+1<strb.size())
          {
            std::string linstr = strb.substr(beglin, eol);
            empaylcont->emit_space(0);
            empaylcont->emit_string(linstr);
            beglin = eol+1;
            continue;
          }
        else
          {
            std::string laststr = strb.substr(beglin);
            empaylcont->emit_space(0);
            empaylcont->emit_string(laststr);
            break;
          }
      }
    empaylcont->emit_newline(0);
  }
  if (py->_pstrobuf_starter)
    {
      empaylcont->emit_newline(0);
      empaylcont->out() << "@STROBUFSTARTER: ";
      empaylcont->emit_objptr(py->_pstrobuf_starter);
    }
} // end MomPaylStrobuf::Emitdump



MomPayload*
MomPaylStrobuf::Initload(MomObject*own,MomLoader*,const char*inits)
{
  MOM_DEBUGLOG(load,"MomPaylStrobuf::Initload own=" << own << " inits='" << inits << "'");
  auto py = own->unsync_make_payload<MomPaylStrobuf>();
  return py;
} // end MomPaylNamed::Initload


void
MomPaylStrobuf::Loadfill(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills)
{
  auto py = static_cast< MomPaylStrobuf*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Loadfill invalid strobuf payload for own=" << own);
  MOM_DEBUGLOG(load,"MomPaylStrobuf::Loadfill own=" << own
               << " fills='" << fills << "'");
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Strobuf fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  if (fillpars.got_cstring("@STROBUFSTR:"))
    {
      bool gotstr = false;
      std::string linstr;
      while ((gotstr=false), (linstr=fillpars.parse_string(&gotstr)), gotstr)
        {
          py->_pstrobuf_out.write(linstr.c_str(), linstr.size());
          linstr.erase();
        }
    }
  if (fillpars.got_cstring("@STROBUFSTARTER:"))
    {
      bool gotpob = false;
      MomObject* pob =fillpars.parse_objptr(&gotpob);
      if (pob)
        py->_pstrobuf_starter = pob;
    }
} // end MomPaylStrobuf::Loadfill




MomValue
MomPaylStrobuf::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Getmagic invalid strobuf payload for own=" << own);
  if (attrob == MOMP_size)
    {
      if (pgotit)
        *pgotit = true;
      return MomValue{(intptr_t)(const_cast<MomPaylStrobuf*>(py)->_pstrobuf_out.tellp())};
    }
  else if (attrob == MOMP_proxy)
    {
      if (pgotit)
        *pgotit = true;
      return py->proxy();
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
} // end   MomPaylStrobuf::Getmagic


MomValue
MomPaylStrobuf::Fetch(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Fetch invalid strobuf payload for own=" << own);
  MOM_ASSERT(pgotit, "MomPaylStrobuf::Fetch no pgotit");
  if (attrob == MOMP_get)
    {
#warning MomPaylStrobuf::Fetch incomplete
    };
  *pgotit = false;
  return nullptr;
} // end MomPaylStrobuf::Fetch



bool
MomPaylStrobuf::Updated(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth)
{
  auto py = static_cast< MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Updated invalid strobuf payload for own=" << own);
#warning incomplete MomPaylStrobuf::Updated
  return false;
} // end MomPaylStrobuf::Updated



void
MomPaylStrobuf::output_value_to_buffer(MomObject*forob, const MomValue v,  MomObject* ctxob, int depth)
{
  if (depth > (int)_max_depth_)
    MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer too deep " << depth << " owner " << owner() << " for " << forob << " ctx " << ctxob);
  if (_pstrobuf_out.tellp() > _max_strobuf_)
    MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer too long buffer " << _pstrobuf_out.tellp() << " owner " << owner() << " for " << forob << " ctx " << ctxob);

  auto k = v.kind();
  switch (k)
    {
    case MomKind::TagNoneK:
      return;
    case MomKind::TagIntK:
      _pstrobuf_out << v.as_tagint();
      break;
    case MomKind::TagStringK:
      _pstrobuf_out << v->as_string()->cstr();
      break;
    case MomKind::TagObjectK:
      _pstrobuf_out << v->as_object();
      break;
    case MomKind::TagIntSqK:
    {
      auto isq = v->as_intsq();
      if (isq->sizew() == 1)
        _pstrobuf_out << isq->unsafe_at(0);
      else
        MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                    << " depth=" << depth << " ctx=" << ctxob
                    << " unexpected intseq:" << v);
    }
    break;
    case MomKind::TagDoubleSqK:
    {
      auto dsq = v->as_doublesq();
      if (dsq->sizew() == 1)
        _pstrobuf_out << dsq->unsafe_at(0);
      else
        MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                    << " depth=" << depth << " ctx=" << ctxob
                    << " unexpected doubleseq:" << v);
    }
    break;
    case MomKind::TagSetK:
      MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                  << " depth=" << depth << " ctx=" << ctxob
                  << " unexpected set:" << v);
      break;
    case MomKind::TagTupleK:
      MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                  << " depth=" << depth << " ctx=" << ctxob
                  << " unexpected tuple:" << v);
      break;
    case MomKind::TagNodeK:
    {
      auto nodv = v->as_node();
      MomObject* connob = nodv->conn();
      MomValue connoutv;
      {
        std::lock_guard<std::recursive_mutex> gu{connob->get_recursive_mutex()};
        connoutv = connob->unsync_get(MOMP_outputter);
      }
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                   << " v=" << v << " connoutv=" << connoutv);
      MomObject* coutob = const_cast<MomObject*>(connoutv->as_object());
      if (!coutob)
        MOM_FAILURE("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                    << " depth=" << depth << " ctx=" << ctxob
                    << " connoutv=" << connoutv
                    << " unexpected node:" << v);
      {
        std::lock_guard<std::recursive_mutex> gu{coutob->get_recursive_mutex()};
        coutob->unsync_step_arg(MomValue(forob),v,MomValue(ctxob),MomValue(depth));
      }
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::output_value_to_buffer done owner=" << owner()
                   << " v=" << v << " connoutv=" << connoutv);
    }
    break;
    case MomKind::Tag_LastK:
      MOM_FATALOG("MomPaylStrobuf::output_value_to_buffer owner=" << owner()
                  << " corrupted depth=" << depth << " ctx=" << ctxob);
      break;
    }
} // end of MomPaylStrobuf::output_value_to_buffer


////////////////////////////////////////////////////////////////

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(genfile);
class MomPaylStrobuf;
class MomPaylGenfile: public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
private:
  const std::string _pgenfile_pathstr;
  MomPaylGenfile(MomObject*own, const char*pathstr)
    : MomPayload(&MOM_PAYLOADVTBL(genfile), own),
      _pgenfile_pathstr(pathstr) {};
  ~MomPaylGenfile()
  {
  };
  MomObject* generated_strbuf_object(void); // gives an object with strbuf payload containing the generated stuff
public:
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
  static MomPyv_fetch_sig Fetch;
  static MomPyv_updated_sig Updated;
}; // end class MomPaylGenfile


const struct MomVtablePayload_st MOM_PAYLOADVTBL(genfile) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylGenfile),
  /**   .pyv_name=       */       "genfile",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylGenfile::Destroy,
  /**   .pyv_scangc=     */       MomPaylGenfile::Scangc,
  /**   .pyv_scandump=   */       MomPaylGenfile::Scandump,
  /**   .pyv_emitdump=   */       MomPaylGenfile::Emitdump,
  /**   .pyv_initload=   */       MomPaylGenfile::Initload,
  /**   .pyv_loadfill=   */       MomPaylGenfile::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylGenfile::Getmagic,
  /**   .pyv_fetch=      */       MomPaylGenfile::Fetch,
  /**   .pyv_update=     */       MomPaylGenfile::Updated,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_genfile(MOM_PAYLOADVTBL(genfile));


MomObject*
MomPaylGenfile::generated_strbuf_object(void)
{
  MomObject* pobown = owner();
  MomObject* pobuf= MomObject::make_object();
  auto pystrobuf = pobuf->unsync_make_payload<MomPaylStrobuf>();
  MOM_DEBUGLOG(gencod, "generated_strbuf_object pobown=" << MomShowObject(pobown)
               << " made pobuf=" << MomShowObject(pobuf)
               << " before start_generation "
               << MOM_SHOW_BACKTRACE("generated_strbuf_object"));
  pobown->unsync_update_arg(MOMP_start_generation, pobuf);
  MOM_DEBUGLOG(gencod, "generated_strbuf_object after start_generation pobown=" << MomShowObject(pobown)
               << " made pobuf=" << MomShowObject(pobuf));
#warning MomPaylGenfile::generated_strbuf_object incomplete
  MOM_WARNLOG("MomPaylGenfile::generated_strbuf_object incomplete pobown=" << MomShowObject(pobown)
              << " pobuf=" << MomShowObject(pobuf));
  return pobuf;
} // end of MomPaylGenfile::generated_strbuf_object



void
MomPaylGenfile::Destroy (struct MomPayload*payl,MomObject*own)
{
  auto py = static_cast<MomPaylGenfile*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "invalid genfile payload for own=" << own);
  delete py;
} // end MomPaylGenfile::Destroy

void
MomPaylGenfile::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "invalid genfile payload for own=" << own);
} // end MomPaylGenfile::Scangc

void
MomPaylGenfile::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Scandump own=" << own
               << " proxy=" << py->proxy());
} // end MomPaylGenfile::Scandump

void
MomPaylGenfile::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "invalid genfile payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << MomShowObject(own)
               << " proxy=" << MomShowObject(py->proxy()));
  empaylinit->out() << py->_pgenfile_pathstr << std::flush;
  auto pobuf = const_cast<MomPaylGenfile*>(py)->generated_strbuf_object();
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << MomShowObject(own)
               << " pobuf=" << MomShowObject(pobuf));
  MOM_ASSERT(pobuf != nullptr && pobuf->vkind() == MomKind::TagObjectK, "genfile emitdump bad pobuf");
  std::lock_guard<std::recursive_mutex> gu{pobuf->get_recursive_mutex()};
  auto pystrobuf = pobuf->unsync_runcast_payload<MomPaylStrobuf>(MOM_PAYLOADVTBL(strobuf));
  MOM_ASSERT(pystrobuf != nullptr, "genfile emitdump bad pystrobuf for pobuf=" << pobuf);
  pystrobuf->unsync_output_all_to_buffer(own);
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << MomShowObject(own)
               << " after unsync_output_all_to_buffer pobuf=" << MomShowObject(pobuf));
  auto tmpath = mom_dump_temporary_file_path(du, py->_pgenfile_pathstr);
  {
    std::ofstream out(tmpath);
    if (!out)
      MOM_WARNLOG("genfile emitdump own=" << MomShowObject(own) << " failed to open tmpath=" << tmpath << " (" << strerror(errno) << ")");
    auto bfstr = pystrobuf->buffer_string();
    out.write(bfstr.c_str(), bfstr.size());
    out.close();
  }
} // end MomPaylGenfile::Emitdump



MomPayload*
MomPaylGenfile::Initload(MomObject*own,MomLoader*,const char*inits)
{
  MOM_DEBUGLOG(load,"MomPaylGenfile::Initload own=" << own << " inits='" << inits << "'");
  auto py = own->unsync_make_payload<MomPaylGenfile>(inits);
  return py;
} // end MomPaylGenfile::Initload

void
MomPaylGenfile::Loadfill(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills)
{
  auto py = static_cast< MomPaylGenfile*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Loadfill invalid genfile payload for own=" << own);
  std::string fillstr{fills};
  MOM_DEBUGLOG(load,"MomPaylGenfile::Loadfill own=" << MomShowObject(own)
               << " fills=" << MomShowString(fillstr));
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Genfile fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  MOM_WARNLOG("incomplete MomPaylGenfile::Loadfill owner=" << MomShowObject(own));
#warning incomplete MomPaylGenfile::Loadfill
  if (fillpars.eof())
    return;
} // end MomPaylGenfile::Loadfill


MomValue
MomPaylGenfile::Getmagic (const struct MomPayload*payl, const MomObject*own, const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Getmagic invalid genfile payload for own=" << own);
  if (attrob == MOMP_proxy)
    {
      if (pgotit)
        *pgotit = true;
      return py->proxy();
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
#warning incomplete MomPaylGenfile::Getmagic
} // end MomPaylGenfile::Getmagic


MomValue
MomPaylGenfile::Fetch(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Fetch invalid genfile payload for own=" << own);
  MOM_ASSERT(pgotit, "MomPaylGenfile::Fetch no pgotit");
#warning incomplete MomPaylGenfile::Fetch
  *pgotit = false;
  return nullptr;
} // end MomPaylGenfile::Fetch


bool
MomPaylGenfile::Updated(struct MomPayload*payl,MomObject*targetob,const MomObject*attrob,
                        const MomValue*vecarr, unsigned veclen, int depth)
{
  auto py = static_cast< MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Updated invalid genfile payload for targetob=" << targetob);
  MOM_DEBUGLOG(gencod, "genfile update targetob=" << MomShowObject(targetob)
               << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob)));
  if (attrob == MOMP_emit)
    {
      if (veclen>0)
        MOM_FAILURE("genfile update emit got " << veclen << " arguments wanted none");
      auto pobuf = py->generated_strbuf_object();
      MOM_DEBUGLOG(gencod, "genfile update emit targetob=" << MomShowObject(targetob) << " pobuf=" << MomShowObject(pobuf));
      std::lock_guard<std::recursive_mutex> gu{pobuf->get_recursive_mutex()};
      auto pystrobuf = pobuf->unsync_runcast_payload<MomPaylStrobuf>(MOM_PAYLOADVTBL(strobuf));
      MOM_ASSERT(pystrobuf != nullptr, "genfile update emit bad pystrobuf for pobuf=" << pobuf);
      pystrobuf->unsync_output_all_to_buffer(targetob);
      std::string pathstr = py->_pgenfile_pathstr;
      {
        std::string tempathstr = pathstr + mom_random_temporary_file_suffix();
        MOM_DEBUGLOG(gencod, "genfile update emit tempathstr=" << tempathstr << " pobuf=" << pobuf);
        std::ofstream out(tempathstr);
        if (!out)
          MOM_WARNLOG("genfile update emit targetob=" << targetob << " failed to open tempathstr="
                      << tempathstr << " (" << strerror(errno) << ")");
        auto bfstr = pystrobuf->buffer_string();
        out.write(bfstr.c_str(), bfstr.size());
        out.close();
        bool same = mom_rename_file_if_changed(tempathstr, pathstr, true);
        if (same)
          MOM_INFORMLOG("genfile update emit targetob=" << MomShowObject(targetob) << " unchanged file " << pathstr);
        else
          MOM_INFORMLOG("genfile update emit targetob=" << MomShowObject(targetob) << " updated file " << pathstr);
      }
      return true;
    }
#warning incomplete MomPaylGenfile::Updated
  else
    return false;
} // end MomPaylGenfile::Updated



////////////////////////////////////////////////////////////////

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(envstack);

class MomPaylEnvstack: public MomPayload
{
public:
  typedef std::map<MomObject*,MomValue,MomObjptrLess> MomEnvBindMap;
  friend struct MomVtablePayload_st;
  friend class MomObject;
  struct MomEnv
  {
    unsigned env_depth;
    MomEnvBindMap env_map;
    MomValue env_val;
    MomEnv(unsigned d=0, MomValue v=nullptr) : env_depth(d), env_map{}, env_val(v) {};
    ~MomEnv() = default;
    MomEnv(const MomEnv&) = default;
    MomEnv(MomEnv&&) = default;
  };
private:
  std::vector<MomEnv> _penvstack_envs;
  MomPaylEnvstack(MomObject*own)
    : MomPayload(&MOM_PAYLOADVTBL(envstack), own),
      _penvstack_envs() {};
  ~MomPaylEnvstack()
  {
    _penvstack_envs.clear();
  };
public:
  static constexpr const bool FAIL_NO_ENV=false;
  static constexpr const bool IGNORE_NO_ENV=true;
  unsigned size() const
  {
    return _penvstack_envs.size();
  };
  void push_env()
  {
    _penvstack_envs.emplace_back(_penvstack_envs.size());
  }
  void pop_env(bool fail=IGNORE_NO_ENV);
  void last_env_set_value(MomValue val, bool fail=IGNORE_NO_ENV);
  void last_env_bind(MomObject*ob, MomValue val, bool fail=IGNORE_NO_ENV);
  ///
  void nth_env_bind(MomObject*ob, MomValue val, int rk, bool fail=IGNORE_NO_ENV);
  void nth_env_set_value(MomValue val, int rk,  bool fail=IGNORE_NO_ENV);
  MomValue var_bind(MomObject*varob, int*prk=nullptr) const;
  MomValue var_rebind(MomObject*varob, MomValue newval, int*prk=nullptr);
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
  static MomPyv_fetch_sig Fetch;
  static MomPyv_updated_sig Updated;
}; // end class MomPaylEnvstack


void
MomPaylStrobuf::unsync_output_all_to_buffer(MomObject*forpob)
{
  auto own = owner();
  MOM_DEBUGLOG(gencod,
               "MomPaylStrobuf::unsync_output_all_to_buffer start own=" << MomShowObject(own)
               << " forpob=" << MomShowObject(forpob)
               << " starter=" << MomShowObject(_pstrobuf_starter)
               << MOM_SHOW_BACKTRACE("unsync_output_all_to_buffer"));
  MomObject* ctxob = MomObject::make_object();
  ctxob->unsync_make_payload<MomPaylEnvstack>();
  MOM_DEBUGLOG(gencod,
               "MomPaylStrobuf::unsync_output_all_to_buffer start own=" << MomShowObject(own)
               << " ctxob=" << MomShowObject(ctxob));
  if (_pstrobuf_starter != nullptr)
    {
      std::lock_guard<std::recursive_mutex> gu{_pstrobuf_starter->get_recursive_mutex()};
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer before starter step own=" << own
                   << " forpob=" << forpob << " ctxob=" << ctxob);
      _pstrobuf_starter->unsync_step_arg(MomValue(own),MomValue(forpob),MomValue(ctxob));
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer after starter step own=" << own
                   << " forpob=" << forpob << " ctxob=" << ctxob);
    }
  unsigned sz= own-> unsync_nb_comps();
  MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer ctxob="<< ctxob
               << " owner=" << own);
  for (unsigned ix=0; ix<sz; ix++)
    {
      auto curcompv = own->unsync_unsafe_comp_at(ix);
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer own=" << own << " ix#" << ix
                   << " curcompv=" << curcompv);
      output_value_to_buffer(forpob, curcompv, ctxob);
      MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer own=" << own << " done ix#" << ix);
    }
  MOM_DEBUGLOG(gencod, "MomPaylStrobuf::unsync_output_all_to_buffer end own=" << own << " forpob=" << forpob);
} // end MomPaylStrobuf::unsync_output_all_to_buffer

const struct MomVtablePayload_st MOM_PAYLOADVTBL(envstack) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylEnvstack),
  /**   .pyv_name=       */       "envstack",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylEnvstack::Destroy,
  /**   .pyv_scangc=     */       MomPaylEnvstack::Scangc,
  /**   .pyv_scandump=   */       MomPaylEnvstack::Scandump,
  /**   .pyv_emitdump=   */       MomPaylEnvstack::Emitdump,
  /**   .pyv_initload=   */       MomPaylEnvstack::Initload,
  /**   .pyv_loadfill=   */       MomPaylEnvstack::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylEnvstack::Getmagic,
  /**   .pyv_fetch=      */       MomPaylEnvstack::Fetch,
  /**   .pyv_update=     */       MomPaylEnvstack::Updated,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_envstack(MOM_PAYLOADVTBL(envstack));

void
MomPaylEnvstack::pop_env(bool fail)
{
  if (_penvstack_envs.empty())
    {
      if (fail)
        MOM_FAILURE("MomPaylEnvstack::pop_env empty stack owner=" << owner());
      else
        return;
    }
  _penvstack_envs.pop_back();
} // end MomPaylEnvstack::pop_env

void
MomPaylEnvstack::last_env_set_value(MomValue val, bool fail)
{
  if (_penvstack_envs.empty())
    {
      if (fail)
        MOM_FAILURE("MomPaylEnvstack::last_env_set_value empty stack owner=" << owner());
      else
        return;
    };
  auto& lastenv = _penvstack_envs[size()-1];
  lastenv.env_val = val;
} // end MomPaylEnvstack::last_env_set_value


void
MomPaylEnvstack::last_env_bind(MomObject*ob, MomValue val, bool fail)
{
  if (_penvstack_envs.empty() || !ob || !val)
    {
      if (fail)
        MOM_FAILURE("MomPaylEnvstack::last_env_bind empty stack owner=" << owner()
                    << " for ob=" << ob << " val=" << val);
      else
        return;
    };
  auto& lastenv = _penvstack_envs[size()-1];
  lastenv.env_map[ob] = val;
} // end MomPaylEnvstack::last_env_bind

void
MomPaylEnvstack::nth_env_bind(MomObject*ob, MomValue val, int rk, bool fail)
{
  int sz = _penvstack_envs.size();
  int origrk = rk;
  if (rk<0) rk += sz;
  if (_penvstack_envs.empty() || rk<0 || rk>=sz || !ob || !val)
    {
      if (fail)
        MOM_FAILURE("MomPaylEnvstack::nth_env_bind bad stack owner=" << owner()
                    << " rk= " << origrk
                    << " for ob=" << ob << " val=" << val);
      else
        return;
    };
  auto& lastenv = _penvstack_envs[rk];
  lastenv.env_map[ob] = val;
} // end MomPaylEnvstack::nth_env_bind


void
MomPaylEnvstack::nth_env_set_value(MomValue val,int rk,  bool fail)
{
  int sz = _penvstack_envs.size();
  int origrk = rk;
  if (rk<0) rk += sz;
  if (_penvstack_envs.empty() || rk<0 || rk>=sz)
    {
      if (fail)
        MOM_FAILURE("MomPaylEnvstack::nth_env_set_value bad stack owner=" << owner()
                    << " rk= " << origrk);
      else
        return;
    };
  auto& lastenv = _penvstack_envs[rk];
  lastenv.env_val = val;
}

MomValue
MomPaylEnvstack::var_bind(MomObject*varob, int*prk) const
{
  if (!varob)
    {
      if (prk)
        *prk = -1;
      return nullptr;
    }
  int sz = _penvstack_envs.size();
  for (int ix= sz-1; ix>=0; ix--)
    {
      auto& curenv = _penvstack_envs[ix];
      auto it = curenv.env_map.find(varob);
      if (it != curenv.env_map.end())
        {
          if (prk)
            *prk = ix;
          return it->second;
        }
    }
  if (prk)
    *prk = -1;
  return nullptr;
} // end MomPaylEnvstack::var_bind

MomValue
MomPaylEnvstack::var_rebind(MomObject*varob, MomValue newval, int*prk)
{
  if (!varob)
    {
      if (prk)
        *prk = -1;
      return nullptr;
    }
  int sz = _penvstack_envs.size();
  for (int ix= sz-1; ix>=0; ix--)
    {
      auto& curenv = _penvstack_envs[ix];
      auto it = curenv.env_map.find(varob);
      if (it != curenv.env_map.end())
        {
          if (prk)
            *prk = ix;
          auto oldval = it->second;
          if (!newval)
            curenv.env_map.erase(it);
          else
            it->second = newval;
          return oldval;
        }
    }
  if (prk)
    *prk = -1;
  return nullptr;
} // end MomPaylEnvstack::var_rebind

////
#warning the MomPaylEnvstack:: functions are empty stubs
void
MomPaylEnvstack::Destroy(MomPayload*payl, MomObject*)
{
  auto py = static_cast<MomPaylEnvstack*>(payl);
  delete py;
} // end MomPaylEnvstack::Destroy


void
MomPaylEnvstack::Scangc(MomPayload const*payl, MomObject*own, MomGC*gc)
{
  auto py = static_cast<const MomPaylEnvstack*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(envstack),
             "invalid envstack payload for own=" << own);
  for (auto& env: py->_penvstack_envs)
    {
      gc->scan_value(env.env_val);
      for (auto p: env.env_map)
        {
          gc->scan_object(p.first);
          gc->scan_value(p.second);
        }
    };
} // end MomPaylEnvstack::Scangc


void
MomPaylEnvstack::Scandump(MomPayload const*payl, MomObject*own, MomDumper*du)
{
  auto py = static_cast<const MomPaylEnvstack*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylEnvstack::Scandump own=" << own
               << " proxy=" << py->proxy());
  for (auto& env: py->_penvstack_envs)
    {
      env.env_val.scan_dump(du);
      for (auto p: env.env_map)
        {
          MOM_ASSERT(p.first, "MomPaylEnvstack::Scandump corrupted env#" << env.env_depth);
          p.first->scan_dump(du);
          if (!mom_dump_is_dumpable_object(du,p.first))
            continue;
          p.second.scan_dump(du);
        }
    }
  MOM_DEBUGLOG(dump, "MomPaylEnvstack::Scandump end own=" << own);
} // end MomPaylEnvstack::Scandump


void
MomPaylEnvstack::Emitdump(MomPayload const*payl, MomObject*own, MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylEnvstack*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(envstack),
             "invalid envstack payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylEnvstack::Emitdump own=" << own
               << " proxy=" << py->proxy());
  empaylcont->out() << "@ENVSTACK: " <<  py->_penvstack_envs.size();
  for (auto& env: py->_penvstack_envs)
    {
      empaylcont->emit_newline(0);
      empaylcont->out() << "@ENVAL: ";
      empaylcont->emit_value(env.env_val);
      for (auto p: env.env_map)
        {
          if (!mom_dump_is_dumpable_object(du,p.first))
            continue;
          empaylcont->emit_newline(0);
          empaylcont->out() << "@*: ";
          empaylcont->emit_objptr(p.first);
          empaylcont->emit_space(0);
          empaylcont->emit_value(p.second);
        }
    }
  empaylcont->emit_newline(0);
} // end MomPaylEnvstack::Emitdump


MomPayload*
MomPaylEnvstack::Initload(MomObject*own, MomLoader*, char const*inits)
{
  MOM_DEBUGLOG(load,"MomPaylEnvstack::Initload own=" << own << " inits='" << inits << "'");
  auto py = own->unsync_make_payload<MomPaylEnvstack>();
  return py;
} // end MomPaylEnvstack::Initload

void
MomPaylEnvstack::Loadfill(MomPayload*payl, MomObject*own, MomLoader*ld, char const*fills)
{
  auto py = static_cast< MomPaylEnvstack*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(envstack),
             "MomPaylEnvstack::Loadfill invalid envstack payload for own=" << own);
  MOM_DEBUGLOG(load,"MomPaylEnvstack::Loadfill own=" << own
               << " fills::" << fills);
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Envstack fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  intptr_t sz = 0;
  bool gotsz = false;
  if (fillpars.got_cstring("@ENVSTACK:") && ((sz=fillpars.parse_int(&gotsz)),gotsz))
    {
      py->_penvstack_envs.reserve(sz+1);
    }
  int envcnt = 0;
  while (fillpars.skip_spaces(), fillpars.got_cstring("@ENVAL:"))
    {
      bool gotval = false;
      MomValue v = fillpars.parse_value(&gotval);
      if (!gotval)
        MOM_PARSE_FAILURE(&fillpars, "missing value for environment#" << envcnt
                          << " of envstack object " << own);
      py->push_env();
      py->last_env_set_value(v);
      while (fillpars.skip_spaces(), fillpars.got_cstring("@*:"))
        {
          bool gotobj = false;
          bool gotval = false;
          MomObject* pob = fillpars.parse_objptr(&gotobj);
          MomValue val = fillpars.parse_value(&gotval);
          if (!gotobj || !pob || !gotval)
            MOM_PARSE_FAILURE(&fillpars, "missing binding for environment#" << envcnt
                              << " of envstack object " << own);
          py->last_env_bind(pob,val);
        }
    }
} // end MomPaylEnvstack::Loadfill

MomValue
MomPaylEnvstack::Getmagic(const struct MomPayload*payl, const MomObject*own, const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylEnvstack*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(envstack),
             "MomPaylEnvstack::Getmagic invalid envstack payload for own=" << own);
  if (attrob == MOMP_proxy)
    {
      if (pgotit)
        *pgotit = true;
      return py->proxy();
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
} // end MomPaylEnvstack::Getmagic

#warning several MomPaylEnvstack::* routines unimplemented
MomValue
MomPaylEnvstack::Fetch(MomPayload const*payl, MomObject const*own, MomObject const*attrob, MomValue const*vecarr, unsigned int veclen, int depth, bool *pgotit)
{
} // end MomPaylEnvstack::Fetch

bool
MomPaylEnvstack::Updated(MomPayload*payl, MomObject*own, MomObject const*attrob, MomValue const*vecarr, unsigned int veclen, int depth)
{
#warning MomPaylEnvstack::Updated unimplemented
  return false;
} // end MomPaylEnvstack::Updated


////////////////////////////////////////////////////////////////



const struct MomVtablePayload_st MOM_PAYLOADVTBL(code) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylCode),
  /**   .pyv_name=       */       "code",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylCode::Destroy,
  /**   .pyv_scangc=     */       MomPaylCode::Scangc,
  /**   .pyv_scandump=   */       MomPaylCode::Scandump,
  /**   .pyv_emitdump=   */       MomPaylCode::Emitdump,
  /**   .pyv_initload=   */       MomPaylCode::Initload,
  /**   .pyv_loadfill=   */       MomPaylCode::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylCode::Getmagic,
  /**   .pyv_fetch=      */       MomPaylCode::Fetch,
  /**   .pyv_updated=    */       MomPaylCode::Updated,
  /**   .pyv_stepped=    */       MomPaylCode::Stepped,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_code(MOM_PAYLOADVTBL(code));

MomPaylCode::MomPaylCode(MomObject*own, MomLoader*, const std::string&bases, bool with_getmagic, bool with_fetch, bool with_update, bool with_step)
  : MomPayload(&MOM_PAYLOADVTBL(code), own),
    _pcode_basename(bases),
    _pcode_getmagic_rout(nullptr), _pcode_fetch_rout(nullptr), _pcode_updated_rout(nullptr),
    _pcode_stepped_rout(nullptr), _pcode_datavec()
{
  if (with_getmagic)
    {
      _pcode_getmagic_rout = (MomCod_Getmagic_sig*)get_symbol(this, bases, MOMCOD_SUFFIX_GETMAGIC);
      if (!_pcode_getmagic_rout)
        MOM_FATALOG("get_symbol failed for getmagic of " << own);
    }
  if (with_fetch)
    {
      _pcode_fetch_rout =  (MomCod_Fetch_sig*)get_symbol(this, bases, MOMCOD_SUFFIX_FETCH);
      if (!_pcode_fetch_rout)
        MOM_FATALOG("get_symbol failed for fetch of " << own);
    }
  if (with_update)
    {
      _pcode_updated_rout =  (MomCod_Updated_sig*)get_symbol(this, bases, MOMCOD_SUFFIX_UPDATED);
      if (!_pcode_updated_rout)
        MOM_FATALOG("get_symbol failed for update of " << own);
    }
  if (with_step)
    {
      _pcode_stepped_rout =  (MomPyv_stepped_sig*)get_symbol(this, bases, MOMCOD_SUFFIX_STEP);
      if (!_pcode_stepped_rout)
        MOM_FATALOG("get_symbol failed for stepped of " << own);
    }
} // end MomPaylCode::MomPaylCode for loading

MomPaylCode::~MomPaylCode()
{
  _pcode_getmagic_rout = nullptr;
  _pcode_updated_rout = nullptr;
  _pcode_stepped_rout = nullptr;
  _pcode_datavec.clear();
} // end MomPaylCode::~MomPaylCode

MomPaylCode::MomPaylCode(MomObject*own,  const std::string&bases, const std::string&mods)
  : MomPayload(&MOM_PAYLOADVTBL(code), own),
    _pcode_basename(bases),
    _pcode_getmagic_rout(nullptr), _pcode_fetch_rout(nullptr), _pcode_updated_rout(nullptr),
    _pcode_stepped_rout(nullptr), _pcode_datavec()
{
  _pcode_getmagic_rout = (MomCod_Getmagic_sig*)get_symbol(bases, MOMCOD_SUFFIX_GETMAGIC);
  _pcode_fetch_rout =  (MomCod_Fetch_sig*)get_symbol(bases, MOMCOD_SUFFIX_FETCH);
  _pcode_updated_rout =  (MomCod_Updated_sig*)get_symbol(bases, MOMCOD_SUFFIX_UPDATED);
  _pcode_stepped_rout =  (MomPyv_stepped_sig*)get_symbol(bases, MOMCOD_SUFFIX_STEP);
} // end  MomPaylCode::MomPaylCode for autodiscovering

MomPaylCode::MomPaylCode(MomObject*own, MomPaylCode*orig)
  : MomPayload(&MOM_PAYLOADVTBL(code), own),
    _pcode_basename(orig?orig->_pcode_basename:nullptr),
    _pcode_getmagic_rout(nullptr), _pcode_fetch_rout(nullptr), _pcode_updated_rout(nullptr),
    _pcode_stepped_rout(nullptr), _pcode_datavec()
{
  if (orig == nullptr || orig->_py_vtbl !=   &MOM_PAYLOADVTBL(code))
    MOM_FAILURE("bad origin for code owned by " << own);
  _pcode_getmagic_rout = orig->_pcode_getmagic_rout;
  _pcode_fetch_rout = orig->_pcode_fetch_rout;
  _pcode_updated_rout =  orig->_pcode_updated_rout;
  _pcode_stepped_rout =  orig->_pcode_stepped_rout;
} // end MomPaylCode::MomPaylCode copying from origin


void
MomPaylCode::Destroy (struct MomPayload*payl, MomObject*own)
{
  auto py = static_cast<MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "invalid code payload for own=" << own);
  delete py;
} // end MomPaylCode::Destroy

void
MomPaylCode::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "invalid code payload for own=" << own);
  for (auto v : py->_pcode_datavec)
    gc->scan_value(v);
} // end MomPaylCode::Scangc


void
MomPaylCode::Scandump(MomPayload const*payl, MomObject*own, MomDumper*du)
{
  auto py = static_cast<const MomPaylCode*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylCode::Scandump own=" << own
               << " proxy=" << py->proxy());
  if (py->_pcode_getmagic_rout)
    {
      own->scan_dump_module_for
      (du,
       reinterpret_cast<void*>(py->_pcode_getmagic_rout));
    };
  if (py->_pcode_fetch_rout)
    {
      own->scan_dump_module_for
      (du, reinterpret_cast<void*>(py->_pcode_fetch_rout));
    };
  if (py->_pcode_updated_rout)
    {
      own->scan_dump_module_for
      (du, reinterpret_cast<void*>(py->_pcode_updated_rout));
    };
  if (py->_pcode_stepped_rout)
    {
      own->scan_dump_module_for
      (du, reinterpret_cast<void*>(py->_pcode_stepped_rout));
    };

  for (auto v : py->_pcode_datavec)
    v.scan_dump(du);
} // end MomPaylCode::Scandump

std::mutex MomPaylCode::_pcode_modumtx_;
std::map<std::string,void*> MomPaylCode::_pcode_modudict_;

void*
MomPaylCode::get_symbol(MomPaylCode*py, const std::string& basename, const char*suffix)
{
  std::string fullnam{MOMCOD_PREFIX};
  fullnam += basename;
  fullnam += suffix;
  MomObject* own = py?py->owner():nullptr;
  MOM_DEBUGLOG(gencod, "paylcod::get_symbol fullnam="<< MomShowString(fullnam) << " own=" << MomShowObject(own));
  void* ad = dlsym(mom_prog_dlhandle, fullnam.c_str());
  if (!ad)
    MOM_FATALOG("paylcode dlsym " << fullnam.c_str() << " own=" << MomShowObject(own)
                << " failure " << dlerror());
  return ad;
} // end  MomPaylCode::get_symbol

void
MomPaylCode::Emitdump(MomPayload const*payl, MomObject*own, MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "invalid code payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylCode::Emitdump own=" << own
               << " proxy=" << py->proxy());
  empaylinit->out() << "@CODEBASE: " << py->_pcode_basename;
  if (py->_pcode_getmagic_rout)
    empaylinit->out() << " @CODEGETMAGIC!";
  if (py->_pcode_fetch_rout)
    empaylinit->out() << " @CODEFETCH!";
  if (py->_pcode_updated_rout)
    empaylinit->out() << " @CODEUPDATE!";
  if (py->_pcode_stepped_rout)
    empaylinit->out() << " @CODESTEP!";
  empaylinit->emit_newline(0);
  if (!py->_pcode_datavec.empty())
    {
      empaylcont->out() << "@CODEDATA " << py->_pcode_datavec.size() << " (";
      for (auto v : py->_pcode_datavec)
        {
          empaylcont->emit_space(1);
          empaylcont->emit_value(v,1);
        }
      empaylcont->emit_space(0);
      empaylcont->out() << ")";
      empaylcont->emit_newline(0);
    }
} // end MomPaylCode::Emitdump


MomPayload*
MomPaylCode::Initload(MomObject*own, MomLoader*ld, char const*inits)
{
  MOM_DEBUGLOG(load,"MomPaylCode::Initload own=" << own << " inits='" << inits << "'");
  bool with_getmagic = false;
  bool with_fetch = false;
  bool with_update = false;
  bool with_step = false;
  std::string initstr{inits};
  std::istringstream initfill(initstr);
  MomParser initpars(initfill);
  initpars.set_loader_for_object(ld, own, "Code init").set_make_from_id(true);
  initpars.next_line();
  initpars.skip_spaces();
  std::string modustr;
  std::string basestr;
  if (initpars.got_cstring("@CODEBASE:"))
    {
      bool gotbase = false;
      basestr = initpars.parse_name(&gotbase);
      if (!gotbase)
        MOM_PARSE_FAILURE(&initpars,
                          "missing base name for init of code object " << own
                          << " curbytes=" << MomShowString(initpars.curbytes()));
    }
  else
    MOM_PARSE_FAILURE(&initpars, "missing @CODEBASE: for init of code object " << own);
  initpars.skip_spaces();
  if (initpars.got_cstring("@CODEGETMAGIC!"))
    with_getmagic = true;
  initpars.skip_spaces();
  if (initpars.got_cstring("@CODEFETCH!"))
    with_fetch = true;
  initpars.skip_spaces();
  if (initpars.got_cstring("@CODEUPDATE!"))
    with_update = true;
  initpars.skip_spaces();
  if (initpars.got_cstring("@CODESTEP!"))
    with_step = true;
  initpars.skip_spaces();
  MOM_DEBUGLOG(gencod, "MomPaylCode::Initload own=" << MomShowObject(own)
               << " basestr=" << basestr
               << " with_getmagic=" << (with_getmagic?"yes":"no")
               << " with_fetch=" << (with_fetch?"yes":"no")
               << " with_update=" << (with_update?"yes":"no")
               << " with_step=" << (with_step?"yes":"no"));
  if (!with_getmagic && !with_fetch && !with_update && !with_step)
    MOM_WARNLOG("MomPaylCode::Initload empty own=" << MomShowObject(own)
		<< " basestr=" << basestr);
  auto py = own->unsync_make_payload<MomPaylCode>(ld, basestr,
            with_getmagic, with_fetch, with_update, with_step);
  return py;
} // end MomPaylCode::Initload


void
MomPaylCode::Loadfill(MomPayload*payl, MomObject*own, MomLoader*ld, char const*fills)
{
  auto py = static_cast< MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MomPaylCode::Loadfill invalid code payload for own=" << own);
  MOM_DEBUGLOG(load,"MomPaylCode::Loadfill own=" << own
               << " fills::" << fills);
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Code fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  if (!py->_pcode_getmagic_rout && !py->_pcode_fetch_rout && !py->_pcode_updated_rout && !py->_pcode_stepped_rout)
    MOM_WARNLOG("loaded paylcode owned by " << MomShowObject(own)
                << " has no routines");
  if (fillpars.got_cstring("@CODEDATA"))
    {
      bool gotsize = false;
      auto sz = fillpars.parse_int(&gotsize);
      if (!gotsize)
        MOM_PARSE_FAILURE(&fillpars, "missing size after @CODEDATA of code object " << own);
      py->_pcode_datavec.reserve(sz+1);
      if (!fillpars.got_cstring("("))
        MOM_PARSE_FAILURE(&fillpars, "missing leftparen after @CODEDATA of code object " << own);
      for (int ix=0; ix<sz; ix++)
        {
          bool gotval = false;
          MomValue v = fillpars.parse_value(&gotval);
          if (!gotval)
            MOM_PARSE_FAILURE(&fillpars, "missing value#" << ix << " for data of code object " << own);
          py->_pcode_datavec.push_back(v);
        }
      if (!fillpars.got_cstring(")"))
        MOM_PARSE_FAILURE(&fillpars, "missing rightparen after @CODEDATA of code object " << own);
    }
} // end MomPaylCode::Loadfill


MomValue
MomPaylCode::Getmagic(const struct MomPayload*payl, const MomObject*targetob, const MomObject*attrob, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylCode*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MomPaylCode::Getmagic invalid code payload for targetob=" << targetob);
  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK, "paylcode getmagic bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK, "paylcode getmagic bad attrob");
  MOM_ASSERT(pgotit, "paylcode getmagic bad pgotit");
  if (attrob == MOMP_proxy)
    {
      *pgotit = true;
      return py->proxy();
    }
  if (py->_pcode_getmagic_rout)
    {
      MomValue resval = nullptr;
      bool got = py->_pcode_getmagic_rout(&resval, payl, targetob, attrob);
      if (got)
        {
          *pgotit = true;
          return resval;
        }
    }
  if (pgotit)
    *pgotit = false;
  return nullptr;
} // end MomPaylCode::Getmagic


MomValue
MomPaylCode::Fetch(const struct MomPayload*payl, const MomObject*targetob, const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth, bool *pgotit)
{
  auto py = static_cast<const MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MomPaylCode::Fetch invalid code payload for targetob=" << targetob);
  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK, "paylcode fetch bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK, "paylcode fetch bad attrob");
  MOM_ASSERT(pgotit, "MomPaylCode::Fetch pgotit");
  if (py->_pcode_fetch_rout)
    {
      MomValue res = nullptr;
      bool fetched = py->_pcode_fetch_rout(&res, payl, targetob, attrob, vecarr, veclen);
      if (fetched)
        {
          *pgotit = true;
          return res;
        }
    }
  *pgotit = false;
  return nullptr;
} // end MomPaylCode::Fetch

bool
MomPaylCode::Updated(struct MomPayload*payl, MomObject*targetob, const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth)
{
  auto py = static_cast<MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MomPaylCode::Updated invalid code payload for targetob=" << targetob);
  if (vecarr==nullptr)
    veclen=0;
  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK, "paylcode updated bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK, "paylcode updated bad attrob");
  if (py->_pcode_updated_rout)
    {
      return py->_pcode_updated_rout(py, targetob, attrob, vecarr, veclen);
    }
  return false;
} // end MomPaylCode::Updated


bool
MomPaylCode::Stepped(struct MomPayload*payl, MomObject*targetob, const MomValue*vecarr, unsigned veclen, int depth)
{
  auto py = static_cast<MomPaylCode*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MomPaylCode::Step invalid code payload for targetob=" << targetob);
  if (py->_pcode_stepped_rout)
    return py->_pcode_stepped_rout(py, targetob, vecarr, veclen, depth);
  return false;
#warning incomplete MomPaylCode::Step
} // end MomPaylCode::Stepped
