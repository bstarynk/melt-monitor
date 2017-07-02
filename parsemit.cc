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


void
MomParser::consume_bytes(unsigned byteoffset)
{
  const char*pc = curbytes();
  const char*inip = pc;
  const char*end = eolptr();
  if (!pc || !end) return;
  pc += byteoffset;
  if (pc > end)
    pc = end;
  int nbc = 0;
  for (const char*ps = inip; ps < pc && *ps; ps = g_utf8_next_char(ps))
    nbc++;
  if (*pc && !g_utf8_validate(pc, end-pc, NULL))
    MOM_PARSE_FAILURE(this, "consume_bytes with wrong byteoffset=" << byteoffset << " @" << location_str());
  _parcolidx += (pc - inip);
  _parcolpos += nbc;
} // end MomParser::consume_bytes

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



MomParser&
MomParser::skip_spaces()
{
  for (;;)
    {
      if (eol())
        {
          if (!_parinp) return *this;
          next_line();
          continue;
        }
      gunichar pc = 0, nc = 0;
      peek_prevcurr_utf8(pc,nc,1);
      if (isspace(pc))
        consume_utf8(1);
      else if ((pc == '/' && nc == '/')
               || (pc == '#' && nc == '!'))
        {
          next_line();
          continue;
        }
      else if (pc == '|')
        {
          const char* rest = curbytes()+1;
          const char* endcomm = strchr(rest, '|');
          if (!endcomm)
            unterminated_small_comment("|");
          else consume_bytes(endcomm-rest + 1);
          continue;
        }
      else if (has_cstring(_par_comment_start1_))
        {
          consume_utf8(1);
          const char* rest = curbytes()+strlen(_par_comment_start1_);
          const char*endcomm = strstr(rest, _par_comment_end1_);
          if (!endcomm)
            unterminated_small_comment(_par_comment_end1_);
          else consume_bytes(endcomm-rest + strlen(_par_comment_end1_));
          continue;
        }
      else break;
    }
  return *this;
} // end MomParser::skip_spaces

MomParser&
MomParser::next_line()
{
  _parlinoffset = _parinp.tellg();
  _parlinstr.clear();
  std::getline(_parinp, _parlinstr);
  if (MOM_UNLIKELY(!g_utf8_validate(_parlinstr.c_str(), _parlinstr.size(),
                                    nullptr)))
    MOM_PARSE_FAILURE(this,"invalid UTF8 line#" << _parlincount
                      << ":" << _parlinstr);
  _parcolidx = 0;
  _parcolpos = 0;
  if (_parinp)
    _parlincount++;
  if (_pardebug)
    MOM_DEBUGLOG(parse, "Mom_Parser::next_line _parlinoffset=" << _parlinoffset
                 << " _parlinstr=" << MomShowString(_parlinstr)
                 << " _parlincount=" << _parlincount
                 << " @" << location_str());
  return *this;
} // end MomParser::next_line



