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

#if __cplusplus < 201412L
#error expecting C++17 standard
#endif

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
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <sstream>
#include <fstream>
#include <iostream>
#include <random>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <functional>
#include <algorithm>
#include <utility>
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
extern "C" const char monimelt_sqliteprog[];


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


#define MOM_FATALOG_AT(Fil,Lin,Log) do {		\
  std::ostringstream _olog_##Lin;			\
  _olog_##Lin << Log << std::flush;			\
  mom_fataprintf_at (Fil,Lin,"%s",			\
		     _olog_##Lin.str().c_str());	\
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
  Dbg(parse)					\
  Dbg(garbcoll)					\
  Dbg(misc)					\
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


#define MOM_DEBUGLOG_AT(Fil,Lin,Dbg,Log) do {		\
    if (MOM_IS_DEBUGGING(Dbg))	{			\
  std::ostringstream _olog_##Lin;			\
  _olog_##Lin << Log << std::flush;			\
  mom_debugprintf_at (Fil,Lin,momdbg_##Dbg,"%s",	\
		     _olog_##Lin.str().c_str());	\
    } } while(0)

#define MOM_DEBUGLOG_AT_BIS(Fil,Lin,Dbg,Log)	\
  MOM_DEBUGLOG_AT(Fil,Lin,Dbg,Log)

#define MOM_DEBUGLOG(Dbg,Log)				\
  MOM_DEBUGLOG_AT_BIS(__FILE__,__LINE__,Dbg,Log)


////////////////

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


#define MOM_INFORMLOG_AT(Fil,Lin,Log) do {		\
  std::ostringstream _olog_##Lin;			\
  _olog_##Lin << Log << std::flush;			\
  mom_informprintf_at (Fil,Lin,"%s",			\
		     _olog_##Lin.str().c_str());	\
  } while(0)

#define MOM_INFORMLOG_AT_BIS(Fil,Lin,Log)	\
  MOM_INFORMLOG_AT(Fil,Lin,Log)

#define MOM_INFORMLOG(Log)			\
  MOM_INFORMLOG_AT_BIS(__FILE__,__LINE__,Log)


////////////////
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


#define MOM_WARNLOG_AT(Fil,Lin,Log) do {		\
  std::ostringstream _olog_##Lin;			\
  _olog_##Lin << Log << std::flush;			\
  mom_warnprintf_at (Fil,Lin,"%s",			\
		     _olog_##Lin.str().c_str());	\
  } while(0)

#define MOM_WARNLOG_AT_BIS(Fil,Lin,Log)	\
  MOM_WARNLOG_AT(Fil,Lin,Log)

#define MOM_WARNLOG(Log)			\
  MOM_WARNLOG_AT_BIS(__FILE__,__LINE__,Log)


// the program handle from dlopen with nullptr
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


extern "C" unsigned mom_nb_jobs;
#define MOM_MIN_JOBS 2
#define MOM_MAX_JOBS 16

extern "C" double mom_elapsed_real_time (void);  /* relative to start of program */
extern "C" double mom_process_cpu_time (void);
extern "C" double mom_thread_cpu_time (void);

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

class MomShowString
{
  std::string _shstr;
public:
  MomShowString(const std::string& str) : _shstr(str) {};
  MomShowString(const char*cstr) : _shstr(cstr) {};
  MomShowString(const MomShowString&) = default;
  MomShowString(MomShowString&&) = default;
  ~MomShowString() = default;
  void output(std::ostream& os) const;
};

inline
std::ostream& operator << (std::ostream& out, const MomShowString shs)
{
  shs.output(out);
  return out;
}

class MomDoShow
{
  std::function<void(std::ostream&)> _shfun;
public:
  MomDoShow(std::function<void(std::ostream&)> f) : _shfun(f) {};
  MomDoShow(const MomDoShow&) = default;
  MomDoShow(MomDoShow&&) = default;
  ~MomDoShow() = default;
  void output(std::ostream& os) const
  {
    _shfun(os);
  }
};

inline
std::ostream& operator << (std::ostream& out, const MomDoShow shd)
{
  shd.output(out);
  return out;
}

// compute the hash of a string (same as the hash of the MomSTRING value)
momhash_t mom_cstring_hash_len (const char *str, int len);
const char *mom_hexdump_data (char *buf, unsigned buflen,
                              const unsigned char *data, unsigned datalen);


typedef std::uint32_t MomHash; // hash codes are on 32 bits, but could become on 56 bits!
#define mom_hash(H) ((MomHash)(H))

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


////////////////////////////////////////////////////////////////

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
  MomSerial63& operator = (const MomSerial63&s)
  {
    _serial = s._serial;
    return *this;
  }
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
  void to_cbuf16(char buf[]) const; // actually char buf[static 16]
  static const MomSerial63 make_from_cstr(const char *s, const char **pend,
                                          bool fail = false);
  static const MomSerial63 make_from_cstr(const char *s, bool fail = false)
  {
    return make_from_cstr(s, nullptr, fail);
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


////////////////
class MomIdent
{
public:
  MomSerial63 _idhi, _idlo;
  static constexpr const unsigned _charlen_ = 2*MomSerial63::_nbdigits_+2;
  MomIdent() : _idhi(), _idlo() {};
  MomIdent(std::nullptr_t) : _idhi(), _idlo() {};
  MomIdent(MomSerial63 hi, MomSerial63 lo) : _idhi(hi), _idlo(lo) {};
  MomIdent(uint64_t hi, uint64_t lo, bool nocheck = false) :
    _idhi(hi, nocheck), _idlo(lo, nocheck) {};
  MomIdent(const MomIdent&id) : _idhi(id._idhi), _idlo(id._idlo) {};
  MomIdent(MomIdent&& id): _idhi(std::move(id._idhi)), _idlo(std::move(id._idlo)) {};
  MomIdent& operator = (const MomIdent&id)
  {
    _idhi = id._idhi;
    _idlo = id._idlo;
    return *this;
  };
  ~MomIdent() {};
  unsigned bucketnum() const
  {
    return _idhi.bucketnum();
  };
  uint64_t buckoffset() const
  {
    return _idhi.buckoffset();
  };
  static inline const MomIdent make_random(void)
  {
    return MomIdent(MomSerial63::make_random(), MomSerial63::make_random());
  }
  static const MomIdent make_random_of_bucket(unsigned bun)
  {
    return MomIdent(MomSerial63::make_random_of_bucket(bun), MomSerial63::make_random());
  }
  MomSerial63 hi() const
  {
    return _idhi;
  };
  MomSerial63 lo() const
  {
    return _idlo;
  };
  bool is_null () const
  {
    return !_idhi && !_idlo;
  };
  bool operator ! () const
  {
    return is_null();
  };
  operator bool () const
  {
    return !is_null();
  };
  bool equal(const MomIdent& rid) const
  {
    return _idhi==rid._idhi && _idlo==rid._idlo;
  };
  bool less(const MomIdent&rid) const
  {
    if (_idhi == rid._idhi)
      {
        return _idlo.less(rid._idlo);
      }
    else if (_idhi.less(rid._idhi)) return true;
    else return false;
  }
  bool less_equal(const MomIdent&rid) const
  {
    if (_idhi == rid._idhi)
      {
        return _idlo.less_equal(rid._idlo);
      }
    else if (_idhi.less(rid._idhi)) return true;
    else return false;
  }
  bool operator==(const MomIdent& r) const
  {
    return equal(r);
  };
  bool operator!=(const MomIdent& r) const
  {
    return !equal(r);
  };
  bool operator<(const MomIdent& r) const
  {
    return less(r);
  };
  bool operator<=(const MomIdent& r) const
  {
    return less_equal(r);
  };
  bool operator>(const MomIdent& r) const
  {
    return !less_equal(r);
  };
  bool operator>=(const MomIdent& r) const
  {
    return !less(r);
  };
  void to_cbuf32(char buf[]) const; // actually char buf[static 32]
  std::string to_string() const;
  static constexpr bool DO_FAIL = true;
  static constexpr bool DONT_FAIL = true;
  static const MomIdent make_from_cstr(const char *s, const char **pend,
                                       bool fail = false);
  static const MomIdent make_from_cstr(const char *s, bool fail = false)
  {
    return make_from_cstr(s, nullptr, fail);
  };
  static const MomIdent make_from_string(const std::string& str, bool fail = false)
  {
    return make_from_cstr(str.c_str(), fail);
  };
  MomHash hash() const
  {
    if (is_null()) return 0;
    uint64_t shi = _idhi.serial();
    uint64_t slo = _idlo.serial();
    MomHash h = ((shi * 81281) ^ (slo * 33769)) + 11*(shi>>35) - 31*(slo>>47);
    if (MOM_UNLIKELY(mom_hash(h)==0))
      h = 3*(shi & 0xffffff) + 11*(slo & 0xffffff) + 315;
    return mom_hash(h);
  }
};				// end class MomIdent


inline std::ostream &operator<<(std::ostream &os, const MomIdent& id)
{
  char buf[32];
  memset(buf, 0, sizeof(buf));
  id.to_cbuf32(buf);
  os << buf;
  MOM_ASSERT(strlen(buf)<sizeof(buf)-1, "bad buf:" << buf);
  return os;
} // end operator << MomIdent




////////////////////////////////////////////////////////////////

class MomLoader;		// in state.cc
class MomDumper;		// in state.cc
class MomGC;
extern "C" void mom_dump_into_directory(const char*dirnam);

#define MOM_GLOBAL_DB "mom_global"
#define MOM_USER_DB "mom_user"

class MomParser;		// in parsemit.cc
class MomEmitter;		// in parsemit.cc

MomSerial63::MomSerial63(uint64_t n, bool nocheck) : _serial(n)
{
  if (nocheck || n == 0)
    return;
  if (n < _minserial_)
    MOM_FAILURE("MomSerial63 too small n:" << n);
  if (n > _maxserial_)
    MOM_FAILURE("MomSerial63 too big n:" << n);
} /* end MomSerial63::MomSerial63 */


////////////////
// forward declarations.
class MomAnyVal;		// abstract superclass of any memory value
class MomIntSq;		// value, read-only hash-consed sequence of integers
class MomDoubleSq;	// value, read-only hash-consed sequence of doubles
class MomString;	// value, UTF8 read-only hash-consed string
class MomAnySeqObjVal;		// abstract superclass of hash-consed object sequences (sets or tuples)
class MomSet;		// value, hash-consed set of objects
class MomTuple;		// value, hash-consed tuple of objects
class MomNode;		/* value, hash-consed node: the
 connective is an object, the sons
 are values */
////
class MomObject;
class MomEmitter;
struct MomPayload;

enum class MomKind : std::int8_t
{
  TagIntK = -1,
  TagNoneK = 0,
  TagIntSqK,
  TagDoubleSqK,
  TagStringK,
  TagSetK,
  TagTupleK,
  TagNodeK,
  TagObjectK,
  Tag_LastK
};
static_assert ((unsigned)MomKind::Tag_LastK <= 8, "bad MomKind::Tag_Last");
////////////////

enum class MomEmpEnum
{
  NoneE = 0,
  EmptyE
};

enum class MomTransEnum
{
  PersistentT = 0,
  TransientT
};

struct MomPersistentTag {};
struct MomTransientTag {};
class MomValue
{
  uintptr_t _v;
  // an odd _v (least bit set) represents some tagged integer (63 bits on 64 bits machine)
  // a _v with two least bits cleared is a pointer to some MomAnyVal
  // a _v with the next to last bit set is a transient pointer
public:
  // rule of five
  MomValue() : _v(0) {}
  MomValue(const MomValue& src)
    : _v(src._v) {};
  MomValue(MomValue&& src)
    : _v(src._v)
  {
    src._v = 0;
  };
  MomValue& operator = (const MomValue& src)
  {
    _v = src._v;
    return *this;
  };
  MomValue& operator = (MomValue&& src)
  {
    _v = src._v;
    src._v = 0;
    return *this;
  };
  ~MomValue()
  {
    _v = 0;
  };
  // nil
  bool is_nil() const
  {
    return _v == 0;
  };
  MomValue(std::nullptr_t) : _v(0) {}
  // empty value
  static constexpr uintptr_t _the_empty_v_ = (uintptr_t)(((void**)nullptr)+2);
  bool is_strict_empty() const
  {
    return _v == _the_empty_v_;
  };
  bool is_empty() const
  {
    return is_nil() || is_strict_empty();
  };
  operator bool () const
  {
    return !is_empty();
  };
  bool operator ! () const
  {
    return is_empty();
  };
  bool operator == (const MomValue r) const
  {
    return _v == r._v;
  };
  bool operator != (const MomValue r) const
  {
    return _v != r._v;
  };
  MomValue(MomEmpEnum ev) :
    _v(ev==MomEmpEnum::NoneE?0:_the_empty_v_) {};
  // tagged integers
  bool is_tagint() const
  {
    return (_v & 1) == 1;
  };
  intptr_t as_tagint() const
  {
    if (!is_tagint())
      MOM_FAILURE("MomValue::as_tagint not an integer");
    return ((intptr_t)_v)>>1;
  };
  intptr_t to_tagint(intptr_t defv=0) const
  {
    if (is_tagint())
      return ((intptr_t)_v)>>1;
    else return defv;
  }
  MomValue(intptr_t i) : _v((i<<1)|1) {};
  // any (transient or persistent) allocated value
  bool is_val() const
  {
    return !is_tagint() && !is_empty();
  };
  MomValue(const MomAnyVal*p) // values are persistent by default
    : MomValue(p, MomPersistentTag{}) {};
  const MomAnyVal* as_val() const
  {
    if (!is_val())
      MOM_FAILURE("MomValue::as_val not any value");
    return (const MomAnyVal*)(_v & ~7);
  }
  operator const MomAnyVal* () const
  {
    return as_val();
  };
  const MomAnyVal* to_val(const MomAnyVal* def = nullptr) const
  {
    if (is_val())
      return (const MomAnyVal*)(_v & ~7);
    else return def;
  }
  inline void scan_gc(MomGC*gc) const;
  // transient value
  MomValue(const MomAnyVal*p, MomTransientTag) : _v(((uintptr_t)p) | 2)
  {
    MOM_ASSERT((((uintptr_t)p) & 7) == 0, "bad transient MomAnyVal " << (void*) p);
  };
  bool is_transient() const
  {
    return is_val() && ((_v & 2) != 0);
  };
  const MomAnyVal* as_transient() const
  {
    if (!is_transient())
      MOM_FAILURE("MomValue::as_transient not transient value");
    return (const MomAnyVal*)(_v & ~7);
  }
  const MomAnyVal* to_transient(const MomAnyVal* def = nullptr) const
  {
    if (is_transient())
      return (const MomAnyVal*)(_v & ~7);
    else return def;
  }
  // persistent value
  MomValue(const MomAnyVal*p, MomPersistentTag) : _v((uintptr_t)p)
  {
    MOM_ASSERT((_v & 7) == 0, "bad MomAnyVal " << (void*) p);
  };
  bool is_persistent() const
  {
    return is_val() && ((_v & 2) == 0);
  };
  const MomAnyVal* as_persistent() const
  {
    if (!is_persistent())
      MOM_FAILURE("MomValue::as_persistent not persistent value");
    return (const MomAnyVal*)(_v & ~7);
  }
  const MomAnyVal* to_persistent(const MomAnyVal* def = nullptr) const
  {
    if (is_persistent())
      return (const MomAnyVal*)(_v & ~7);
    else return def;
  }
  inline void scan_dump(MomDumper*du) const;
  inline MomHash hash() const;
  void output(std::ostream& out) const; /// in file parsemit.cc
};				// end class MomValue

inline std::ostream& operator << (std::ostream& out, const MomValue val)
{
  val.output(out);
  return out;
} // end operator << for MomValue


typedef std::uint32_t MomSize; // sizes have 27 bits
typedef std::uint8_t MomGCMark; // garbage collector marks have 2 bits


struct MomNewTag
{
};

constexpr const MomNewTag mom_newtg;
#define mom_new(...) new(mom_newtg,__VA_ARGS__)

inline size_t mom_align(size_t sz);
//// abstract super-class of all boxed values
class MomAnyVal
{
public:
  friend class MomIntSq;
  friend class MomDoubleSq;
  friend class MomString;
  friend class MomAnySeqObjVal;
  friend class MomSet;
  friend class MomTuple;
  friend class MomNode;
  friend class MomGC;
  static constexpr MomSize _max_size = 1 << 27; // 134217728
  static constexpr size_t _alignment = 2*sizeof(void*);
  static constexpr std::uint32_t MARK_BIT = 1U<<27;
  static constexpr std::uint32_t GREY_BIT = 1U<<28;
  template <typename AnyValType> struct PtrBag
  {
    std::mutex _bag_mtx;
    std::unordered_multimap<MomHash,AnyValType*> _bag_map;
#warning PtrBag incomplete and not yet used
  };				// end PtrBag
private:
  static thread_local bool _allocok;
  // we start with the vtable ptr, probably 64 bits
  // the header word contains:
  //// 3 bits for the kind (constant)
  //// 2 bits (grey & mark) for the GC marking (could change during GC operations)
  //// 27 bits for the size (constant)
  mutable std::atomic<std::uint32_t> _headera;
  const MomHash _hashw;
  static std::atomic<std::uint64_t> _wordalloca;
public:
  static void enable_allocation(void)
  {
    _allocok = true;
  };
  static void forbid_allocation(void)
  {
    _allocok = false;
  };
  static std::uint64_t allocation_word_count(void)
  {
    return _wordalloca.load();
  };
  bool gc_mark(MomGC*) const volatile
  {
    return (_headera.load()>>27) & 1;
  }
  bool gc_grey(MomGC*) const volatile
  {
    return (_headera.load()>>27) & 2;
  }
  unsigned gc_info(MomGC*) const volatile
  {
    return (_headera.load()>>27) & 3;
  }
  static bool gc_info_to_mark(MomGC*,unsigned info)
  {
    return info & 1;
  };
  static bool gc_info_to_grey(MomGC*,unsigned info)
  {
    return info & 2;
  };
  bool gc_set_mark(MomGC*, bool m) volatile
  {
    if (m)
      {
        auto oldh = _headera.fetch_or(MARK_BIT);
        return (oldh>>27) & 1;
      }
    else
      {
        auto oldh = _headera.fetch_and(~(MARK_BIT));
        return (oldh>>27) & 1;
      }
  }
  bool gc_set_grey(MomGC*, bool g) volatile
  {
    if (g)
      {
        auto oldh = _headera.fetch_or(GREY_BIT);
        return (oldh>>28) & 1;
      }
    else
      {
        auto oldh = _headera.fetch_and(~(GREY_BIT));
        return (oldh>>28) & 1;
      }
  }
  MomKind kindw() const
  {
    return MomKind(_headera.load()>>29);
  };
  MomSize sizew() const
  {
    return _headera.load() & ((1<<27)-1);
  };
  MomHash hash() const
  {
    return mom_hash(_hashw);
  };
  // integer sequences
  const MomIntSq* as_intsq() const
  {
    if (kindw() != MomKind::TagIntSqK)
      MOM_FAILURE("MomAnyVal::as_intsq not intsq " << this);
    return reinterpret_cast<const MomIntSq*>(this);
  }
  bool is_intsq() const
  {
    return kindw() == MomKind::TagIntSqK;
  };
  const MomIntSq* to_intsq(const MomIntSq* def=nullptr) const
  {
    if (is_intsq()) return  reinterpret_cast<const MomIntSq*>(this);
    else return def;
  }
  // double sequences
  const MomDoubleSq* as_doublesq() const
  {
    if (kindw() != MomKind::TagDoubleSqK)
      MOM_FAILURE("MomAnyVal::as_doublesq not doublesq " << this);
    return reinterpret_cast<const MomDoubleSq*>(this);
  }
  bool is_doublesq() const
  {
    return kindw() == MomKind::TagDoubleSqK;
  };
  const MomDoubleSq* to_doublesq(const MomDoubleSq* def=nullptr) const
  {
    if (is_doublesq()) return  reinterpret_cast<const MomDoubleSq*>(this);
    else return def;
  }
  // strings
  const MomString* as_string() const
  {
    if (kindw() != MomKind::TagStringK)
      MOM_FAILURE("MomAnyVal::as_string not string " << this);
    return reinterpret_cast<const MomString*>(this);
  }
  bool is_string() const
  {
    return kindw() == MomKind::TagStringK;
  };
  const MomString* to_string(const MomString* def=nullptr) const
  {
    if (is_string()) return  reinterpret_cast<const MomString*>(this);
    else return def;
  }
  // sequences of objects (sets or tuples)
  const MomAnySeqObjVal* as_seqobjval() const
  {
    if (kindw() != MomKind::TagSetK && kindw() != MomKind::TagTupleK)
      MOM_FAILURE("MomAnyVal::as_seqobj not seqobjval " << this);
    return reinterpret_cast<const MomAnySeqObjVal*>(this);
  }
  bool is_seqobjval() const
  {
    return kindw() == MomKind::TagSetK || kindw() == MomKind::TagTupleK;
  };
  const MomAnySeqObjVal* to_seqobjval(const MomAnySeqObjVal* def=nullptr) const
  {
    if (is_seqobjval()) return  reinterpret_cast<const MomAnySeqObjVal*>(this);
    else return def;
  }
  // sets
  const MomSet* as_set() const
  {
    if (kindw() != MomKind::TagSetK)
      MOM_FAILURE("MomAnyVal::as_set not set " << this);
    return reinterpret_cast<const MomSet*>(this);
  }
  bool is_set() const
  {
    return kindw() == MomKind::TagSetK;
  };
  const MomSet* to_set(const MomSet* def=nullptr) const
  {
    if (is_set()) return  reinterpret_cast<const MomSet*>(this);
    else return def;
  }
  // tuples
  const MomTuple* as_tuple() const
  {
    if (kindw() != MomKind::TagTupleK)
      MOM_FAILURE("MomAnyVal::as_tuple not tuple " << this);
    return reinterpret_cast<const MomTuple*>(this);
  }
  bool is_tuple() const
  {
    return kindw() == MomKind::TagTupleK;
  };
  const MomTuple* to_tuple(const MomTuple* def=nullptr) const
  {
    if (is_tuple()) return  reinterpret_cast<const MomTuple*>(this);
    else return def;
  }
  // nodes
  const MomNode* as_node() const
  {
    if (kindw() != MomKind::TagNodeK)
      MOM_FAILURE("MomAnyVal::as_node not node " << this);
    return reinterpret_cast<const MomNode*>(this);
  }
  bool is_node() const
  {
    return kindw() == MomKind::TagNodeK;
  };
  const MomNode* to_node(const MomNode* def=nullptr) const
  {
    if (is_node()) return  reinterpret_cast<const MomNode*>(this);
    else return def;
  }
  //
protected:
  void* operator new (size_t sz) = delete;
  inline void* operator new (size_t sz, MomNewTag, size_t gap);
  MomAnyVal(MomKind k, MomSize sz, MomHash h) :
    _headera(((std::uint32_t)k)<<29 | GREY_BIT | std::uint32_t(sz & (_max_size-1))),
    _hashw(h)
  {
    MOM_ASSERT(k>MomKind::TagNoneK && k<MomKind::Tag_LastK, "MomAnyVal bad kind " << (int)k);
    MOM_ASSERT(h!=0, "MomAnyVal zero hash");
    MOM_ASSERT(sz<_max_size, "MomAnyVal huge size " << sz);
  };
  virtual ~MomAnyVal() {};
public:
  /// scan the value for garbage collection:
  virtual void scan_gc(MomGC*) const =0;
  //
  /// scan the value for dump
  virtual void scan_dump(MomDumper*du) const =0;
  //
  /// get the kind thru vtable
  virtual MomKind vkind() const =0;
  //
  // return a mutex appropriate for the value, e.g. for garbage collection
  virtual std::mutex* valmtx() const =0;
};				// end class MomAnyVal

size_t mom_align(size_t sz)
{
  if (sz==0) return 0;
  return ((sz-1)|(MomAnyVal::_alignment-1))+1;
}

MomHash
MomValue::hash() const
{
  if (is_empty()) return 0;
  if (is_tagint())
    {
      auto iv = as_tagint();
      MomHash h = (iv * 241) ^ (iv >> 28);
      if (MOM_UNLIKELY(h==0))
        h = (iv & 0xfffff) + 130;
      return mom_hash(h);
    }
  auto v = as_val();
  MomHash h = v->hash();
  if (MOM_UNLIKELY(is_transient()))
    {
      h = (317*h) ^ (31*(((unsigned)v->vkind())+10));
      if (MOM_UNLIKELY(h==0))
        h = 31*(((unsigned)v->vkind()) & 0xfffff) + 10;
    }
  return mom_hash(h);
} // end MomValue::hash

/// internally, in many values, we end with a flexible array member of scalar or pointers
/// since that do not exist in standard C++, we use the fictious dimension:
#define MOM_FLEXIBLE_DIM 1



////////////////////////////////////////////////////////////////

//// a constant hash-consed sequence of integers
class MomIntSq final : public MomAnyVal   // in scalarv.cc
{
  friend class MomGC;
  const intptr_t _ivalarr[MOM_FLEXIBLE_DIM];
  MomIntSq(const intptr_t* iarr, MomSize sz, MomHash h);
  static constexpr const int _swidth_ = 256;
  static std::mutex _mtxarr_[_swidth_];
  static std::unordered_multimap<MomHash,const MomIntSq*> _maparr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h / 2316179)) % _swidth_;
  };
  static constexpr const int _chunklen_ = 512;
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomIntSq*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
  intptr_t unsafe_at(unsigned ix) const
  {
    return _ivalarr[ix];
  };
  const intptr_t *begin() const
  {
    return _ivalarr;
  };
  const intptr_t *end() const
  {
    return _ivalarr+sizew();
  };
  static MomHash compute_hash(const intptr_t* iarr, MomSize sz);
  static const MomIntSq* make_from_array(const intptr_t* iarr, MomSize sz);
  static const MomIntSq* make_from_vector(const std::vector<intptr_t>& ivec)
  {
    return make_from_array(ivec.data(), ivec.size());
  };
  static const MomIntSq* make_from_ilist(std::initializer_list<intptr_t> il)
  {
    return make_from_array(il.begin(), il.size());
  }
  /// example: MonIntSq::make_from_ints(1,-2,3)
  /// see http://stackoverflow.com/q/43180477/841108
  template <typename... Ts>
  static const MomIntSq* make_from_ints(Ts... args)
  {
    return make_from_ilist(std::initializer_list<intptr_t> {args...});
  }
  ///
  bool has_content(const intptr_t* iarr, MomSize sz) const
  {
    if (sz !=  sizew()) return false;
    if (sz > 0 && iarr==nullptr) return false;
    for (unsigned ix=0; ix<(unsigned)sz; ix++)
      if (MOM_LIKELY(_ivalarr[ix] != iarr[ix])) return false;
    return true;
  };
  virtual MomKind vkind() const
  {
    return MomKind::TagIntSqK;
  };
  virtual std::mutex* valmtx() const;
  virtual void scan_gc(MomGC*) const {};
  virtual void scan_dump(MomDumper*) const {};
};				// end class MomIntSq


