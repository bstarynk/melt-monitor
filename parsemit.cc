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

constexpr std::uint64_t MomParser::_par_word_limit_;
constexpr double MomParser::_par_plain_time_limit_;
constexpr double MomParser::_par_debug_time_limit_;
constexpr const char MomParser::_par_comment_start1_[];
constexpr const char MomParser::_par_comment_end1_[];

#define MOM_PARSERDEBUGLOG(Pa,Log) do { if ((Pa)->_pardebug) MOM_DEBUGLOG(parse,Log); } while(0)
#define MOM_THISPARSDBGLOG(Log) MOM_PARSERDEBUGLOG(this,Log)

MomParser&
MomParser::set_loader_for_object(MomLoader*ld, MomObject*pob, const char*tit)
{
  if (!ld)
    MOM_FATALOG("set_loader_for_object missing ld tit=" << tit);
  disable_exhaustion();
  char idbuf[32];
  memset(idbuf, 0, sizeof(idbuf));
  if (pob)
    pob->id().to_cbuf32(idbuf);
  std::string titstr{"*"};
  if (tit) titstr += tit;
  if (pob)
    {
      titstr += " " ;
      titstr += idbuf;
      titstr += "*";
    };
  set_name(titstr);
  return *this;
} // end MomParser::set_loader_for_object


std::string
MomParser::parse_string(bool *pgotstr)
{
  skip_spaces();
  check_exhaustion();
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
  pc = peekbyte(0);
  nc = peekbyte(1);
  if (pc=='"')   // JSON encoded UTF8 string, on the same line
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
      if (pgotstr)
        *pgotstr = true;
      if (_parfun)
        _parfun(PtokString,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " string "
                         << MomShowString(str));
      return _parnobuild?nullptr:str;
    }
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
      if (pgotstr)
        *pgotstr = true;
      if (_parfun)
        _parfun(PtokString,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " rawstring "
                         << MomShowString(str));
      return _parnobuild?nullptr:str;
    }
failure:
  if (pgotstr)
    *pgotstr = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end  MomParser::parse_string

intptr_t
MomParser::parse_int(bool *pgotint)
{
  skip_spaces();
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  int pc = 0;
  int nc = 0;
  pc = peekbyte(0);
  nc = peekbyte(1);
  check_exhaustion();
  if (pc>0 && (((pc=='+' || pc=='-') && isdigit(nc)) || isdigit(pc)))
    {
      const char*curp = peekchars();
      char*endp = nullptr;
      long long ll = strtoll(curp, &endp, 0);
      if (endp>curp)
        consume(endp-curp);
      if (pgotint)
        *pgotint = true;
      if (_parfun)
        _parfun(PtokInt,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " int:" << ll);
      return ll;
    }
  else
    {
      if (pgotint)
        *pgotint = false;
      return 0;
    }
} // end MomParser::parse_int


void
MomParser::unterminated_small_comment(const char*missing)
{
  MOM_PARSE_FAILURE(this, "small comment not ending with " << missing);
} // end MomParser::unterminated_small_comment

