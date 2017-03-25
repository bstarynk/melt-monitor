// file meltmoni.hh - common header file to be included everywhere.

/**   Copyright (C)  2015 - 2017 Basile Starynkevitch, later FSF
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
#include <functional>
#include <algorithm>
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


extern "C" void mom_backtracestr_at(const char *fil, int lin,
                                    const std::string &msg);

#define MOM_BACKTRACELOG_AT(Fil, Lin, Log)                     \
  do {                                                         \
    std::ostringstream _out_##Lin;                             \
    _out_##Lin << Log << std::flush;                           \
    mom_backtracestr_at((Fil), (Lin), _out_##Lin.str());       \
  } while (0)
#define MOM_BACKTRACELOG_AT_BIS(Fil, Lin, Log)   \
  MOM_BACKTRACELOG_AT(Fil, Lin, Log)
#define MOM_BACKTRACELOG(Log) MOM_BACKTRACELOG_AT_BIS(__FILE__, __LINE__, Log)

extern "C" void mom_abort(void) __attribute__((noreturn));
#ifndef NDEBUG
#define MOM_ASSERT_AT(Fil, Lin, Prop, Log)                         \
  do {                                                             \
    if (MOM_UNLIKELY(!(Prop))) {                                   \
      MOM_BACKTRACELOG_AT(Fil, Lin,                                \
                          "**MOM_ASSERT FAILED** " #Prop ":"       \
                          " @ "                                    \
                          << __PRETTY_FUNCTION__                   \
        << std::endl  << "::" << Log);           \
      mom_abort();                                                 \
    }                                                              \
  } while (0)
#else
#define MOM_ASSERT_AT(Fil, Lin, Prop, Log)         \
  do {                                             \
    if (false && !(Prop))                          \
      MOM_BACKTRACELOG_AT(Fil, Lin, Log);          \
  } while (0)
#endif // NDEBUG
#define MOM_ASSERT_AT_BIS(Fil, Lin, Prop, Log)                                 \
  MOM_ASSERT_AT(Fil, Lin, Prop, Log)
#define MOM_ASSERT(Prop, Log) MOM_ASSERT_AT_BIS(__FILE__, __LINE__, Prop, Log)

class Mom_runtime_failure : std::runtime_error
{
  const char* _rf_file;
  const int _rf_line;
public:
  Mom_runtime_failure(const char*fil, int lin, const std::string& msg)
    : std::runtime_error(std::string(fil) + ":" + std::to_string(lin) + "\t" + msg), _rf_file(fil), _rf_line(lin) {}
  ~Mom_runtime_failure() = default;

};				// end class Mom_runtime_failure

extern "C" void mom_failure_backtrace_at(const char*fil, int lin, const std::string& str);
#define MOM_FAILURE_AT(Fil,Lin,Log) do {		\
    std::ostringstream _olog_##Lin;			\
    _olog_##Lin << Log << std::flush;			\
    mom_failure_backtrace_at(Fil, Lin,			\
				_olog_##Lin.str());	\
    throw Mom_runtime_failure(Fil, Lin,			\
		      _olog_##Lin.str());		\
} while(0)

#define MOM_FAILURE_AT_BIS(Fil,Lin,Log)	\
  MOM_FAILURE_AT(Fil,Lin,Log)

#define MOM_FAILURE(Log)			\
  MOM_FAILURE_AT_BIS(__FILE__,__LINE__,Log)


// for debugging; the color level are user-definable:
#define MOM_DEBUG_LIST_OPTIONS(Dbg)		\
  Dbg(cmd)					\
  Dbg(dump)					\
  Dbg(load)					\
  Dbg(blue)					\
  Dbg(green)					\
  Dbg(red)					\
  Dbg(yellow)

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

std::string mom_input_quoted_utf8_string(std::istream& ins);

extern "C" bool mom_valid_name_radix_len (const char *str, int len);

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


#define MOM_B62DIGITS                   \
  "0123456789"                          \
  "abcdefghijklmnopqrstuvwxyz"          \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

class MomSerial63
{
  uint64_t _serial;

public:
  static constexpr const uint64_t _minserial_ = 62*62; /// 3884
  static constexpr const uint64_t _maxserial_ = /// 8392993658683402240, about 8.392994e+18
    (uint64_t)10 * 62 * (62 * 62 * 62) * (62 * 62 * 62) * (62 * 62 * 62);
  static constexpr const uint64_t _deltaserial_ = _maxserial_ - _minserial_;
  static constexpr const char *_b62digstr_ = MOM_B62DIGITS;
  static constexpr unsigned _nbdigits_ = 11;
  static constexpr unsigned _base_ = 62;
  static_assert(_maxserial_ < ((uint64_t)1 << 63),
                "corrupted _maxserial_ in MomSerial63");
  static_assert(_deltaserial_ > ((uint64_t)1 << 62),
                "corrupted _deltaserial_ in MomSerial63");
  static constexpr const unsigned _maxbucket_ = 10 * 62;
  inline MomSerial63(uint64_t n = 0, bool nocheck = false);
  MomSerial63(std::nullptr_t) : _serial(0) {};
  ~MomSerial63()
  {
    _serial = 0;
  };
  uint64_t serial() const
  {
    return _serial;
  };
  unsigned bucketnum() const
  {
    return _serial / (_deltaserial_ / _maxbucket_);
  };
  uint64_t buckoffset() const
  {
    return _serial % (_deltaserial_ / _maxbucket_);
  };
  std::string to_string(void) const;
  static const MomSerial63 make_from_cstr(const char *s, const char *&end,
                                          bool fail = false);
  static const MomSerial63 make_from_cstr(const char *s, bool fail = false)
  {
    const char *end = nullptr;
    return make_from_cstr(s, end, fail);
  };
  static const MomSerial63 make_random(void);
  static const MomSerial63 make_random_of_bucket(unsigned bun);
  MomSerial63(const MomSerial63 &s) : _serial(s._serial) {};
  MomSerial63(MomSerial63 &&s) : _serial(std::move(s._serial)) {};
  operator bool() const
  {
    return _serial != 0;
  };
  bool operator!() const
  {
    return _serial == 0;
  };
  bool equal(const MomSerial63 r) const
  {
    return _serial == r._serial;
  };
  bool less(const MomSerial63 r) const
  {
    return _serial < r._serial;
  };
  bool less_equal(const MomSerial63 r) const
  {
    return _serial <= r._serial;
  };
  bool operator==(const MomSerial63 r) const
  {
    return equal(r);
  };
  bool operator!=(const MomSerial63 r) const
  {
    return !equal(r);
  };
  bool operator<(const MomSerial63 r) const
  {
    return less(r);
  };
  bool operator<=(const MomSerial63 r) const
  {
    return less_equal(r);
  };
  bool operator>(const MomSerial63 r) const
  {
    return !less_equal(r);
  };
  bool operator>=(const MomSerial63 r) const
  {
    return !less(r);
  };
}; /* end class MomSerial63 */

inline std::ostream &operator<<(std::ostream &os, const MomSerial63 s)
{
  os << s.to_string();
  return os;
} // end operator << MomSerial63

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

MomSerial63::MomSerial63(uint64_t n, bool nocheck) : _serial(n)
{
  if (nocheck || n == 0)
    return;
  if (n < _minserial_)
    MOM_FAILURE("MomSerial63 too small n:" << n);
  if (n > _maxserial_)
    MOM_FAILURE("MomSerial63 too big n:" << n);
} /* end MomSerial63::MomSerial63 */



#endif /*MONIMELT_INCLUDED_ */
