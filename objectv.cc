// file objectv.cc - mutable object values

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



#define MOM_HAS_PREDEF(Id,Hi,Lo,Hash) MomObject*MOM_PREDEF(Id);
#include "_mom_predefined.h"

#define MOM_HAS_GLOBDATA(Nam) std::atomic<MomObject*> MOM_GLOBDATA_VAR(Nam) \
  = ATOMIC_VAR_INIT(nullptr);
#include "_mom_globdata.h"

#define MOM_HAS_GLOBDATA(Nam) MomRegisterGlobData momreglobdata_##Nam(#Nam,MOM_GLOBDATA_VAR(Nam));
#include "_mom_globdata.h"

constexpr int MomPayload::_py_max_proxdepth_;

bool
mom_valid_name_radix_len (const char *str, int len)
{
  if (!str)
    return false;
  if (len < 0)
    len = strlen (str);
  if (len <= 0)
    return false;
  if (!isalpha (str[0]))
    return false;
  const char *end = str + len;
  for (const char *pc = str; pc < end; pc++)
    {
      if (isalnum (*pc))
        continue;
      else if (*pc == '_')
        {
          if (pc[-1] == '_')
            return false;
        }
      else return false;
    }
  return true;
}                               /* end mom_valid_name_radix */


const MomSerial63
MomSerial63::make_random(void)
{
  uint64_t s = 0;
  do
    {
      s = MomRandom::random_64u() & (((uint64_t)1<<63)-1);
    }
  while (s<=_minserial_ || s>=_maxserial_);
  return MomSerial63{s};
} // end MomSerial63::make_random


const MomSerial63
MomSerial63::make_random_of_bucket(unsigned bucknum)
{
  if (MOM_UNLIKELY(bucknum >= _maxbucket_))
    {
      MOM_FAILURE("MomSerial63::random_of_bucket too big bucknum="
                  << bucknum);
    }
  uint64_t ds = MomRandom::random_64u() % (_deltaserial_ / _maxbucket_);
  uint64_t s = (bucknum * (_deltaserial_ / _maxbucket_)) + ds + _minserial_;
  MOM_ASSERT(s>=_minserial_ && s<=_maxserial_,
             "good s=" << s << " between _minserial_=" << _minserial_
             << " and _maxserial_=" << _maxserial_
             << " with ds=" << ds << " and bucknum=" << bucknum
             << " and _deltaserial_=" << _deltaserial_
             << " and _maxbucket_=" << _maxbucket_);
  return MomSerial63{s};
} // end of MomSerial63::make_random_of_bucket

//constexpr const char MomSerial63::_b62digits_[] = MOM_B62DIGITS;

void
MomSerial63::to_cbuf16(char cbuf[]) const
{
  static_assert(sizeof(MOM_B62DIGITS)==_base_+1, "bad MOM_B62DIGITS in MomSerial63");
  memset (cbuf, 0, 16);
  cbuf[0] = '_';
  uint64_t n = _serial;
  char*pc = cbuf+_nbdigits_;
  while (n != 0)
    {
      unsigned d = n % _base_;
      n = n / _base_;
      *pc = _b62digstr_[d];
      pc--;
    }
  while (pc > cbuf)
    *(pc--) = '0';
  cbuf[16-1] = (char)0;
  MOM_ASSERT(pc>=cbuf, "MomSerial63::to_cbuf16 bad pc - buffer underflow");
  MOM_ASSERT(cbuf[0] == '_' && isalnum(cbuf[1]) && isalnum(cbuf[_nbdigits_-1]),
             "MomSerial63::to_cbuf16 wrong cbuf=" << cbuf << "; for serial=" << _serial);
  MOM_ASSERT(strlen(cbuf) < 16, "MomSerial63::to_cbuf16 overflow cbuf=" << cbuf << ';');
  MOM_ASSERT(strlen(cbuf) == _nbdigits_ + 1, "MomSerial63::to_cbuf16 bad cbuf=" << cbuf);
} // end MomSerial63::to_cbuf16

