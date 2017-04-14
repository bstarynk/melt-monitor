// file parsemit.cc - our parser and emitter

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

MomValue
MomParser::parse_value(bool *pgotval)
{
  auto inioff = _parlinoffset;
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  int pc = 0;
  int nc = 0;
again:
  if (eol())
    {
      skip_spaces();
      if (eol() && !_parinp) goto failure;
      goto again;
    }
  pc = peekbyte(0);
  nc = peekbyte(1);
  if (pc>0 && (((pc=='+' || pc=='-') && isdigit(nc)) || isdigit(pc)))
    {
      const char*curp = peekchars();
      char*endp = nullptr;
      long long ll = strtoll(curp, &endp, 0);
      if (endp>curp)
        consume(endp-curp);
      if (pgotval) *pgotval = true;
      return MomValue((intptr_t)ll);
    }
  else if (isspace(pc))
    {
      skip_spaces();
      goto again;
    }
  else if (pc=='/' && nc == '/')
    {
      next_line();
      goto again;
    }
  else if (pc=='_' && nc=='_')
    {
      consume(2);
      return MomValue{nullptr};
    }
  else if (pc=='(' && nc=='#')	// (# integer sequence #)
    {
      std::vector<intptr_t> v;
      for (;;)
        {
          skip_spaces();
          pc = peekbyte(0);
          nc = peekbyte(1);
          if (pc==EOF)
            {
              goto failure;
            }
          else if (pc=='#' && nc==')')
            {
              consume(2);
              break;
            }
          else if ((pc<127 && isdigit(pc)) || (nc<127 && isdigit(nc) && (pc=='+' || pc=='-')))
            {
              const char*curp = peekchars();
              char*endp = nullptr;
              long long ll = strtoll(curp, &endp, 0);
              consume(endp-curp);
              v.push_back(ll);
            }
          else
            MOM_PARSE_FAILURE(this, "invalid integer sequence");
        }
      if (pgotval) *pgotval = true;
      return MomValue(MomIntSq::make_from_vector(v));
    }
  else if (pc=='(' && nc==':')	// (: double sequence :)
    {
      std::vector<double> v;
      for (;;)
        {
          skip_spaces();
          pc = peekbyte(0);
          nc = peekbyte(1);
          if (pc==EOF)
            {
              goto failure;
            }
          else if (pc==':' && nc==')')
            {
              consume(2);
              break;
            }
          else if ((pc<127 && isdigit(pc)) || (nc<127 && isdigit(nc) && (pc=='+' || pc=='-')))
            {
              const char*curp = peekchars();
              char*endp = nullptr;
              double x = strtod(curp, &endp);
              consume(endp-curp);
              v.push_back(x);
            }
          else
            MOM_PARSE_FAILURE(this, "invalid double sequence");
        }
      if (pgotval) *pgotval = true;
      return MomValue(MomDoubleSq::make_from_vector(v));
    }
  else if (pc=='[') // tuple
    {
      consume(1);
      MomObjptrVector vec;
      for (;;)
        {
          bool gotcurobj = false;
          MomObject* curob = nullptr;
          skip_spaces();
          pc = peekbyte(0);
          if (pc == '}')
            {
              consume(1);
              break;
            }
          curob = parse_objptr(&gotcurobj);
          if (!curob || !gotcurobj)
            MOM_PARSE_FAILURE(this, "missing component object in vector");
        }
      if (pgotval)
        *pgotval = true;
      return MomValue(MomTuple::make_from_objptr_vector(vec));
    }
  else if (pc=='{') // set
    {
      MomObjptrSet set;
      consume(1);
      for (;;)
        {
          bool gotcurobj = false;
          skip_spaces();
          MomObject* curob = nullptr;
          pc = peekbyte(0);
          if (pc == '}')
            {
              consume(1);
              break;
            }
          curob = parse_objptr(&gotcurobj);
          if (!curob || !gotcurobj)
            MOM_PARSE_FAILURE(this, "missing element object in set");
          set.insert(curob);
        }
      if (pgotval)
        *pgotval = true;
      return MomValue(MomSet::make_from_objptr_set(set));
    }
  else if (pc=='*' && nc<127 && !(nc>0 && ispunct(nc))) // node
    {
      MomObject* connob = nullptr;
      bool gotconn = false;
      std::vector<MomValue> sonvec;
      consume(1);
      skip_spaces();
      connob = parse_objptr(&gotconn);
      if (!connob || !gotconn)
        MOM_PARSE_FAILURE(this, "missing connective object in node");
      skip_spaces();
      pc = peekbyte(0);
      if (pc != '(')
        MOM_PARSE_FAILURE(this, "missing left parenthesis in node of " << connob);
      consume(1);
      for (;;)
        {
          bool gotcurval = false;
          MomValue curval{nullptr};
          skip_spaces();
          pc = peekbyte(0);
          if (pc == ')')
            {
              consume(1);
              break;
            }
          curval = parse_value(&gotcurval);
          if (!curval || !gotcurval)
            MOM_PARSE_FAILURE(this, "missing son#" << sonvec.size() << " in node of " << connob);
          sonvec.push_back(curval);
        }
      if (pgotval)
        *pgotval = true;
      return MomValue(MomNode::make_from_vector(connob,sonvec));
    }
  else if (pc=="°"[0] && nc=="°"[1])
    {
      static_assert(sizeof("°")==3, "wrong length for °");
      consume(2);
      bool gotvval = false;
      auto vv = MomParser::parse_value(&gotvval);
      if (gotvval)
        {
          if (pgotval)
            *pgotval = true;
          if (vv.is_val())
            return MomValue(vv.to_val(),MomTransientTag{});
          else return vv;
        }
      else
        goto failure;
    }
failure:
  if (pgotval)
    *pgotval = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end of MomParser::parse_value


MomObject*
MomParser::parse_objptr(bool *pgotob)
{
  MomObject*respob = nullptr;
  auto inioff = _parlinoffset;
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  int pc = 0;
  int nc = 0;
again:
  if (eol())
    {
      skip_spaces();
      if (eol() && !_parinp) goto failure;
      goto again;
    }
  pc = peekbyte(0);
  nc = peekbyte(1);
  if (pc < 127 && isspace(pc))
    {
      skip_spaces();
      goto again;
    }
  else if (pc=='_' && nc<127 && isdigit(nc))
    {
      const char* endp = nullptr;
      const char*curp = peekchars();
      auto id = MomIdent::make_from_cstr(curp,&endp);
      if (id.is_null()) goto failure;
      if (_parmakefromid)
        respob = MomObject::make_object_of_id(id);
      else
        respob = MomObject::find_object_of_id(id);
      if (respob)
        {
          consume(endp-curp);
          if (pgotob)
            *pgotob = true;
          return respob;
        }
      else goto failure;
    }
  else if (pc=='_' && nc=='_')
    {
      consume(2);
      if (pgotob)
        *pgotob = true;
      return nullptr;
    }
failure:
  if (pgotob)
    *pgotob = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end of MomParser::parse_value



void
MomEmitter::emit_space()
{
  MOM_FATAPRINTF("unimplemented MomEmitter::emit_space");
#warning unimplemented MomEmitter::emit_space
} // end MomEmitter::emit_space

void
MomEmitter::emit_value(const MomValue v, int depth)
{
  if (!v)
    {
      _emout << "__";
      return;
    }
  else if (v.is_tagint())
    {
      intptr_t i = v.to_tagint();
      _emout << i;
      return;
    }
  else if (v.is_transient())
    {
      _emout << "°";
      emit_value(MomValue{v.to_transient()}, depth);
    }
  else if (v.is_val())
    {
      auto vv = v.as_val();
      switch (vv->kindw())
        {
        case MomKind::TagIntSqK:
        {
          _emout << "(#";
#warning should emit integer sequence content
          _emout << "#)";
        }
        break;
        case MomKind::TagDoubleSqK:
        {
          _emout << "(:";
#warning should emit double sequence content
          _emout << ":)";
        }
        break;
        case MomKind::TagStringK:
        {
        }
        break;
        case MomKind::TagSetK:
        {
        }
        break;
        case MomKind::TagTupleK:
        {
        }
        break;
        case MomKind::TagNodeK:
        {
          _emout << "*";
#warning should emit node content
        }
        break;
        case MomKind::TagObjectK:
        {
        }
        break;
        case MomKind::TagIntK:
        case MomKind::TagNoneK:
        case MomKind::Tag_LastK:
          MOM_FATAPRINTF("impossible tag #%d for vv@%p", (int) vv->kindw(), vv);
        }
    }
} // end of MomEmitter::emit_value