std::string
MomParser::parse_string(bool *pgotstr)
{
  skip_spaces();
  check_exhaustion();
  auto inioff = _parlinoffset;
  auto inicolidx = _parcolidx;
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  gunichar pc = 0;
  gunichar nc = 0;
#define MOM_BORDER_SIZE 32
#define MOM_BORDER_FORMAT "%30[A-Za-z0-9_]"
  char border[MOM_BORDER_SIZE];
  constexpr unsigned bordersize = sizeof(border);
  int borderpos = 0;
  memset (border, 0, bordersize);
  peek_prevcurr_utf8(pc,nc,1);
  if (pc=='"')   // JSON encoded UTF8 string, on the same line
    {
      consume_utf8(1);
      std::string restinline{curbytes()};
      std::istringstream ins{restinline};
      auto str = mom_input_quoted_utf8_string(ins);
      consume_bytes(ins.tellg());
      pc = peek_utf8(0);
      if (pc != '"')
        MOM_PARSE_FAILURE(this, "expecting doublequote ending string, but got " << (char)pc);
      consume_utf8(1);
      if (pgotstr)
        *pgotstr = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " string "
                         << MomShowString(str));
      return _parnobuild?nullptr:str;
    }
  else if (pc=='`' && (memset(border, 0, sizeof(border)), (borderpos=0), (nc=='_' || isalnum(nc)))
           && sscanf(curbytes(), "`" MOM_BORDER_FORMAT "|%n",
                     border, &borderpos) >= 1 && borderpos>0 && border[0]>0)   // raw strings
    {
      /* the raw string may take many lines, it is ending with the |BORDER` */
      MOM_DEBUGLOG(parse, "raw string starting " << location_str()
                   << " border=" << MomShowString(border)
                   << " borderpos=" << borderpos);
      int borderlen = strlen(border);
      std::string str;
      str.reserve(48);
      int nblines = 0;
      consume_bytes(borderlen+2);
      long loopcnt = 0;
      for (;;)
        {
          loopcnt++;
          pc = peek_utf8(0);
          bool lastofline = (pc == 0) || (peek_utf8(1)==0);
          char s[4];
          memset(s, 0, sizeof(s));
          s[0] = pc;
          MOM_DEBUGLOG(parsestring, "parse_string raw string pc=" << (int)pc << "==" << MomShowString(s)
                       << " curbytes=" << MomShowString(curbytes())
                       << " loopcnt=" << loopcnt
                       << " lastofline=" << (lastofline?"true":"false"));
          if (pc > 0 && pc != '|' && !lastofline)
            {
              str.push_back((char)pc);
              MOM_DEBUGLOG(parsestring, "parse_string raw string plainchar nblines=" << nblines
                           << " loopcnt=" << loopcnt
                           << " curbytes=" << MomShowString(curbytes())
                           << " pc=" << (int)pc << "==" << MomShowString(s)
                           << " str=" << MomShowString(str)
                           << " lastofline=" << (lastofline?"true":"false")
                           << " @" << location_str());
              consume_utf8(1);
              continue;
            }
          if (loopcnt%16 == 0 || lastofline || MOM_IS_DEBUGGING(parsestring))
            MOM_DEBUGLOG(parse, "parse_string raw string nblines=" << nblines
                         << " loopcnt=" << loopcnt
                         << " curbytes=" << MomShowString(curbytes())
                         << " str=" << MomShowString(str)
                         << " lastofline=" << (lastofline?"true":"false")
                         << " @" << location_str());
          if (eol() || pc == '\n')
            {
              nblines++;
              str.append("\n");
              MOM_DEBUGLOG(parse, "parse_string eol raw string nblines=" << nblines
                           << " loopcnt=" << loopcnt
                           << " str=" << MomShowString(str)
                           << " pc=" << pc
                           << std::endl
                           << " @" << location_str());
              if (!_parinp) goto failure;
              continue;
            }
          if (pc == '|')
            {
              nc = peek_utf8(1);
              MOM_DEBUGLOG(parse, "parse_string pipe raw string curbytes="
                           << MomShowString(curbytes())
                           << " @" << location_str()
                           << " border=" << MomShowString(border)
                           << ", borderlen=" << borderlen);
              const char*ps = curbytes();
              if ((char)nc == border[0] && ps && !strncmp(ps+1, border, borderlen)
                  && ps[borderlen+1]=='`')
                {
                  MOM_DEBUGLOG(parsestring,
                               "parse_string pipe got ending curbytes="
                               << MomShowString(curbytes())
                               << " @" << location_str());
                  consume_bytes(borderlen+2);
                  MOM_DEBUGLOG(parse, "parse_string raw string pipe ending curbytes="
                               << MomShowString(curbytes())
                               << " @" << location_str());
                  break;
                }
              else
                {
                  MOM_DEBUGLOG(parsestring, "parse_string pipe nonending curbytes="
                               << MomShowString(curbytes())
                               << " @" << location_str());
                }
            }
          else
            {
              char s[4];
              memset(s, 0, sizeof(s));
              s[0] = pc;
              MOM_DEBUGLOG(parsestring,
                           "parse_string nonpipe pc=" << (int)pc
                           << "==" << MomShowString(s)
                           << "  curbytes="
                           << MomShowString(curbytes())
                           << " @" << location_str());
            }
          str.push_back(pc);
          consume_utf8(1);
          pc = peek_utf8(0);
          if (lastofline)
            {
              nblines++;
              str.append("\n");
              MOM_DEBUGLOG(parse, "parse_string lasteol raw string nblines=" << nblines
                           << " loopcnt=" << loopcnt
                           << " str=" << MomShowString(str)
                           << " pc=" << pc
                           << std::endl
                           << " @" << location_str());
              if (!_parinp) goto failure;
              next_line();
              continue;
            }
        };
      MOM_DEBUGLOG(parse, "parse_string raw string ended str=" << MomShowString(str)
                   << std::endl
                   << " @" << location_str()
                   << std::endl);
      if (pgotstr)
        *pgotstr = true;
      MOM_THISPARSDBGLOG("parse_string ended L"<< inilincnt << ",C" << inicolpos << " final rawstring "
                         << MomShowString(str)
                         << std::endl
                         << " @" << location_str()
                         << std::endl);
      return _parnobuild?nullptr:str;
    }
failure:
  if (pgotstr)
    *pgotstr = false;
  restore_state(inioff, inilincnt, inicolidx, inicolpos);
  return nullptr;
} // end  MomParser::parse_string