MomValue
MomParser::parse_value(bool *pgotval)
{
  auto inioff = _parlinoffset;
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  int pc = 0;
  int nc = 0;
again:
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
  check_exhaustion();
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
      if (_parfun)
        _parfun(PtokInt,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " int:" << ll);
      return _parnobuild?nullptr:MomValue((intptr_t)ll);
    }
  else if (isspace(pc))
    {
      skip_spaces();
      goto again;
    }
  else if (pc=='/' && nc == '/')
    {
      next_line();
      if (_parfun)
        _parfun(PtokComment,inicol,inilincnt);
      goto again;
    }
  else if (pc=='_' && nc=='_')
    {
      consume(2);
      if (pgotval) *pgotval = true;
      if (_parfun)
        _parfun(PtokNil,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " NIL");
      return MomValue{nullptr};
    }
  else if ((pc<127 && isalpha(pc))
           || (pc=='_' && nc<127 && isdigit(nc))
           || (pc=='$' && nc=='%' && _parobjeval))
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
          if ((!_parnobuild && !curob) || !gotcurobj)
            MOM_PARSE_FAILURE(this, "missing component object in vector");
          if (!_parnobuild)
            vec.push_back(curob);
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      MOM_ASSERT(_parnobuild || cnt == (int)vec.size(),
                 "parsing tuple expecting " << cnt << " got " << vec.size());
      if (_parfun)
        _parfun(PtokTuple,inicol,inilincnt);
      auto resv = _parnobuild?nullptr:MomValue(MomTuple::make_from_objptr_vector(vec));
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "tuple/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
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
          if ((!_parnobuild && !curob) || !gotcurobj)
            MOM_PARSE_FAILURE(this, "missing element object in set");
          if (!_parnobuild)
            set.insert(curob);
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      MOM_ASSERT(cnt >= (int)set.size(),
                 "parsing set expecting at most " << cnt << " got " << set.size());
      if (_parfun)
        _parfun(PtokSet,inicol,inilincnt);
      auto resv= _parnobuild?nullptr:MomValue(MomSet::make_from_objptr_set(set));
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "set/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
    }
  else if (pc=='"')   // JSON encoded UTF8 string, on the same line
    {
      bool gotstr = false;
      std::string str = parse_string(&gotstr);
      if (!gotstr)
        MOM_PARSE_FAILURE(this, "failed to parse string");
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " string "
                         << MomShowString(str));
      return _parnobuild?nullptr:MomValue(MomString::make_from_string(str));
    }
  // multi-line raw strings like `ab0|foobar\here|ab0`
  else if (pc=='`' && (nc=='_' || isalnum(nc) || nc=='|'))   // raw strings
    {
      bool gotstr = false;
      auto str = parse_string(&gotstr);
      if (!gotstr)
        MOM_PARSE_FAILURE(this, "failed to parse raw string");
      return _parnobuild?nullptr:MomValue(MomString::make_from_string(str));
    }
  else if (pc=='(' && nc=='#')	// (# integer sequence #)
    {
      int cnt = 0;
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
              if (!_parnobuild)
                v.push_back(ll);
              cnt++;
            }
          else
            MOM_PARSE_FAILURE(this, "invalid integer sequence");
        }
      if (pgotval)
        *pgotval = true;
      if (_parfun)
        _parfun(PtokIntSeq,inicol,inilincnt);
      auto resv = _parnobuild?nullptr:MomValue(MomIntSq::make_from_vector(v));
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "intseq/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
    }
  else if (pc=='(' && nc==':')	// (: double sequence :)
    {
      int cnt =0;
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
              if (!_parnobuild)
                v.push_back(x);
              cnt++;
            }
          else
            MOM_PARSE_FAILURE(this, "invalid double sequence");
        }
      if (pgotval)
        *pgotval = true;
      if (_parfun)
        _parfun(PtokDoubleSeq,inicol,inilincnt);
      auto resv = _parnobuild?nullptr:MomValue(MomDoubleSq::make_from_vector(v));
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "doubleseq/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
    }
  else if (pc=='*' /* && nc<127 && !(nc>0 && nc!='_' && ispunct(nc))*/) // node
    {
      MomObject* connob = nullptr;
      bool gotconn = false;
      std::vector<MomValue> sonvec;
      int cnt = 0;
      consume(1);
      skip_spaces();
      connob = parse_objptr(&gotconn);
      if ((!_parnobuild && !connob) || !gotconn)
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
          if ((!_parnobuild && !curval) || !gotcurval)
            MOM_PARSE_FAILURE(this, "missing son#" << sonvec.size() << " in node of " << connob);
          if (!_parnobuild)
            sonvec.push_back(curval);
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      if (_parfun)
        _parfun(PtokNode,inicol,inilincnt);
      auto resv =  _parnobuild?nullptr:MomValue(MomNode::make_from_vector(connob,sonvec));
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "node/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
    }
  else if (pc=="°"[0] && nc=="°"[1])
    {
      static_assert(sizeof("°")==3, "wrong length for ° (degree sign)");
      consume(2);
      bool gotvval = false;
      auto vv = MomParser::parse_value(&gotvval);
      if (gotvval)
        {
          if (pgotval)
            *pgotval = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "transient"
                             << (_parnobuild?"!":" ") << vv);
          if (vv.is_val())
            return  _parnobuild?nullptr:MomValue(vv.to_val(),MomTransientTag{});
          else return vv;
        }
      else
        goto failure;
    }
  else if (pc=='$' && nc=='(' && _parhaschunk)
    {
      consume(2);
      bool gotvchunk = false;
      auto vc = MomParser::parse_chunk(&gotvchunk);
      if (gotvchunk)
        {
          if (pgotval)
            *pgotval = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "chunk"
                             << (_parnobuild?"!":" ") << vc);
          return  _parnobuild?nullptr:vc;
        }
      else
        goto failure;
    }
  else if (pc=='$' && nc=='!' && _parvaleval)
    {
      consume(2);
      bool gotvnod = false;
      auto vnod = MomParser::parse_value(&gotvnod);
      if (!_parnobuild && !vnod.is_val() && !vnod.to_val()->is_node())
        MOM_PARSE_FAILURE(this, "evaluated value expects node after $!, got " << vnod);
      check_exhaustion();
      bool gotok = false;
      auto resv = _parnobuild?nullptr:_parvaleval(this,vnod.to_val()->as_node(),&gotok);
      check_exhaustion();
      if (!_parnobuild && !gotok)
        MOM_PARSE_FAILURE(this, "evaluated value failed with node " << vnod);
      if (pgotval)
        *pgotval = true;
      return  _parnobuild?nullptr:resv;
    }
