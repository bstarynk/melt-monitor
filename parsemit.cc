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
#define MOM_BORDER_SIZE 32
#define MOM_BORDER_FORMAT "%30[A-Za-z0-9_]"
  char border[MOM_BORDER_SIZE];
  constexpr unsigned bordersize = sizeof(border);
  int borderpos = 0;
  memset (border, 0, bordersize);
again:
  border[0] = border[1] = (char)0;
  skip_spaces();
  inioff = _parlinoffset;
  inicol = _parcol;
  inilincnt = _parlincount;
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
  else if (pc<127 && (isalpha(pc) || (pc=='_' && nc<127 && isdigit(nc))))
    {
      return MomValue(parse_objptr(pgotval));
    }
  else if (pc=='[') // tuple
    {
      consume(1);
      MomObjptrVector vec;
      int cnt=0;
      for (;;)
        {
          bool gotcurobj = false;
          MomObject* curob = nullptr;
          skip_spaces();
          pc = peekbyte(0);
          if (pc == ']')
            {
              consume(1);
              break;
            }
          curob = parse_objptr(&gotcurobj);
          if (!curob || !gotcurobj)
            MOM_PARSE_FAILURE(this, "missing component object in vector");
          vec.push_back(curob);
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      MOM_ASSERT(cnt == (int)vec.size(), "parsing tuple expecting " << cnt << " got " << vec.size());
      return MomValue(MomTuple::make_from_objptr_vector(vec));
    }
  else if (pc=='{') // set
    {
      MomObjptrSet set;
      int cnt = 0;
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
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      MOM_ASSERT(cnt >= (int)set.size(), "parsing set expecting at most " << cnt << " got " << set.size());
      return MomValue(MomSet::make_from_objptr_set(set));
    }
  else if (pc=='"')   // JSON encoded UTF8 string, on the same line
    {
      consume(1);
      std::string restinline{peekchars()};
      std::istringstream ins{restinline};
      auto str = mom_input_quoted_utf8_string(ins);
      consume(ins.tellg());
      pc = peekbyte(0);
      if (pc != '"')
        MOM_PARSE_FAILURE(this, "expecting doublequote ending string, but got " << (char)pc);
      consume(1);
      if (pgotval)
        *pgotval = true;
      return MomValue(MomString::make_from_string(str));
    }
  // multi-line raw strings like `ab0|foobar\here|ab0`
  else if (pc=='`' && (memset(border, 0, sizeof(border)), (borderpos=0), (nc=='_' || isalnum(nc)))
           && sscanf(peekchars(0), "`" MOM_BORDER_FORMAT "|%n",
                     border, &borderpos) >= 1 && borderpos>0 && border[0]>0)   // raw strings
    {
      /* the raw string may take many lines, it is ending with the |BORDER` */
      std::string str;
      consume(borderpos);
      for (;;)
        {
          if (eol())
            {
              if (!_parinp) goto failure;
              next_line();
              continue;
            }
          pc = peekbyte(0);
          if (pc == '|')
            {
              nc = peekbyte(1);
              if (nc == border[0] && haskeyword(border, 1) && peekbyte(2+borderpos)=='`')
                {
                  consume(2+borderpos);
                  break;
                }
            }
          str.push_back(pc);
        }
      if (pgotval)
        *pgotval = true;
      return MomValue(MomString::make_from_string(str));
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
  else if (pc=='*' /* && nc<127 && !(nc>0 && nc!='_' && ispunct(nc))*/) // node
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
  else if (isalpha(pc))
    {
      const char*begnamp = peekchars();
      const char*endnamp = begnamp;
      while (isalnum(*endnamp) || (*endnamp == '_' && isalnum(endnamp[-1])))
        endnamp++;
      std::string namstr(begnamp, endnamp-begnamp);
      MomObject* ob = fetch_named_object(namstr);
      if (ob)
        {
          consume(endnamp-begnamp);
          if (pgotob)
            *pgotob = true;
          return ob;
        }
      else goto failure;
    }
failure:
  if (pgotob)
    *pgotob = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end of MomParser::parse_value




void
MomEmitter::emit_raw_newline()
{
  _emout << std::endl;
  _emlastnewline = _emout.tellp();
}// end MomEmitter::emit_raw_newline


void
MomEmitter::emit_newline(int depth)
{
  emit_raw_newline();
  if (depth>0)
    for (int ix=depth % _max_indent; ix>0; ix--)
      _emout << ' ';
} // end MomEmitter::emit_newline

bool
MomEmitter::skippable_object(const MomObject*pob) const
{
  return pob==nullptr;
}

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
      if (_emnotransient)
        return;
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
          auto isv = reinterpret_cast<const MomIntSq*>(vv);
          unsigned sz = isv->sizew();
          emit_maybe_newline(depth);
          _emout << "(#";
          for (unsigned ix=0; ix<sz; ix++)
            {
              if (ix>0) emit_space(depth+1);
              out() << isv->unsafe_at(ix);
            }
          _emout << "#)";
        }
        break;
        case MomKind::TagDoubleSqK:
        {
          auto dsv = reinterpret_cast<const MomDoubleSq*>(vv);
          unsigned sz = dsv->sizew();
          emit_maybe_newline(depth);
          _emout << "(:";
          for (unsigned ix=0; ix<sz; ix++)
            {
              if (ix>0) emit_space(depth+1);
              out() << dsv->unsafe_at(ix);
            }
          _emout << ":)";
        }
        break;
        case MomKind::TagStringK:
        {
          auto strv = reinterpret_cast<const MomString*>(vv);
          unsigned sz = strv->sizew();
          emit_maybe_newline(depth);
          emit_string_value(strv, depth, sz>(unsigned)_emlinewidth);
        }
        break;
        case MomKind::TagSetK:
        {
          auto setv = reinterpret_cast<const MomSet*>(vv);
          unsigned sz = setv->sizew();
          emit_maybe_newline(depth);
          _emout << "{";
          int cnt=0;
          for (unsigned ix=0; ix<sz; ix++)
            {
              auto elemob = setv->unsafe_at(ix);
              if (skippable_object(elemob)) continue;
              if (cnt>0) emit_space(depth+1);
              cnt++;
              emit_objptr(elemob, depth+1);
            }
          _emout << "}";
        }
        break;
        case MomKind::TagTupleK:
        {
          auto tupv = reinterpret_cast<const MomTuple*>(vv);
          unsigned sz = tupv->sizew();
          emit_maybe_newline(depth);
          _emout << "[";
          int cnt=0;
          for (unsigned ix=0; ix<sz; ix++)
            {
              auto compob = tupv->unsafe_at(ix);
              if (skippable_object(compob)) continue;
              if (cnt>0) emit_space(depth+1);
              cnt++;
              emit_objptr(compob, depth+1);
            }
          _emout << "]";
        }
        break;
        case MomKind::TagNodeK:
        {
          auto ndv = reinterpret_cast<const MomNode*>(vv);
          auto co = ndv->conn();
          if (skippable_connective(co))
            return;
          _emout << "*";
          emit_objptr(co, depth);
          _emout << '(';
          emit_maybe_newline(depth+1);
          unsigned sz = ndv->sizew();
          for (unsigned ix=0; ix<sz; ix++)
            {
              if (ix>0) emit_space(depth+1);
              emit_value(ndv->unsafe_at(ix), depth+1);
            }
          _emout << ')';
        }
        break;
        case MomKind::TagObjectK:
        {
          auto pob = reinterpret_cast<const MomObject*>(vv);
          if (skippable_object(pob)) return;
          emit_objptr(pob, depth);
        }
        break;
        case MomKind::TagIntK:
        case MomKind::TagNoneK:
        case MomKind::Tag_LastK:
          MOM_FATAPRINTF("impossible tag #%d for vv@%p", (int) vv->kindw(), vv);
        }
    }
} // end of MomEmitter::emit_value