std::string
MomSerial63::to_string(void) const
{
  static_assert(sizeof(MOM_B62DIGITS)==_base_+1, "bad MOM_B62DIGITS in MomSerial63");
  char buf[16];
  memset(buf, 0, sizeof(buf));
  to_cbuf16(buf);
  MOM_ASSERT(buf[0] == '_' && isalnum(buf[1]) && isalnum(buf[_nbdigits_-1]),
             "MomSerial63::to_string wrong buf=" << buf << ';');
  MOM_ASSERT(strlen(buf) == _nbdigits_ + 1, "MomSerial63::to_string bad buf=" << buf << ';');
  return std::string{buf};
} // end  MomSerial63::to_string



const MomSerial63
MomSerial63::make_from_cstr(const char*s, const char**pend, bool fail)
{
  const char *failmsg = nullptr;
  uint64_t n = 0;
  if (!s)
    {
      failmsg = "nil s";
      goto failure;
    }
  if (s[0] != '_')
    {
      failmsg = "no underscore";
      goto failure;
    }
  if (!isdigit(s[1]))
    {
      failmsg = "no starting digit";
      goto failure;
    }
  for (auto i=0U; i<_nbdigits_; i++)
    {
      if (!s[i+1])
        {
          failmsg = "missing digit";
          goto failure;
        }
      auto p = strchr(_b62digstr_,s[i+1]);
      if (!p)
        {
          failmsg = "bad digit";
          goto failure;
        }
      n = n*_base_ + (p-_b62digstr_);
    }
  if (n!=0 && n<_minserial_)
    {
      failmsg = "too small serial";
      goto failure;
    }
  if (n>_maxserial_)
    {
      failmsg = "too big serial";
      goto failure;
    }
  if (pend != nullptr)
    *pend = s+_nbdigits_+1;
  return MomSerial63{n};
failure:
  if (pend != nullptr)
    *pend = s;
  if (fail)
    {
      std::string str{s};
      if (str.size() > _nbdigits_+2)
        str.resize(_nbdigits_+2);
      MOM_FAILURE("MomSerial63::make_from_cstr failure with " << str << "; " << failmsg);
    }
  else
    {
      MOM_WARNPRINTF("MomSerial63::make_from_cstr mistaken with %.30s: %s", s, failmsg);
    }
  return MomSerial63{nullptr};
} // end MomSerial63::make_from_cstr



void
MomIdent::to_cbuf32(char buf[]) const
{
  memset(buf, 0, 32);
  if (is_null())
    {
      buf[0] = buf[1] = '_';
      buf[2] = (char)0;
      return;
    }
  _idhi.to_cbuf16(buf);
  buf [32-1] = (char)0;
  MOM_ASSERT(strlen(buf)==MomSerial63::_nbdigits_+1, "after idhi bad buf:" << buf);
  _idlo.to_cbuf16(buf+MomSerial63::_nbdigits_+1);
  buf [32-1] = (char)0;
  MOM_ASSERT(strlen(buf)==MomIdent::_charlen_, "after idlo bad buf:" << buf);
} // end MomIdent::to_cbuf32


std::string
MomIdent::to_string() const
{
  char buf[32];
  to_cbuf32(buf);
  MOM_ASSERT(strlen(buf) == MomIdent::_charlen_, "bad buf:" << buf);
  return std::string{buf};
} // end MomIdent::to_string()