failure:
  if (pgotval)
    *pgotval = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end of MomParser::parse_value


MomValue
MomParser::parse_chunk(bool *pgotchunk)
{
  check_exhaustion();
  std::vector<MomValue> vecelem;
  //auto inioff = _parlinoffset;
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  while (parse_chunk_element(vecelem))
    {
      // do nothing
    };
  if (peekbyte(0) == ')' && peekbyte(1) == '$')
    {
      consume(2);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "endchunk/"
                         << vecelem.size() << ":"
                         << (MomDoShow([=,&vecelem](std::ostream&out)
      {
        int cnt=0;
        for (auto vcomp : vecelem)
          {
            if (cnt>0) out << ' ';
            out << vcomp;
            cnt++;
          }
      }))
          << std::endl);
      if (pgotchunk)
        *pgotchunk = true;
      if (_parnobuild)
        return nullptr;
      else
        {
          auto vch = chunk_value(vecelem);
          if (!vch)
            MOM_PARSE_FAILURE(this, "failed to build chunk started at line#" << inilincnt << " column:" << inicol);
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << "endchunk=" << vch);
          return vch;
        }
    }
  else
    MOM_PARSE_FAILURE(this, "failed to parse chunk started at line#" << inilincnt << " column:" << inicol);
} // end MomParser::parse_chunk




