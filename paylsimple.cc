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
  friend const std::string mom_get_unsync_string_name(MomObject*obj);
  friend void mom_forget_name(const char*name);
  friend MomObject*mom_unsync_named_object_proxy(MomObject*objn);
  friend void mom_unsync_named_object_set_proxy(MomObject*objn, MomObject*obproxy);
  typedef std::string stringty;
private:
  const stringty _nam_str;
  MomObject* _nam_proxy;
  static std::mutex _nam_mtx_;
  static std::map<std::string,MomObject*> _nam_dict_;
  MomPaylNamed(MomObject*own, const char*name)
    : MomPayload(&MOM_PAYLOADVTBL(named), own), _nam_str(name), _nam_proxy(nullptr) {};
  ~MomPaylNamed()
  {
    (const_cast<stringty*>(&_nam_str))->clear();
    _nam_proxy = nullptr;
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
  py->_nam_proxy = obproxy;
} // end mom_unsync_named_object_set_proxy

MomObject*
mom_unsync_named_object_proxy(MomObject*objn)
{
  if (!objn) return nullptr;
  auto py = static_cast<MomPaylNamed*>(objn->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return nullptr;
  return py->_nam_proxy;
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


const char*
mom_get_unsync_name(const MomObject*obj)
{
  auto py = static_cast<MomPaylNamed*>(obj->unsync_payload());
  if (!py || py-> _py_vtbl !=  &MOM_PAYLOADVTBL(named)) return nullptr;
  return py->_nam_str.c_str();
} // end mom_get_unsync_name


const std::string
mom_get_unsync_string_name(MomObject*obj)
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
  /**   .pyv_update=     */       nullptr,
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
  if (py->_nam_proxy)
    gc->scan_object(py->_nam_proxy);
} // end MomPaylNamed::Scangc


void
MomPaylNamed::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MOM_DEBUGLOG(dump, "PaylNamed::Scandump own=" << own << " name=" << py->_nam_str
               << " proxy=" << py->_nam_proxy);
  if (py->_nam_proxy)
    py->_nam_proxy->scan_dump(du);
} // end MomPaylNamed::Scandump


void
MomPaylNamed::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "invalid named payload for own=" << own);
  MOM_DEBUGLOG(dump, "PaylNamed::Emitdump own=" << own << " name=" << py->_nam_str
               << " proxy=" << py->_nam_proxy);
  mom_dump_named_update_defer(du, own, py->_nam_str);
  empaylinit->out() << py->_nam_str;
  empaylcont->out() << "@NAMEDPROXY: ";
  empaylcont->emit_objptr(py->_nam_proxy);
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
  if (fillpars.hasdelim("@NAMEDPROXY:"))
    {
      bool gotproxy = false;
      fillpars.skip_spaces();
      MomObject* pobproxy = fillpars.parse_objptr(&gotproxy);
      if (!gotproxy)
        MOM_PARSE_FAILURE(&fillpars, "missing proxy for fill of named object " << own);
      py->_nam_proxy = pobproxy;
    }
  else
    MOM_PARSE_FAILURE(&fillpars, "missing @NAMEDPROXY: for fill of named object " << own);
} // end MomPaylNamed::Loadfill


MomValue
MomPaylNamed::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  MomObject*proxob = nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(named),
             "MomPaylNamed::Getmagic invalid named payload for own=" << own);
  if (attrob == MOMP_name)
    return MomString::make_from_string(py->_nam_str);
  else if (attrob == MOMP_proxy)
    return py->_nam_proxy;