void
MomEmitter::emit_string_value(const MomString*strv, int depth, bool asraw)
{
  if (asraw)
    {
      std::string border;
      MomHash hs = strv->hash();
      char prefbuf[16];
      memset (prefbuf, 0, sizeof(prefbuf));
      MomSerial63 sr {(hs & 0xfffff) +  MomSerial63::_minserial_};
      sr.to_cbuf16(prefbuf);
      prefbuf[0] = '|';
      prefbuf[4] = 0;
      if (!strstr(strv->cstr(),prefbuf))
        border=prefbuf;
      else
        {
          for (uint64_t n = (hs & 0xfffffff) + MomSerial63::_minserial_;
               border.empty();
               n++)
            {
              sr= n;
              sr.to_cbuf16(prefbuf);
              prefbuf[0] = '|';
              prefbuf[8] = 0;
              if (!strstr(strv->cstr(),prefbuf))
                border=prefbuf;
            }
        }
      out() << '`' << border.substr(1) << '|';
      out().write(strv->cstr(),strv->bytelen());
      out() << border << '`';
      emit_newline(depth);
    }
  else
    {
      out() << '"';
      char *buf = nullptr;
      size_t bsz = 0;
      FILE* f = open_memstream(&buf, &bsz);
      if (!f) MOM_FATAPRINTF("open_memstream failure");
      mom_output_utf8_encoded (f, strv->cstr(), strv->bytelen());
      fflush(f);
      out().write(buf, bsz);
      out() << '"';
      fclose(f);
      free (buf), buf=nullptr;
    }
} // end of MomEmitter::emit_string_value


void
MomEmitter::emit_objptr(const MomObject*pob, int depth MOM_UNUSED)
{
  if (skippable_object(pob)) return;
#warning MomEmitter::emit_objptr should deal with named objects
  out() << pob->id();
} // end MomEmitter::emit_objptr


void
MomValue::output(std::ostream& out) const
{
  MomEmitter em(out);
  em.emit_value(*this);
} // end MomValue::output
