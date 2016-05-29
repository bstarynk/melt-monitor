// file meltmoni.hh - common header file to be included everywhere.

/**   Copyright (C)  2015 - 2016 Basile Starynkevitch, later FSF
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
#ifndef MONIMELT_INCLUDED_
#define MONIMELT_INCLUDED_ 1

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#define GC_THREADS 1
#define HAVE_PTHREADS 1


#include <features.h>           // GNU things
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <sched.h>
#include <syslog.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <fcntl.h>
#include <dlfcn.h>
#if __GLIBC__
#include <execinfo.h>
#endif

#include <gc/gc.h>

// libonion from http://www.coralbits.com/libonion/ &
// https://github.com/davidmoreno/onion
#include <onion/onion.h>
#include <onion/version.h>
#include <onion/low.h>
#include <onion/codecs.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/block.h>
#include <onion/handler.h>
#include <onion/dict.h>
#include <onion/log.h>
#include <onion/shortcuts.h>
#include <onion/exportlocal.h>
#include <onion/internal_status.h>
#include <onion/websocket.h>


// jansson, a JSON library in C which is Boehm-GC friendly
// see http://www.digip.org/jansson/
#include <jansson.h>

#include <glib.h>

// standard C++11 stuff
#include <atomic>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <random>
#include <mutex>
#include <thread>
#include <new>

#define MOM_FLEXIBLE_DIM 1

// in generated _timestamp.c
extern "C" const char monimelt_timestamp[];
extern "C" const char monimelt_lastgitcommit[];
extern "C" const char monimelt_lastgittag[];
extern "C" const char monimelt_compilercommand[];
extern "C" const char monimelt_compilerflags[];
extern "C" const char monimelt_optimflags[];
extern "C" const char monimelt_checksum[];
extern "C" const char monimelt_directory[];
extern "C" const char monimelt_makefile[];


// increasing array of primes and its size
extern "C" const int64_t mom_primes_tab[];
extern "C" const unsigned mom_primes_num;
/// give a prime number above or below a given n, or else 0
extern "C" int64_t mom_prime_above (int64_t n);
extern "C" int64_t mom_prime_below (int64_t n);

static_assert (sizeof (intptr_t) == sizeof (double)
               || 2 * sizeof (intptr_t) == sizeof (double),
               "double-s should be the same size or twice as intptr_t");

typedef uint32_t momhash_t;

typedef __int128 mom_int128_t;
typedef unsigned __int128 mom_uint128_t;
extern "C" const char *mom_hostname (void);

// mark unlikely conditions to help optimization
#ifdef __GNUC__
#define MOM_UNLIKELY(P) __builtin_expect(!!(P),0)
#define MOM_LIKELY(P) !__builtin_expect(!(P),0)
#define MOM_UNUSED __attribute__((unused))
#else
#define MOM_UNLIKELY(P) (P)
#define MOM_LIKELY(P) (P)
#define MOM_UNUSED
#endif

typedef std::atomic<int> mom_atomic_int;

#ifdef NDEBUG
#define MOM_PRIVATE static
#else
#define MOM_PRIVATE             /*nothing */
#endif  /*NDEBUG*/
// A non nil address which is *never* dereferencable and can be used
// as an empty placeholder; in practice all Unix & POSIX systems dont
// use that address
#define MOM_EMPTY_SLOT ((void*)-1L)

// maximum number of threads
#define MOM_JOB_MAX 16
extern __thread int mom_worker_num;

#define MOM_SIZE_MAX (1<<24)

#define MOM_ITEMPERRADIX_MAX  (INT32_MAX/2)
static inline pid_t mom_gettid (void)
{
  return syscall (SYS_gettid, 0L);
}

/// generate a GPLv3 notice
void mom_output_gplv3_notice (FILE *out, const char *prefix,
                              const char *suffix, const char *filename);

static inline void *mom_gc_alloc (size_t sz)
{
  void *p = GC_MALLOC (sz);
  if (MOM_LIKELY (p != nullptr))
    memset (p, 0, sz);
  return p;
}

void *mom_gc_calloc (size_t nmemb, size_t size);

static inline void *mom_gc_alloc_scalar (size_t sz)
{
  void *p = GC_MALLOC_ATOMIC (sz);
  if (MOM_LIKELY (p != nullptr))
    memset (p, 0, sz);
  return p;
}


