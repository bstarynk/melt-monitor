// file main.cc - main program and utilities

/**   Copyright (C)  2015 - 2016  Basile Starynkevitch and later the FSF
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

// libbacktrace from GCC 6, i.e. libgcc-6-dev package
#include <backtrace.h>
#include <cxxabi.h>
#define BASE_YEAR_MOM 2015

static struct backtrace_state *btstate_mom;
static bool syslogging_mom;
static bool should_dump_mom;
static const char*load_state_mom;
thread_local MomRandom MomRandom::_rand_thr_;

typedef std::function<void(void)> todo_t;
static std::vector<todo_t> todo_after_load_mom;


unsigned mom_debugflags;
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
  MomBacktraceData backdata(fil,lin);
  backdata.bt_outs << " !!! " << str << std::endl;
  backtrace_full (btstate_mom, 1,
                  MomBacktraceData::bt_callback,
                  MomBacktraceData::bt_err_callback,
                  (void *) &backdata);
  backdata.bt_outs << std::endl;
  mom_warnprintf_at (fil, lin, "FAILURE %s", backdata.bt_outs.str().c_str());
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

  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, nullptr));
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
    return std::string{nullptr};
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
          continue;
        }
      else // ordinary character
        {
          bufzon[0] = c;
          continue;
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
        syslog (LOG_DEBUG, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s",
                dbg_level_mom (dbg).c_str(), thrname, fil, lin, timbuf, msg);
        if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
          syslog (LOG_DEBUG, "MONIMELT DEBUG#%04ld ~ %s *^*^*", nbdbg,
                  datebuf);
      }
    else
      {
        fprintf (stderr, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s\n",
                 dbg_level_mom (dbg).c_str(), thrname, fil, lin, timbuf, msg);
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
};

static const struct option mom_long_options[] =
{
  {"help", no_argument, nullptr, 'h'},
  {"version", no_argument, nullptr, 'V'},
  {"debug", required_argument, nullptr, 'D'},
  {"dump", no_argument, nullptr, 'd'},
  {"load", required_argument, nullptr, 'L'},
  {"chdir-first", required_argument, nullptr, xtraopt_chdir_first},
  {"chdir-after-load", required_argument, nullptr, xtraopt_chdir_after_load},
  {"add-predefined", required_argument, nullptr, xtraopt_addpredef},
  {"comment-predefined", required_argument, nullptr, xtraopt_commentpredef},
  {"test-id", no_argument, nullptr, xtraopt_testid},
  {"parse-id", required_argument, nullptr, xtraopt_parseid},
  {"parse-val", required_argument, nullptr, xtraopt_parseval},
  /* Terminating nullptr placeholder.  */
  {nullptr, no_argument, nullptr, 0},
};


static void
usage_mom (const char *argv0)
{
  printf ("Usage: %s\n", argv0);
  printf ("\t -h | --help " " \t# Give this help.\n");
  printf ("\t -V | --version " " \t# Give version information.\n");
  printf ("\t -D | --debug <debug-features>"
          " \t# Debugging comma separated features\n\t\t##");
  for (unsigned ix = 1; ix < momdbg__last; ix++)
    printf (" %s", mom_debug_names[ix]);
  putchar ('\n');
  printf ("\t -d | --dump " " \t# Dump the state.\n");
  printf ("\t --chdir-first dirpath" " \t#Change directory at first \n");
  printf ("\t --chdir-after-load dirpath"
          " \t#Change directory after load\n");
  printf ("\t --add-predefined predefname" " \t#Add a predefined\n");
  printf ("\t --comment-predefined comment"
          " \t#Set comment of next predefined\n");
  printf ("\t --test-id" " \t#generate a few random ids\n");
  printf ("\t --parse-id id" " \t#parse an id\n");
  printf ("\t --parse-val <val>" " \t#parse some value\n");
}


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
  while ((opt = getopt_long (argc, argv, "hVdsD:L:",
                             mom_long_options, nullptr)) >= 0)
    {
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
          should_dump_mom = true;
          break;
        case 'D':              /* --debug debugopt */
          mom_set_debugging (optarg);
          break;
        case 'L': /* --load filepath */
          if (!optarg || access (optarg, R_OK))
            MOM_FATAPRINTF ("bad load state %s : %m", optarg);
          load_state_mom = optarg;
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
              should_dump_mom = true;
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
        case xtraopt_chdir_after_load:
          if (optarg != nullptr)
            {
              if (access(optarg,F_OK))
                MOM_WARNPRINTF("cannot access %s for --chdir-after-load (%m)",
                               optarg);
              std::string dirstr {optarg};
              todo_after_load_mom.push_back([=](void)
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
          MOM_INFORMLOG("test-id __cplusplus=" << __cplusplus);
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
        }
        break;
        case xtraopt_parseval:
        {
          if (optarg == nullptr)
            MOM_FATAPRINTF("missing value for --parse-val");
          std::string pstr{optarg};
          std::istringstream ins{pstr};
          MomParser pars(ins);
          MOM_INFORMLOG("parse-val '" << optarg << "'" << std::endl
                        << "peekbyte(0)=" << pars.peekbyte(0) << ' '
                        << "peekbyte(1)=" << pars.peekbyte(1) << ' '
                        << "peekbyte(2)=" << pars.peekbyte(2) << ' '
                        << "peekbyte(3)=" << pars.peekbyte(3) << ' '
                        << "peekbyte(4)=" << pars.peekbyte(4) << std::endl);
#warning incomplete --parse-val
        }
        break;
        default:
          MOM_FATAPRINTF ("bad option (%c) at %d", isalpha (opt) ? opt : '?',
                          optind);
          return;
        }
    }
} // end of parse_program_arguments_mom


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
  parse_program_arguments_mom(&argc, &argv);
} // end of main


#warning TODO: should code an explicit garbage collector

#warning TODO: code a parser, usable from loader & elsewhere

#warning TODO: code the loader using sqlite then several threads for parsing

#warning TODO: code a printer, compatible with the parser

#warning TODO: code the (multi-threaded) dumper using the printer & sqlite
