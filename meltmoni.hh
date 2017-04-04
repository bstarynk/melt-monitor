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
#include <cstdint>
#include <vector>
#include <map>
#include <unordered_map>
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


typedef std::uint32_t MomHash; // hash codes are on 32 bits, but could become on 56 bits!

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
  void to_cbuf16(char buf[16]) const;
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


////////////////
class MomIdent
{
public:
  const MomSerial63 _idhi, _idlo;
  static constexpr const unsigned _charlen_ = 2*MomSerial63::_nbdigits_+2;
  MomIdent() : _idhi(), _idlo() {};
  MomIdent(std::nullptr_t) : _idhi(), _idlo() {};
  MomIdent(MomSerial63 hi, MomSerial63 lo) : _idhi(hi), _idlo(lo) {};
  MomIdent(uint64_t hi, uint64_t lo, bool nocheck = false) :
    _idhi(hi, nocheck), _idlo(lo, nocheck) {};
  MomIdent(const MomIdent&id) = default;
  MomIdent(MomIdent&&id) = default;
  ~MomIdent() = default;
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
  void to_cbuf32(char buf[32]) const;
  std::string to_string() const;
  static const MomIdent make_from_cstr(const char *s, const char *&end,
                                       bool fail = false);
  static const MomIdent make_from_cstr(const char *s, bool fail = false)
  {
    const char *end = nullptr;
    return make_from_cstr(s, end, fail);
  };
  MomHash hash() const
  {
    if (is_null()) return 0;
    uint64_t shi = _idhi.serial();
    uint64_t slo = _idlo.serial();
    MomHash h = ((shi * 81281) ^ (slo * 33769)) + 11*(shi>>35) - 31*(slo>>47);
    if (MOM_UNLIKELY(h==0))
      h = 3*(shi & 0xffffff) + 5*(slo & 0xffffff) + 315;
    return h;
  }
};				// end class MomIdent


inline std::ostream &operator<<(std::ostream &os, const MomIdent id)
{
  char buf[32];
  id.to_cbuf32(buf);
  os << buf;
  return os;
} // end operator << MomIdent




////////////////////////////////////////////////////////////////

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


////////////////
// forward declarations.
class MomAnyVal;		// abstract superclass of any memory value
class MomIntSq;		// value, read-only hash-consed sequence of integers
class MomDoubleSq;	// value, read-only hash-consed sequence of doubles
class MomString;	// value, UTF8 read-only hash-consed string
class MomAnySeqVal;		// abstract superclass of hash-consed object sequences (sets or tuples)
class MomSetVal;		// value, hash-consed set of objects
class MomTupleVal;		// value, hash-consed tuple of objects
class MomNodeVal;		/* value, hash-consed node: the
 connective is an object, the sons
 are values */
////
class MomObject;

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
};				// end class MomValue


typedef std::uint32_t MomSize; // sizes have 27 bits
typedef std::uint8_t MomGCMark; // garbage collector marks have 2 bits
class MomGC;

//// abstract super-class of all boxed values
class MomAnyVal
{
public:
  friend class MomIntSq;
  friend class MomDoubleSq;
  friend class MomString;
  friend class MomAnySeqVal;
  friend class MomSetVal;
  friend class MomTupleVal;
  friend class MomNodeVal;
  friend class MomGC;
  static constexpr MomSize _max_size = 1 << 27; // 134217728
  static constexpr size_t _alignment = 2*sizeof(void*);
private:
  // we start with the vtable ptr, probably 64 bits
  // the header word contains:
  //// 3 bits for the kind (constant)
  //// 2 bits for the GC marking (could change during GC operations)
  //// 27 bits for the size (constant)
  mutable std::uint32_t _headerw;
  const MomHash _hashw;
public:
  MomKind kindw() const
  {
    return MomKind(_headerw>>29);
  };
  MomGCMark gcmarkw(MomGC*) const
  {
    return (_headerw>>27) & 03;
  };
  void set_gcmarkw(MomGC*, MomGCMark gm)
  {
    _headerw = (_headerw & ~((std::uint32_t)3<<27)) | ((gm & 3)<<27);
  };
  MomSize sizew() const
  {
    return _headerw & ((1<<27)-1);
  };
  MomHash hash() const
  {
    return _hashw;
  };
protected:
  void* operator new (size_t sz) = delete;
  void* operator new (size_t sz, size_t gap)
  {
    MOM_ASSERT (sz % _alignment == 0, "MomAnyVal::new misaligned sz " << sz);
    MOM_ASSERT (gap % _alignment == 0, "MomAnyVal::new misaligned gap " << gap);
    return ::operator new(sz + gap);
  }
  MomAnyVal(MomKind k, MomSize sz, MomHash h) :
    _headerw(((std::uint32_t)k)<<29 | std::uint32_t(sz & (_max_size-1))),
    _hashw(h)
  {
    MOM_ASSERT(k>MomKind::TagNoneK && k<MomKind::Tag_LastK, "MomAnyVal bad kind " << (int)k);
    MOM_ASSERT(h!=0, "MomAnyVal zero hash");
    MOM_ASSERT(sz<_max_size, "MomAnyVal huge size " << sz);
  };
  virtual ~MomAnyVal() {};
  virtual MomKind vkind() const =0;
  virtual void scan_gc(MomGC*) const =0;
};				// end class MomAnyVal

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
  static constexpr const int _width_ = 256;
  static std::mutex _mtxarr_[_width_];
  static std::unordered_multimap<MomHash,const MomIntSq*> _maparr_[_width_];