static inline void *mom_gc_alloc_uncollectable (size_t sz)
{
  void *p = GC_MALLOC_UNCOLLECTABLE (sz);
  if (MOM_LIKELY (p != nullptr))
    memset (p, 0, sz);
  return p;
}

static inline char *mom_gc_strdup (const char *s)
{
  return GC_STRDUP (s);
}

void
mom_fataprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4), noreturn));

#define MOM_FATAPRINTF_AT(Fil,Lin,Fmt,...) do {	\
    mom_fataprintf_at (Fil,Lin,Fmt,		\
		       ##__VA_ARGS__);		\
  } while(0)

#define MOM_FATAPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_FATAPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_FATAPRINTF(Fmt,...)			\
  MOM_FATAPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)


#define MOM_FATALOG_AT(Fil,Lin,Log) do {	\
  std::ostringstream _olog_#Lin;		\
  _olog_#Lin << Log << std::flush;		\
  mom_fataprintf_at (Fil,Lin,"%s",		\
		     _olog_#Lin.c_str());	\
  } while(0)

#define MOM_FATALOG_AT_BIS(Fil,Lin,Log)	\
  MOM_FATALOG_AT(Fil,Lin,Log)

#define MOM_FATALOG(Log)			\
  MOM_FATALOG_AT_BIS(__FILE__,__LINE__,Log)


// for debugging; the color level are user-definable:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
  Dbg(cmd)					\
  Dbg(dump)					\
  Dbg(gencod)					\
  Dbg(item)					\
  Dbg(load)					\
  Dbg(blue)					\
  Dbg(green)					\
  Dbg(red)					\
  Dbg(yellow)					\
  Dbg(web)

#define MOM_DEBUG_DEFINE_OPT(Nam) momdbg_##Nam,
enum mom_debug_en
{
  momdbg__none,
  MOM_DEBUG_LIST_OPTIONS (MOM_DEBUG_DEFINE_OPT) momdbg__last
};

extern "C" unsigned mom_debugflags;

#define MOM_IS_DEBUGGING(Dbg) (mom_debugflags & (1<<momdbg_##Dbg))

void mom_set_debugging (const char *dbgopt);


void
mom_debugprintf_at (const char *fil, int lin, enum mom_debug_en dbg,
                    const char *fmt, ...)
__attribute__ ((format (printf, 4, 5)));

#define MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,...) do {	\
    if (MOM_IS_DEBUGGING(Dbg))				\
      mom_debugprintf_at (Fil,Lin,momdbg_##Dbg,Fmt,	\
			  ##__VA_ARGS__);		\
  } while(0)