////////////////////////////////////////////////////////////////

//// a constant hash-consed sequence of non-NaN doubles
class MomDoubleSq final : public MomAnyVal   // in scalarv.cc
{
  friend class MomGC;
  const double _dvalarr[MOM_FLEXIBLE_DIM];
  MomDoubleSq(const double* iarr, MomSize sz, MomHash h);
  static constexpr const int _swidth_ = 128;
  static constexpr const unsigned _chunklen_ = 256;
  static std::mutex _mtxarr_[_swidth_];
  static std::unordered_multimap<MomHash,const MomDoubleSq*> _maparr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h / 2317057)) % _swidth_;
  };
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomDoubleSq*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
public:
  const double *begin() const
  {
    return _dvalarr;
  };
  const double *end() const
  {
    return _dvalarr+sizew();
  };
  double unsafe_at(unsigned ix) const
  {
    return _dvalarr[ix];
  };
  static MomHash hash_double (double d);
  static MomHash compute_hash(const double* iarr, MomSize sz);
  static const MomDoubleSq* make_from_array(const double* darr, MomSize sz);
  static const MomDoubleSq* make_from_vector(const std::vector<double>& dvec)
  {
    return make_from_array(dvec.data(), dvec.size());
  };
  static const MomDoubleSq* make_from_ilist(std::initializer_list<double> il)
  {
    return make_from_array(il.begin(), il.size());
  }
  /// example: MonDoubleSq::make_from_doubles(1.2,-2.4,3.0)
  template <typename... Ts>
  static const MomIntSq* make_from_doubles(Ts... args)
  {
    return make_from_ilist(std::initializer_list<double> {args...});
  }
  bool has_content(const double* darr, MomSize sz) const
  {
    if (sz !=  sizew()) return false;
    if (sz > 0 && darr==nullptr) return false;
    for (unsigned ix=0; ix<(unsigned)sz; ix++)
      if (MOM_LIKELY(_dvalarr[ix] != darr[ix])) return false;
    return true;
  };
  virtual MomKind vkind() const
  {
    return MomKind::TagDoubleSqK;
  };
  virtual std::mutex* valmtx() const;
  virtual void scan_gc(MomGC*) const {};
  virtual void scan_dump(MomDumper*) const {};
};				// end class MomDoubleSq