intptr_t
MomParser::parse_int(bool *pgotint)
{
  skip_spaces();
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  gunichar pc = 0, nc = 0;
  peek_prevcurr_utf8(pc,nc,1);
  check_exhaustion();
  if (pc>0 && (((pc=='+' || pc=='-') && isdigit(nc)) || isdigit(pc)))
    {
      const char*curp = curbytes();
      char*endp = nullptr;
      long long ll = strtoll(curp, &endp, 0);
      if (endp>curp)
        consume_bytes(endp-curp);
      if (pgotint)
        *pgotint = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " int:" << ll);
      return ll;
    }
  else
    {
      if (pgotint)
        *pgotint = false;
      return 0;
    }
} // end MomParser::parse_int

double
MomParser::parse_double(bool *pgotdouble)
{
  skip_spaces();
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  gunichar pc = 0, nc = 0;
  peek_prevcurr_utf8(pc,nc,1);
  if (pc<127 && (isdigit(pc)||(pc=='+'||pc=='-')))
    {
      const char*curp = curbytes();
      char*endp = nullptr;
      double d = strtod(curp, &endp);
      if (endp>curp)
        consume_bytes(endp-curp);
      if (pgotdouble)
        *pgotdouble = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " double:" << d);
      return d;
    };
  if (pgotdouble)
    *pgotdouble = false;
  return 0.0;
} // end MomParser::parse_double

void
MomParser::unterminated_small_comment(const char*missing)
{
  MOM_PARSE_FAILURE(this, "small comment not ending with " << missing
                    << " curbytes=" << MomShowString(curbytes())
                    << " @" << location_str());
} // end MomParser::unterminated_small_comment

