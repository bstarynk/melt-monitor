// file main.cc - main program and utilities

/**   Copyright (C)  2015 - 2017  Basile Starynkevitch and later the FSF
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

bool mom_with_gui;

// libbacktrace from GCC, i.e. libgcc-6-dev package
#include <backtrace.h>
#include <cxxabi.h>

#include <sqlite3.h>

#define BASE_YEAR_MOM 2015


static struct backtrace_state *btstate_mom;
static bool syslogging_mom;
static bool sequential_load_mom = false;
thread_local MomRandom MomRandom::_rand_thr_;

typedef std::function<void(void)> todo_t;
static std::deque<todo_t> todo_after_load_mom;


unsigned mom_debugflags;
unsigned mom_nb_jobs;
mom_atomic_int mom_nb_warnings;

static char hostname_mom[80];

void *mom_prog_dlhandle;

const char *
mom_hostname (void)
{
  return hostname_mom;
};

void
mom_output_gplv3_notice (FILE *out, const char *prefix, const char *suffix,
                         const char *filename)
{
  time_t now = 0;
  time (&now);
  struct tm nowtm;
  memset (&nowtm, 0, sizeof (nowtm));
  localtime_r (&now, &nowtm);
  if (!prefix)
    prefix = "";
  if (!suffix)
    suffix = "";
  fprintf (out, "%s *** generated file %s - DO NOT EDIT %s\n", prefix,
           filename, suffix);
  if (1900 + nowtm.tm_year != BASE_YEAR_MOM)
    fprintf (out,
             "%s Copyright (C) %d - %d Free Software Foundation, Inc. %s\n",
             prefix, BASE_YEAR_MOM, 1900 + nowtm.tm_year, suffix);
  else
    fprintf (out,
             "%s Copyright (C) %d Basile Starynkevitch & later the Free Software Foundation, Inc. %s\n",
             prefix, BASE_YEAR_MOM, suffix);
  fprintf (out,
           "%s MONIMELT is a monitor for MELT - see http://gcc-melt.org/ %s\n",
           prefix, suffix);
  fprintf (out,
           "%s This generated file %s is part of MONIMELT, part of GCC %s\n",
           prefix, filename, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
           "%s GCC is free software; you can redistribute it and/or modify %s\n",
           prefix, suffix);
  fprintf (out,
           "%s it under the terms of the GNU General Public License as published by %s\n",
           prefix, suffix);
  fprintf (out,
           "%s the Free Software Foundation; either version 3, or (at your option) %s\n",
           prefix, suffix);
  fprintf (out, "%s any later version. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
  fprintf (out,
           "%s  GCC is distributed in the hope that it will be useful, %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  but WITHOUT ANY WARRANTY; without even the implied warranty of %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the %s\n",
           prefix, suffix);
  fprintf (out, "%s  GNU General Public License for more details. %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  You should have received a copy of the GNU General Public License %s\n",
           prefix, suffix);
  fprintf (out,
           "%s  along with GCC; see the file COPYING3.   If not see %s\n",
           prefix, suffix);
  fprintf (out, "%s  <http://www.gnu.org/licenses/>. %s\n", prefix, suffix);
  fprintf (out, "%s%s\n", prefix, suffix);
}                               /* end mom_output_gplv3_notice */



struct MomBacktraceData
{
  static constexpr const unsigned _bt_magic = 0x1211fe35 /*303169077*/;
  static constexpr const unsigned _bt_maxdepth = 80;
  unsigned bt_magicnum;
  std::ostringstream bt_outs;
  int bt_count;
  const char*bt_fil;
  int bt_lin;
  MomBacktraceData (const char*fil, int lin) :
    bt_magicnum(_bt_magic), bt_outs(), bt_count(0), bt_fil(fil), bt_lin(lin) {};
  ~MomBacktraceData() = default;
  static int bt_callback(void*data, uintptr_t pc, const char*filnam, int lineno, const char*funcname);
  static void bt_err_callback(void *data, const char *msg, int errnum);
};


/* A callback function passed to the backtrace_full function.  */
int
MomBacktraceData::bt_callback(void*data, uintptr_t pc, const char*filenam, int lineno, const char*funcnam)
{
  auto bt = (MomBacktraceData*)data;
  assert (bt != nullptr && bt->bt_magicnum == _bt_magic);
  /* If we don't have any useful information, don't print
     anything.  */
  if (filenam == nullptr && funcnam == nullptr)
    return 0;
  if (bt->bt_count >= (int)_bt_maxdepth)
    {
      bt->bt_outs << "...etc..." << std::endl;
      return 1;
    }
  bt->bt_count++;
  int demstatus = -1;
  char* demfun = nullptr;
  if (funcnam)
    {
      demfun = abi::__cxa_demangle(funcnam, nullptr, nullptr, &demstatus);
      if (demstatus != 0)
        {
          if (demfun)
            free(demfun);
          demfun = nullptr;
        };
    }
  char pcbuf[32];
  memset(pcbuf, 0, sizeof(pcbuf));
  bt->bt_outs << "MoniMELT["
              << snprintf(pcbuf, sizeof(pcbuf), "%#08lx",
                          (unsigned long)pc)
              << "]/" << bt->bt_count << ' ';
  if (demfun)
    bt->bt_outs << demfun;
  else if (funcnam != nullptr)
    bt->bt_outs << ' ' << funcnam;
  else
    bt->bt_outs << "???";
  if (filenam != nullptr && lineno > 0)
    bt->bt_outs << '\t' << filenam << ':' << lineno;
  bt->bt_outs << std::endl;
  if (demfun)
    {
      free (demfun);
      demfun = nullptr;
    }
  return 0;
} // end MomBacktraceData::bt_callback


/* An error callback function passed to the backtrace_full function.  This is
   called if backtrace_full has an error.  */
void
MomBacktraceData::bt_err_callback(void *data, const char *msg, int errnum)
{
  if (errnum < 0)
    {
      /* This means that no debug info was available.  Just quietly
         skip printing backtrace info.  */
      return;
    }
  auto bt = (MomBacktraceData*)data;
  assert (bt != nullptr && bt->bt_magicnum == _bt_magic);
  bt->bt_outs << "BACKTRACE ERRORED:" << msg;
  if (errnum > 0)
    {
      bt->bt_outs << " (" << errnum << ":" << strerror(errnum) << ")";
    };
  bt->bt_outs << std::endl;
} // end MomBacktraceData::bt_err_callback


void mom_failure_backtrace_at(const char*fil, int lin, const std::string& str)
{
  if (btstate_mom)
    {
      MomBacktraceData backdata(fil,lin);
      backdata.bt_outs << " !!! " << str << std::endl;
      backtrace_full (btstate_mom, 1,
                      MomBacktraceData::bt_callback,
                      MomBacktraceData::bt_err_callback,
                      (void *) &backdata);
      backdata.bt_outs << std::endl;
      mom_warnprintf_at (fil, lin, "FAILURE %s", backdata.bt_outs.str().c_str());
    }
  else
    mom_warnprintf_at (fil, lin, "FAILURE WITHOUT BACKTRACE %s", str.c_str());
} // end mom_failure_backtrace_at