////////////////////////////////////////////////////////////////

//// a constant hash-consed UTF-8 null-terminated string
class MomString final : public MomAnyVal   // in scalarv.cc
{
  friend class MomGC;
  const uint32_t _bylen;
  const char _bstr[MOM_FLEXIBLE_DIM];
  MomString(const char*cstr, MomSize sz, uint32_t bylen, MomHash h);
  static constexpr const int _swidth_ = 256;
  static constexpr const unsigned _chunklen_ = 512;
  static std::mutex _mtxarr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h / 2318021)) % _swidth_;
  };
  static std::unordered_multimap<MomHash,const MomString*> _maparr_[_swidth_];
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomString*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
public:
  const char*cstr() const
  {
    return _bstr;
  };
  unsigned bytelen() const
  {
    return _bylen;
  };
  static MomHash compute_hash_dim(const char*cstr, MomSize*psiz=nullptr, uint32_t*pbylen=nullptr);
  static const MomString* make_from_cstr(const char*cstr);
  static const MomString* make_from_string(const std::string&str)
  {
    return make_from_cstr(str.c_str());
  };
  static const MomString* make_from_stream(std::ostringstream&outs)
  {
    outs << std::flush;
    return make_from_string(outs.str());
  }
  static const MomString* make_sprintf(const char*fmt, ...) __attribute__((format(printf,1,2)));
  bool has_cstr_content(const char* cstr, int len= -1) const
  {
    if (MOM_UNLIKELY(cstr==nullptr)) return false;
    if (len<0) len = strlen(cstr);
    if (MOM_UNLIKELY((unsigned)len != _bylen))
      return false;
    return !strcmp(cstr, _bstr);
  }
  virtual MomKind vkind() const
  {
    return MomKind::TagStringK;
  };
  virtual void scan_gc(MomGC*) const {};
  virtual void scan_dump(MomDumper*) const {};
  virtual std::mutex* valmtx() const;
};				// end class MomString

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