#define MOM_DEBUGPRINTF_AT_BIS(Fil,Lin,Dbg,Fmt,...)	\
  MOM_DEBUGPRINTF_AT(Fil,Lin,Dbg,Fmt,			\
		     ##__VA_ARGS__)

#define MOM_DEBUGPRINTF(Dbg,Fmt,...)			\
  MOM_DEBUGPRINTF_AT_BIS(__FILE__,__LINE__,Dbg,Fmt,	\
			 ##__VA_ARGS__)


void
mom_informprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,...) do {	\
    mom_informprintf_at (Fil,Lin,Fmt,			\
			 ##__VA_ARGS__);		\
  } while(0)

#define MOM_INFORMPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_INFORMPRINTF_AT(Fil,Lin,Fmt,			\
		      ##__VA_ARGS__)

#define MOM_INFORMPRINTF(Fmt,...)			\
  MOM_INFORMPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			  ##__VA_ARGS__)


void
mom_warnprintf_at (const char *fil, int lin, const char *fmt, ...)
__attribute__ ((format (printf, 3, 4)));

#define MOM_WARNPRINTF_AT(Fil,Lin,Fmt,...) do {	\
    mom_warnprintf_at (Fil,Lin,Fmt,		\
		       ##__VA_ARGS__);		\
  } while(0)

#define MOM_WARNPRINTF_AT_BIS(Fil,Lin,Fmt,...)	\
  MOM_WARNPRINTF_AT(Fil,Lin,Fmt,		\
		    ##__VA_ARGS__)

#define MOM_WARNPRINTF(Fmt,...)			\
  MOM_WARNPRINTF_AT_BIS(__FILE__,__LINE__,Fmt,	\
			##__VA_ARGS__)

// the program handle from GC_dlopen with nullptr
extern void *mom_prog_dlhandle;

// time measurement, in seconds
// query a clock
static inline double mom_clock_time (clockid_t cid)
{
  struct timespec ts = { 0, 0 };
  if (clock_gettime (cid, &ts))
    return NAN;
  else
    return (double) ts.tv_sec + 1.0e-9 * ts.tv_nsec;
}

static inline struct timespec mom_timespec (double t)
{
  struct timespec ts = { 0, 0 };
  if (isnan (t) || t < 0.0)
    return ts;
  double fl = floor (t);
  ts.tv_sec = (time_t) fl;
  ts.tv_nsec = (long) ((t - fl) * 1.0e9);
  // this should not happen
  if (MOM_UNLIKELY (ts.tv_nsec < 0))
    ts.tv_nsec = 0;
  while (MOM_UNLIKELY (ts.tv_nsec >= 1000 * 1000 * 1000))
    {
      ts.tv_sec++;
      ts.tv_nsec -= 1000 * 1000 * 1000;
    };
  return ts;
}


double mom_elapsed_real_time (void);  /* relative to start of program */
double mom_process_cpu_time (void);
double mom_thread_cpu_time (void);

// call strftime on ti, but replace .__ with centiseconds for ti
char *mom_strftime_centi (char *buf, size_t len, const char *fmt, double ti)
__attribute__ ((format (strftime, 3, 0)));
#define mom_now_strftime_centi(Buf,Len,Fmt) mom_strftime_centi((Buf),(Len),(Fmt),mom_clock_time(CLOCK_REALTIME))
#define mom_now_strftime_bufcenti(Buf,Fmt) mom_now_strftime_centi(Buf,sizeof(Buf),(Fmt))

// output with backslashes an UTF8-encoded string str; if len<0 take
// its strlen; without enclosing quotes
void mom_output_utf8_encoded (FILE *f, const char *str, int len);
// output with HTML encoding an UTF8-encoded string str; if len<0 take
// its strlen; without enclosing quotes; if nlisbr is true, newlines are emitted as <br/>
void mom_output_utf8_html (FILE *f, const char *str, int len, bool nlisbr);

typedef void mom_utf8escape_sig_t (FILE *f, gunichar uc,
                                   const char *cescstr, void *clientdata);
void mom_output_utf8_escaped (FILE *f, const char *str, int len,
                              mom_utf8escape_sig_t * rout,
                              void *clientdata);


// compute the hash of a string (same as the hash of the MomSTRING value)
momhash_t mom_cstring_hash_len (const char *str, int len);

const char *mom_hexdump_data (char *buf, unsigned buflen,
                              const unsigned char *data, unsigned datalen);


const char *mom_double_to_cstr (double x, char *buf, size_t buflen);

// input and parse and GC-allocate such an UTF-8 quoted string; stop
// en EOL, etc..

struct mom_string_and_size_st
{
  const char *ss_str;
  int ss_len;
};
struct mom_string_and_size_st mom_input_quoted_utf8 (FILE *f);


#define MOM_HI_LO_SUFFIX_LEN 16
// convert a hi+lo pair to a suffix and return it
const char *mom_hi_lo_suffix (char buf[MOM_HI_LO_SUFFIX_LEN],
                              uint16_t hi, uint64_t lo);

// convert a suffix to a hi & lo pair, or return false
bool mom_suffix_to_hi_lo (const char *buf, uint16_t *phi, uint64_t *plo);



class MomRandom
{
  static thread_local MomRandom _rand_thr_;
  unsigned long _rand_count;
  std::mt19937 _rand_generator;
  uint32_t generate_32u(void)
  {
    if (MOM_UNLIKELY(_rand_count++ % 4096 == 0))
      {
        std::random_device randev;
        auto s1=randev(), s2=randev(), s3=randev(), s4=randev(),
             s5=randev(), s6=randev(), s7=randev();
        std::seed_seq seq {s1,s2,s3,s4,s5,s6,s7};
        _rand_generator.seed(seq);
      }
    return _rand_generator();
  };
  uint32_t generate_nonzero_32u(void)
  {
    uint32_t r = 0;
    do
      {
        r = generate_32u();
      }
    while (MOM_UNLIKELY(r==0));
    return r;
  };
  uint64_t generate_64u(void)
  {
    return (static_cast<uint64_t>(generate_32u())<<32) | static_cast<uint64_t>(generate_32u());
  };
public:
  static uint32_t random_32u(void)
  {
    return _rand_thr_.generate_32u();
  };
  static uint64_t random_64u(void)
  {
    return _rand_thr_.generate_64u();
  };
  static uint32_t random_nonzero_32u(void)
  {
    return _rand_thr_.generate_nonzero_32u();
  };
};				// end class MomRandom


class MomLoader;
class MomDumper;

class MomGuardPmutex
{
  pthread_mutex_t* _pmtx;
public:
  MomGuardPmutex(pthread_mutex_t& mtx)
  {
    _pmtx = &mtx;
    pthread_mutex_lock (_pmtx);
  };
  ~MomGuardPmutex()
  {
    pthread_mutex_unlock(_pmtx);
    _pmtx = nullptr;
  }
  MomGuardPmutex(const MomGuardPmutex&) = delete;
  MomGuardPmutex(MomGuardPmutex&&) = delete;
  MomGuardPmutex() = delete;
}; // end MomGuardPmutex

#include "_mom_aggr.h"

typedef MomITEM MomPredefinedITEM;

static inline const char *mom_item_hi_lo_suffix (char  buf[MOM_HI_LO_SUFFIX_LEN],
    MomITEM*itm)
{
  memset (buf, 0, MOM_HI_LO_SUFFIX_LEN);
  if (itm && (itm->itm_hid || itm->itm_lid))
    mom_hi_lo_suffix (buf, itm->itm_hid, itm->itm_lid);
  return buf;
}

static inline const char *mom_item_radix_str (const MomITEM*itm)
{
  if (itm && itm != MOM_EMPTY_SLOT && itm->vtype == MomItypeEn::ITEM)
    return itm->itm_radix->rad_name->cstr;
  else
    return NULL;
}

extern "C" bool mom_valid_name_radix_len (const char *str, int len);

extern "C" MomSTRING* mom_make_string(const char*cstr, int len= -1);

static inline MomSTRING* mom_make_string(const std::string&str)
{
  return mom_make_string(str.c_str(), str.size());
}

static inline const char*
mom_radix_cstring(const MomRADIXdata*radix)
{
  if (radix==nullptr || radix==MOM_EMPTY_SLOT
      || radix->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  auto nam = radix->rad_name;
  assert (nam != nullptr && nam->vtype == MomItypeEn::STRING);
  return nam->cstr;
} // end mom_radix_cstring

MomRADIXdata* mom_register_radix_str(const std::string&str);

MomRADIXdata* mom_find_radix_str(const std::string&str);

// compute the hash of an item of given radix and hid & lid
momhash_t mom_hash_radix_id(MomRADIXdata*radix, uint16_t hid=0, uint64_t lid=0);

// find an item from its radix and hid&lid
MomITEM* mom_find_item_from_radix_id(MomRADIXdata*rad, uint16_t hid=0, uint64_t lid=0);

// make an item (if not found) from its radix & hid&lid
MomITEM* mom_make_item_from_radix_id(MomRADIXdata*rad, uint16_t hid=0, uint64_t lid=0);

const char*mom_radix_id_cstr(MomRADIXdata*rad, uint16_t hid=0, uint64_t lid=0);

const std::string mom_radix_id_string(MomRADIXdata*rad, uint16_t hid=0, uint64_t lid=0);

static inline const char*mom_item_cstr(const MomITEM*itm)
{
  if (itm==nullptr || itm==MOM_EMPTY_SLOT||itm->vtype != MomItypeEn::ITEM) return nullptr;
  return mom_radix_id_cstr(itm->itm_radix,itm->itm_hid,itm->itm_lid);
}

static inline const std::string mom_item_string(const MomITEM*itm)
{
  if (itm==nullptr || itm==MOM_EMPTY_SLOT||itm->vtype != MomItypeEn::ITEM) return nullptr;
  return mom_radix_id_string(itm->itm_radix,itm->itm_hid,itm->itm_lid);
}


enum class MomStateTypeEn : uint8_t
{
  EMPTY,
  MARK,
  INT,
  DOUBLE,
  STRING,
  VAL
};

class MomStatElem
{
public:
protected:
  struct MomMarkSt {};
  MomStateTypeEn _st_type;
  union
  {
    const void* _st_nptr;		// null when MomStateTypeEn::EMPTY
    int _st_mark; // when MomStateTypeEn::MARK
    long _st_int; // when MomStateTypeEn::INT
    double _st_dbl; // when MomStateTypeEn::DOUBLE
    std::string _st_str;		// when MomStateTypeEn::STRING
    const MomVal* _st_val;		// when MomStateTypeEn::VAL;
  };
  explicit MomStatElem(std::nullptr_t)
    : _st_type(MomStateTypeEn::EMPTY), _st_nptr(nullptr) {};
  explicit MomStatElem() : MomStatElem(nullptr) {};
  explicit MomStatElem(MomMarkSt, int m)
    : _st_type(MomStateTypeEn::MARK), _st_mark(m) {};
  explicit MomStatElem(long l)
    : _st_type(MomStateTypeEn::INT), _st_int(l) {};
  explicit MomStatElem(double d)
    : _st_type(MomStateTypeEn::DOUBLE), _st_dbl(d) {};
  explicit MomStatElem(const std::string&s)
    : _st_type(MomStateTypeEn::STRING), _st_str(s) {};
  explicit MomStatElem(const MomVal*v)
    : _st_type(MomStateTypeEn::VAL), _st_val(v) {};
  MomStatElem(const MomStatElem&e)
    :_st_type(e._st_type), _st_nptr(nullptr)
  {
    switch (e._st_type)
      {
      case MomStateTypeEn::EMPTY:
        break;
      case MomStateTypeEn::MARK:
        _st_mark = e._st_mark;
        return;
      case MomStateTypeEn::INT:
        _st_int = e._st_int;
        return;
      case MomStateTypeEn::DOUBLE:
        _st_dbl = e._st_dbl;
        return;
      case MomStateTypeEn::STRING:
        _st_str = e._st_str;
        return;
      case MomStateTypeEn::VAL:
        _st_val = e._st_val;
        return;
      }
  }
  MomStatElem(MomStatElem&&e) :
    _st_type(e._st_type), _st_nptr(nullptr)
  {
    switch (e._st_type)
      {
      case MomStateTypeEn::EMPTY:
        break;
      case MomStateTypeEn::MARK:
        _st_mark = e._st_mark;
        break;
      case MomStateTypeEn::INT:
        _st_int = e._st_int;
        break;
      case MomStateTypeEn::DOUBLE:
        _st_dbl = e._st_dbl;
        break;
      case MomStateTypeEn::STRING:
        _st_str = std::move(e._st_str);
        break;
      case MomStateTypeEn::VAL:
        _st_val = e._st_val;
        break;
      }
    e._st_type =  MomStateTypeEn::EMPTY;
    e._st_nptr = nullptr;
  }
  ~MomStatElem()
  {
    typedef std::string str_t;
    switch (_st_type)
      {
      case MomStateTypeEn::EMPTY:
        break;
      case MomStateTypeEn::MARK:
        break;
      case MomStateTypeEn::INT:
        break;
      case MomStateTypeEn::DOUBLE:
        break;
      case MomStateTypeEn::STRING:
        _st_str.~str_t();
        break;
      case MomStateTypeEn::VAL:
        break;
      }
    _st_nptr = nullptr;
  };				// end ~MomStatElem
};

class MomStatEmpty : public MomStatElem
{
public:
  explicit MomStatEmpty() : MomStatElem(nullptr) {};
};

class MomStatMark : public MomStatElem
{
public:
  explicit MomStatMark(int m) :
    MomStatElem(MomStatElem::MomMarkSt(), m) {};
};

class MomStatInt : public MomStatElem
{
public:
  explicit MomStatInt(long l) :
    MomStatElem(l) {}
};

class MomStatDouble : public MomStatElem
{
public:
  explicit MomStatDouble(double d) :
    MomStatElem(d) {}
};

class MomStatString : public MomStatElem
{
public:
  explicit MomStatString(const std::string&s) :
    MomStatElem(s) {}
};

class MomStatVal : public MomStatElem
{
public:
  explicit MomStatVal(const MomVal*v=nullptr) :
    MomStatElem(v) {}
};


class MomLoader
{
  std::vector<MomStatElem> _ld_stack;
};				// end of class MomLoader

#endif /*MONIMELT_INCLUDED_ */