void mom_backtracestr_at (const char*fil, int lin, const std::string&str)
{
  MomBacktraceData backdata(fil,lin);
  backdata.bt_outs << " !!! " << str << std::endl;
  backtrace_full (btstate_mom, 1,
                  MomBacktraceData::bt_callback,
                  MomBacktraceData::bt_err_callback,
                  (void *) &backdata);
  backdata.bt_outs << std::endl;
  mom_informprintf_at (fil, lin, "BACKTRACE %s", backdata.bt_outs.str().c_str());
} // end mom_backtracestr_at

void
MomShowBacktraceAt::output(std::ostream&outs) const
{
  MomBacktraceData backdata(_sb_fil, _sb_lin);
  backdata.bt_outs << " !+! " << _sb_msg << std::endl;
  backtrace_full (btstate_mom, 2,
                  MomBacktraceData::bt_callback,
                  MomBacktraceData::bt_err_callback,
                  (void *) &backdata);
  backdata.bt_outs << std::endl;
  outs << backdata.bt_outs.str();
} // end MomShowBacktraceAt::output


void mom_abort(void)
{
  fflush(NULL);
  abort();
} // end of mom_abort

static struct timespec start_realtime_ts_mom;

double
mom_elapsed_real_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_REALTIME, &curts);
  return 1.0 * (curts.tv_sec - start_realtime_ts_mom.tv_sec)
         + 1.0e-9 * (curts.tv_nsec - start_realtime_ts_mom.tv_nsec);
} // end mom_elapsed_real_time

double
mom_process_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
} // end mom_process_cpu_time

double
mom_thread_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_THREAD_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
} // end mom_thread_cpu_time




static std::string
dbg_level_mom (enum mom_debug_en dbg)
{
#define LEVDBG(Dbg) case momdbg_##Dbg: return #Dbg;
  switch (dbg)
    {
      MOM_DEBUG_LIST_OPTIONS (LEVDBG);
    default:
    {
      char dbglev[16];
      memset (dbglev, 0, sizeof(dbglev));
      snprintf (dbglev, sizeof (dbglev), "?DBG?%d", (int) dbg);
      return std::string(dbglev);
    }
    }
#undef LEVDBG
}


void
mom_output_utf8_escaped (FILE *f, const char *str, int len,
                         mom_utf8escape_sig_t * rout, void *clientdata)
{
  if (!f)
    return;
  if (!str)
    return;
  assert (rout != nullptr);
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, nullptr));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case 0:
          (*rout) (f, uc, "\\0", clientdata);
          break;
        case '\"':
          (*rout) (f, uc, "\\\"", clientdata);
          break;
        case '\'':
          (*rout) (f, uc, "\\\'", clientdata);
          break;
        case '&':
          (*rout) (f, uc, "&", clientdata);
          break;
        case '<':
          (*rout) (f, uc, "<", clientdata);
          break;
        case '>':
          (*rout) (f, uc, ">", clientdata);
          break;
          break;
        case '\a':
          (*rout) (f, uc, "\\a", clientdata);
          break;
        case '\b':
          (*rout) (f, uc, "\\b", clientdata);
          break;
        case '\f':
          (*rout) (f, uc, "\\f", clientdata);
          break;
        case '\n':
          (*rout) (f, uc, "\\n", clientdata);
          break;
        case '\r':
          (*rout) (f, uc, "\\r", clientdata);
          break;
        case '\t':
          (*rout) (f, uc, "\\t", clientdata);
          break;
        case '\v':
          (*rout) (f, uc, "\\v", clientdata);
          break;
        case '\033' /*ESCAPE*/:
          (*rout) (f, uc, "\\e", clientdata);
          break;
        case '0' ... '9':
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '+':
        case '-':
        case '*':
        case '/':
        case ',':
        case ';':
        case '.':
        case ':':
        case '^':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
        case ' ':
          goto print1char;
        default:
          if (uc < 127 && uc > 0 && isprint ((char) uc))
print1char:
            fputc ((char) uc, f);
          else
            {
              char buf[16];
              memset (buf, 0, sizeof (buf));
              if (uc <= 0xffff)
                {
                  snprintf (buf, sizeof (buf), "\\u%04x", (int) uc);
                }
              else
                {
                  snprintf (buf, sizeof (buf), "\\U%08x", (int) uc);
                }
              (*rout) (f, uc, buf, clientdata);
            }
          break;
        }
    };
}                               /* end of mom_output_utf8_escaped */


momhash_t
mom_cstring_hash_len (const char *str, int len)
{
  if (!str)
    return 0;
  if (len < 0)
    len = strlen (str);
  int l = len;
  momhash_t h1 = 0, h2 = len, h;
  while (l > 4)
    {
      h1 =
        (509 * h2 +
         307 * ((signed char *) str)[0]) ^ (1319 * ((signed char *) str)[1]);
      h2 =
        (17 * l + 5 + 5309 * h2) ^ ((3313 * ((signed char *) str)[2]) +
                                    9337 * ((signed char *) str)[3] + 517);
      l -= 4;
      str += 4;
    }
  if (l > 0)
    {
      h1 = (h1 * 7703) ^ (503 * ((signed char *) str)[0]);
      if (l > 1)
        {
          h2 = (h2 * 7717) ^ (509 * ((signed char *) str)[1]);
          if (l > 2)
            {
              h1 = (h1 * 9323) ^ (11 + 523 * ((signed char *) str)[2]);
              if (l > 3)
                {
                  h2 =
                    (h2 * 7727 + 127) ^ (313 +
                                         547 * ((signed char *) str)[3]);
                }
            }
        }
    }
  h = (h1 * 29311 + 59) ^ (h2 * 7321 + 120501);
  if (!h)
    {
      h = h1;
      if (!h)
        {
          h = h2;
          if (!h)
            h = (len & 0xffffff) + 11;
        }
    }
  return h;
}                               // end mom_cstring_hash_len



void
mom_output_utf8_encoded (FILE *f, const char *str, int len)
{
  if (!f)
    return;
  if (!str)
    {
      fputs("*nullstring*", f);
      MOM_WARNLOG("nil string to mom_output_utf8_encoded with len=" << len
                  << MOM_SHOW_BACKTRACE("nil mom_output_utf8_encoded"));
      return;
    };
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  MOM_ASSERT (s && g_utf8_validate (s, len, nullptr),
              "mom_output_utf8_encoded invalid str=" << (void*)str
              << "::" << (s?s:""));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      /// notice that the single quote should not be escaped, e.g. for JSON
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case 0:
          fputs ("\\0", f);
          break;
        case '\"':
          fputs ("\\\"", f);
          break;
        case '\\':
          fputs ("\\\\", f);
          break;
        case '\a':
          fputs ("\\a", f);
          break;
        case '\b':
          fputs ("\\b", f);
          break;
        case '\f':
          fputs ("\\f", f);
          break;
        case '\n':
          fputs ("\\n", f);
          break;
        case '\r':
          fputs ("\\r", f);
          break;
        case '\t':
          fputs ("\\t", f);
          break;
        case '\v':
          fputs ("\\v", f);
          break;
        case '\033' /*ESCAPE*/:
          fputs ("\\e", f);
          break;
        default:
          if (uc < 127 && uc > 0 && isprint ((char) uc))
            fputc ((char) uc, f);
          else
            {
              if (uc <= 0xffff)
                {
                  fprintf (f, "\\u%04x", (int) uc);
                }
              else
                {
                  fprintf (f, "\\U%08x", (int) uc);
                }
            }
          break;
        }
    }
}                               // end mom_output_utf8_encoded