struct MomObjptrLess
{
  inline bool operator()  (const MomObject*, const MomObject*);
};				// end MomObjptrLess

struct MomIdentBucketHash
{
  inline size_t operator() (const MomIdent& id) const
  {
    return ((145219L * id.hi().serial()) ^ (415271L * id.lo().serial()))
           + id.hi().serial();
  }
}; //end MomIdentBucketHash


struct MomObjptrHash
{
  inline size_t operator() (const MomObject*pob) const;
};				// end MomObjptrHash

typedef std::set<MomObject*,MomObjptrLess> MomObjptrSet;
typedef std::vector<MomObject*> MomObjptrVector;

// common super class for sets and tuples of objects

class MomAnyObjSeq : public MomAnyVal   // in seqobjv.cc
{
  friend class MomGC;
  MomObject*const _obseq[MOM_FLEXIBLE_DIM];
protected:
  template<unsigned hinit, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
  static inline MomHash compute_hash_seq(MomObject*const* obarr, unsigned sz);
  MomAnyObjSeq(MomKind kd, MomObject*const* obarr, MomSize sz, MomHash h);
  inline bool has_content(MomObject*const*obarr, unsigned sz) const
  {
    if (sz !=  sizew()) return false;
    if (sz > 0 && obarr==nullptr) return false;
    for (unsigned ix=0; ix<(unsigned)sz; ix++)
      if (MOM_LIKELY(_obseq[ix] != obarr[ix])) return false;
    return true;
  }
  virtual void scan_gc(MomGC*) const;
  virtual void scan_dump(MomDumper*) const;
public:
  typedef MomObject*const* iterator;
  iterator begin() const
  {
    return _obseq+0;
  };
  iterator end() const
  {
    return _obseq+sizew();
  };
  MomObject* unsafe_at(unsigned ix) const
  {
    return _obseq[ix];
  };
  MomObject* at(unsigned ix) const
  {
    if (ix < sizew()) return unsafe_at(ix);
    return nullptr;
  };
  MomObject* checked_at(unsigned ix) const
  {
    if (ix < sizew()) return unsafe_at(ix);
    MOM_FAILURE("MomAnyObjSeq::checked_at index " << ix << " out of range");
  };
  MomObject* nth(int i) const
  {
    int sz = (int)sizew();
    if (i<0) i+=sz;
    if (i<sz) return unsafe_at((unsigned)i);
    return nullptr;
  }
  MomObject* checked_nth(int i) const
  {
    int origi = i;
    int sz = (int)sizew();
    if (i<0) i+=sz;
    if (i<sz) return unsafe_at((unsigned)i);
    MOM_FAILURE("MomAnyObjSeq::checked_nth index " << origi << " out of range");
  }
};				// end class MomAnyObjSeq