bool
MomParser::parse_chunk_element(std::vector<MomValue>& vecelem)
{
  int pc = 0;
  int nc = 0;
  check_exhaustion();
  //auto inioff = _parlinoffset;
  auto inicol = _parcol;
  auto inilincnt = _parlincount;
  MomIdent id;
  const char* endid = nullptr;
  pc = peekbyte(0);
  nc = peekbyte(1);
  /* in chunks, )$ is ending the chunk */
  if (pc == ')' && nc == '$')
    return false;
  if (eof())
    return false;
  /* in chunks, the end-of-line is parsed as a newline */
  if (eol())
    {
      next_line();
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem EOL");
      if (!_parnobuild)
        vecelem.push_back(MomString::make_from_string("\n"));
      return true;
    }
  /* in chunks, a sequence of space is agglomerated into a single string */
  if (isspace(pc))
    {
      std::string spacestr;
      while (isspace(pc))
        {
          spacestr += (char)pc;
          consume(1);
          pc = peekbyte(0);
        };
      if (eol())
        {
          spacestr += '\n';
          next_line();
        }
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem spacestr " << MomShowString(spacestr));
      if (!_parnobuild)
        vecelem.push_back(MomString::make_from_string(spacestr));
      return true;
    }
  /* handling names in chunks */
  else if (pc<127 && isalpha(pc))
    {
      std::string namestr;
      while (pc>0 && pc<127 && (isalnum(pc) || pc=='_'))
        {
          namestr += (char)pc;
          consume(1);
          pc = peekbyte(0);
        }
      auto namv = _parnobuild?nullptr:chunk_name(namestr);
      if (namv)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem name " << namestr << " = " << namv);
          vecelem.push_back(namv);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem namestr " << namestr);
          if (!_parnobuild)
            vecelem.push_back(MomString::make_from_string(namestr));
        }
      return true;
    }
  /* handling objids in chunks */
  else if (pc=='_' && nc<127 && isdigit(nc) && (id=MomIdent::make_from_cstr(peekchars(0),&endid))
           && endid!=nullptr)
    {
      std::string idstr(peekchars(0), endid - peekchars());
      consume(endid - peekchars());
      auto vid = _parnobuild?nullptr:chunk_id(id);
      if (vid)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem id=" << id << " = " << vid);
          vecelem.push_back(vid);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem idstr=" << idstr);
          if (!_parnobuild)
            vecelem.push_back(MomString::make_from_string(idstr));
        }
      return true;
    }
  /* $, and $; and $. are all handled as separators */
  else if (pc=='$' && (nc == ',' || nc==';' || nc=='.'))
    {
      consume(2);
      return true;
    }
  /* handle $<name> as a dollarobj, or else a string */
  else if (pc=='$' && nc<127 && isalpha(nc))
    {
      std::string dollstr;
      consume(1);
      while ((pc==peekbyte(0))>0 && pc<127 && (isalnum(pc) || pc=='_'))
        {
          dollstr += (char)pc;
          consume(1);
        }
      if (_parnobuild)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem dollarname nobuild dollstr=$" << dollstr);
          return true;
        }
      auto pob = fetch_named_object(dollstr);
      MomValue v = pob?chunk_dollarobj(pob):nullptr;
      if (v)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem dollarname dollar=" << dollstr << " v=" << v);
          vecelem.push_back(v);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " chunkelem dollarname dollstr=$" << dollstr);
          vecelem.push_back(MomString::make_from_string(std::string{"$"} + dollstr));
        }
      return true;
    }
  /* handle $%<value> as an embedded value */
  else if (pc=='$' && nc=='%')
    {
      bool gotval = false;
      consume(2);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem start embedded value");
      auto embv = parse_value(&gotval);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem got embedded value" <<(_parnobuild?"!":" ") << embv);
      auto v = _parnobuild?nullptr:chunk_value(embv);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem embedding value" <<(_parnobuild?"!":" ") << v);
      if (!_parnobuild)
        vecelem.push_back(v);
      return true;
    }
  /* any other non-space non-letter non-dollar characters, is
     agglomerated, hence handling UTF8; two dollars $$ are parsed as a
     single one */
  else
    {
      std::string str;
      while ((pc = peekbyte(0))>0 && !eol() && !(isalpha(pc) || isspace(pc)))
        {
          nc = peekbyte(1);
          if (pc=='$')
            {
              if (nc == '$')
                consume(1);
              else
                break;
            }
          str += (char)pc;
          consume(1);
        }
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " chunkelem other " << MomShowString(str));
      if (!_parnobuild)
        vecelem.push_back(MomString::make_from_string(str));
      return true;
    }
} // end MomParser::parse_chunk_element



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
  check_exhaustion();
  pc = peekbyte(0);
  nc = peekbyte(1);
  if (gotspacing())
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
      if (!_parnobuild)
        {
          if (_parmakefromid)
            respob = MomObject::make_object_of_id(id);
          else
            respob = MomObject::find_object_of_id(id);
        }
      if (_parnobuild || respob)
        {
          consume(endp-curp);
          if (pgotob)
            *pgotob = true;
          if (_parfun)
            _parfun(PtokId,inicol,inilincnt);
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " objid:" << id);
          return respob;
        }
      else goto failure;
    }
  else if (pc=='_' && nc=='_')
    {
      consume(2);
      if (pgotob)
        *pgotob = true;
      if (_parfun)
        _parfun(PtokNil,inicol,inilincnt);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol << " OBJNIL");
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
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                         << " namstr=" << MomShowString(namstr)
                         << " ob=" << ob);
      if (ob || _parnobuild)
        {
          consume(endnamp-begnamp);
          if (pgotob)
            *pgotob = true;
          if (_parfun)
            _parfun(PtokName,inicol,inilincnt);
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicol
                             << " namedobj: " << namstr << " = " << ob);
          return ob;
        }
      else goto failure;
    }
  else if (pc=='$' && nc=='%' && _parobjeval)
    {
      consume(2);
      bool gotvnod = false;
      auto vnod = MomParser::parse_value(&gotvnod);
      if (!_parnobuild && !vnod.is_val() && !vnod.as_val()->is_node())
        MOM_PARSE_FAILURE(this, "evaluated object expects node after $%, got " << vnod);
      check_exhaustion();
      bool gotok = false;
      MomObject* resob = _parnobuild?nullptr:_parobjeval(this,vnod.as_val()->as_node(),&gotok);
      check_exhaustion();
      if (!_parnobuild && !gotok)
        MOM_PARSE_FAILURE(this, "evaluated object failed with node " << vnod);
      if (pgotob)
        *pgotob = true;
      return  _parnobuild?nullptr:resob;
    }