MomValue
MomParser::parse_value(bool *pgotval, int depth)
{
  auto inioff = _parlinoffset;
  auto inicolidx = _parcolidx;
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  gunichar pc = 0, nc = 0;
again:
  skip_spaces();
  inioff = _parlinoffset;
  inicolidx = _parcolidx;
  inicolpos = _parcolpos;
  inilincnt = _parlincount;
  if (eol())
    {
      skip_spaces();
      if (eof())
        {
          if (pgotval)
            *pgotval = false;
          MOM_DEBUGLOG(parse, "parse_value got EOF @" << location_str());
          return nullptr;
        }
      if (eol() && !_parinp) goto failure;
      goto again;
    }
  check_exhaustion();
  peek_prevcurr_utf8(pc, nc, 1);
  if (pc>0 && (((pc=='+' || pc=='-') && isdigit(nc)) || isdigit(pc)))
    {
      const char*curp = curbytes();
      char*endp = nullptr;
      long long ll = strtoll(curp, &endp, 0);
      if (endp>curp)
        consume_bytes(endp-curp);
      if (pgotval) *pgotval = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " int:" << ll);
      if (_parsignalvalue)
        parsed_value_int((intptr_t)ll, inioff, inilincnt, inicolpos, _parcolpos+1);
      return _parnobuild?nullptr:MomValue((intptr_t)ll);
    }
  else if (isspace(pc))
    {
      skip_spaces();
      goto again;
    }
  else if ((pc=='/' && nc == '/') || (pc=='#' && nc == '!'))
    {
      next_line();
      goto again;
    }
  else if (pc=='_' && nc=='_')
    {
      consume_utf8(2);
      if (pgotval) *pgotval = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " NIL");
      if (_parsignalvalue)
        parsed_value_null(inioff, inicolpos, inilincnt);
      return MomValue{nullptr};
    }
  else if ((pc<127 && isalpha(pc))
           || (pc=='_' && nc<127 && isdigit(nc))
           || (pc=='$' && nc=='%' && _parobjeval)
           || (pc=='@' && nc<127 && isalpha(nc)))
    {
      return MomValue(parse_objptr(pgotval, depth));
    }
  else if (pc=='[') // tuple
    {
      consume_utf8(1);
      MomObjptrVector vec;
      int cnt=0;
      for (;;)
        {
          bool gotcurobj = false;
          MomObject* curob = nullptr;
          skip_spaces();
          pc = peek_utf8(0);
          if (pc == ']')
            {
              consume_utf8(1);
              break;
            }
          curob = parse_objptr(&gotcurobj, depth+1);
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
      auto tupv = _parnobuild?nullptr:MomTuple::make_from_objptr_vector(vec);
      auto resv = _parnobuild?nullptr:MomValue(tupv);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "tuple/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      if (_parsignalvalue)
        parsed_value_sequence(tupv,true,
                              inioff, inilincnt, inicolpos,
                              _parlinoffset+_parcolidx,
                              _parlincount, _parcolpos, depth);
      return resv;
    }
  else if (pc=='{') // set
    {
      MomObjptrSet set;
      int cnt = 0;
      consume_utf8(1);
      for (;;)
        {
          bool gotcurobj = false;
          skip_spaces();
          MomObject* curob = nullptr;
          pc = peek_utf8(0);
          if (pc == '}')
            {
              consume_utf8(1);
              break;
            }
          curob = parse_objptr(&gotcurobj, depth+1);
          if ((!_parnobuild && !curob) || !gotcurobj)
            MOM_PARSE_FAILURE(this,
                              "missing element object in set; curbytes=" << MomShowString(curbytes()) << "; _parlinstr=" << MomShowString(_parlinstr));
          if (!_parnobuild)
            set.insert(curob);
          cnt++;
        }
      if (pgotval)
        *pgotval = true;
      MOM_ASSERT(cnt >= (int)set.size(),
                 "parsing set expecting at most " << cnt << " got " << set.size());
      auto setv = _parnobuild?nullptr:MomSet::make_from_objptr_set(set);
      auto resv = _parnobuild?nullptr:MomValue(setv);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "set/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      if (_parsignalvalue)
        parsed_value_sequence(setv,/*istuple*/false,
                              inioff, inilincnt, inicolpos,
                              _parlinoffset+_parcolidx,
                              _parlincount, _parcolpos, depth);
      return resv;
    }
  else if (pc=='"')   // JSON encoded UTF8 string, on the same line
    /// however, a string may be continued with &+& (it is a literal string catenation operator)
    /// eg "abc" &+& "def"  is the same as "abcdef"
    {
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "start of string");
      bool gotstr = false;
      bool again = false;
      std::string fullstr;
      do
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " fullstr="
                             << MomShowString(fullstr) << " @" << location_str());
          again = false;
          std::string str = parse_string(&gotstr);
          if (!gotstr)
            MOM_PARSE_FAILURE(this, "failed to parse string");
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " got string "
                             << MomShowString(str));
          skip_spaces();
          fullstr += str;
          if (got_cstring("&+&"))
            again = true;
        }
      while (again);
      if (pgotval)
        *pgotval = true;
      auto res = _parnobuild?nullptr:MomValue(MomString::make_from_string(fullstr));
      if (_parsignalvalue)
        parsed_value_string(fullstr,
                            inioff, inilincnt, inicolpos,
                            _parlinoffset+_parcolidx,
                            _parlincount, _parcolpos);
      return res;
    }
  // multi-line raw strings like `ab0|foobar\here|ab0`
  else if (pc=='`' && (nc=='_' || isalnum(nc) || nc=='|'))   // raw strings
    {
      bool gotstr = false;
      auto str = parse_string(&gotstr);
      if (!gotstr)
        MOM_PARSE_FAILURE(this, "failed to parse raw string");
      if (pgotval)
        *pgotval = true;
      if (_parsignalvalue)
        parsed_value_string(str,
                            inioff, inilincnt, inicolpos,
                            _parlinoffset+_parcolidx,
                            _parlincount, _parcolpos);
      return _parnobuild?nullptr:MomValue(MomString::make_from_string(str));
    }
  else if (pc=='(' && nc=='#')	// (# integer sequence #)
    {
      int cnt = 0;
      std::vector<intptr_t> v;
      consume_utf8(2);
      MOM_DEBUGLOG(parse, "parse_value intseq start @" << location_str());
      for (;;)
        {
          skip_spaces();
          peek_prevcurr_utf8(pc,nc,1);
          if (pc==0)
            {
              goto failure;
            }
          else if (pc=='#' && nc==')')
            {
              consume_utf8(2);
              break;
            }
          else if ((pc >0 && pc<127 && isdigit(pc)) || (nc > 0 && nc<127 && isdigit(nc) && (pc=='+' || pc=='-')))
            {
              const char*curp = curbytes();
              char*endp = nullptr;
              long long ll = strtoll(curp, &endp, 0);
              cnt++;
              MOM_DEBUGLOG(parse, "parse_value intseq#" << cnt << "=" << ll << " @" << location_str());
              consume_bytes(endp-curp);
              if (!_parnobuild)
                v.push_back(ll);
            }
          else
            MOM_PARSE_FAILURE(this, "invalid integer sequence " << MomShowString(curbytes()));
        }
      if (pgotval)
        *pgotval = true;
      auto intsq = _parnobuild?nullptr:MomIntSq::make_from_vector(v);
      auto resv = _parnobuild?nullptr:MomValue(intsq);
      if (_parsignalvalue)
        parsed_value_intsq(intsq,
                           inioff, inilincnt, inicolpos,
                           _parlinoffset+_parcolidx,
                           _parlincount, _parcolpos,
                           depth);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "intseq/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      return resv;
    }
  else if (pc=='(' && nc==':')	// (: double sequence :)
    {
      int cnt =0;
      std::vector<double> v;
      consume_utf8(2);
      MOM_DEBUGLOG(parse, "parse_value doubleseq start @" << location_str());
      for (;;)
        {
          skip_spaces();
          peek_prevcurr_utf8(pc,nc,1);
          if (pc==0)
            {
              goto failure;
            }
          else if (pc==':' && nc==')')
            {
              consume_utf8(2);
              break;
            }
          else if ((pc<127 && isdigit(pc)) || (nc<127 && isdigit(nc) && (pc=='+' || pc=='-')))
            {
              const char*curp = curbytes();
              char*endp = nullptr;
              double x = strtod(curp, &endp);
              cnt++;
              MOM_DEBUGLOG(parse, "parse_value dblseq#" << cnt << "=" << x << " @" << location_str());
              consume_bytes(endp-curp);
              if (!_parnobuild)
                v.push_back(x);
            }
          else
            MOM_PARSE_FAILURE(this, "invalid double sequence");
        }
      if (pgotval)
        *pgotval = true;
      auto dblsq = _parnobuild?nullptr:MomDoubleSq::make_from_vector(v);
      auto resv = _parnobuild?nullptr:MomValue(dblsq);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "doubleseq/" << cnt
                         << (_parnobuild?"!":" ") << resv);
      if (_parsignalvalue)
        parsed_value_doublesq(dblsq,
                              inioff, inilincnt, inicolpos,
                              _parlinoffset+_parcolidx,
                              _parlincount, _parcolpos,
                              depth);
      return resv;
    }
  else if (pc=='*' /* && nc<127 && !(nc>0 && nc!='_' && ispunct(nc))*/) // node
    {
      MomObject* connob = nullptr;
      bool gotconn = false;
      std::vector<MomValue> sonvec;
      int cnt = 0;
      consume_utf8(1);
      skip_spaces();
      connob = parse_objptr(&gotconn, depth+1);
      if ((!_parnobuild && !connob) || !gotconn)
        MOM_PARSE_FAILURE(this, "missing connective object in node");
      skip_spaces();
      pc = peek_utf8(0);
      if (pc != '(')
        MOM_PARSE_FAILURE(this, "missing left parenthesis in node of " << connob);

      auto leftoff = _parlinoffset;
      auto leftcolidx = _parcolidx;
      auto leftcolpos = _parcolpos;
      auto leftlincnt = _parlincount;
      consume_utf8(1);
      for (;;)
        {
          bool gotcurval = false;
          MomValue curval{nullptr};
          skip_spaces();
          pc = peek_utf8(0);
          if (pc == ')')
            {
              consume_utf8(1);
              break;
            }
          curval = parse_value(&gotcurval, depth+1);
          if ((!_parnobuild && !curval) || !gotcurval)
            MOM_PARSE_FAILURE(this, "missing son#" << sonvec.size()
                              << " in node of " << MomShowObject(connob));
          if (!_parnobuild)
            sonvec.push_back(curval);
          cnt++;
        };
      if (pgotval)
        *pgotval = true;
      auto nodv = _parnobuild?nullptr:MomNode::make_from_vector(connob,sonvec);
      auto resv = _parnobuild?nullptr:MomValue(nodv);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << ", node/" << cnt
                         << (_parnobuild?"!":" ") << resv
                         << " @" << location_str());
      if (_parsignalvalue)
        parsed_value_node(nodv,
                          inioff,inilincnt,inicolpos,
                          leftoff, leftlincnt, leftcolpos,
                          _parlinoffset+_parcolidx,
                          _parlincount, _parcolpos,
                          depth
                         );
      return resv;
    }
  else if ((char)pc=="°"[0] && (char)nc=="°"[1])
    {
      static_assert(sizeof("°")==3, "wrong length for ° (degree sign)");
      consume_utf8(1);
      bool gotvval = false;
      auto vv = MomParser::parse_value(&gotvval,depth+1);
      if (gotvval)
        {
          if (pgotval)
            *pgotval = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "transient"
                             << (_parnobuild?"!":" ") << vv);
          if (vv.is_val())
            return  _parnobuild?nullptr:MomValue(vv.to_val(),MomTransientTag{});
          else return vv;
        }
      else
        goto failure;
    }
  else if (pc=='#' && nc=='{' && _parhaschunk)
    {
      consume_utf8(2);
      bool gotvchunk = false;
      auto vc = MomParser::parse_chunk(&gotvchunk, depth);
      if (gotvchunk)
        {
          if (pgotval)
            *pgotval = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "chunk"
                             << (_parnobuild?"!":" ") << vc);
          return  _parnobuild?nullptr:vc;
        }
      else
        goto failure;
    }
  else if (pc=='$' && nc=='!' && _parvaleval)
    {
      consume_utf8(2);
      bool gotvnod = false;
      auto vnod = MomParser::parse_value(&gotvnod, depth+1);
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
  restore_state(inioff, inilincnt, inicolidx, inicolpos);
  return nullptr;
} // end of MomParser::parse_value


