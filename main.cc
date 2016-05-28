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

#define BASE_YEAR_MOM 2015

static bool syslogging_mom;
thread_local MomRandom MomRandom::_rand_thr_;

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




static struct timespec start_realtime_ts_mom;

double
mom_elapsed_real_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_REALTIME, &curts);
  return 1.0 * (curts.tv_sec - start_realtime_ts_mom.tv_sec)
         + 1.0e-9 * (curts.tv_nsec - start_realtime_ts_mom.tv_nsec);
}

double
mom_process_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
}

double
mom_thread_cpu_time (void)
{
  struct timespec curts = { 0, 0 };
  clock_gettime (CLOCK_THREAD_CPUTIME_ID, &curts);
  return 1.0 * (curts.tv_sec) + 1.0e-9 * (curts.tv_nsec);
}




static const char *
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
      return GC_STRDUP (dbglev);
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
  assert (rout != NULL);
  if (len < 0)
    len = strlen (str);
  const char *end = str + len;
  gunichar uc = 0;
  const char *s = str;
  assert (s && g_utf8_validate (s, len, NULL));
  assert (s && g_utf8_validate (s, len, NULL));
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
}                               /* end mom_cstring_hash_len */



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
  assert (s && g_utf8_validate (s, len, NULL));
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
  assert (s && g_utf8_validate (s, len, NULL));
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
    return NULL;
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