failure:
  if (pgotob)
    *pgotob = false;
  restore_state(inioff, inilincnt, inicol);
  return nullptr;
} // end of MomParser::parse_objptr




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
MomEmitter::emit_string(const std::string&str, int depth, bool asraw)
{
  if (asraw)
    {
      std::string border;
      MomHash hs = mom_cstring_hash_len (str.c_str(), str.size());
      char prefbuf[16];
      memset (prefbuf, 0, sizeof(prefbuf));
      MomSerial63 sr {(hs & 0xfffff) +  MomSerial63::_minserial_};
      sr.to_cbuf16(prefbuf);
      prefbuf[0] = '|';
      prefbuf[4] = 0;
      if (!strstr(str.c_str(),prefbuf))
        border=prefbuf;
      else
        {
          border.clear();
          for (uint64_t n = (hs & 0xfffffff) + MomSerial63::_minserial_;
               border.empty();
               n++)
            {
              sr= n;
              sr.to_cbuf16(prefbuf);
              prefbuf[0] = '|';
              prefbuf[8] = 0;
              if (!strstr(str.c_str(),prefbuf))
                border=prefbuf;
            }
        }
      out() << '`' << border.substr(1) << '|';
      out().write(str.c_str(),str.size());
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
      mom_output_utf8_encoded (f, str.c_str(), str.size());
      fflush(f);
      out().write(buf, bsz);
      out() << '"';
      fclose(f);
      free (buf), buf=nullptr;
    }
} // end of MomEmitter::emit_string


void
MomEmitter::emit_objptr(const MomObject*pob, int depth MOM_UNUSED)
{
  if (!pob || skippable_object(pob))
    out() << "__";
  else
    out() << pob->id();
} // end MomEmitter::emit_objptr


void
MomValue::output(std::ostream& out) const
{
  MomEmitter em(out);
  em.emit_value(*this);
} // end MomValue::output