MomValue
MomParser::parse_chunk(bool *pgotchunk, int depth)
{
  check_exhaustion();
  std::vector<MomValue> vecelem;
  //auto inioff = _parlinoffset;
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  while (parse_chunk_element(vecelem, depth))
    {
      // do nothing
    };
  if (peek_utf8(0) == '}' && peek_utf8(1) == '#')
    {
      consume_utf8(2);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "endchunk/"
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
            MOM_PARSE_FAILURE(this, "failed to build chunk started at line#" << inilincnt << " column:" << inicolpos);
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << "endchunk=" << vch);
          return vch;
        }
    }
  else
    MOM_PARSE_FAILURE(this, "failed to parse chunk started at line#" << inilincnt << " column:" << inicolpos);
} // end MomParser::parse_chunk




bool
MomParser::parse_chunk_element(std::vector<MomValue>& vecelem, int depth)
{
  gunichar pc=0, nc=0;
  check_exhaustion();
  //auto inioff = _parlinoffset;
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  MomIdent id;
  const char* endid = nullptr;
  peek_prevcurr_utf8(pc,nc,1);
  /* in chunks, }# is ending the chunk */
  if (pc == '}' && nc == '#')
    return false;
  if (eof())
    return false;
  /* in chunks, the end-of-line is parsed as a newline */
  if (eol())
    {
      next_line();
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
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
          consume_utf8(1);
          pc = peek_utf8(0);
        };
      if (eol())
        {
          spacestr += '\n';
          next_line();
        }
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
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
          consume_utf8(1);
          pc = peek_utf8(0);
        }
      auto namv = _parnobuild?nullptr:chunk_name(namestr);
      if (namv)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem name " << namestr << " = " << namv);
          vecelem.push_back(namv);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem namestr " << namestr);
          if (!_parnobuild)
            vecelem.push_back(MomString::make_from_string(namestr));
        }
      return true;
    }
  /* handling objids in chunks */
  else if (pc=='_' && nc<127 && isdigit(nc) && (id=MomIdent::make_from_cstr(curbytes(),&endid))
           && endid!=nullptr)
    {
      std::string idstr(curbytes(), endid - curbytes());
      consume_bytes(endid - curbytes());
      auto vid = _parnobuild?nullptr:chunk_id(id);
      if (vid)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem id=" << id << " = " << vid);
          vecelem.push_back(vid);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem idstr=" << idstr);
          if (!_parnobuild)
            vecelem.push_back(MomString::make_from_string(idstr));
        }
      return true;
    }
  /* $, and $; and $. are all handled as separators */
  else if (pc=='$' && (nc == ',' || nc==';' || nc=='.'))
    {
      consume_utf8(2);
      return true;
    }
  /* handle $<name> as a dollarobj, or else a string */
  else if (pc=='$' && nc<127 && isalpha(nc))
    {
      std::string dollstr;
      consume_utf8(1);
      while ((pc=peek_utf8(0))>0 && pc<127 && (isalnum(pc) || pc=='_'))
        {
          dollstr += (char)pc;
          consume_utf8(1);
        }
      if (_parnobuild)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem dollarname nobuild dollstr=$" << dollstr);
          return true;
        }
      auto pob = fetch_named_object(dollstr);
      MomValue v = pob?chunk_dollarobj(pob):nullptr;
      if (v)
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem dollarname dollar=" << dollstr << " v=" << v);
          vecelem.push_back(v);
        }
      else
        {
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " chunkelem dollarname dollstr=$" << dollstr);
          vecelem.push_back(MomString::make_from_string(std::string{"$"} + dollstr));
        }
      return true;
    }
  /* handle $%<value> as an embedded value */
  else if (pc=='$' && nc=='%')
    {
      bool gotval = false;
      consume_utf8(2);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                         << " chunkelem start embedded value");
      auto embv = parse_value(&gotval, depth+1);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                         << " chunkelem got embedded value" <<(_parnobuild?"!":" ") << embv);
      auto v = _parnobuild?nullptr:chunk_embedded_value(embv);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
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
      while ((pc = peek_utf8(0))>0 && !eol() && !(isalpha(pc) || isspace(pc)))
        {
          nc = peek_utf8(1);
          if (pc=='}' && nc=='#')
            break;
          if (pc=='$')
            {
              if (nc == '$')
                consume_utf8(1);
              else
                break;
            }
          str += (char)pc;
          consume_utf8(1);
        }
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                         << " chunkelem other " << MomShowString(str));
      if (!_parnobuild && !str.empty())
        vecelem.push_back(MomString::make_from_string(str));
      return true;
    }
} // end MomParser::parse_chunk_element