const MomIdent
MomIdent::make_from_cstr(const char *s, const char **pend,   bool fail)
{
  MomSerial63 hi(nullptr), lo(nullptr);
  const char *endhi = nullptr;
  const char *endlo = nullptr;
  const char* failmsg = nullptr;
  if (!s)
    {
      failmsg="nil s";
      goto failure;
    };
  if (s[0] != '_')
    {
      failmsg="want underscore";
      goto failure;
    };
  if (s[1] == '_')
    {
      if (pend)
        *pend = s+2;
      return MomIdent{nullptr};
    };
  for (unsigned ix=0; ix<MomSerial63::_nbdigits_; ix++)
    if (!strchr(MomSerial63::_b62digstr_,s[ix+1]))
      {
        failmsg="want base62 hi-digit";
        goto failure;
      };
  if (s[MomSerial63::_nbdigits_+1] != '_')
    {
      failmsg="want low underscore";
      goto failure;
    };
  for (unsigned ix=0; ix<MomSerial63::_nbdigits_; ix++)
    if (!strchr(MomSerial63::_b62digstr_,s[MomSerial63::_nbdigits_+ix+2]))
      {
        failmsg="want base62 lo-digit";
        goto failure;
      };
  MOM_ASSERT(s && s[0], "MomIdent::make_from_cstr bad s=" << s);
  hi = MomSerial63::make_from_cstr(s,&endhi,false);
  if (!hi)
    {
      failmsg = "bad hi";
      goto failure;
    }
  if (!endhi)
    {
      failmsg = "unended hi";
      goto failure;
    }
  lo = MomSerial63::make_from_cstr(endhi,&endlo,false);
  if (!lo)
    {
      failmsg = "bad lo";
      goto failure;
    }
  if (endlo != s+_charlen_)
    {
      failmsg = "short lo";
      goto failure;
    }
  if (pend)
    *pend = endlo;
  return MomIdent(hi,lo);
failure:
  if (pend)
    *pend = s;
  if (fail)
    {
      std::string str{s};
      if (str.size() > _charlen_+1)
        str.resize(_charlen_+2);
      MOM_FAILURE("MomIdent::make_from_cstr failure with:" << str << "; " << failmsg);
    }
  return MomIdent(nullptr);
} // end MomIdent::make_from_cstr


////////////////////////////////////////////////////////////////



MomObject*
MomObject::find_object_of_id(const MomIdent id)
{
  if (id.is_null()) return nullptr;
  unsigned buix = id.bucketnum();
  auto& curbuck = _ob_bucketarr_[buix];
  std::lock_guard<std::mutex> _gu(curbuck._obu_mtx);
  if (MOM_UNLIKELY(curbuck._obu_map.bucket_count() < _bumincount_))
    curbuck._obu_map.rehash(_bumincount_);
  auto it = curbuck._obu_map.find(id);
  if (it != curbuck._obu_map.end())
    return it->second;
  return nullptr;
} // end of MomObject::find_object_of_id



MomObject::MomObject(const MomIdent id, MomHash h)
  : MomAnyVal(MomKind::TagObjectK, 0, h),
    _ob_id(id),
    _ob_space(ATOMIC_VAR_INIT(MomSpace::TransientSp)),
    _ob_magic(false),
    _ob_mtim(ATOMIC_VAR_INIT(0.0)),
    _ob_recmtx(),
    _ob_attrs{},
    _ob_comps(),
    _ob_payl(nullptr)
{
  MOM_ASSERT(h != 0 && id.hash() == h, "MomObject::MomObject corrupted h=" << h << " for id=" << id);
} // end MomObject::MomObject

void
MomObject::unsync_clear_all()
{
  _ob_attrs.clear();
  unsync_clear_payload();
} // end MomObject::unsync_clear_all

MomObject::~MomObject()
{
  unsync_clear_all();
} // end MomObject::~MomObject

MomObject*
MomObject::make_object_of_id(const MomIdent id)
{
  if (id.is_null()) return nullptr;
  unsigned buix = id.bucketnum();
  auto& curbuck = _ob_bucketarr_[buix];
  std::lock_guard<std::mutex> _gu(curbuck._obu_mtx);
  if (MOM_UNLIKELY(curbuck._obu_map.bucket_count() < _bumincount_))
    curbuck._obu_map.rehash(_bumincount_);
  auto it = curbuck._obu_map.find(id);
  if (it != curbuck._obu_map.end())
    return it->second;
  MomObject*resob = new(mom_newtg,0) MomObject(id,id.hash());
  curbuck._obu_map.insert({id,resob});
  if (MOM_UNLIKELY(MomRandom::random_32u() % _bumincount_ == 0))
    {
      curbuck._obu_map.reserve(9*curbuck._obu_map.size()/8 + 5);
    }
  return resob;
} // end of MomObject::make_object_of_id


