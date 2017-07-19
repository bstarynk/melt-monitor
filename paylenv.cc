// file paylenv.cc - environment payload

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


MomValue
MomPaylEnvstack::env_eval(const MomValue exprv, int depth)
{
  check_depth_and_limits(depth);
  auto expk = exprv.kind();
  switch (expk)
    {
    case MomKind::TagNoneK:
    case MomKind::TagIntK:
    case MomKind::TagStringK:
    case MomKind::TagIntSqK:
    case MomKind::TagDoubleSqK:
    case MomKind::TagSetK:
    case MomKind::TagTupleK:
      return exprv;
    case MomKind::TagObjectK:
      return env_eval_object(const_cast<MomObject*>(exprv.as_val()->as_object()), depth);
    case MomKind::TagNodeK:
      return env_eval_node(exprv.as_val()->as_node(), depth);
    case MomKind::Tag_LastK:
      MOM_FATAPRINTF("env_eval invalid expr");
    }
} // end of MomPaylEnvstack::env_eval


MomValue
MomPaylEnvstack::env_eval_object(MomObject*obj, int depth)
{
  MOM_ASSERT(obj != nullptr && obj->vkind() == MomKind::TagObjectK,
             "env_eval_object corrupted obj");
  check_depth_and_limits(depth);
  if (obj == MOMP_undefined)
    return nullptr;
  int ln = -1;
  MomValue bindv = var_bind(obj, &ln);
  if (bindv || ln>=0) return  bindv;
  return MomValue(obj);
} // end MomPaylEnvstack::env_eval_object

void
MomPaylEnvstack::env_collect_variables(MomObjptrSet& setvar, const MomValue exprv, int depth)
{
  check_depth_and_limits(depth);
  auto expk = exprv.kind();
  if (expk == MomKind::TagObjectK)
    {
      auto pob = const_cast<MomObject*>(exprv.as_val()->as_object());
      if (pob == MOMP_undefined)
        return;
      int ln = -1;
      MomValue bindv = var_bind(pob, &ln);
      if (bindv || ln>=0)
        setvar.insert(pob);
    }
  else if (expk == MomKind::TagNodeK)
    {
      auto nod = exprv.as_val()->as_node();
      env_collect_node_variables(setvar, nod, depth);
    }
} // end env_collect_variables

void
MomPaylEnvstack::env_collect_node_variables(MomObjptrSet& setvar, const MomNode*nod, int depth)
{
  check_depth_and_limits(depth);
} // end env_collect_node_variables


MomValue
MomPaylEnvstack::env_eval_node(const MomNode*nod, int depth)
{
  MOM_ASSERT(nod != nullptr && nod->vkind() == MomKind::TagNodeK,
             "env_eval_node corrupted nod");
  check_depth_and_limits(depth);
} // end MomPaylEnvstack::env_eval_node