MomIdent
MomParser::parse_id(bool *pgotid)
{
  skip_spaces();
  gunichar pc = 0, nc = 0;
  peek_prevcurr_utf8(pc,nc,1);
  if (pc != '_')
    {
      if (pgotid)
        *pgotid = false;
      return nullptr;
    };
  if (nc == '_')
    {
      consume_utf8(2);
      if (pgotid)
        *pgotid = true;
      return nullptr;
    }
  else if (nc>127 || !isdigit(nc))
    {
      if (pgotid)
        *pgotid = false;
      return nullptr;
    };
  const char* endp = nullptr;
  const char*curp = curbytes();
  auto id = MomIdent::make_from_cstr(curp,&endp);
  if (id.is_null())
    {
      if (pgotid)
        *pgotid = false;
      return nullptr;
    };
  consume_bytes(endp-curp);
  if (pgotid)
    *pgotid = true;
  return id;
} // end MomParser::parse_id


std::string
MomParser::parse_name(bool *pgotname)
{
  skip_spaces();
  gunichar pc = 0, nc = 0;
  peek_prevcurr_utf8(pc,nc,1);
  if (isalpha(pc))
    {
      const char*begnamp = curbytes();
      const char*endnamp = begnamp;
      while (*endnamp<127 && (isalnum(*endnamp) || (*endnamp == '_' && isalnum(endnamp[-1]))))
        endnamp++;
      std::string namstr(begnamp, endnamp-begnamp);
      if (pgotname)
        *pgotname = true;
      consume_bytes(endnamp-begnamp);
      return namstr;
    }
  else if (pgotname)
    *pgotname = false;
  return "";
} // end MomParser::parse_name