#warning perhaps we need to have some lazy pseudo-value. What about cycles of proxys and locking...
  else if ((proxob=py->_nam_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
      return proxob->unsync_get_magic_attr(attrob);
    }
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
  MomObject* _pset_proxy;
  MomPaylSet(MomObject*own)
    : MomPayload(&MOM_PAYLOADVTBL(set), own), _pset_set(), _pset_proxy(nullptr) {};
  ~MomPaylSet()
  {
    _pset_set.clear();
    _pset_proxy = nullptr;
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
  static MomPyv_update_sig Update;
}; // end class MomPaylSet



MomObject*
mom_unsync_pset_object_proxy(MomObject*objs)
{
  if (!objs) return nullptr;
  auto py = static_cast<MomPaylSet*>(objs->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(set)) return nullptr;
  return py->_pset_proxy;
} // end mom_unsync_pset_object_proxy

void mom_unsync_pset_object_set_proxy(MomObject*objn, MomObject*obproxy)
{
  if (!objn) return;
  auto py = static_cast<MomPaylSet*>(objn->unsync_payload());
  if (py-> _py_vtbl !=  &MOM_PAYLOADVTBL(set)) return;
  py->_pset_proxy = obproxy;
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
  /**   .pyv_update=     */       MomPaylSet::Update,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_set(MOM_PAYLOADVTBL(set));

void
MomPaylSet::Destroy (struct MomPayload*payl,MomObject*own)
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
  if (py->_pset_proxy)
    gc->scan_object(py->_pset_proxy);
  for (const MomObject* pob : py->_pset_set)
    gc->scan_object(const_cast<MomObject*>(pob));
} // end MomPaylSet::Scangc

void
MomPaylSet::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylSet::Scandump own=" << own
               << " proxy=" << py->_pset_proxy);
  if (py->_pset_proxy)
    py->_pset_proxy->scan_dump(du);
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
               << " proxy=" << py->_pset_proxy);
  empaylcont->out() << "@SET: ";
  for (const MomObject* pob : py->_pset_set)
    {
      if (!mom_dump_is_dumpable_object(du, const_cast<MomObject*>(pob)))
        continue;
      empaylcont->emit_space(1);
      empaylcont->emit_objptr(pob);
    }
  if (py->_pset_proxy)
    {
      empaylcont->emit_newline(0);
      empaylcont->out() << "@SETPROXY: ";
      empaylcont->emit_objptr(py->_pset_proxy);
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
               << " fills='" << fills << "'");
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Set fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
  if (fillpars.hasdelim("@SET:"))
    {
      MomObject* pob = nullptr;
      bool gotpob = false;
      while ((pob = fillpars.parse_objptr(&gotpob)), gotpob)
        {
          if (pob)
            py->_pset_set.insert(pob);
        }
    };
  if (fillpars.hasdelim("@SETPROXY:"))
    {
      bool gotpob = false;
      MomObject* pob =fillpars.parse_objptr(&gotpob);
      if (pob)
        py->_pset_proxy = pob;
    }
} // end MomPaylSet::Loadfill




MomValue
MomPaylSet::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Getmagic invalid set payload for own=" << own);
  if (attrob == MOMP_size)
    return MomValue{(intptr_t)(py->_pset_set.size())};
  else if (attrob == MOMP_set)
    return MomSet::make_from_objptr_set(py->_pset_set);
  else if (attrob == MOMP_proxy)
    return py->_pset_proxy;
  else if ((proxob=py->_pset_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
      return proxob->unsync_get_magic_attr(attrob);
    }
  return nullptr;
} // end   MomPaylSet::Getmagic


MomValue
MomPaylSet::Fetch(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<const MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Fetch invalid set payload for own=" << own);
  if (attrob == MOMP_get)
    {
      auto elemval = vecarr[0].to_val();
      MomObject*elemob =
        elemval
        ?const_cast<MomObject*>(elemval->to_object())
        :nullptr;
      if (elemob && py->_pset_set.find(elemob) != py->_pset_set.end())
        return elemob;
      else return nullptr;
    };
  if ((proxob=py->_pset_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning should define some unsync_fetch....
    }
  return nullptr;
} // end MomPaylSet::Fetch



void
MomPaylSet::Update(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast< MomPaylSet*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(set),
             "MomPaylSet::Update invalid set payload for own=" << own);
  if (attrob == MOMP_put)
    {
      for (unsigned ix=0; ix<veclen; ix++)
        {
          auto elemval = vecarr[ix].to_val();
          auto elemob = elemval?const_cast<MomObject*>(elemval->to_object()):nullptr;
          if (elemob)
            py->_pset_set.insert(elemob);
        };
    }
  if ((proxob=py->_pset_proxy) != nullptr)
    {
      std::unique_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning should define some unsync_update....
    }
} // end MomPaylSet::Update