void
MomShowString::output(std::ostream&os) const
{
  char *buf = nullptr;
  size_t siz = 0;
  if (_shnil)
    {
      os << "*nullstr*";
      return;
    }
  FILE *f = open_memstream(&buf,&siz);
  if (MOM_UNLIKELY(!f))
    MOM_FATAPRINTF("MomShowString::output open_memstream failure %m");
  mom_output_utf8_encoded (f, _shstr.c_str(), _shstr.size());
  fputc(0,f);
  fflush(f);
  os << '"' << buf << '"';
  free (buf);
} // end MomShowString::output

void
mom_output_utf8_html (FILE *f, const char *str, int len, bool nlisbr)
{
  if (!f)
    return;
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, nullptr));
  for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
    {
      uc = g_utf8_get_char (pc);
      switch (uc)
        {
        case '\'':
          fputs ("&apos;", f);
          break;
        case '\"':
          fputs ("&quot;", f);
          break;
        case '<':
          fputs ("&lt;", f);
          break;
        case '>':
          fputs ("&gt;", f);
          break;
        case '&':
          fputs ("&amp;", f);
          break;
        case '\n':
          if (nlisbr)
            fputs ("<br/>", f);
          else
            fputc ('\n', f);
          break;
        case ' ':
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case '+':
        case '-':
        case '*':
        case '/':
        case ',':
        case ';':
        case '.':
        case ':':
        case '^':
        case '(':
        case ')':
        case '[':
        case ']':
        case '{':
        case '}':
          fputc ((char) uc, f);
          break;
        default:
          if (uc < 127 && isprint (uc))
            fputc ((char) uc, f);
          else
            fprintf (f, "&#%d;", uc);
          break;
        }
    }
}                               /* end of mom_output_utf8_html */


const char *
mom_hexdump_data (char *buf, unsigned buflen, const unsigned char *data,
                  unsigned datalen)
{
  if (!buf || !data)
    return nullptr;
  if (2 * datalen + 3 < buflen)
    {
      for (unsigned ix = 0; ix < datalen; ix++)
        snprintf (buf + 2 * ix, 3, "%02x", (unsigned) data[ix]);
    }
  else
    {
      unsigned maxln = (buflen - 3) / 2;
      if (maxln > datalen)
        maxln = datalen;
      for (unsigned ix = 0; ix < maxln; ix++)
        snprintf (buf + 2 * ix, 3, "%02x", (unsigned) data[ix]);
      if (maxln > datalen)
        strcpy (buf + 2 * maxln, "..");
    }
  return buf;
}                               /* end of mom_hexdump_data */