MomObject*
MomParser::parse_objptr(bool *pgotob, int depth)
{
  MomObject*respob = nullptr;
  auto inioff = _parlinoffset;
  auto inicolidx = _parcolidx;
  auto inicolpos = _parcolpos;
  auto inilincnt = _parlincount;
  gunichar pc = 0, nc = 0;
again:
  inioff = _parlinoffset;
  inicolidx = _parcolidx;
  inicolpos = _parcolpos;
  inilincnt = _parlincount;
  if (eol())
    {
      skip_spaces();
      if (eof())
        {
          if (pgotob)
            *pgotob = false;
          MOM_DEBUGLOG(parse, "parse_objptr got EOF @" << location_str());
          return nullptr;
        }
      if (eol() && !_parinp) goto failure;
      goto again;
    }
  check_exhaustion();
  peek_prevcurr_utf8(pc,nc,1);
  if ((pc<127 && isspace(pc)) || has_spacing())
    {
      skip_spaces();
      goto again;
    }
  else if (pc=='_' && nc<127 && isdigit(nc))
    {
      bool gotid = false;
      auto id = parse_id(&gotid);
      if (id.is_null() || !gotid)
        goto failure;
      if (!_parnobuild)
        {
          if (_parmakefromid)
            respob = MomObject::make_object_of_id(id);
          else
            respob = MomObject::find_object_of_id(id);
        }
      if (_parnobuild || respob)
        {
          if (pgotob)
            *pgotob = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " objid:" << id);
          if (_parsignalvalue)
            parsed_value_objptr(respob,
                                inioff, inilincnt, inicolpos,
                                _parlinoffset+_parcolidx,
                                _parlincount, _parcolpos,
                                depth);
          return respob;
        }
      else goto failure;
    }
  else if (pc=='_' && nc=='_')
    {
      consume_utf8(2);
      if (pgotob)
        *pgotob = true;
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos << " OBJNIL");
      if (_parsignalvalue)
        parsed_value_null(inioff, inicolpos, inilincnt);
      return nullptr;
    }
  else if (isalpha(pc))
    {
      const char*begnamp = curbytes();
      const char*endnamp = begnamp;
      while (*endnamp<127 && (isalnum(*endnamp) || (*endnamp == '_' && isalnum(endnamp[-1]))))
        endnamp++;
      std::string namstr(begnamp, endnamp-begnamp);
      MomObject* ob = fetch_named_object(namstr);
      MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                         << " namstr=" << MomShowString(namstr)
                         << " NAMEDOB=" << MomShowObject(ob));
      if (ob || _parnobuild)
        {
          consume_bytes(endnamp-begnamp);
          if (pgotob)
            *pgotob = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " namedobj: " << namstr << " = " << ob);
          parsed_value_objptr(ob,
                              inioff, inilincnt, inicolpos,
                              _parlinoffset+_parcolidx,
                              _parlincount, _parcolpos,
                              depth);
          return ob;
        }
      else goto failure;
    }
  // @glob gets the global named glob
  else if (pc=='@' && nc<127 && isalpha(nc))
    {
      const char*begnamp = curbytes();
      const char*endnamp = begnamp+1;
      while (*endnamp<127 && (isalnum(*endnamp) || (*endnamp == '_' && isalnum(endnamp[-1]))))
        endnamp++;
      std::string globnamstr(begnamp, endnamp-begnamp);
      MomObject* ob = nullptr;
      bool foundglobname = false;
      {
        auto globptr = MomRegisterGlobData::find_globdata(globnamstr);
        if (globptr)
          {
            foundglobname = true;
            ob = globptr->load();
          }
      }
      if (ob || (_parnobuild && foundglobname))
        {
          consume_bytes(globnamstr.size() + 1);
          if (pgotob)
            * pgotob = true;
          MOM_THISPARSDBGLOG("L"<< inilincnt << ",C" << inicolpos
                             << " globdata: " << globnamstr << " = " << ob);

          parsed_value_objptr(ob,
                              inioff, inilincnt, inicolpos,
                              _parlinoffset+_parcolidx,
                              _parlincount, _parcolpos,
                              depth);
          return ob;
        }
      else goto failure;
    }
  else if (pc=='$' && nc=='%' && _parobjeval)
    {
      consume_utf8(2);
      bool gotvnod = false;
      auto vnod = MomParser::parse_value(&gotvnod,depth+1);
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
  restore_state(inioff, inilincnt, inicolidx, inicolpos);
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
MomEmitter::emit_objptr(const MomObject*pob, int depth)
{
  if (!pob || skippable_object(pob))
    out() << "__";
  else if (_emwithname)
    {
      std::string obnamstr;
      {
        std::shared_lock<std::shared_mutex> lk(pob->get_shared_mutex());
        obnamstr = mom_get_unsync_string_name(const_cast<MomObject*>(pob));
      }
      if (!obnamstr.empty())
        {
          out() << obnamstr << " |=" << pob->id() << "|";
          emit_space(depth);
        }
      else
        out() << pob->id();
    }
  else
    out() << pob->id();
} // end MomEmitter::emit_objptr


void
MomValue::output(std::ostream& out) const
{
  MomEmitter em(out);
  em.with_name(true);
  em.emit_value(*this);
} // end MomValue::output


MomObject*
MomSimpleParser::simple_named_object(const std::string&nam)
{
  auto pob = mom_find_named(nam.c_str());
  if (!pob)
    MOM_PARSE_FAILURE(this, "name " << MomShowString(nam) << " not found");
  return pob;
} // end MomSimpleParser::simple_named_object

MomValue
MomSimpleParser::simple_chunk_embedded_value(const MomValue v)
{
  ///  make an *embed(v) node
  MomValue res = MomNode::make_from_values(MOMP_embed, v);
  MOM_DEBUGLOG(parse, "simple_chunk_embedded_value v=" << v
               << " @" << location_str()
               << "; res=" << res);
  return res;
} // end MomSimpleParser::simple_chunk_embedded_value

MomValue
MomSimpleParser::simple_chunk_dollarobj(MomObject*pob)
{
  MomValue res;
  MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK, "simple_chunk_dollarobj bad pob");
  // make a *dollar(pob) node
  res = MomNode::make_from_values(MOMP_dollar, pob);
  MOM_DEBUGLOG(parse, "simple_chunk_dollarobj pob=" << MomShowObject(pob)
               << " @" << location_str()
               << "; res=" << res);
  return res;
} // end MomSimpleParser::simple_chunk_dollarobj