////////////////////////////////////////////////////////////////
class MomSet : public MomAnyObjSeq
{
  static constexpr unsigned hinit = 123017;
  static constexpr unsigned k1 = 103049;
  static constexpr unsigned k2 = 13063;
  static constexpr unsigned k3 = 143093;
  static constexpr unsigned k4 = 14083;
  static constexpr const int _swidth_ = 256;
  static constexpr const unsigned _chunklen_ = 256;
  static std::mutex _mtxarr_[_swidth_];
  static std::unordered_multimap<MomHash,const MomSet*> _maparr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h / 2325097)) % _swidth_;
  };
  MomSet(MomObject*const* obarr, MomSize sz, MomHash h)
    : MomAnyObjSeq(MomKind::TagSetK, obarr,sz, h) {};
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomSet*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
public:
  static MomHash compute_hash(MomObject*const* obarr, unsigned sz);
  static const MomSet* make_from_ascending_array(MomObject*const* obarr, MomSize sz);
  static const MomSet* make_from_objptr_set(const MomObjptrSet&oset);
  static const MomSet* make_from_objptr_ilist(std::initializer_list<MomObject*> il)
  {
    MomObjptrSet set(il);
    return make_from_objptr_set(set);
  };
  template <typename... Ts>
  static const MomSet* make_from_objptrs(Ts... args)
  {
    return make_from_objptr_ilist(std::initializer_list<MomObject*> {args...});
  };
  static const MomSet* make_from_ascending_objptr_vector(const MomObjptrVector&ovec)
  {
    return make_from_ascending_array(ovec.data(), ovec.size());
  };
  virtual MomKind vkind() const
  {
    return MomKind::TagSetK;
  };
  virtual std::mutex* valmtx() const;
}; // end class MomSet




////////////////////////////////////////////////////////////////
class MomTuple : public MomAnyObjSeq
{
  static constexpr unsigned hinit = 173081;
  static constexpr unsigned k1 = 135089;
  static constexpr unsigned k2 = 35671;
  static constexpr unsigned k3 = 5693;
  static constexpr unsigned k4 = 56873;
  MomTuple(MomObject*const* obarr, MomSize sz, MomHash h)
    : MomAnyObjSeq(MomKind::TagTupleK, obarr,sz, h) {};
  static constexpr const int _swidth_ = 256;
  static constexpr const unsigned _chunklen_ = 256;
  static std::mutex _mtxarr_[_swidth_];
  static std::unordered_multimap<MomHash,const MomTuple*> _maparr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h / 2327183)) % _swidth_;
  };
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomTuple*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
public:
  static const MomTuple* make_from_array(MomObject*const* obarr, MomSize sz);
  static const MomTuple* make_from_objptr_vector(const MomObjptrVector&ovec)
  {
    return make_from_array(ovec.data(), ovec.size());
  };
  static const MomTuple* make_from_objptr_ilist(std::initializer_list<MomObject*> il)
  {
    return make_from_array(il.begin(), il.size());
  };
  template <typename... Ts>
  static const MomSet* make_from_objptrs(Ts... args)
  {
    return make_from_objptr_ilist(std::initializer_list<MomObject*> {args...});
  };
  static MomHash compute_hash(MomObject*const* obarr, unsigned sz);
  virtual MomKind vkind() const
  {
    return MomKind::TagTupleK;
  };
  virtual std::mutex* valmtx() const;
}; // end class MomTuple




////////////////////////////////////////////////////////////////
class MomNode final : public MomAnyVal // in nodev.cc
{
  const MomObject* const _nod_conn;
  const MomValue _nod_sons[MOM_FLEXIBLE_DIM];
  MomNode(const MomObject*conn, const MomValue*sons, unsigned arity, MomHash h)
    : MomAnyVal(MomKind::TagNodeK, arity, h), _nod_conn(conn), _nod_sons{nullptr}
  {
    memcpy (const_cast<MomValue*>(_nod_sons), sons, arity * sizeof(MomValue));
  };
  static constexpr const int _swidth_ = 512;
  static constexpr const unsigned _chunklen_ = 512;
  static std::mutex _mtxarr_[_swidth_];
  static std::unordered_multimap<MomHash,const MomNode*> _maparr_[_swidth_];
  static unsigned slotindex(MomHash h)
  {
    return (h ^ (h /3500183)) % _swidth_;
  };
  static void gc_todo_clear_mark_slot(MomGC*gc,unsigned slotix);
  static void gc_todo_clear_mark_chunk(MomGC*gc,unsigned slotix, unsigned chunkix, std::array<MomNode*,_chunklen_> arrptr);
public:
  static void gc_todo_clear_marks(MomGC*gc);
public:
  static MomHash compute_hash(const MomObject*conn, const MomValue*arr, MomSize sz);
  bool has_content(const MomObject*conn, const MomValue*varr, MomSize sz) const
  {
    if (_nod_conn != conn) return false;
    if (sizew() != sz) return false;
    if (sz > 0 && varr == nullptr) return false;
    for (unsigned ix=0; ix<(unsigned)sz; ix++)
      if (MOM_LIKELY(_nod_sons[ix] != varr[ix])) return false;
    return true;
  }
  static const MomNode* make_from_array(const MomObject*conn, const MomValue*varr, MomSize sz);
  static const MomNode* make_from_vector(const MomObject*conn, const std::vector<MomValue>& vvec)
  {
    return make_from_array(conn, vvec.data(), vvec.size());
  };
  static const MomNode* make_from_ilist(const MomObject*conn, std::initializer_list<const MomValue> il)
  {
    return make_from_array(conn, il.begin(), il.size());
  };
  template <typename... Ts>
  static const MomNode* make_from_values(const MomObject*conn, Ts... args)
  {
    return make_from_ilist(conn, std::initializer_list<const MomValue> {args...});
  };
  virtual MomKind vkind() const
  {
    return MomKind::TagNodeK;
  };
  const MomObject* conn() const
  {
    return  _nod_conn;
  };
  typedef const MomValue* iterator;
  const MomValue* begin() const
  {
    return _nod_sons;
  };
  const MomValue* end() const
  {
    return _nod_sons + sizew();
  };
  const MomValue unsafe_at(unsigned ix) const
  {
    return _nod_sons[ix];
  };
  virtual std::mutex* valmtx() const;
protected:
  ~MomNode() {};
  virtual void scan_gc(MomGC*) const ;
  virtual void scan_dump(MomDumper*) const;
};    // end class MomNode




////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

enum class MomSpace : std::uint8_t
{
  TransientSp = 0,
  PredefSp,
  GlobalSp,
  UserSp
};