MomObject*
MomObject::make_object(void)
{
  MomIdent id = MomIdent::make_random();
  if (id.is_null()) return nullptr;
  unsigned buix = id.bucketnum();
  auto& curbuck = _ob_bucketarr_[buix];
  std::lock_guard<std::mutex> _gu(curbuck._obu_mtx);
  if (MOM_UNLIKELY(curbuck._obu_map.bucket_count() < _bumincount_))
    curbuck._obu_map.rehash(_bumincount_);
  auto it = curbuck._obu_map.end();
  for(;;)
    {
      it = curbuck._obu_map.find(id);
      if (MOM_LIKELY(it == curbuck._obu_map.end()))
        break;
      id = MomIdent::make_random_of_bucket(buix);
    };
  MomObject*resob = new(mom_newtg,0) MomObject(id,id.hash());
  curbuck._obu_map.insert({id,resob});
  if (MOM_UNLIKELY(MomRandom::random_32u() % _bumincount_ == 0))
    {
      curbuck._obu_map.reserve(9*curbuck._obu_map.size()/8 + 5);
    }
  return resob;
} // end MomObject::make_object


std::mutex*
MomObject::valmtx() const
{
  return &(_ob_bucketarr_+_ob_id.bucketnum())->_obu_mtx;
} // end MomObject::valmtx

void
MomObject::scan_gc(MomGC*gc) const
{
  std::lock_guard<std::recursive_mutex> gu{_ob_recmtx};
  for (auto p: _ob_attrs)
    {
      MomObject* pobattr = p.first;
      MomValue atval = p.second;
      gc->scan_object(pobattr);
      gc->scan_value(atval);
    }
  for (auto vcomp : _ob_comps)
    {
      gc->scan_value(vcomp);
    }
  if (_ob_payl)
    {
      _ob_payl->scan_gc_payl(gc);
    }
} // end of MomObject::scan_gc

MomObject*
MomObject::set_space(MomSpace sp)
{
  auto oldsp = std::atomic_exchange(&_ob_space, sp);
  if (oldsp == sp) return this;
  if (sp == MomSpace::PredefSp)
    {
      // add predefined
      std::lock_guard<std::mutex> _gu{_predefmtx_};
      _predefset_.insert(this);
    }
  else if (oldsp == MomSpace::PredefSp)
    {
      // remove predefined
      std::lock_guard<std::mutex> _gu{_predefmtx_};
      _predefset_.erase(this);
    }
  return this;
} // end MomObject::set_space


bool
MomObject::less_named(const MomObject*ob) const
{
  if (!ob) return false;
  if (ob == this) return false;
  std::string thisname;
  std::string obname;
  {
    std::lock_guard<std::recursive_mutex> gu{this->_ob_recmtx};
    thisname = mom_get_unsync_string_name(const_cast<MomObject*>(this));
  }
  {
    std::lock_guard<std::recursive_mutex> gu{ob->_ob_recmtx};
    obname = mom_get_unsync_string_name(const_cast<MomObject*>(ob));
  }
  if (thisname.empty() && obname.empty())
    return less(ob);
  else if (thisname.empty())
    return false;
  else if (obname.empty())
    return true;
  else return thisname < obname;
} // end MomObject::less_named

const MomSet*
MomObject::predefined_set(void)
{
  std::lock_guard<std::mutex> _gu{_predefmtx_};
  return MomSet::make_from_objptr_set(_predefset_);
} // end of MomObject::predefined_set

void
MomObject::do_each_predefined(std::function<bool(const MomObject*)>fun)
{
  std::lock_guard<std::mutex> _gu{_predefmtx_};
  for (const MomObject*pob : _predefset_)
    {
      MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
                 "do_each_predefined bad pob");
      if (fun(pob)) return;
    }
} // end MomObject::do_each_predefined


void
MomObject::initialize_predefined(void)
{
#define MOM_HAS_PREDEF(Id,Hi,Lo,Hash) do {		\
  MOM_PREDEF(Id) = make_object_of_id(MomIdent(Hi,Lo));	\
  MOM_PREDEF(Id)->set_space(MomSpace::PredefSp);        \
  MOM_PREDEF(Id)->touch();				\
} while(0);
#include "_mom_predefined.h"
} // end MomObject::initialize_predefined