MomValue
MomSimpleParser::simple_chunk_value(const std::vector<MomValue>&vec)
{
  MomValue res = MomNode::make_from_vector(MOMP_chunk, vec);
  MOM_DEBUGLOG(parse, "simple_chunk_value vec/" << vec.size()
               << " @" << location_str()
               << "; res=" << res);
  return res;
} // end MomSimpleParser::simple_chunk_value


MomObject*
MomSimpleParser::fetch_named_object(const std::string&nam)
{
  MomObject* pob = nullptr;
  if (_spar_namedfetchfun) pob = _spar_namedfetchfun(this,nam);
  if (!pob) pob = simple_named_object(nam);
  return pob;
} // end MomSimpleParser::fetch_named_object

MomValue
MomSimpleParser::simple_chunk_name(const std::string&name)
{
  auto pob = mom_find_named(name.c_str());
  if (pob)
    return pob;
  return MomString::make_from_string(name);
} // end MomSimpleParser::simple_chunk_name

MomValue
MomSimpleParser::chunk_embedded_value(const MomValue val)
{
  MomValue res = nullptr;
  if (_spar_chunkvalfun)
    res = _spar_chunkvalfun(this,val);
  if (!res)
    res = simple_chunk_embedded_value(val);
  return res;
} // end MomSimpleParser::chunk_embedded_value

MomValue
MomSimpleParser::chunk_name(const std::string&name)
{
  MomValue res;
  if (_spar_chunknamefun)
    res = _spar_chunknamefun(this,name);
  if (!res)
    res = simple_chunk_name(name);
  return res;
} // end MomSimpleParser::simple_chunk_name

MomValue
MomSimpleParser::chunk_dollarobj(MomObject*pob)
{
  MomValue res = nullptr;
  if (_spar_chunkdollarobjfun)
    res = _spar_chunkdollarobjfun(this,pob);
  if (!res)
    res = simple_chunk_dollarobj(pob);
  return res;
} // end MomSimpleParser::chunk_dollarobj

MomValue
MomSimpleParser::chunk_value(const std::vector<MomValue>&vec)
{
  MomValue res=nullptr;
  if (_spar_chunknodefun) res = _spar_chunknodefun(this,vec);
  if (!res) res = simple_chunk_value(vec);
  return res;
} // end MomSimpleParser::chunk_value