////////////////@@@@@@@@@@
class MomObject final : public MomAnyVal // in objectv.cc
{
  friend struct MomPayload;
  friend class MomDumper;
  friend class MomLoader;
  static std::mutex _bumtxarr_[MomSerial63::_maxbucket_];
  static std::unordered_map<MomIdent,MomObject*,MomIdentBucketHash> _bumaparr_[MomSerial63::_maxbucket_];
  static std::mutex _predefmtx_;
  static MomObjptrSet _predefset_;
  static constexpr unsigned _bumincount_ = 16;
  const MomIdent _ob_id;
  std::atomic<MomSpace> _ob_space;
  double _ob_mtime;
  mutable std::shared_mutex _ob_shmtx;
  std::unordered_map<MomObject*,MomValue,MomObjptrHash> _ob_attrs;
  std::vector<MomValue> _ob_comps;
  MomPayload* _ob_payl;
  MomObject(const MomIdent id, MomHash h);
public:
  struct PayloadEmission
  {
    std::string pye_kind;
    std::string pye_module;
    std::string pye_init;
    std::string pye_content;
  };
  ~MomObject();
  template<typename ResType>
  ResType locked_access(std::function<ResType(MomObject const*)>fun) const
  {
    std::shared_lock<std::shared_mutex> lk(_ob_shmtx);
    return fun(this);
  }
  void locked_access(std::function<void(MomObject const*)>fun) const
  {
    std::shared_lock<std::shared_mutex> lk(_ob_shmtx);
    fun(this);
  }
  template<typename ResType>
  ResType locked_modify(std::function<ResType(MomObject*)>fun)
  {
    std::unique_lock<std::shared_mutex> lk(_ob_shmtx);
    return fun(this);
  }
  void locked_modify(std::function<void(MomObject*)>fun)
  {
    std::unique_lock<std::shared_mutex> lk(_ob_shmtx);
    fun(this);
  }
  static MomObject*find_object_of_id(const MomIdent id);
  static MomObject*make_object_of_id(const MomIdent id);
  static MomObject*make_object(void); // of random id
  static void initialize_predefined(void);
  static const MomSet* predefined_set(void);
  MomSpace space() const
  {
    return std::atomic_load(&_ob_space);
  };
  MomObject* set_space(MomSpace sp);
  bool same(const MomObject*ob) const
  {
    return this == ob;
  };
  const MomIdent id() const
  {
    return _ob_id;
  };
  bool less(const MomObject*ob) const
  {
    if (!ob || this==ob) return false;
    if (_ob_id < ob->_ob_id) return true;
    else if (_ob_id > ob->_ob_id) return false;
    MOM_FATALOG("non-identical objects sharing same id=" << _ob_id);
  };
  static bool less2(const MomObject*ob1, const MomObject*ob2)
  {
    if (ob1==ob2) return false;
    if (!ob1) return true;
    if (!ob2) return false;
    return ob1->less(ob2);
  }
  bool less_equal(const MomObject*ob) const
  {
    if (!ob) return false;
    if (this == ob) return true;
    return less (ob);
  };
  static bool less_equal2(const MomObject*ob1, const MomObject*ob2)
  {
    if (ob1==ob2) return true;
    if (!ob1) return true;
    if (!ob2) return false;
    return ob1->less_equal(ob2);
  }
  bool greater(const MomObject*ob) const
  {
    if (!ob) return true;
    return ob->less(this);
  };
  bool greater_equal(const MomObject*ob) const
  {
    if (!ob) return true;
    return ob->less_equal(this);
  };
  virtual MomKind vkind() const
  {
    return MomKind::TagObjectK;
  };
  virtual void scan_gc(MomGC*) const;
  virtual void scan_dump(MomDumper*du) const; // in state.cc
  virtual void scan_dump_content(MomDumper*du) const; // in state.cc
  virtual std::mutex* valmtx() const;
  void unsync_emit_dump_content(MomDumper*du, MomEmitter&em) const; // in state.cc
  void unsync_emit_dump_payload(MomDumper*du, PayloadEmission&) const; // in state.cc
  inline void unsync_clear_payload();
  void unsync_clear_all();
  MomValue unsync_get_phys_attr(const MomObject*pobattr) const
  {
    if (!pobattr)
      return MomValue{nullptr};
    auto it = _ob_attrs.find(const_cast<MomObject*>(pobattr));
    if (it != _ob_attrs.end())
      return it->second;
    return MomValue{nullptr};
  }
  void unsync_put_phys_attr(const MomObject*pobattr, const MomValue valattr)
  {
    if (!pobattr)
      return;
    if (!valattr)
      _ob_attrs.erase(const_cast<MomObject*>(pobattr));
    else
      _ob_attrs.insert({const_cast<MomObject*>(pobattr),valattr});
  }
  void unsync_remove_phys_attr(const MomObject*pobattr)
  {
    if (!pobattr)
      return;
    _ob_attrs.erase(const_cast<MomObject*>(pobattr));
  }
  void unsync_append_comp(const MomValue vcomp)
  {
    _ob_comps.push_back(vcomp);
  }
  void unsync_reserve_more_comp(unsigned n)
  {
    _ob_comps.reserve(_ob_comps.size()+n);
  }
  void unsync_touch(double t)
  {
    _ob_mtime = t;
  }
  void unsync_touch(void)
  {
    unsync_touch(mom_clock_time(CLOCK_REALTIME));
  };
  MomPayload* unsync_payload() const
  {
    return _ob_payl;
  }
  template<class PaylClass, typename...ArgsType> PaylClass* unsync_make_payload(ArgsType... args)
  {
    unsync_clear_payload();
    auto py = _ob_payl = new PaylClass(this,args...);
    return static_cast<PaylClass*>(py);
  }
}; // end class MomObject


inline std::ostream& operator << (std::ostream& out, const MomObject*pob)
{
  if (pob)
    out << pob->id();
  else
    out << "__";
  return out;
} // end operator << for MomObject*

bool
MomObjptrLess::operator()  (const MomObject*ob1, const MomObject*ob2)
{
  return MomObject::less2(ob1, ob2);
}      // end MomObjptrLess::operator


size_t
MomObjptrHash::operator() (const MomObject*pob) const
{
  if (!pob) return 0;
  return pob->hash();
}


#define MOM_PREDEF(Id) mompredef##Id

#define MOM_HAS_PREDEF(Id,Hi,Lo,Hash) extern "C" MomObject*MOM_PREDEF(Id);
#include "_mom_predef.h"

#define MOM_GLOBDATA_VAR(Nam) momgdata_##Nam
#define MOM_LOAD_GLOBDATA(Nam) (MOM_GLOBDATA_VAR(Nam).load())
#define MOM_STORE_GLOBDATA(Nam,Obp) (MOM_GLOBDATA_VAR(Nam).store(Obp))
#define MOM_XCHG_GLOBDATA(Nam,Obp) (MOM_GLOBDATA_VAR(Nam).exchange(Obp))
#define MOM_HAS_GLOBDATA(Nam) extern "C" std::atomic<MomObject*> MOM_GLOBDATA_VAR(Nam);
#include "_mom_globdata.h"

class MomRegisterGlobData
{
  static std::mutex _gd_mtx_;
  static std::map<std::string,std::atomic<MomObject*>*> _gd_dict_;
  std::string _gd_name;
public:
  static void register_globdata(const std::string&nam,std::atomic<MomObject*>&gdata)
  {
    if (!mom_valid_name_radix_len(nam.c_str(), nam.size()))
      MOM_FAILURE("register_globdata bad name:" << nam);
    std::lock_guard<std::mutex> gu(_gd_mtx_);
    _gd_dict_.insert({nam,&gdata});
  }
  static void forget_globdata(const std::string&nam)
  {
    std::lock_guard<std::mutex> gu(_gd_mtx_);
    _gd_dict_.erase(nam);
  }
  static std::atomic<MomObject*>*find_globdata(const std::string&nam)
  {
    std::lock_guard<std::mutex> gu(_gd_mtx_);
    auto it = _gd_dict_.find(nam);
    if (it != _gd_dict_.end())
      return it->second;
    return nullptr;
  };
  static void every_globdata(std::function<bool(const std::string&nam,std::atomic<MomObject*>*pdata)> f)
  {
    std::lock_guard<std::mutex> gu(_gd_mtx_);
    for (auto p : _gd_dict_)
      if (f(p.first,p.second))
        return;
  }
  MomRegisterGlobData(const std::string&nam,std::atomic<MomObject*>&gdata)
    : _gd_name(nam)
  {
    register_globdata(nam,gdata);
  }
  ~MomRegisterGlobData()
  {
    forget_globdata(_gd_name);
  };
  MomRegisterGlobData(const MomRegisterGlobData&) = delete;
  MomRegisterGlobData(MomRegisterGlobData&&) = delete;
};				// end MomRegisterGlobData

////////////////
typedef void MomPyv_destr_sig(struct MomPayload*payl,MomObject*own);
typedef void MomPyv_scangc_sig(const struct MomPayload*payl,MomObject*own,MomGC*gc);
typedef void MomPyv_scandump_sig(const struct MomPayload*payl,MomObject*own,MomDumper*du);
typedef void MomPyv_emitdump_sig(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont);
typedef MomPayload* MomPyv_initload_sig(MomObject*own,MomLoader*ld,const char*inits);
typedef void MomPyv_loadfill_sig(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills);
typedef MomValue MomPyv_getmagic_sig(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob);
typedef MomValue MomPyv_fetch_sig(const struct MomPayload*payl,const MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen);
typedef void MomPyv_update_sig(struct MomPayload*payl,MomObject*own,const MomObject*attrob, const MomValue*vecarr, unsigned veclen);
typedef void MomPyv_step_sig(struct MomPayload*payl,MomObject*own,const MomValue*vecarr, unsigned veclen);
#define MOM_PAYLOADVTBL_MAGIC 0x1aef1d65 /* 451878245 */
/// a payloadvtbl named FOO is declared as mompyvtl_FOO
#define MOM_PAYLOADVTBL_SUFFIX "mompyvtl_"
#define MOM_PAYLOADVTBL(Nam) mompyvtl_##Nam
struct MomVtablePayload_st // explicit "vtable-like" of payload
{
  const unsigned pyv_magic; // always MOM_PAYLOADVTBL_MAGIC
  const unsigned pyv_size; // the actual sizeof the payload
  const char*pyv_name;
  const char*pyv_module;
  const MomPyv_destr_sig* pyv_destroy;
  const MomPyv_scangc_sig* pyv_scangc;
  const MomPyv_scandump_sig* pyv_scandump;
  const MomPyv_emitdump_sig* pyv_emitdump;
  const MomPyv_initload_sig* pyv_initload;
  const MomPyv_loadfill_sig* pyv_loadfill;
  const MomPyv_getmagic_sig* pyv_getmagic;
  const MomPyv_fetch_sig* pyv_fetch;
  const MomPyv_update_sig* pyv_update;
  const MomPyv_step_sig* pyv_step;
  const void*pyv__spare1;
  const void*pyv__spare2;
  const void*pyv__spare3;
};