public:
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
  virtual void scan_gc(MomGC*) const {};
};				// end class MomIntSq


////////////////////////////////////////////////////////////////

//// a constant hash-consed sequence of non-NaN doubles
class MomDoubleSq final : public MomAnyVal   // in scalarv.cc
{
  friend class MomGC;
  const double _dvalarr[MOM_FLEXIBLE_DIM];
  MomDoubleSq(const double* iarr, MomSize sz, MomHash h);
  static constexpr const int _width_ = 128;
  static std::mutex _mtxarr_[_width_];
  static std::unordered_multimap<MomHash,const MomDoubleSq*> _maparr_[_width_];
public:
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
  virtual void scan_gc(MomGC*) const {};
};				// end class MomDoubleSq




////////////////////////////////////////////////////////////////

//// a constant hash-consed UTF-8 null-terminated string
class MomString final : public MomAnyVal   // in scalarv.cc
{
  friend class MomGC;
  const uint32_t _bylen;
  const char _bstr[MOM_FLEXIBLE_DIM];
  MomString(const char*cstr, MomSize sz, uint32_t bylen, MomHash h);
  static constexpr const int _width_ = 256;
  static std::mutex _mtxarr_[_width_];
  static std::unordered_multimap<MomHash,const MomString*> _maparr_[_width_];
public:
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
};				// end class MomString

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

// common super class for sets and tuples of objects

class MomAnyObjSeq : public MomAnyVal   // in seqobjv.cc
{
  friend class MomGC;
  MomObject*const _obseq[MOM_FLEXIBLE_DIM];
protected:
  template<unsigned hinit, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
  static inline MomHash compute_hash_seq(MomObject*const* obarr, unsigned sz);
  MomAnyObjSeq(MomObject*const* obarr, MomSize sz, MomHash h);
  inline bool has_content(MomObject*const*obarr, unsigned sz) const
  {
    if (sz !=  sizew()) return false;
    if (sz > 0 && obarr==nullptr) return false;
    for (unsigned ix=0; ix<(unsigned)sz; ix++)
      if (MOM_LIKELY(_obseq[ix] != obarr[ix])) return false;
    return true;
  }
  virtual void scan_gc(MomGC*) const;
};				// end class MomAnyObjSeq


////////////////////////////////////////////////////////////////
class MomSet : public MomAnyObjSeq
{
  static constexpr unsigned hinit = 123017;
  static constexpr unsigned k1 = 103049;
  static constexpr unsigned k2 = 13063;
  static constexpr unsigned k3 = 143093;
  static constexpr unsigned k4 = 14083;
  static constexpr const int _width_ = 256;
  static std::mutex _mtxarr_[_width_];
  static std::unordered_multimap<MomHash,const MomSet*> _maparr_[_width_];
  MomSet(MomObject*const* obarr, MomSize sz, MomHash h)
    : MomAnyObjSeq(obarr,sz, h) {};
public:
  static MomHash compute_hash(MomObject*const* obarr, unsigned sz);
  static const MomSet* make_from_ascending_array(MomObject*const* obarr, MomSize sz);
  virtual MomKind vkind() const
  {
    return MomKind::TagSetK;
  };
}; // end class MomSet

////////////////////////////////////////////////////////////////
class MomTuple : public MomAnyObjSeq
{
  static constexpr unsigned hinit = 173081;
  static constexpr unsigned k1 = 135089;
  static constexpr unsigned k2 = 35671;
  static constexpr unsigned k3 = 5693;
  static constexpr unsigned k4 = 56873;
public:
  static MomHash compute_hash(MomObject*const* obarr, unsigned sz);
}; // end class MomTuple

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

class MomObject : public MomAnyVal // in objectv.cc
{
  const MomIdent _id;
public:
  bool same(const MomObject*ob) const
  {
    return this == ob;
  };
  bool less(const MomObject*ob) const
  {
    if (!ob || this==ob) return false;
    if (_id < ob->_id) return true;
    else if (_id > ob->_id) return false;
    MOM_FATALOG("non-identical objects sharing same id=" << _id);
  };
  bool less_equal(const MomObject*ob) const
  {
    if (!ob) return false;
    if (this == ob) return true;
    return less (ob);
  };
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
}; // end class MomObject


#endif /*MONIMELT_INCLUDED_ */