std::string
mom_input_quoted_utf8_string(std::istream&ins)
{
  if (!ins)
    return std::string{""};
  std::string restr;
  auto inipos = ins.tellg();
  constexpr const int roundsizelog = 5;
  constexpr const int roundsize = 1 << roundsizelog;
  int cnt = 0;
  do
    {
      bool failed = false;
      char bufzon[8];
      memset(bufzon, 0, sizeof(bufzon));
      if (MOM_UNLIKELY(cnt % roundsize == 0))
        {
          restr.reserve(((9 * restr.size() / 8) | (roundsize - 1))+1);
        }
      if (MOM_UNLIKELY (restr.size() > INT32_MAX / 3))
        {
          ins.seekg(inipos);
          MOM_FAILURE("too long string " << restr.size());
        }
      if (ins.eof())
        break;
      int c = ins.get();
      if (iscntrl (c) || c == '\'' || c == '"')
        {
          ins.unget();
          break;
        }
      else if (c==EOF)
        break;
      else if (c == '\\')
        {
          char inpzon[16];
          memset (inpzon, 0, sizeof(inpzon));
          int pos = -1;
          unsigned b = 0;
          int nc = ins.get ();
          if (ins.eof() || iscntrl (nc))
            {
              ins.unget();
              break;
            }
          switch (nc)
            {
            case '\'':
            case '\"':
            case '\\':
              bufzon[0] = nc;
              break;
            case 'a':
              bufzon[0] = '\a';
              break;
            case 'b':
              bufzon[0] = '\b';
              break;
            case 'f':
              bufzon[0] = '\f';
              break;
            case 'n':
              bufzon[0] = '\n';
              break;
            case 'r':
              bufzon[0] = '\r';
              break;
            case 't':
              bufzon[0] = '\t';
              break;
            case 'v':
              bufzon[0] = '\v';
              break;
            case 'e':
              bufzon[0] = '\033' /* ESCAPE */ ;
              break;
            case 'x':
            {
              ins.read(inpzon, 2);
              if (ins.eof())
                failed = true;
              else if (sscanf (inpzon, "%02x%n", &b, &pos) > 0 && pos > 0)
                bufzon[0] = b;
              else failed = true;
            }
            break;
            case 'u':
            {
              ins.read(inpzon, 4);
              if (ins.eof())
                failed = true;
              else if (sscanf (inpzon, "%04x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon, ebuf);
                }
              else failed = true;
            }
            break;
            case 'U':
            {
              ins.read(inpzon, 8);
              if (ins.eof())
                failed = true;
              else if (sscanf (inpzon, "%08x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon, ebuf);
                };
            }
            break;
            default:
              bufzon[0] = nc;
              break;
            }
        }
      else // ordinary character
        {
          bufzon[0] = c;
        }
      if (failed)
        {
          {
            ins.seekg(inipos);
            MOM_FAILURE("bad string");
          }
        }
      else restr.append(bufzon);
    }
  while (!ins.eof());
  return restr;
}                               /* end of mom_input_quoted_utf8 */


static pthread_mutex_t dbgmtx_mom = PTHREAD_MUTEX_INITIALIZER;
void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
                    const char *fmt, ...)
{
  static long countdbg;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  int len = 0;
  char *msg = nullptr;
  char *bigbuf = nullptr;
  memset (thrname, 0, sizeof (thrname));
  memset (buf, 0, sizeof (buf));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (nullptr);
  mom_now_strftime_bufcenti (timbuf, "%H:%M:%S.__ ");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      char *bigbuf = static_cast<char*>(malloc (len + 10));
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;
  {
    pthread_mutex_lock (&dbgmtx_mom);
    long nbdbg = countdbg++;
#define DEBUG_DATE_PERIOD_MOM 64
    char datebuf[48] = { 0 };
    if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
      {
        mom_now_strftime_bufcenti (datebuf, "%Y-%b-%d@%H:%M:%S.__ %Z");
      };
    if (syslogging_mom)
      {
        syslog (LOG_DEBUG, "MONIMELT DEBUG %7s <%s:%d> @%s:%d %s %s",
                dbg_level_mom (dbg).c_str(), thrname, (int)mom_gettid(), fil, lin, timbuf, msg);
        if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
          syslog (LOG_DEBUG, "MONIMELT DEBUG#%04ld ~ %s *^*^*", nbdbg,
                  datebuf);
      }
    else
      {
        fprintf (stderr, "MONIMELT DEBUG %7s <%s:%d> @%s:%d %s %s\n",
                 dbg_level_mom (dbg).c_str(), thrname, (int)mom_gettid(), fil, lin, timbuf, msg);
        fflush (stderr);
        if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
          fprintf (stderr, "MONIMELT DEBUG#%04ld ~ %s *^*^*\n", nbdbg,
                   datebuf);
        fflush (nullptr);
      }
    pthread_mutex_unlock (&dbgmtx_mom);
  }
  if (bigbuf)
    free (bigbuf);
}


/************************* inform *************************/

void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  char *bigbuf = nullptr;
  char *msg = nullptr;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (nullptr);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      bigbuf = static_cast<char*>(malloc (len + 10));
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;
  if (syslogging_mom)
    {
      syslog (LOG_INFO, "MONIMELT INFORM @%s:%d <%s:%d> %s %s",
              fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      fprintf (stderr, "MONIMELT INFORM @%s:%d <%s:%d> %s %s\n",
               fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (nullptr);
    }
  if (bigbuf)
    free (bigbuf);
}

/************************* warning *************************/

void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  char *bigbuf = nullptr;
  char *msg = nullptr;
  int err = errno;
  int nbwarn = 1 + atomic_fetch_add (&mom_nb_warnings, 1);
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (nullptr);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 2))
    {
      bigbuf = static_cast<char*>(malloc (len + 10));
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;
  if (syslogging_mom)
    {
      if (err)
        syslog (LOG_WARNING, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s (%s)",
                nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf,
                msg, strerror (err));
      else
        syslog (LOG_WARNING, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s",
                nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      if (err)
        fprintf (stderr, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s (%s)\n",
                 nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf,
                 msg, strerror (err));
      else
        fprintf (stderr, "MONIMELT WARNING#%d @%s:%d <%s:%d> %s %s\n",
                 nbwarn, fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (nullptr);
    }
  if (bigbuf)
    free (bigbuf);
}                               /* end of mom_warnprintf_at */


/************************* fatal *************************/
void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
{
  int len = 0;
  char thrname[24];
  char buf[256];
  char timbuf[64];
  char *bigbuf = nullptr;
  char *msg = nullptr;
  int err = errno;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  fflush (nullptr);
  va_list alist;
  va_start (alist, fmt);
  len = vsnprintf (buf, sizeof (buf), fmt, alist);
  va_end (alist);
  if (MOM_UNLIKELY (len >= (int) sizeof (buf) - 1))
    {
      bigbuf = static_cast<char*>(malloc (len + 10));
      if (bigbuf)
        {
          memset (bigbuf, 0, len + 10);
          va_start (alist, fmt);
          (void) vsnprintf (bigbuf, len + 1, fmt, alist);
          va_end (alist);
          msg = bigbuf;
        }
    }
  else
    msg = buf;
  if (syslogging_mom)
    {
      if (err)
        syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s (%s)",
                fil, lin, thrname, (int) mom_gettid (), timbuf,
                msg, strerror (err));
      else
        syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s",
                fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
    }
  else
    {
      if (err)
        fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s (%s)\n",
                 fil, lin, thrname, (int) mom_gettid (), timbuf,
                 msg, strerror (err));
      else
        fprintf (stderr, "MONIMELT FATAL @%s:%d <%s:%d> %s %s\n",
                 fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      fflush (nullptr);
    }
  MomBacktraceData backdata(fil,lin);
  backdata.bt_outs << " !*!*! Monimelt FATAL !*!*!* " << msg << std::endl;
  backtrace_full (btstate_mom, 1,
                  MomBacktraceData::bt_callback,
                  MomBacktraceData::bt_err_callback,
                  (void *) &backdata);
  backdata.bt_outs << std::endl;
  if (syslogging_mom)
    {
      syslog (LOG_ALERT, "MONIMELT FATALBACKTRACE\n\t %s\n",
              backdata.bt_outs.str().c_str());
    }
  else
    {
      fprintf (stderr, "MONIMELT FATALBACKTRACE\n\t %s\n",
               backdata.bt_outs.str().c_str());
    }
  /* no need to free bigbuf, we are aborting! */
  mom_abort ();
}





char *
mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
{
  struct tm tm;
  time_t tim = (time_t) ti;
  memset (&tm, 0, sizeof (tm));
  if (!buf || !fmt || !len)
    return nullptr;
  strftime (buf, len, fmt, localtime_r (&tim, &tm));
  char *dotundund = strstr (buf, ".__");
  if (dotundund)
    {
      double ind = 0.0;
      double fra = modf (ti, &ind);
      char minibuf[16];
      memset (minibuf, 0, sizeof (minibuf));
      snprintf (minibuf, sizeof (minibuf), "%.02f", fra);
      strncpy (dotundund, strchr (minibuf, '.'), 3);
    }
  return buf;
}


static void
parse_file_of_commands_mom(const std::string& path)
{
  std::ifstream insf{path};
  MomSimpleParser pars(insf);
  MomObject *pobfocus = nullptr;
  MOM_DEBUGLOG(parse, "start parse_file_of_commands_mom path=" << path);
  pars
  .set_name(path)
  .set_make_from_id(true)
  .set_debug(MOM_IS_DEBUGGING(parse))
  .set_has_chunk(true)
  .disable_exhaustion();
  pars
  .set_getfocusedobjectfun([&](MomSimpleParser*)
  {
    return pobfocus;
  })
  .set_putfocusedobjectfun
  ([&](MomSimpleParser*theparser,MomObject*pob)
  {
    MOM_DEBUGLOG(parse, "setting focus to pob=" << MomShowObject(pob)
                 << " @" << theparser->location_str());
    pobfocus = pob;
  })
  .set_updatedfocusedobjectfun([&](MomSimpleParser*theparser,MomObject*pob)
  {
    MOM_DEBUGLOG(parse, "touching updated pob=" << MomShowObject(pob)
                 << " @" << theparser->location_str());
    pob->touch();
  })
  ;
  pars.next_line();
  pars.skip_spaces();
  MOM_DEBUGLOG(parse, "start of parse-command-file @"
               << pars.location_str());
  int nbcommands = 0;
  for (;;)
    {
      MOM_DEBUGLOG(parse, "parse_file_of_commands_mom ***start loop nbcommands=" << nbcommands
                   << " @" << pars.location_str() <<std::endl);
      pars.skip_spaces();
      MOM_DEBUGLOG(parse, "parse_file_of_commands_mom loop#" << nbcommands
                   << " after skip_spaces @" << pars.location_str()
                   << ' ' << (pars.eof()?"eof":"noeof")
                   << ' ' << (pars.eol()?"eol":"noeol"));
      if (pars.eof())
        break;
      bool gotcommand=false;
      MOM_DEBUGLOG(parse, "parse_file_of_commands_mom before parse_command @" << pars.location_str()
                   << " " << MomShowString(pars.curbytes())
                   << " nbcommands=" << nbcommands);
      pars.parse_command(&gotcommand);
      MOM_DEBUGLOG(parse, "parse_file_of_commands_mom after parse_command @" << pars.location_str()
                   << " " << MomShowString(pars.curbytes())
                   << " nbcommands=" << nbcommands
                   << " gotcommand=" << (gotcommand?"true":"false")
                   << ' ' << (pars.eof()?"eof":"noeof"));
      if (!gotcommand)
        {
          MOM_DEBUGLOG(parse, "parse_command failed @" << pars.location_str()
                       << ' ' << (pars.eof()?"eof":"noeof"));
          MOM_WARNLOG("failed to parse command @" << pars.location_str());
          break;
        }
      nbcommands++;
      MOM_DEBUGLOG(parse, "after parsing command#" << nbcommands << " @" << pars.location_str()
                   << ' ' << (pars.eof()?"eof":"noeof")
                   << ' ' << (pars.eol()?"eol":"noeol")
                   << std::endl);
    }
  MOM_DEBUGLOG(parse, "parse_file_of_commands_mom ending path=" << path << " @" << pars.location_str()
               << ' ' << (pars.eof()?"eof":"noeof")
               << ' ' << (pars.eol()?"eol":"noeol")
               << std::endl);
  MOM_INFORMPRINTF("parse %d commands from %s", nbcommands, path.c_str());
} // end parse_file_of_commands_mom



static void
parse_file_of_values_mom(const std::string& path)
{
  std::ifstream insf{path};
  MomSimpleParser pars(insf);
  pars
  .set_name(path)
  .set_make_from_id(true)
  .set_debug(MOM_IS_DEBUGGING(parse))
  .set_has_chunk(true)
  .disable_exhaustion();
  pars.skip_spaces();
  MOM_DEBUGLOG(parse, "start of parse-file @"
               << pars.location_str());
  MOM_INFORMLOG("parse-file '" << path << "'" << std::endl
                << "peek_utf8(0)=" << pars.peek_utf8(0) << ' '
                << "peek_utf8(1)=" << pars.peek_utf8(1) << ' '
                << "peek_utf8(2)=" << pars.peek_utf8(2) << ' '
                << "peek_utf8(3)=" << pars.peek_utf8(3) << ' '
                << "peek_utf8(4)=" << pars.peek_utf8(4) << std::endl);
  int parsecnt = 0;
  for (;;)
    {
      pars.skip_spaces();
      MOM_DEBUGLOG(parse, "parse-file @" << pars.location_str());
      bool gotval = false;
      std::string locstr = pars.location_str();
      auto val = pars.parse_value(&gotval);
      if (!gotval || pars.eof())
        break;
      parsecnt++;
      pars.skip_spaces();
      MOM_INFORMLOG("parse-file " << (gotval?"with":"without") << " value=" << val
                    << std::endl << "...@ " << pars.location_str()
                    << std::endl << "/// raw val#" << parsecnt << " @" << locstr
                    << std::endl
                    << MomDoShow([&](std::ostream&os)
      {
        MomEmitter rawem(os);
        rawem.emit_value(val);
      })
          << std::endl);
    }
  if (pars.eof())
    MOM_INFORMLOG("parse-file eof at " << pars.location_str() << std::endl);
} // end parse_file_of_values_mom


extern "C" const char *const mom_debug_names[momdbg__last] =
{
#ifdef __clang__
#define DEFINE_DBG_NAME_MOM(Dbg) [momdbg_##Dbg]= #Dbg,
  MOM_DEBUG_LIST_OPTIONS (DEFINE_DBG_NAME_MOM)
#else
#define   DEFINE_DBG_NAME_MOM(Dbg) #Dbg,
  nullptr,
  MOM_DEBUG_LIST_OPTIONS (DEFINE_DBG_NAME_MOM)
#endif
};

#undef DEFINE_DBG_NAME_MOM


/* Option specification for getopt_long.  */
enum extraopt_en
{
  xtraopt__none = 0,
  xtraopt_chdir_first = 1024,
  xtraopt_chdir_after_load,
  xtraopt_addpredef,
  xtraopt_commentpredef,
  xtraopt_testid,
  xtraopt_parseid,
  xtraopt_parseval,
  xtraopt_parsefileofvalues,
  xtraopt_parsefileofcommands,
  xtraopt_runcmd,
  xtraopt_loadsequential,
  xtraopt_loadspyid1,
  xtraopt_loadspyid2,
};

static const struct option mom_long_options[] =
{
  {"help", no_argument, nullptr, 'h'},
  {"version", no_argument, nullptr, 'V'},
  {"debug", required_argument, nullptr, 'D'},
  {"dump", required_argument, nullptr, 'd'},
  {"load", required_argument, nullptr, 'L'},
  {"jobs", required_argument, nullptr, 'J'},
  {"web", required_argument, nullptr, 'W'},
  {"chdir-first", required_argument, nullptr, xtraopt_chdir_first},
  {"chdir-after-load", required_argument, nullptr, xtraopt_chdir_after_load},
  {"add-predefined", required_argument, nullptr, xtraopt_addpredef},
  {"comment-predefined", required_argument, nullptr, xtraopt_commentpredef},
  {"load-sequential", required_argument, nullptr, xtraopt_loadsequential},
  {"test-id", no_argument, nullptr, xtraopt_testid},
  {"parse-id", required_argument, nullptr, xtraopt_parseid},
  {"parse-val", required_argument, nullptr, xtraopt_parseval},
  {"parse-file", required_argument, nullptr, xtraopt_parsefileofvalues},
  {"parse-command-file", required_argument, nullptr, xtraopt_parsefileofcommands},
  {"run-cmd", required_argument, nullptr, xtraopt_runcmd},
  {"load-spy-id1", required_argument, nullptr, xtraopt_loadspyid1},
  {"load-spy-id2", required_argument, nullptr, xtraopt_loadspyid2},
  /* Terminating nullptr placeholder.  */
  {nullptr, no_argument, nullptr, 0},
};

static void
usage_mom (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t -J | --jobs " " <nb-jobs> \t# Give number of jobs (working threads), default %u.\n", mom_nb_jobs);
  printf ("\t -D | --debug <debug-features>"
          " \t# Debugging comma separated features\n\t\t##");
  for (unsigned ix = 1; ix < momdbg__last; ix++)
    printf (" %s", mom_debug_names[ix]);
  putchar ('\n');
  printf ("\t -d | --dump <dumpdir>" " \t# Dump the state.\n");
  printf ("\t -L | --load <loaddir>" " \t# Load the state.\n");
  printf ("\t -G | --gui" "\t# Run the GTK GUI\n");
  printf ("\t --chdir-first dirpath" " \t#Change directory at first \n");
  printf ("\t --chdir-after-load dirpath"
          " \t#Change directory after load\n");
  printf ("\t --add-predefined predefname" " \t#Add a predefined\n");
  printf ("\t --comment-predefined comment"
          " \t#Set comment of next predefined\n");
  printf ("\t --test-id" " \t#generate a few random ids\n");
  printf ("\t --parse-id id" " \t#parse an id\n");
  printf ("\t --parse-val <val>" " \t#parse some value after load\n");
  printf ("\t --parse-file <file-path>" " \t#parse several values from file after load\n");
  printf ("\t --parse-command-file <file-path>" " \t#parse several commands from file after load\n");
  printf ("\t --run-cmd <command>" " \t#run that shell command\n");
  printf ("\t --load-sequential <loaddir>" " \t# Load the state sequentially (no threads).\n");
  printf ("\t --load-spy-id1 <id>" " \t# Set the id1 for spying the loader.\n");
  printf ("\t --load-spy-id2 <id>" " \t# Set the id2 for spying the loader.\n");
} // end usage_mom


static void
print_version_mom (const char *argv0)
{
  printf ("%s built on %s gitcommit %s\n", argv0,
          monimelt_timestamp, monimelt_lastgitcommit);
}

void
mom_set_debugging (const char *dbgopt)
{
  char dbuf[256];
  if (!dbgopt)
    return;
  memset (dbuf, 0, sizeof (dbuf));
  if (strlen (dbgopt) >= sizeof (dbuf) - 1)
    MOM_FATAPRINTF ("too long debug option %s", dbgopt);
  strcpy (dbuf, dbgopt);
  char *comma = nullptr;
  if (!strcmp (dbuf, ".") || !strcmp (dbuf, "_"))
    {
      mom_debugflags = ~0;
      MOM_INFORMPRINTF ("set all debugging");
    }
  else
    for (char *pc = dbuf; pc != nullptr; pc = comma ? comma + 1 : nullptr)
      {
        comma = strchr (pc, ',');
        if (comma)
          *comma = (char) 0;
#define MOM_TEST_DEBUG_OPTION(Nam)			\
	if (!strcmp(pc,#Nam))		{		\
	  mom_debugflags |=  (1<<momdbg_##Nam); } else	\
	  if (!strcmp(pc,"!"#Nam))			\
	    mom_debugflags &=  ~(1<<momdbg_##Nam); else
        if (!pc)
          break;
        MOM_DEBUG_LIST_OPTIONS (MOM_TEST_DEBUG_OPTION) if (pc && *pc)
          MOM_WARNPRINTF ("unrecognized debug flag %s", pc);
      }
  char alldebugflags[2 * sizeof (dbuf) + 120];
  memset (alldebugflags, 0, sizeof (alldebugflags));
  int nbdbg = 0;
#define MOM_SHOW_DEBUG_OPTION(Nam) do {		\
    if (mom_debugflags & (1<<momdbg_##Nam)) {	\
     strcat(alldebugflags, " " #Nam);		\
     assert (strlen(alldebugflags)		\
	     <sizeof(alldebugflags)-3);		\
     nbdbg++;					\
    } } while(0);
  MOM_DEBUG_LIST_OPTIONS (MOM_SHOW_DEBUG_OPTION);
  if (nbdbg > 0)
    MOM_INFORMPRINTF ("%d debug flags active:%s.", nbdbg, alldebugflags);
  else
    MOM_INFORMPRINTF ("no debug flags active.");
}


static void create_predefined_mom(std::string nam, std::string comment)
{
#warning unimplemented create_predefined_mom
  MOM_FATAPRINTF("unimplemented create_predefined_mom nam:%s comment:%s",
                 nam.c_str(), comment.c_str());
} // end of create_predefined_mom


void
parse_program_arguments_mom (int *pargc, char ***pargv)
{
  int argc = *pargc;
  char **argv = *pargv;
  int opt = -1;
  char *commentstr = nullptr;
  int myargindex = 0;
  int nbparsval = 0;
  while ((opt = getopt_long (argc, argv, "hVGd:sD:L:J:",
                             mom_long_options, &myargindex)) >= 0)
    {
      std::string pstr;
      switch (opt)
        {
        case 'h':              /* --help */
          usage_mom (argv[0]);
          putchar ('\n');
          fputs ("\nVersion info:::::\n", stdout);
          print_version_mom (argv[0]);
          exit (EXIT_FAILURE);
          return;
        case 'V':              /* --version */
          print_version_mom (argv[0]);
          exit (EXIT_SUCCESS);
          return;
        case 'd':              /* --dump */
          mom_dump_dir = optarg;
          break;
        case 'D':              /* --debug debugopt */
          mom_set_debugging (optarg);
          break;
        case 'J':		// --jobs nb-jobs
          if (optarg && isdigit(optarg[0]))
            {
              mom_nb_jobs = atoi(optarg);
              if (mom_nb_jobs<MOM_MIN_JOBS)
                mom_nb_jobs = MOM_MIN_JOBS;
              if (mom_nb_jobs>MOM_MAX_JOBS)
                mom_nb_jobs = MOM_MAX_JOBS;
            }
          MOM_INFORMPRINTF("with %d jobs (working threads)", mom_nb_jobs);
          break;
        case 'G': /* --gui */
          mom_with_gui = true;
          break;
        case 'L': /* --load filepath */
          if (!optarg || access (optarg, R_OK))
            MOM_FATAPRINTF ("bad load state %s : %m", optarg);
          mom_load_dir = optarg;
          break;
        case xtraopt_loadsequential: /* --load-sequential filepath */
          if (!optarg || access (optarg, R_OK))
            MOM_FATAPRINTF ("bad load sequential state %s : %m", optarg);
          mom_load_dir = optarg;
          MOM_INFORMPRINTF("will load sequentially from %s", mom_load_dir);
          sequential_load_mom = true;
          break;
        case xtraopt_commentpredef: /* --comment-predefined comment */
          if (optarg)
            commentstr=optarg;
          break;
        case xtraopt_addpredef: /* --add-predefined name */
          if (optarg)
            {
              if (!mom_valid_name_radix_len(optarg,strlen(optarg)))
                MOM_FATAPRINTF ("invalid predefined name %s", optarg);
              std::string namestr {optarg};
              std::string commstr {commentstr};
              todo_after_load_mom.push_back([=](void)
              {
                create_predefined_mom(namestr,commstr);
              });
              commentstr=nullptr;
              if (!mom_dump_dir)
                MOM_WARNPRINTF("add predefined %s without dumping!", optarg);
            }
          else
            MOM_FATAPRINTF("--add-predefined option requires a valid item name");
          break;
        case xtraopt_chdir_first: /* --chdir-first dirname */
          if (optarg)
            {
              if (chdir(optarg))
                MOM_FATAPRINTF("failed to --chdir-first %s (%m)", optarg);
              else
                {
                  char*cwd = get_current_dir_name();
                  MOM_INFORMPRINTF("changed directory at first to %s",
                                   cwd?cwd:".");
                  free (cwd);
                }
            }
          else
            MOM_FATAPRINTF("missing argument to --chdir-first");
          break;
        case xtraopt_runcmd: /* --run-cmd command */
          if (optarg)
            {
              MOM_INFORMPRINTF("running command: %s\n", optarg);
              fflush(nullptr);
              int cmdres = system(optarg);
              if (cmdres)
                MOM_WARNPRINTF("command '%s' failed with %d", optarg, cmdres);
              else
                MOM_INFORMPRINTF("command '%s' run successfully", optarg);
            }
          else
            MOM_FATAPRINTF("missing argument to --run-cmd");
          break;
        case xtraopt_chdir_after_load:
          if (optarg != nullptr)
            {
              if (access(optarg,F_OK))
                MOM_WARNPRINTF("cannot access %s for --chdir-after-load (%m)",
                               optarg);
              std::string dirstr {optarg};
              todo_after_load_mom.push_back
              ([=](void)
              {
                if (chdir(dirstr.c_str()))
                  MOM_FATAPRINTF("failed to --chdir-after-load %s (%m)", dirstr.c_str());
                else
                  {
                    char*cwd = get_current_dir_name();
                    MOM_INFORMPRINTF("changed directory after load to %s",
                                     cwd?cwd:".");
                    free (cwd);
                  }
              });
            }
          else
            MOM_FATAPRINTF("missing argument to --chdir-after-load");
          break;
        case xtraopt_testid:
        {
          auto id1 = MomIdent::make_random();
          auto id2 = MomIdent::make_random();
          auto id3 = MomIdent::make_random();
          auto id4 = MomIdent::make_random();
          auto id5 = MomIdent::make_random();
          auto id6 = MomIdent::make_random();
          auto id7 = MomIdent::make_random();
          auto id8 = MomIdent::make_random();
          MOM_INFORMLOG("test-id __cplusplus=" << __cplusplus
                        << " sizeof(MomObject)=" << sizeof(MomObject)
                        << " sizeof(MomAnyVal)=" << sizeof(MomAnyVal)
                        << " sizeof(MomTuple)=" << sizeof(MomTuple)
                        << " sizeof(MomNode)=" << sizeof(MomNode));
          MOM_INFORMLOG("test-id hardware_concurrency=" << std::thread::hardware_concurrency());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id1= " << id1 << " =(" << id1.hi().serial() << "," << id1.lo().serial()
                        << ")/h" << id1.hash() << ",b#" << id1.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id2= " << id2 << " =(" << id2.hi().serial() << "," << id2.lo().serial()
                        << ")/h" << id2.hash() << ",b#" << id2.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id3= " << id3 << " =(" << id3.hi().serial() << "," << id3.lo().serial()
                        << ")/h" << id3.hash() << ",b#" << id3.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id4= " << id4 << " =(" << id4.hi().serial() << "," << id4.lo().serial()
                        << ")/h" << id4.hash() << ",b#" << id4.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id5= " << id5 << " =(" << id5.hi().serial() << "," << id5.lo().serial()
                        << ")/h" << id5.hash() << ",b#" << id5.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id6= " << id6 << " =(" << id6.hi().serial() << "," << id6.lo().serial()
                        << ")/h" << id6.hash() << ",b#" << id6.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id7= " << id7 << " =(" << id7.hi().serial() << "," << id7.lo().serial()
                        << ")/h" << id7.hash() << ",b#" << id7.bucketnum());
          MOM_INFORMLOG("test-id:" << std::endl
                        << " .. id8= " << id8 << " =(" << id8.hi().serial() << "," << id8.lo().serial()
                        << ")/h" << id8.hash() << ",b#" << id8.bucketnum());
          MOM_INFORMLOG("test-id all "  << std::endl
                        << "... " << id1 << " " << id2 << " " << id3 << std::endl
                        << "... " << id4 << " " << id5 << " " << id6);
          std::cout << std::endl
                    << "/// for _mom_predef.h id1 id2 id3 id4 id5 id6 id7" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id1 << "," << id1.hi().serial()
                    << "," << id1.lo().serial() << "," << id1.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id2 << "," << id2.hi().serial()
                    << "," << id2.lo().serial() << "," << id2.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id3 << "," << id3.hi().serial()
                    << "," << id3.lo().serial() << "," << id3.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id4 << "," << id4.hi().serial()
                    << "," << id4.lo().serial() << "," << id4.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id5 << "," << id5.hi().serial()
                    << "," << id5.lo().serial() << "," << id5.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id6 << "," << id6.hi().serial()
                    << "," << id6.lo().serial() << "," << id6.hash() << ")" << std::endl;
          std::cout << "MOM_HAS_PREDEF("<< id7 << "," << id7.hi().serial()
                    << "," << id7.lo().serial() << "," << id7.hash() << ")" << std::endl;
          std::cout << std::endl;
        }
        break;
        case xtraopt_parseid:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing id for --parse-id");
          auto idp = MomIdent::make_from_cstr(optarg, true);
          MOM_INFORMLOG("parse-id '" << optarg << "'" << std::endl
                        << " ... idp= " << idp << " =(" << idp.hi().serial() << "," << idp.lo().serial()
                        << ")/h" << idp.hash() << ",b#" << idp.bucketnum());
	  std::cout << "MOM_HAS_PREDEF("<< idp << "," << idp.hi().serial() << "," << idp.lo().serial()
		    << "," << idp.hash() << ")" << std::endl;
        }
        break;
        case xtraopt_parseval:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing value for --parse-val");
          pstr = std::string{optarg};
          nbparsval++;
          todo_after_load_mom.push_back
          ([=](void)
          {
            std::istringstream ins{pstr};
            MomSimpleParser pars(ins);
            pars
            .set_name(std::string{"--parse-val!"}+std::to_string(nbparsval))
            .set_make_from_id(true)
            .set_debug(MOM_IS_DEBUGGING(parse))
            .disable_exhaustion();
            pars.skip_spaces();
            MOM_INFORMLOG("parse-val " << MomShowString(optarg) << std::endl);
            bool gotval = false;
            pars.skip_spaces();
            std::string locstr = pars.location_str();
            auto val = pars.parse_value(&gotval);
            pars.skip_spaces();
            MOM_INFORMLOG("parse-val " << (gotval?"with":"without") << " value=" << val << std::endl
                          << "...@ " << pars.location_str()
                          << std::endl << "/// raw val @" << locstr
                          << std::endl
                          << MomDoShow([&](std::ostream&os)
            {
              MomEmitter rawem(os);
              rawem.emit_value(val);
            })
                << std::endl);
          });
        }
        break;
        case xtraopt_parsefileofvalues:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing filepath for --parse-file");
          pstr = std::string{optarg};
          MOM_INFORMLOG("should parse file of values " << MomShowString(pstr));
          todo_after_load_mom.push_back
          ([=](void)
          {
            parse_file_of_values_mom(pstr);
          });
        }
        break;
        case xtraopt_parsefileofcommands:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing filepath for --parse-command-file");
          pstr = std::string{optarg};
          MOM_INFORMLOG("should parse file of commands " << MomShowString(pstr));
          todo_after_load_mom.push_back
          ([=](void)
          {
            parse_file_of_commands_mom(pstr);
          });
        }
        break;
        case xtraopt_loadspyid1:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing id for --load-spy-id1");
          auto idp = MomIdent::make_from_cstr(optarg, true);
          MOM_INFORMLOG("load-spy-id1 '" << optarg << "'" << std::endl
                        << " ... idp= " << idp << " =(" << idp.hi().serial() << "," << idp.lo().serial()
                        << ")/h" << idp.hash() << ",b#" << idp.bucketnum());
          mom_load_spyid1= idp;
        }
        break;
        case xtraopt_loadspyid2:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing id for --load-spy-id2");
          auto idp = MomIdent::make_from_cstr(optarg, true);
          MOM_INFORMLOG("load-spy-id2 '" << optarg << "'" << std::endl
                        << " ... idp= " << idp << " =(" << idp.hi().serial() << "," << idp.lo().serial()
                        << ")/h" << idp.hash() << ",b#" << idp.bucketnum());
          mom_load_spyid2= idp;
        }
        break;
        default:
          MOM_FATAPRINTF ("bad option (%c/%d) at %d", isalpha (opt) ? opt : '?', opt,
                          optind);
          return;
        }
    }
  *pargc -= optind;
  argc = *pargc;
  *pargv = argv+optind;
  argv = *pargv;
  MOM_DEBUGLOG(misc, "parse_program_arguments final argc=" << argc);
} // end of parse_program_arguments_mom



static void mom_sqlite_errlog(void*, int errcode, const char*msg)
{
  MOM_BACKTRACELOG("SQLITE ERROR#" << errcode
                   << " (" << sqlite3_errstr(errcode) << "):: "
                   << msg << std::endl);
} // end mom_sqlite_errlog


const MomVtablePayload_st*
MomRegisterPayload::find_payloadv(const std::string&nam)
{
  if (nam.empty()) return nullptr;
  std::lock_guard<std::mutex> gu(_pd_mtx_);
  auto it=_pd_dict_.find(nam);
  if (it!=_pd_dict_.end()) return it->second;
  std::string fullnam = std::string{MOM_PAYLOADVTBL_SUFFIX}+nam;
  auto pv = reinterpret_cast<const struct MomVtablePayload_st*>(dlsym(mom_prog_dlhandle,fullnam.c_str()));
  if (pv != nullptr && pv->pyv_magic == MOM_PAYLOADVTBL_MAGIC
      && nam == std::string{pv->pyv_name})
    {
      _pd_dict_.insert({nam,pv});
      return pv;
    }
  return nullptr;
} // end MomRegisterPayload::find_payloadv

void
MomRegisterPayload::register_payloadv(const MomVtablePayload_st*pv)
{
  if (!pv || pv->pyv_magic != MOM_PAYLOADVTBL_MAGIC)
    MOM_FAILURE("register_payloadv bad pv");
  if (!pv->pyv_name || !mom_valid_name_radix_len(pv->pyv_name, -1))
    MOM_FAILURE("register_payloadv bad name:" <<pv->pyv_name);
  std::lock_guard<std::mutex> gu(_pd_mtx_);
  _pd_dict_.insert({std::string{pv->pyv_name},pv});
}

void
MomRegisterPayload::forget_payloadv(const MomVtablePayload_st*pv)
{
  if (!pv || pv->pyv_magic != MOM_PAYLOADVTBL_MAGIC)
    MOM_FAILURE("forget_payloadv bad pv");
  if (!pv->pyv_name || !mom_valid_name_radix_len(pv->pyv_name, -1))
    MOM_FAILURE("forget_payloadv bad name:" <<pv->pyv_name);
  std::lock_guard<std::mutex> gu(_pd_mtx_);
  _pd_dict_.erase(std::string{pv->pyv_name});
}

int
main (int argc_main, char **argv_main)
{
  clock_gettime (CLOCK_REALTIME, &start_realtime_ts_mom);
  gethostname (hostname_mom, sizeof (hostname_mom) - 1);
  char **argv = argv_main;
  int argc = argc_main;
  btstate_mom = backtrace_create_state(argv_main[0], /*multithreaded*/TRUE,
                                       MomBacktraceData::bt_err_callback,
                                       NULL);
  if (MOM_UNLIKELY(btstate_mom==nullptr))
    {
      fprintf(stderr, "%s: backtrace_create_state failed fatally (%m)\n", argv_main[0]);
      abort();
    }
  mom_prog_dlhandle = dlopen (nullptr, RTLD_NOW);
  if (MOM_UNLIKELY(!mom_prog_dlhandle))
    MOM_FATAPRINTF ("failed to dlopen program (%s)", dlerror ());
  sqlite3_initialize();		// explicit initialization is needed before sqlite3_config
  sqlite3_config(SQLITE_CONFIG_LOG, mom_sqlite_errlog, NULL);
  mom_nb_jobs = (3*std::thread::hardware_concurrency())/4;
  if (mom_nb_jobs<MOM_MIN_JOBS) mom_nb_jobs = MOM_MIN_JOBS;
  if (mom_nb_jobs>MOM_MAX_JOBS) mom_nb_jobs = MOM_MAX_JOBS;
  MomAnyVal::enable_allocation();
  parse_program_arguments_mom(&argc, &argv);
  MomObject::initialize_predefined();
  if (mom_load_dir && mom_load_dir[0] && mom_load_dir[0] != '-')
    {
      if (sequential_load_mom)
        mom_load_sequential_from_directory(mom_load_dir);
      else
        mom_load_from_directory(mom_load_dir);
    }
  MOM_INFORMLOG("running timestamp " << monimelt_timestamp << " lastgitcommit " << monimelt_lastgitcommit << " pid=" << (int)getpid());
  if (!todo_after_load_mom.empty())
    {
      MOM_INFORMLOG("todo after load: " << todo_after_load_mom.size() << " entries.");
      usleep(10000);
      long todocnt = 0;
      while (!todo_after_load_mom.empty())
        {
          auto todofun = todo_after_load_mom.front();
          todo_after_load_mom.pop_front();
          todocnt++;
          todofun();
        }
      MOM_INFORMLOG("todo after load done " << todocnt << " todos.");
    }
  if (mom_with_gui)
    {
      int res = mom_run_gtkmm_gui(argc, argv);
      if (res!=0)
        MOM_WARNPRINTF("mom_run_gtkmm_gui returned %d", res);
    }
#warning missing stuff in main
  if (mom_dump_dir)
    mom_dump_in_directory(mom_dump_dir);
} // end of main