class MomRegisterPayload
{
  static std::mutex _pd_mtx_;
  static std::map<std::string,const MomVtablePayload_st*> _pd_dict_;
  const MomVtablePayload_st*_pd_vtp;
public:
  static void register_payloadv(const MomVtablePayload_st*pv);
  static void forget_payloadv(const MomVtablePayload_st*pv);
  static const MomVtablePayload_st*find_payloadv(const std::string&nam);
  MomRegisterPayload() = delete;
  MomRegisterPayload(const MomRegisterPayload&) = delete;
  MomRegisterPayload(MomRegisterPayload&&) = delete;
  MomRegisterPayload(const MomVtablePayload_st&pv): MomRegisterPayload(&pv) {};
  static void every_payloadv(std::function<bool(const MomVtablePayload_st*)> f)
  {
    std::lock_guard<std::mutex> gu(_pd_mtx_);
    for (auto p : _pd_dict_)
      if (f(p.second))
        return;
  }
  MomRegisterPayload(const MomVtablePayload_st*pv)
    : _pd_vtp(pv)
  {
    register_payloadv(pv);
  }
  ~MomRegisterPayload()
  {
    forget_payloadv(_pd_vtp);
    *const_cast<MomVtablePayload_st**>(&_pd_vtp) = nullptr;
  }
}; // class MomRegisterPayload

struct MomPayload
{
  friend class MomObject;
  const struct MomVtablePayload_st* _py_vtbl;
  MomObject* _py_owner;
  ~MomPayload()
  {
    auto ownob = _py_owner;
    _py_owner = nullptr;
    if (_py_vtbl->pyv_destroy)
      _py_vtbl->pyv_destroy(this,ownob);
    if (ownob) const_cast<MomObject*>(ownob)->unsync_clear_payload();
  }
  MomPayload(const struct MomVtablePayload_st*vtbl, MomObject* owner) :
    _py_vtbl(vtbl),
    _py_owner(owner)
  {
    if (!vtbl)
      MOM_FAILURE("missing vtbl in MomPayload");
    if (vtbl->pyv_magic != MOM_PAYLOADVTBL_MAGIC)
      MOM_FAILURE("bad magic in vtbl in MomPayload");
    if (!owner)
      MOM_FAILURE("missing owner in MomPayload");
  };
  void scan_gc_payl(MomGC*gc) const
  {
    auto ownob = _py_owner;
    if (!ownob) return;
    if (_py_vtbl->pyv_scangc)
      _py_vtbl->pyv_scangc(this,ownob,gc);
  }
  void scan_dump_payl(MomDumper*du) const
  {
    auto ownob = _py_owner;
    if (!ownob) return;
    if (_py_vtbl->pyv_scandump)
      _py_vtbl->pyv_scandump(this,ownob,du);
  }
};    // end MomPayload
////////////////////////////////////////////////////////////////

class MomParser			// in file parsemit.cc
{
public:
  enum TokenKind
  {
    Ptok_NONE,
    PtokComment,
    PtokNil,
    PtokInt,
    PtokString,
    PtokId,
    PtokName,
    PtokTuple,
    PtokSet,
    PtokIntSeq,
    PtokDoubleSeq,
    PtokNode,
  };
private:
  std::istream &_parinp; // input stream
  std::string _parlinstr; // line string
  unsigned _parlincount; // line count
  long _parlinoffset; // offset of current line
  int _parcol; // current column
  bool _pardebug;	// if set, activate MOM_DEBUGLOG(parse, ...)
  bool _parsilent; // if set, failure is silent without backtrace
  bool _parnobuild; // if set, no values are built
  bool _parmakefromid; // if set, make objects from id
  bool _parhaschunk;   // if set, accept code chunks
  std::string _parname;		// name of parser in messages
  std::function<void(TokenKind tok,int startcol, unsigned startlineno)>_parfun; // function called (e.g. for colorization)
public:
  class Mom_parse_failure : public Mom_runtime_failure
  {
    const MomParser *_pars;
  public:
    Mom_parse_failure(const MomParser* pa, const char*fil, int lin, const std::string&msg)
      : Mom_runtime_failure(fil,lin,msg+"@"+pa->location_str()), _pars(pa) {}
    ~Mom_parse_failure() = default;
    const MomParser* parser() const
    {
      return _pars;
    };
  };
  MomParser(std::istream&inp, unsigned lincount=0)
    : _parinp(inp),  _parlinstr{}, _parlincount(lincount), _parcol{0},
      _pardebug{false}, _parsilent{false}, _parmakefromid{false},
      _parhaschunk{false},
      _parname(), _parfun()
  {
  }
  MomParser& set_name(const std::string&nam)
  {
    _parname=nam;
    return *this;
  };
  MomParser& set_no_build(bool nobuild)
  {
    _parnobuild = nobuild;
    return *this;
  };
  MomParser& set_debug(bool hasdebug)
  {
    _pardebug = hasdebug;
    return *this;
  };
  MomParser& set_has_chunk(bool haschunk)
  {
    _parhaschunk = haschunk;
    return *this;
  };
  MomParser& set_make_from_id(bool mf)
  {
    _parmakefromid=mf;
    return *this;
  };
  MomParser& set_parser_fun(std::function<void(TokenKind tok,int startcol, unsigned startlineno)>fun)
  {
    _parfun = fun;
    return *this;
  };
  std::string name() const
  {
    return _parname;
  };
  bool making_from_id() const
  {
    return _parmakefromid;
  };
  std::string location_str() const
  {
    char lbuf[48];
    memset(lbuf, 0, sizeof(lbuf));
    snprintf(lbuf, sizeof(lbuf), ":L%u,C%d", _parlincount, _parcol);
    return _parname + lbuf;
  };
  ~MomParser()
  {
  }
  std::istream& input() const
  {
    return _parinp;
  };
  bool silent() const
  {
    return _parsilent;
  };
  bool eol() const
  {
    return _parcol >= (int)_parlinstr.size();
  };
  bool eof() const
  {
    return eol() && !_parinp;
  };
  static constexpr unsigned _maxpeek_ = 16;
  int peekbyte(unsigned off=0) const
  {
    if (_parcol<0) return EOF;
    if (_parcol+off > _parlinstr.size()) return EOF;
    return _parlinstr[_parcol+off];
  }
  const char* peekchars(unsigned off=0) const
  {
    if (_parcol<0) return nullptr;
    if (_parcol+off > _parlinstr.size()) return nullptr;
    return _parlinstr.c_str()+_parcol+off;
  }
  bool gotcstr (const char*str, unsigned off=0) const
  {
    return !strncmp(str, peekchars(off), strlen(str));
  }
  bool haskeyword(const char*str, unsigned off=0)
  {
    unsigned slen=strlen(str);
    int nc=0;
    if (!strncmp(str, peekchars(off), slen)
        && ((nc=peekbyte(off+slen))<127 && !(nc=='_' || isalnum(nc))))
      {
        consume(off+slen);
        return true;
      }
    return false;
  }
  bool hasdelim(const char*str, unsigned off=0)
  {
    unsigned slen=strlen(str);
    int nc=0;
    if (!strncmp(str, peekchars(off), slen)
        && ((nc=peekbyte(off+slen))<127 && !(nc=='_' || ispunct(nc))))
      {
        consume(off+slen);
        return true;
      }
    return false;
  }
  void consume(unsigned nbytes)
  {
    auto linsiz = _parlinstr.size();
    if (_parcol + nbytes < linsiz)
      {
        _parcol+=nbytes;
      }
    else
      {
        next_line();
      }
  }
  void restore_state(long offset, int linecount, int col)
  {
    if ((long)_parinp.tellg() != (long)(_parlinoffset+_parlinstr.size()+1))
      {
        _parlinstr.clear();
        _parinp.seekg(offset);
        std::getline(_parinp, _parlinstr);
        _parcol = 0;
        _parlincount = linecount;
      }
    _parcol = col;
  };
  MomParser& skip_spaces()
  {
    for (;;)
      {
        if (eol())
          {
            if (!_parinp) return *this;
            next_line();
          }
        else if (isspace(peekbyte()))
          _parcol++;
        else break;
      }
    return *this;
  }
  inline void next_line(void);
  MomValue parse_value(bool* pgotval);
  MomValue parse_chunk(bool* pgotchunk); // parse a code chunk, including the ending )$
  bool parse_chunk_element(std::vector<MomValue>&vec);
  MomObject* parse_objptr(bool* pgotob);
  /// given some name, fetch the corresponding named object
  virtual MomObject* fetch_named_object(const std::string&)
  {
    return nullptr;
  };
  /// from some name in a code chunk, gives a value
  virtual MomValue chunk_name(const std::string&)
  {
    return nullptr;
  };
  /// from some id in a code chunk, gives a value
  virtual MomValue chunk_id(MomIdent)
  {
    return nullptr;
  };
  /// from some embedded value in a code chunk with $% gives a non-nil value
  virtual MomValue chunk_value(const MomValue)
  {
    return nullptr;
  };
  /// from some object ($-prefixed) in a code chunk, gives the value
  virtual MomValue chunk_dollarobj(MomObject*)
  {
    return nullptr;
  }
  /// return the chunk value from a vector
  virtual MomValue chunk_value(const std::vector<MomValue>&)
  {
    return nullptr;
  }
};				// end class MomParser

