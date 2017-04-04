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
MomSerial63::to_cbuf16(char cbuf[16]) const
{
  static_assert(sizeof(MOM_B62DIGITS)==_base_+1, "bad MOM_B62DIGITS in MomSerial63");
  memset (cbuf, 0, _nbdigits_+2);
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
  MOM_ASSERT(pc>=cbuf, "MomSerial63::to_cbuf16 bad pc - buffer underflow");
  MOM_ASSERT(cbuf[0] == '_' && isalnum(cbuf[1]) && isalnum(cbuf[_nbdigits_+1]),
             "MomSerial63::to_cbuf16 wrong cbuf=" << cbuf);
  MOM_ASSERT(strlen(cbuf) == _nbdigits_ + 1, "MomSerial63::to_cbuf16 bad cbuf=" << cbuf);
} // end MomSerial63::to_cbuf16

std::string
MomSerial63::to_string(void) const
{
  static_assert(sizeof(MOM_B62DIGITS)==_base_+1, "bad MOM_B62DIGITS in MomSerial63");
  char buf[16];
  memset(buf, 0, sizeof(buf));
  to_cbuf16(buf);
  MOM_ASSERT(buf[0] == '_' && isalnum(buf[1]) && isalnum(buf[_nbdigits_+1]),
             "MomSerial63::to_string wrong buf=" << buf);
  MOM_ASSERT(strlen(buf) == _nbdigits_ + 1, "MomSerial63::to_string bad buf=" << buf);
  return std::string{buf};
} // end  MomSerial63::to_string



const MomSerial63
MomSerial63::make_from_cstr(const char*s, const char*&end, bool fail)
{
  uint64_t n = 0;
  if (!s)
    goto failure;
  if (s[0] != '_')
    goto failure;
  if (!isdigit(s[1]))
    goto failure;
  for (auto i=0U; i<_nbdigits_; i++)
    {
      if (!s[i+1])
        goto failure;
      auto p = strchr(_b62digstr_,s[i+1]);
      if (!p)
        goto failure;
      n = n*_base_ + (p-_b62digstr_);
    }
  if (n!=0 && n<_minserial_)
    goto failure;
  if (n>_maxserial_)
    goto failure;
  end = s+_nbdigits_+1;
  return MomSerial63{n};
failure:
  if (fail)
    {
      std::string str{s};
      if (str.size() > _nbdigits_+2)
        str.resize(_nbdigits_+2);
      MOM_FAILURE("MomSerial63::make_from_cstr failure with " << str);
    }
  end = s;
  return MomSerial63{nullptr};
} // end MomSerial63::make_from_cstr


void
MomIdent::to_cbuf32(char buf[32]) const
{
  memset(buf, 0, 32);
  if (is_null())
    {
      buf[0] = buf[1] = '_';
      buf[2] = (char)0;
      return;
    }
  _idhi.to_cbuf16(buf);
  MOM_ASSERT(strlen(buf)==MomSerial63::_nbdigits_+1, "bad buf:" << buf);
  _idlo.to_cbuf16(buf+MomSerial63::_nbdigits_+1);
} // end MomIdent::to_cbuf32


std::string
MomIdent::to_string() const
{
  char buf[32];
  to_cbuf32(buf);
  return std::string{buf};
} // end MomIdent::to_string()



const MomIdent
MomIdent::make_from_cstr(const char *s, const char *&end,   bool fail)
{
  MomSerial63 hi(nullptr), lo(nullptr);
  char *endhi = nullptr;
  char *endlo = nullptr;
  if (!s) goto failure;
  if (s[0] != '_') goto failure;
  if (s[1] == '_')
    {
      end = s+2;
      return MomIdent{nullptr};
    };
  for (unsigned ix=0; ix<MomSerial63::_nbdigits_; ix++)
    if (!strchr(MomSerial63::_b62digstr_,s[ix+1]))
      goto failure;
  if (s[MomSerial63::_nbdigits_+1] != '_')
    goto failure;
  for (unsigned ix=0; ix<MomSerial63::_nbdigits_; ix++)
    if (!strchr(MomSerial63::_b62digstr_,s[MomSerial63::_nbdigits_+ix+2]))
      goto failure;
  hi = MomSerial63::make_from_cstr(s,endhi);
  lo = MomSerial63::make_from_cstr(endhi,endlo);
  if (!hi || !lo || endlo != s+_charlen_) goto failure;
  end = endlo;
  return MomIdent(hi,lo);
failure:
  if (fail)
    {
      std::string str{s};
      if (str.size() > _charlen_+1)
        str.resize(_charlen_+2);
      MOM_FAILURE("MomIdent::make_from_cstr failure with " << str);
    }
  end = s;
  return MomIdent(nullptr);
} // end MomIdent::make_from_cstr