////////////////////////////////////////////////////////////////

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(strobuf);

class MomPaylStrobuf: public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
private:
  std::ostringstream _pstrobuf_out;
  MomObject* _pstrobuf_proxy;
  MomPaylStrobuf(MomObject*own)
    : MomPayload(&MOM_PAYLOADVTBL(strobuf), own), _pstrobuf_out(), _pstrobuf_proxy(nullptr) {};
  ~MomPaylStrobuf()
  {
    _pstrobuf_proxy = nullptr;
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
  static MomPyv_update_sig Update;
  void output_value(const MomValue v, int depth=0);
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
  /**   .pyv_update=     */       MomPaylStrobuf::Update,
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
  if (py->_pstrobuf_proxy)
    gc->scan_object(py->_pstrobuf_proxy);
} // end MomPaylStrobuf::Scangc

void
MomPaylStrobuf::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylStrobuf::Scandump own=" << own
               << " proxy=" << py->_pstrobuf_proxy);
  if (py->_pstrobuf_proxy)
    py->_pstrobuf_proxy->scan_dump(du);
} // end MomPaylStrobuf::Scandump

void
MomPaylStrobuf::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "invalid strobuf payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylStrobuf::Emitdump own=" << own
               << " proxy=" << py->_pstrobuf_proxy);
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
  if (py->_pstrobuf_proxy)
    {
      empaylcont->emit_newline(0);
      empaylcont->out() << "@STROBUFPROXY: ";
      empaylcont->emit_objptr(py->_pstrobuf_proxy);
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
  if (fillpars.hasdelim("@STROBUFSTR:"))
    {
      bool gotstr = false;
      std::string linstr;
      while ((gotstr=false), (linstr=fillpars.parse_string(&gotstr)), gotstr)
        {
          py->_pstrobuf_out.write(linstr.c_str(), linstr.size());
          linstr.erase();
        }
    }
  if (fillpars.hasdelim("@STROBUFPROXY:"))
    {
      bool gotpob = false;
      MomObject* pob =fillpars.parse_objptr(&gotpob);
      if (pob)
        py->_pstrobuf_proxy = pob;
    }
} // end MomPaylStrobuf::Loadfill




MomValue
MomPaylStrobuf::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Getmagic invalid strobuf payload for own=" << own);
  if (attrob == MOMP_size)
    return MomValue{(intptr_t)(const_cast<MomPaylStrobuf*>(py)->_pstrobuf_out.tellp())};
  else if (attrob == MOMP_proxy)
    return py->_pstrobuf_proxy;
  else if ((proxob=py->_pstrobuf_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
      return proxob->unsync_get_magic_attr(attrob);
    }
  return nullptr;
} // end   MomPaylStrobuf::Getmagic


MomValue
MomPaylStrobuf::Fetch(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<const MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Fetch invalid strobuf payload for own=" << own);
  if (attrob == MOMP_get)
    {
    };
  if ((proxob=py->_pstrobuf_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning should define some unsync_fetch....
    }
  return nullptr;
} // end MomPaylStrobuf::Fetch



void
MomPaylStrobuf::Update(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast< MomPaylStrobuf*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(strobuf),
             "MomPaylStrobuf::Update invalid strobuf payload for own=" << own);
  if ((proxob=py->_pstrobuf_proxy) != nullptr)
    {
      std::unique_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning should define some unsync_update....
    }
} // end MomPaylStrobuf::Update