#define MOM_PARSE_FAILURE_AT(Par,Fil,Lin,Log) do {		\
    std::ostringstream _olog_##Lin;				\
    const MomParser* _pars_##Lin = Par;				\
    _olog_##Lin << "PARSE FAIL:" << Log << std::flush;		\
    if (!_pars_##Lin->silent())					\
      mom_failure_backtrace_at(Fil, Lin,			\
			       _olog_##Lin.str());		\
    throw MomParser::Mom_parse_failure (_pars_##Lin,Fil,Lin,	\
					_olog_##Lin.str());	\
  } while(0)

#define MOM_PARSE_FAILURE_AT_BIS(Par,Fil,Lin,Log)	\
  MOM_PARSE_FAILURE_AT(Par,Fil,Lin,Log)

#define MOM_PARSE_FAILURE(Par,Log)			\
  MOM_PARSE_FAILURE_AT_BIS((Par),__FILE__,__LINE__,Log)


void
MomParser::next_line()
{
  _parlinoffset = _parinp.tellg();
  std::getline(_parinp, _parlinstr);
  if (MOM_UNLIKELY(!g_utf8_validate(_parlinstr.c_str(), _parlinstr.size(),
                                    nullptr)))
    MOM_PARSE_FAILURE(this,"invalid UTF8 line#" << _parlincount
                      << ":" << _parlinstr);
  _parcol = 0;
  if (_parinp)
    _parlincount++;
} // end MomParser::next_line

////////////////
class MomEmitter 		// in file parsemit.cc
{
  std::ostream &_emout;
  std::ostream::pos_type _emlastnewline;
  int _emlinewidth;
  bool _emnotransient;
public:
  static constexpr int _default_line_width_ = 96;
  static constexpr int _min_line_width_ = 48;
  static constexpr int _max_indent = 16;
  MomEmitter(std::ostream&out)
    : _emout(out),
      _emlastnewline(out.tellp()),
      _emlinewidth(_default_line_width_),
      _emnotransient(false)
  {
  }
  MomEmitter& set_line_width(int lw)
  {
    if (lw < _min_line_width_)
      lw = _min_line_width_;
    _emlinewidth = lw;
    return *this;
  };
  MomEmitter& show_transient(bool show=true)
  {
    _emnotransient = !show;
    return *this;
  };
  virtual ~MomEmitter() {};
  virtual bool skippable_object(const MomObject*pob) const;
  virtual bool skippable_connective(const MomObject*pob) const
  {
    return skippable_object(pob);
  };
  void emit_space(int depth)
  {
    if (column() >= _emlinewidth+1)
      emit_newline(depth);
    else
      _emout << ' ';
  }
  void emit_maybe_newline(int depth)
  {
    if (column() >= _emlinewidth) emit_newline(depth);
  }
  void emit_newline(int depth);
  void emit_raw_newline();
  void emit_value(const MomValue v, int depth=0);
  void emit_string_value(const MomString*strv, int depth=0, bool asraw=false);
  void emit_objptr(const MomObject*pob, int depth=0);
  std::ostream &out()
  {
    return _emout;
  };
  int column() const
  {
    return _emout.tellp() - _emlastnewline;
  };
  bool too_wide(int off=0) const
  {
    return column()+off >= _emlinewidth;
  };
};				// end class MomEmitter

////////////////////////////////////////////////////////////////
class MomGC
{
  friend class MomAnyVal;
  static std::atomic<bool> _forbid_allocation_;
  std::thread::id _gc_thrid;
  std::mutex _gc_mtx;
  std::condition_variable _gc_changecond;
  std::deque<MomAnyVal*> _gc_valque;
  std::deque<MomObject*> _gc_objque;
  std::deque<std::function<void(MomGC*)>> _gc_todoque;
  MomGC(const MomGC&) = delete;
  MomGC(MomGC&&) = delete;
  MomGC();
  ~MomGC();
  void unsync_start_gc_cycle(void);
  void unsync_add_todo(std::function<void(MomGC*)> fun)
  {
    MOM_ASSERT(fun, "empty fun for GC unsync_add_todo");
    _gc_todoque.push_back(fun);

  }
public:
  static MomGC the_garbcoll;
  static void incremental_run(void);
  void scan_anyval(MomAnyVal*);
  void scan_value(const MomValue);
  void scan_object(MomObject*);
  void add_todo(std::function<void(MomGC*)>);
};				// end class MomGC

////////////////////////////////////////////////////////////////
void
MomObject::unsync_clear_payload()
{
  auto py = _ob_payl;
  _ob_payl = nullptr;
  if (py)
    delete py;
}

void
MomValue::scan_gc(MomGC*gc) const
{
  auto pv = to_val();
  if (pv)
    pv->scan_gc(gc);
} // end MomValue::scan_gc

void
MomValue::scan_dump(MomDumper*du) const
{
  auto pv = to_persistent();
  if (pv)
    pv->scan_dump(du);
}      // end MomValue::scan_dump


void*
MomAnyVal::operator new (size_t sz, MomNewTag, size_t gap)
{
  if (MOM_UNLIKELY(MomGC::_forbid_allocation_.load()))
    MOM_FAILURE("forbidden globally to allocate sz=" << sz << " gap=" << gap);
  if (MOM_UNLIKELY(!_allocok))
    MOM_FAILURE("forbidden thread to allocate sz=" << sz << " gap=" << gap);
  auto fulsiz = mom_align(sz) + mom_align(gap);
  _wordalloca.fetch_add(fulsiz / sizeof(void*));
  return ::operator new(fulsiz);
} // end MomAnyVal::operator new


/// in state.cc
extern "C" void mom_dump_in_directory(const char*dirname);
extern "C" void mom_load_from_directory(const char*dirname);
extern "C" void mom_dump_todo_scan(MomDumper*du, std::function<void(MomDumper*)> todofun);
extern "C" void mom_dump_todo_emit(MomDumper*du, std::function<void(MomDumper*)> todofun);
extern "C" void mom_dump_named_update_defer(MomDumper*du, MomObject*pob, std::string nam);


/// in paylsimple.cc
extern "C" void mom_register_unsync_named(MomObject*obj, const char*name);
extern "C" void mom_forget_unsync_named_object(MomObject*obj);
extern "C" void mom_forget_name(const char*name);
extern "C" MomObject*mom_find_named(const char*name);
extern "C" const char* mom_get_unsync_name(MomObject*obj);
extern "C" const std::string mom_get_unsync_string_name(MomObject*obj);
extern "C" MomObject*mom_unsync_named_object_proxy(MomObject*objn);
extern "C" void mom_unsync_named_object_set_proxy(MomObject*objn, MomObject*obproxy);
extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(named);
extern "C" MomObject*mom_unsync_pset_object_proxy(MomObject*objn);
extern "C" void mom_unsync_pset_object_set_proxy(MomObject*objn, MomObject*obproxy);
extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(set);
#endif /*MONIMELT_INCLUDED_ */