struct mom_string_and_size_st
mom_input_quoted_utf8 (FILE *f)
{
  struct mom_string_and_size_st ss = { NULL, 0 };
  if (!f)
    return ss;
  char bufarr[64];
  int bufsiz = sizeof (bufarr);
  char *bufzon = bufarr;
  int bufoff = 0;
  long off = ftell (f);
  do
    {
      if (MOM_UNLIKELY (bufoff > INT32_MAX / 3))
        {
          fseek (f, off, SEEK_SET);
          MOM_FATAPRINTF ("too long (%d) input quoted UTF-8 string %.50s...",
                          bufoff, bufzon);
        }
      if (MOM_UNLIKELY (bufoff + 9 >= bufsiz))
        {
          int newsiz = ((6 * bufsiz / 5 + 30) | 0x1f) + 1;
          char *newbuf = static_cast<char*>(mom_gc_alloc_scalar (newsiz));
          memcpy (newbuf, bufzon, bufoff);
          if (bufzon != bufarr)
            GC_FREE (bufzon);
          bufzon = newbuf;
          bufsiz = newsiz;
        };
      int c = fgetc (f);
      if (c == EOF)
        break;
      if (iscntrl (c) || c == '\'' || c == '"')
        {
          ungetc (c, f);
          break;
        }
      else if (c == '\\')
        {
          int pos = -1;
          unsigned b = 0;
          int nc = fgetc (f);
          if (nc == EOF || iscntrl (nc))
            {
              if (nc != EOF)
                ungetc (nc, f);
              break;
            }
          switch (nc)
            {
            case '\'':
            case '\"':
            case '\\':
              bufzon[bufoff++] = nc;
              break;
            case 'a':
              bufzon[bufoff++] = '\a';
              break;
            case 'b':
              bufzon[bufoff++] = '\b';
              break;
            case 'f':
              bufzon[bufoff++] = '\f';
              break;
            case 'n':
              bufzon[bufoff++] = '\n';
              break;
            case 'r':
              bufzon[bufoff++] = '\r';
              break;
            case 't':
              bufzon[bufoff++] = '\t';
              break;
            case 'v':
              bufzon[bufoff++] = '\v';
              break;
            case 'e':
              bufzon[bufoff++] = '\033' /* ESCAPE */ ;
              break;
            case 'x':
              if (fscanf (f, "%02x%n", &b, &pos) > 0 && pos > 0)
                bufzon[bufoff++] = b;
              break;
            case 'u':
              if (fscanf (f, "%04x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon + bufoff, ebuf);
                  bufoff += strlen (ebuf);
                };
              break;
            case 'U':
              if (fscanf (f, "%08x%n", &b, &pos) > 0 && pos > 0)
                {
                  char ebuf[8];
                  memset (ebuf, 0, sizeof (ebuf));
                  g_unichar_to_utf8 ((gunichar) b, ebuf);
                  strcpy (bufzon + bufoff, ebuf);
                  bufoff += strlen (ebuf);
                };
              break;
            default:
              bufzon[bufoff++] = nc;
              break;
            }
          continue;
        }
      else
        {
          bufzon[bufoff++] = c;
          continue;
        }
    }
  while (!feof (f));
  char *res = static_cast<char*>(mom_gc_alloc_scalar (bufoff + 1));
  memcpy (res, bufzon, bufoff);
  if (bufzon != bufarr)
    GC_FREE (bufzon);
  ss.ss_str = res;
  ss.ss_len = bufoff;
  return ss;
}                               /* end of mom_input_quoted_utf8 */


static pthread_mutex_t dbgmtx_mom = PTHREAD_MUTEX_INITIALIZER;
static const char *dbg_level_mom (enum mom_debug_en dbg);
void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
                    const char *fmt, ...)
{
  static long countdbg;
  char thrname[24];
  char buf[160];
  char timbuf[64];
  int len = 0;
  char *msg = NULL;
  char *bigbuf = NULL;
  memset (thrname, 0, sizeof (thrname));
  memset (buf, 0, sizeof (buf));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
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
                dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
        if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
          syslog (LOG_DEBUG, "MONIMELT DEBUG#%04ld ~ %s *^*^*", nbdbg,
                  datebuf);
      }
    else
      {
        fprintf (stderr, "MONIMELT DEBUG %7s <%s> @%s:%d %s %s\n",
                 dbg_level_mom (dbg), thrname, fil, lin, timbuf, msg);
        fflush (stderr);
        if (nbdbg % DEBUG_DATE_PERIOD_MOM == 0)
          fprintf (stderr, "MONIMELT DEBUG#%04ld ~ %s *^*^*\n", nbdbg,
                   datebuf);
        fflush (NULL);
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
  char *bigbuf = NULL;
  char *msg = NULL;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
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
      fflush (NULL);
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
  char *bigbuf = NULL;
  char *msg = NULL;
  int err = errno;
  int nbwarn = 1 + atomic_fetch_add (&mom_nb_warnings, 1);
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  fflush (NULL);
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
      fflush (NULL);
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
  char *bigbuf = NULL;
  char *msg = NULL;
  int err = errno;
  memset (buf, 0, sizeof (buf));
  memset (thrname, 0, sizeof (thrname));
  memset (timbuf, 0, sizeof (timbuf));
  pthread_getname_np (pthread_self (), thrname, sizeof (thrname) - 1);
  mom_now_strftime_bufcenti (timbuf, "%Y-%b-%d %H:%M:%S.__ %Z");
  fflush (NULL);
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
#if __GLIBC__
#define BACKTRACE_MAX_MOM 100
  void *bbuf[BACKTRACE_MAX_MOM];
  int blev = 0;
  memset (bbuf, 0, sizeof (bbuf));
  blev = backtrace (bbuf, BACKTRACE_MAX_MOM - 1);
  char **bsym = backtrace_symbols (bbuf, blev);
  if (syslogging_mom)
    {
      if (err)
        syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s (%s)",
                fil, lin, thrname, (int) mom_gettid (), timbuf,
                msg, strerror (err));
      else
        syslog (LOG_ALERT, "MONIMELT FATAL! @%s:%d <%s:%d> %s %s",
                fil, lin, thrname, (int) mom_gettid (), timbuf, msg);
      for (int i = 0; i < blev; i++)
        syslog (LOG_ALERT, "MONIMELTB![%d]: %s", i, bsym[i]);
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
      for (int i = 0; i < blev; i++)
        fprintf (stderr, "MONIMELTB[%d]: %s\n", i, bsym[i]);
      fflush (NULL);
    }
#endif
  if (bigbuf)
    free (bigbuf);
  abort ();
}





char *
mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
{
  struct tm tm;
  time_t tim = (time_t) ti;
  memset (&tm, 0, sizeof (tm));
  if (!buf || !fmt || !len)
    return NULL;
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


////////////////
void *
mom_gc_calloc (size_t nmemb, size_t size)
{
  if (nmemb == 0 || size == 0)
    return NULL;
  uint64_t totsz = 0;
  if (nmemb > INT32_MAX / 3 || size > INT32_MAX / 3
      || (totsz = nmemb * size) > INT32_MAX)
    MOM_FATAPRINTF ("too big nmemb=%zd or size=%zd for mom_gc_calloc",
                    nmemb, size);
  return mom_gc_alloc (totsz);
}



int
main (int argc_main, char **argv_main)
{
  clock_gettime (CLOCK_REALTIME, &start_realtime_ts_mom);
  gethostname (hostname_mom, sizeof (hostname_mom) - 1);
  GC_INIT ();
  GC_set_handle_fork (1);
  char **argv = argv_main;
  int argc = argc_main;
  mom_prog_dlhandle = dlopen (NULL, RTLD_NOW);
  if (!mom_prog_dlhandle)
    MOM_FATAPRINTF ("failed to dlopen program (%s)", dlerror ());
} // end of main