void
MomPaylStrobuf::output_value(const MomValue v, int depth)
{
#warning MomPaylStrobuf::output_value unimplemented
  MOM_FATALOG("MomPaylStrobuf::output_value unimplemented owner=" << owner()
              << " v=" << v);
} // end of MomPaylStrobuf::output_value


////////////////////////////////////////////////////////////////

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(genfile);

class MomPaylGenfile: public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
private:
  const std::string _pgenfile_pathstr;
  MomObject* _pgenfile_proxy;
  MomPaylGenfile(MomObject*own, const char*pathstr)
    : MomPayload(&MOM_PAYLOADVTBL(genfile), own),
      _pgenfile_pathstr(pathstr), _pgenfile_proxy(nullptr) {};
  ~MomPaylGenfile()
  {
    _pgenfile_proxy = nullptr;
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
  static MomPyv_update_sig Update;
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
  /**   .pyv_update=     */       MomPaylGenfile::Update,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

MomRegisterPayload mompy_genfile(MOM_PAYLOADVTBL(genfile));

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
  if (py->_pgenfile_proxy)
    gc->scan_object(py->_pgenfile_proxy);
#warning incomplete MomPaylGenfile::Scangc
} // end MomPaylGenfile::Scangc

void
MomPaylGenfile::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Scandump own=" << own
               << " proxy=" << py->_pgenfile_proxy);
  if (py->_pgenfile_proxy)
    py->_pgenfile_proxy->scan_dump(du);
#warning incomplete MomPaylGenfile::Scandump
} // end MomPaylGenfile::Scandump

void
MomPaylGenfile::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "invalid genfile payload for own=" << own);
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << own
               << " proxy=" << py->_pgenfile_proxy);
  empaylinit->out() << py->_pgenfile_pathstr << std::flush;
  auto pobuf = MomObject::make_object();
  auto pystrobuf = pobuf->unsync_make_payload<MomPaylStrobuf>();
  MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << own
               << " pobuf=" << pobuf);
  unsigned sz= own-> unsync_nb_comps();
  for (unsigned ix=0; ix<sz; ix++)
    {
      auto curcompv = own->unsync_unsafe_comp_at(ix);
      MOM_DEBUGLOG(dump, "MomPaylGenfile::Emitdump own=" << own << " ix#" << ix
                   << " curcompv=" << curcompv);
      pystrobuf->output_value(curcompv);
    }
  auto tmpath = mom_dump_temporary_file_path(du, py->_pgenfile_pathstr);
  {
    std::ofstream out(tmpath);
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
  MOM_DEBUGLOG(load,"MomPaylGenfile::Loadfill own=" << own
               << " fills='" << fills << "'");
  std::string fillstr{fills};
  std::istringstream infill(fillstr);
  MomParser fillpars(infill);
  fillpars.set_loader_for_object(ld, own, "Genfile fill").set_make_from_id(true);
  fillpars.next_line();
  fillpars.skip_spaces();
#warning incomplete MomPaylGenfile::Loadfill
} // end MomPaylGenfile::Loadfill


MomValue
MomPaylGenfile::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Getmagic invalid genfile payload for own=" << own);
#warning incomplete MomPaylGenfile::Getmagic
} // end MomPaylGenfile::Getmagic


MomValue
MomPaylGenfile::Fetch(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<const MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Fetch invalid genfile payload for own=" << own);
  if (attrob == MOMP_get)
    {
    };
  if ((proxob=py->_pgenfile_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning incomplete MomPaylGenfile::Fetch
    }
  return nullptr;
} // end MomPaylGenfile::Fetch

void
MomPaylGenfile::Update(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast< MomPaylGenfile*>(payl);
  MomObject*proxob=nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(genfile),
             "MomPaylGenfile::Update invalid genfile payload for own=" << own);
  if ((proxob=py->_pgenfile_proxy) != nullptr)
    {
      std::unique_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
#warning incomplete MomPaylGenfile::Update
    }
} // end MomPaylGenfile::Update