void
MomObject::gc_todo_clear_marks(MomGC*gc)
{
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_clear_marks start");
  _ob_nbclearedbuckets_.store(0);
  for (unsigned ix=0; ix<_obmaxbucket_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_clear_mark_bucket(thisgc,ix);
      if (1+_ob_nbclearedbuckets_.fetch_add(1) >= _obmaxbucket_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_start_scan();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_clear_marks end");
} // end MomObject::gc_todo_clear_marks

void
MomObject::gc_todo_clear_mark_bucket(MomGC*gc,unsigned buckix)
{
  MOM_ASSERT(buckix<_obmaxbucket_, "gc_todo_clear_mark_bucket invalid buckix=" << buckix);
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_clear_mark_bucket start buckix=" << buckix);
  auto& curbuck = _ob_bucketarr_[buckix];
  std::lock_guard<std::mutex> gu(curbuck._obu_mtx);
  curbuck.unsync_buck_gc_clear_marks(gc);
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_clear_mark_bucket end buckix=" << buckix);
} // end MomObject::gc_todo_clear_mark_bucket

void
MomObject::MomBucketObj::unsync_buck_gc_clear_marks(MomGC*gc)
{
  for (auto it : _obu_map)
    {
      MomObject* pob = it.second;
      MOM_ASSERT(pob != nullptr, "unsync_buck_gc_clear_marks null pob");
      pob->gc_set_mark(gc,false);
    }
} // end MomObject::MomBucketObj::unsync_buck_gc_clear_marks


void
MomObject::gc_todo_destroy_dead(MomGC* gc)
{
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_destroy_dead end");
  _ob_nbsweepedbuckets_.store(0);
  for (unsigned ix=0; ix<_obmaxbucket_; ix++)
    gc->add_todo([=](MomGC*thisgc)
    {
      gc_todo_sweep_bucket(thisgc,ix);
      if (1+_ob_nbsweepedbuckets_.fetch_add(1) >= _obmaxbucket_)
        thisgc->add_todo([=](MomGC*ourgc)
        {
          ourgc->maybe_done_sweep();
        });
    });
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_destroy_dead end");
} // end MomObject::gc_todo_destroy_dead

void
MomObject::gc_todo_sweep_bucket(MomGC*gc, unsigned buckix)
{
  MOM_ASSERT(buckix<_obmaxbucket_, "gc_todo_sweep_bucket invalid buckix=" << buckix);
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_sweep_bucket start buckix=" << buckix);
  auto& curbuck = _ob_bucketarr_[buckix];
  std::lock_guard<std::mutex> gu(curbuck._obu_mtx);
  curbuck.unsync_buck_gc_sweep_destroy(gc);
  MOM_DEBUGLOG(garbcoll, "MomObject::gc_todo_clear_mark_bucket end buckix=" << buckix);
} // end MomObject::gc_todo_sweep_bucket

void
MomObject::MomBucketObj::unsync_buck_gc_sweep_destroy(MomGC*)
{
  std::vector<MomIdent> delidvec;
  auto nbent = _obu_map.size();
  delidvec.reserve(nbent/8+30);
  for (auto it : _obu_map)
    {
      MomObject* pob = it.second;
      if (pob)
        {
          delidvec.push_back(it.first);
          it.second = nullptr;
          delete pob;
        }
    }
  for (auto id: delidvec)
    _obu_map.erase(id);
  _obu_map.rehash(0);
} // end MomObject::MomBucketObj::unsync_buck_gc_sweep_destroy


bool
MomObject::valid_prefixid(const char*prefixid)
{
  if (!prefixid || prefixid[0] != '_') return false;
  if (prefixid[1]>=127 || prefixid[1]<=0 || !isdigit(prefixid[1])) return false;
  if (prefixid[2]>=127 || prefixid[2]<=0 || !isalnum(prefixid[2])) return false;
  if (strlen(prefixid)>=MomSerial63::_nbdigits_+1) return false;
  for (const char*pc = prefixid+1; *pc; pc++)
    if (!isalnum(*pc) || !strchr(MomSerial63::_b62digstr_,*pc)) return false;
  return true;
} // end MomObject::valid_prefixid

MomObjptrSet
MomObject::objectset_prefixed(const char*prefixid)
{
  if (!valid_prefixid(prefixid)) return MomObjptrSet{};
  unsigned prefixlen = strlen(prefixid);
  auto b64digits = MomSerial63::_b62digstr_;
  auto oc1 = strchr(b64digits, prefixid[1]);
  auto oc2 = strchr(b64digits, prefixid[2]);
  MOM_ASSERT(oc1 != nullptr && oc2 != nullptr,
             "objectset_prefixed bad prefixid=" << MomShowString(prefixid));
  unsigned bn = (oc1-b64digits)*62 + (oc2-b64digits);
  MOM_ASSERT(bn < MomSerial63::_maxbucket_,
             "objectset_prefixed bad bn=" << bn);
  MomBucketObj&curbuck = _ob_bucketarr_[bn];
  MomObjptrSet setob;
  std::lock_guard<std::mutex> _gu(curbuck._obu_mtx);
  for (auto& it : curbuck._obu_map)
    {
      char idbuf[32];
      memset (idbuf, 0, sizeof(idbuf));
      MomIdent id = it.first;
      MomObject* pob = it.second;
      id.to_cbuf32(idbuf);
      if (!strncmp(idbuf, prefixid, prefixlen))
        setob.insert(pob);
    }
  return setob;
} // end MomObject::objectset_prefixed


void
MomObject::do_each_object_prefixed(const char*prefixid,
                                   std::function<bool(MomObject*)> fun)
{
  if (!valid_prefixid(prefixid)) return;
  unsigned prefixlen = strlen(prefixid);
  auto b64digits = MomSerial63::_b62digstr_;
  auto oc1 = strchr(b64digits, prefixid[1]);
  auto oc2 = strchr(b64digits, prefixid[2]);
  MOM_ASSERT(oc1 != nullptr && oc2 != nullptr,
             "objectset_prefixed bad prefixid=" << MomShowString(prefixid));
  unsigned bn = (oc1-b64digits)*62 + (oc2-b64digits);
  MOM_ASSERT(bn < MomSerial63::_maxbucket_,
             "objectset_prefixed bad bn=" << bn);
  MomBucketObj&curbuck = _ob_bucketarr_[bn];
  MomObjptrSet setob;
  std::lock_guard<std::mutex> _gu(curbuck._obu_mtx);
  for (auto& it : curbuck._obu_map)
    {
      char idbuf[32];
      memset (idbuf, 0, sizeof(idbuf));
      MomIdent id = it.first;
      MomObject* pob = it.second;
      id.to_cbuf32(idbuf);
      if (!strncmp(idbuf, prefixid, prefixlen))
        {
          if (fun(pob)) return;
        }
    }
} // end MomObject::do_each_object_prefixed


void
MomShowObject::output(std::ostream& os) const
{
  if (!_shob)
    {
      os << "__";
      return;
    }
  MOM_ASSERT(_shob->vkind() == MomKind::TagObjectK, "MomShowObject::output bad _shob");
  std::lock_guard<std::recursive_mutex> gu{_shob->get_recursive_mutex()};
  std::string name = mom_get_unsync_string_name(_shob);
  if (!name.empty())
    {
      os << name << " |=" << _shob << "|";
    }
  else if (_shob->unsync_payload())
    {
      std::string pyname = _shob->unsync_paylname();
      os << _shob << " |:" << pyname << "%| ";
    }
  else
    os << _shob;
} // end MomShowObject::output



MomValue
MomPayload::payl_getmagic_deep(MomObject*targetob, const MomObject*attrob, int depth, bool *pgot)
{
  MomValue res = nullptr;
  bool got = false;
  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK, "payl_getmagic bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK, "payl_getmagic bad attrob");
  MOM_ASSERT(_py_vtbl && _py_vtbl->pyv_magic == MOM_PAYLOADVTBL_MAGIC,
             "payl_getmagic bad payload");
  if (depth >= MomPayload::_py_max_proxdepth_)
    MOM_FAILURE("too deep named getmagic targetob=" << MomShowObject(targetob) << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
                << " owner=" << MomShowObject(owner()));
  if (_py_vtbl->pyv_getmagic)
    {
      res = _py_vtbl->pyv_getmagic(this, targetob, attrob, depth, &got);
      if (got)
        {
          if (pgot)
            *pgot = true;
          return res;
        }
    };
  MomObject*pobproxy = _py_proxy;
  if (pobproxy)
    {
      MOM_ASSERT(pobproxy->vkind() == MomKind::TagObjectK, "bad proxy");
      std::lock_guard<std::recursive_mutex> gu{pobproxy->get_recursive_mutex()};
      MomPayload*paylproxy = pobproxy->unsync_payload();
      if (paylproxy)
        {
          res = paylproxy->payl_getmagic_deep(targetob, attrob, depth+1, &got);
          if (got)
            {
              if (pgot)
                *pgot = true;
              return res;
            }
        }
    }
  if (pgot)
    *pgot = false;
  return nullptr;
} // end MomPayload::payl_getmagic_deep



MomValue
MomPayload::payl_fetch_deep(MomObject*targetob,  const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth, bool *pgot)
{
  MomValue res = nullptr;
  if (!vecarr)
    veclen = 0;
  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK, "payl_getmagic bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK, "payl_getmagic bad attrob");
  MOM_ASSERT(_py_vtbl && _py_vtbl->pyv_magic == MOM_PAYLOADVTBL_MAGIC,
             "payl_fetch bad payload");
  MOM_ASSERT(pgot, "payl_fetch no pgot");
  if (depth >= MomPayload::_py_max_proxdepth_)
    MOM_FAILURE("too deep named fetch targetob=" << MomShowObject(targetob) << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
                << " owner=" << MomShowObject(owner()));
  if (_py_vtbl->pyv_fetch)
    {
      bool got = false;
      res = _py_vtbl->pyv_fetch(this, targetob, attrob,
                                vecarr, veclen, depth, &got);
      if (got)
        {
          *pgot = true;
          return res;
        };
    }
  MomObject*pobproxy = _py_proxy;
  if (pobproxy)
    {
      MOM_ASSERT(pobproxy->vkind() == MomKind::TagObjectK, "bad proxy");
      std::lock_guard<std::recursive_mutex> gu{pobproxy->get_recursive_mutex()};
      MomPayload*paylproxy = pobproxy->unsync_payload();
      if (paylproxy)
        {
          bool got = false;
          res = paylproxy->payl_fetch_deep(targetob, attrob,
                                           vecarr, veclen,
                                           depth+1, &got);
          if (got)
            {
              *pgot = true;
              return res;
            }
        }
    }
  *pgot = false;
  return nullptr;
} // end MomPayload::payl_fetch_deep



bool
MomPayload::payl_updated_deep(MomObject*targetob, const MomObject*attrob, const MomValue*vecarr, unsigned veclen, int depth)
{

  MOM_ASSERT(targetob && targetob->vkind() == MomKind::TagObjectK,
             "payl_update_deep bad targetob");
  MOM_ASSERT(attrob && attrob->vkind() == MomKind::TagObjectK,
             "payl_update_deep bad attrob");
  MOM_ASSERT(_py_vtbl && _py_vtbl->pyv_magic == MOM_PAYLOADVTBL_MAGIC,
             "payl_update bad payload");
  if (depth >= MomPayload::_py_max_proxdepth_)
    MOM_FAILURE("too deep named update targetob=" << MomShowObject(targetob) << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
                << " owner=" << MomShowObject(owner()));
  if (_py_vtbl->pyv_updated)
    {
      bool got = _py_vtbl->pyv_updated(this, targetob, attrob, vecarr, veclen, depth);
      if (got)
        {
          return true;
        }
    }
  MomObject*pobproxy = _py_proxy;
  if (pobproxy)
    {
      MOM_ASSERT(pobproxy->vkind() == MomKind::TagObjectK, "bad proxy");
      std::lock_guard<std::recursive_mutex> gu{pobproxy->get_recursive_mutex()};
      MomPayload*paylproxy = pobproxy->unsync_payload();
      if (paylproxy)
        {
          return paylproxy->payl_updated_deep(targetob, attrob,
                                              vecarr, veclen,
                                              depth+1);
        }
    };
  return false;
} // end MomPayload::payl_updated_deep


std::string
MomObject::unsync_paylname() const
{
  std::string nam = "";
  if (_ob_payl)
    {
      MOM_ASSERT(_ob_payl->_py_vtbl && _ob_payl->_py_vtbl->pyv_magic == MOM_PAYLOADVTBL_MAGIC,
                 "bad payload in " << this);
      nam= _ob_payl->_py_vtbl->pyv_name;
    }
  else nam = "*nopayl*";
  return nam;
}
