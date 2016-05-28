// file items.cc - items

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

static pthread_mutexattr_t item_mutexattr_mom;

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
        if (pc[-1] == '_')
          return false;
    }
  return true;
}                               /* end mom_valid_name_radix */

// we choose base 60, because with a 0-9 decimal digit then 13 extended
// digits in base 60 we can express a 80-bit number.  Notice that
// log(2**80/10)/log(60) is 12.98112
#define ID_DIGITS_MOM "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNPRSTUVWXYZ"
#define ID_BASE_MOM 60
static_assert (sizeof (ID_DIGITS_MOM) - 1 == ID_BASE_MOM,
               "invalid number of id digits");



static inline const char *
num80_to_char14_mom (mom_uint128_t num, char *buf)
{
  mom_uint128_t initnum = num;
  for (int ix = 13; ix > 0; ix--)
    {
      unsigned dig = num % ID_BASE_MOM;
      num = num / ID_BASE_MOM;
      buf[ix + 1] = ID_DIGITS_MOM[dig];
    }
  if (MOM_UNLIKELY (num > 9))
    MOM_FATAPRINTF ("bad num %d for initnum %16llx/%016llx", (int) num,
                    (unsigned long long) (initnum >> 64),
                    (unsigned long long) (initnum));
  assert (num <= 9);
  buf[1] = '0' + num;
  buf[0] = '_';
  return buf;
}

static inline mom_uint128_t
char14_to_num80_mom (const char *buf)
{
  mom_uint128_t num = 0;
  if (buf[0] != '_')
    return 0;
  if (buf[1] < '0' || buf[1] > '9')
    return 0;
  for (int ix = 1; ix <= 14; ix++)
    {
      char c = buf[ix];
      const char *p = strchr (ID_DIGITS_MOM, c);
      if (!p)
        return 0;
      num = (num * ID_BASE_MOM + (mom_uint128_t) (p - ID_DIGITS_MOM));
    }
  return num;
}


const char *
mom_hi_lo_suffix (char buf[MOM_HI_LO_SUFFIX_LEN], uint16_t hi,
                  uint64_t lo)
{
  mom_uint128_t i = (((mom_uint128_t) hi) << 64) | lo;
  return num80_to_char14_mom (i, buf);
}

bool
mom_suffix_to_hi_lo (const char *buf, uint16_t *phi, uint64_t *plo)
{
  if (!buf || buf[0] != '_')
    return false;
  mom_uint128_t i = char14_to_num80_mom (buf);
  if (i)
    {
      if (phi)
        *phi = (uint16_t) (i >> 64);
      if (plo)
        *plo = (uint64_t) i;
      return true;
    }
  return false;
}




static std::map<std::string,MomRADIXdata*> mom_radix_dict;
static std::mutex mom_radix_mtx;

MomRADIXdata* mom_register_radix_str(const std::string&str)
{
  if (str.empty()
      || !mom_valid_name_radix_len(str.c_str(), str.size()))
    return nullptr;
  std::lock_guard<std::mutex> gu {mom_radix_mtx};
  auto it = mom_radix_dict.find(str);
  if (it != mom_radix_dict.end())
    return it->second;
  void* p = mom_gc_alloc_uncollectable(sizeof(MomRADIXdata));
  if (MOM_UNLIKELY(p != NULL))
    MOM_FATAPRINTF("failed to allocate radix for %s", str.c_str());
  auto rad = new(p) MomRADIXdata;
  rad->vtype = MomItypeEn::RADIXdata;
  pthread_mutex_init(&rad->rad_mtx,nullptr);
  const unsigned inisiz = 5;
  rad->rad_sizehash = inisiz;
  rad->rad_counthash = 0;
  rad->rad_name = mom_make_string(str);
  rad->rad_primitem = nullptr;
  rad->rad_hasharr = (MomITEM**)mom_gc_alloc_scalar(inisiz*sizeof(MomITEM*));
  mom_radix_dict[str] = rad;
  return rad;
} // end mom_register_radix_str



MomRADIXdata* mom_find_radix_str(const std::string&str)
{
  if (str.empty()
      || !mom_valid_name_radix_len(str.c_str(), str.size()))
    return nullptr;
  std::lock_guard<std::mutex> gu {mom_radix_mtx};
  auto it = mom_radix_dict.find(str);
  if (it == mom_radix_dict.end())
    return nullptr;
  return it->second;
} // end mom_find_radix_str


// compute the hash of an item of given radix and hid & lid
momhash_t mom_hash_radix_id(MomRADIXdata*rad, uint16_t hid, uint64_t loid)
{
  if (rad==nullptr || rad==MOM_EMPTY_SLOT
      || rad->vtype != MomItypeEn::RADIXdata) return 0;
  auto nam = rad->rad_name;
  assert (nam != nullptr && nam != MOM_EMPTY_SLOT
          && nam->vtype == MomItypeEn::STRING);
  momhash_t hn = nam->hashv;
  momhash_t hi =
    17 * hid +
    ((443 * (uint32_t) (loid >> 32)) ^ (541 *
                                        (uint32_t) (loid & 0xffffffff)));
  momhash_t h = ((601 * hn) ^ (647 * hi)) + ((hn - hi) & 0xffff);
  if (MOM_UNLIKELY (h == 0))
    h =
      11 * (hn & 0xffff) + 17 * (hi & 0xffff) + (hid % 1637) +
      (loid & 0xfffff) + 5;
  assert (h != 0);
  return h;
} // end of mom_hash_radix_id


// return the pointer where an item should sit in a radix
static MomITEM** ptr_radix_id_mom(MomRADIXdata*rad, uint16_t hid, uint64_t lid)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  if (hid==0 && lid==0) return &rad->rad_primitem;
  auto h = mom_hash_radix_id (rad,hid,lid);
  unsigned sz = rad->rad_sizehash;
  assert (sz > 0);
  unsigned startix = h % sz;
  int pos = -1;
  for (unsigned ix=startix; ix<sz; ix++)
    {
      MomITEM*curitm = rad->rad_hasharr[ix];
      if (curitm == nullptr)
        {
          if (pos<0) pos=(int)ix;
          return rad->rad_hasharr+pos;
        }
      else if (curitm==MOM_EMPTY_SLOT)
        {
          if (pos<0) pos=(int)ix;
          continue;
        }
      else
        {
          assert (curitm->vtype == MomItypeEn::ITEM);
          assert (curitm->itm_radix == rad);
          if (curitm->itm_lid == lid && curitm->itm_hid == hid) return &rad->rad_hasharr[ix];
        }
    }
  for (unsigned ix=0; ix<startix; ix++)
    {
      MomITEM*curitm = rad->rad_hasharr[ix];
      if (curitm == nullptr)
        {
          if (pos<0) pos=(int)ix;
          return rad->rad_hasharr+pos;
        }
      else if (curitm==MOM_EMPTY_SLOT)
        {
          if (pos<0) pos=(int)ix;
          continue;
        }
      else
        {
          assert (curitm->vtype == MomItypeEn::ITEM);
          assert (curitm->itm_radix == rad);
          if (curitm->itm_lid == lid && curitm->itm_hid == hid) return &rad->rad_hasharr[ix];
        }
    }
  if (pos>=0) return rad->rad_hasharr+pos;
  return nullptr;
} // end ptr_radix_id_mom


MomITEM* mom_find_item_from_radix_id(MomRADIXdata*rad, uint16_t hid, uint64_t lid)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  MomGuardPmutex g {rad->rad_mtx};
  if (lid==0 && hid==0) return rad->rad_primitem;
  auto ptr = ptr_radix_id_mom(rad,hid,lid);
  if (ptr != nullptr)
    {
      MomITEM*curitm = *ptr;
      if (curitm == nullptr || curitm == MOM_EMPTY_SLOT) return nullptr;
      assert (curitm->vtype == MomItypeEn::ITEM);
      assert (curitm->itm_radix == rad);
      if (curitm->itm_lid == lid && curitm->itm_hid == hid) return curitm;
    }
  return nullptr;
} // end mom_find_item_from_radix_id

static void reorganize_radix_hashtable_mom(MomRADIXdata*rad, unsigned gap)
{
  assert (rad != nullptr && rad!=MOM_EMPTY_SLOT && rad->vtype == MomItypeEn::RADIXdata);
  auto oldsz = rad->rad_sizehash;
  auto oldcnt = rad->rad_counthash;
  auto oldarr = rad->rad_hasharr;
  unsigned long newsiz = mom_prime_above(4*oldcnt/3 + oldcnt/32 + gap + 5);
  if (MOM_UNLIKELY(newsiz >= MOM_ITEMPERRADIX_MAX))
    MOM_FATAPRINTF("too many items (%ld) for radix '%s' (oldcnt=%u gap=%u)",
                   newsiz, rad->rad_name->cstr, oldcnt, gap);
  auto newarr = (MomITEM**)mom_gc_alloc_scalar(newsiz*sizeof(MomITEM*));
  rad->rad_sizehash = newsiz;
  rad->rad_counthash = 0;
  rad->rad_hasharr = newarr;
  for (unsigned ix=0; ix<oldsz; ix++)
    {
      MomITEM*curitm = oldarr[ix];
      if (curitm==nullptr || curitm==MOM_EMPTY_SLOT) continue;
      assert (curitm->vtype == MomItypeEn::ITEM);
      assert (curitm->itm_radix == rad);
      auto ptr = ptr_radix_id_mom(rad, curitm->itm_hid, curitm->itm_lid);
      assert (ptr != nullptr);
      *ptr = curitm;
    }
  GC_FREE(oldarr);
} // end reorganize_radix_hashtable_mom


static void cleanup_item_mom (void *itmad, void *clad)
{
  assert (itmad != NULL);
  assert (clad == NULL);
  MomITEM*itm = (MomITEM*)itmad;
  assert(itm->vtype == MomItypeEn::ITEM);
  MomRADIXdata*rad = itm->itm_radix;
  assert (rad != nullptr && rad!=MOM_EMPTY_SLOT && rad->vtype==MomItypeEn::RADIXdata);
  MomGuardPmutex g {rad->rad_mtx};
  auto hid = itm->itm_hid;
  auto lid = itm->itm_lid;
  auto ptr = ptr_radix_id_mom(rad,hid,lid);
  assert (ptr != nullptr);
  *ptr = (MomITEM*)MOM_EMPTY_SLOT;
  if (hid != 0 || lid != 0)
    {
      rad->rad_counthash--;
      if (MOM_UNLIKELY(rad->rad_sizehash > 20 && rad->rad_counthash < rad->rad_sizehash/4))
        reorganize_radix_hashtable_mom(rad,0);
    }
  itm->vtype = MomItypeEn::_NONE;
#warning cleanup_item_mom incomplete
} // end cleanup_item_mom


// make an item (if not found) from its radix & hid&lid
MomITEM* mom_make_item_from_radix_id(MomRADIXdata*rad, uint16_t hid, uint64_t lid)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  MomGuardPmutex g {rad->rad_mtx};
  if (lid==0 && hid==0)
    {
      if (rad->rad_primitem != nullptr && rad->rad_primitem != MOM_EMPTY_SLOT)
        return rad->rad_primitem;
      MomITEM*newitm = new(mom_gc_alloc(sizeof(MomITEM))) MomITEM;
      newitm->vtype = MomItypeEn::ITEM;
      newitm->hashv = mom_hash_radix_id(rad,0,0);
      pthread_mutex_init(&newitm->itm_mtx, &item_mutexattr_mom);
      newitm->itm_hid = 0;
      newitm->itm_lid = 0;
      newitm->itm_mtime = time(nullptr);
      newitm->itm_radix = rad;
      newitm->itm_attrs = nullptr;
      newitm->itm_comps = nullptr;
      newitm->itm_paylsig = nullptr;
      newitm->itm_payldata = nullptr;
      rad->rad_primitem = newitm;
      GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, cleanup_item_mom,
                                         nullptr, nullptr, nullptr);
      return newitm;
    }
  if (MOM_UNLIKELY(3*rad->rad_sizehash + 2 < 4*rad->rad_counthash))
    reorganize_radix_hashtable_mom(rad,3+rad->rad_counthash/8);
  auto ptr =  ptr_radix_id_mom(rad,hid,lid);
  assert (ptr != nullptr);
  MomITEM*olditm = *ptr;
  if (olditm != nullptr && olditm != MOM_EMPTY_SLOT)
    {
      assert (olditm->vtype == MomItypeEn::ITEM);
      assert (olditm->itm_radix == rad);
      assert (olditm->itm_hid == hid && olditm->itm_lid == lid);
      return olditm;
    }
  MomITEM*newitm = new(mom_gc_alloc(sizeof(MomITEM))) MomITEM;
  newitm->vtype = MomItypeEn::ITEM;
  newitm->hashv = mom_hash_radix_id(rad,0,0);
  pthread_mutex_init(&newitm->itm_mtx, &item_mutexattr_mom);
  newitm->itm_hid = hid;
  newitm->itm_lid = lid;
  newitm->itm_mtime = time(nullptr);
  newitm->itm_radix = rad;
  newitm->itm_attrs = nullptr;
  newitm->itm_comps = nullptr;
  newitm->itm_paylsig = nullptr;
  newitm->itm_payldata = nullptr;
  *ptr = newitm;
  GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, cleanup_item_mom,
                                     nullptr, nullptr, nullptr);
  return newitm;
} // end of mom_make_item_from_radix_id


MomITEM* mom_clone_item_from_radix(MomRADIXdata*rad)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  MomGuardPmutex g {rad->rad_mtx};
  if (MOM_UNLIKELY(3*rad->rad_sizehash < 4*rad->rad_counthash + 2))
    reorganize_radix_hashtable_mom(rad,3+rad->rad_counthash/8);
  MomITEM* newitm = nullptr;
  do
    {
      uint16_t hid=0;
      uint64_t lid=0;
      do
        {
          hid = MomRandom::random_32u() & 0xffff;
        }
      while(MOM_UNLIKELY(hid==0));
      do
        {
          lid = MomRandom::random_64u();
        }
      while(MOM_UNLIKELY(lid==0));
      auto ptr =  ptr_radix_id_mom(rad,hid,lid);
      assert (ptr != nullptr);
      if (*ptr == nullptr || *ptr == MOM_EMPTY_SLOT)
        {
          MomITEM*newitm = new(mom_gc_alloc(sizeof(MomITEM))) MomITEM;
          newitm->vtype = MomItypeEn::ITEM;
          newitm->hashv = mom_hash_radix_id(rad,hid,lid);
          pthread_mutex_init(&newitm->itm_mtx, &item_mutexattr_mom);
          newitm->itm_hid = hid;
          newitm->itm_lid = lid;
          newitm->itm_mtime = time(nullptr);
          newitm->itm_radix = rad;
          newitm->itm_attrs = nullptr;
          newitm->itm_comps = nullptr;
          newitm->itm_paylsig = nullptr;
          newitm->itm_payldata = nullptr;
          *ptr = newitm;
          rad->rad_counthash++;
          GC_REGISTER_FINALIZER_IGNORE_SELF (newitm, cleanup_item_mom,
                                             nullptr, nullptr, nullptr);
        }
    }
  while(MOM_UNLIKELY(newitm==nullptr));
  return newitm;
} // end of mom_clone_item_from_radix


const char*
mom_radix_id_cstr(MomRADIXdata*rad, uint16_t hid, uint64_t lid)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  auto nam = rad->rad_name;
  assert (nam != nullptr && nam->vtype == MomItypeEn::STRING);
  if (lid==0 && hid==0)
    return nam->cstr;
  auto nsz = nam->usize;
  assert (nam->cstr[nsz] == (char)0);
  char smallbuf[64];
  memset (smallbuf, 0, sizeof(smallbuf));
  if (MOM_LIKELY(nsz+MOM_HI_LO_SUFFIX_LEN+4 < sizeof(smallbuf)))
    {
      memcpy(smallbuf,nam->cstr,nsz);
      smallbuf[nsz] = '_';
      (void)mom_hi_lo_suffix(smallbuf+nsz+1,hid,lid);
      return GC_STRDUP(smallbuf);
    }
  else
    {
      char*buf = (char*)mom_gc_alloc_scalar(nsz+MOM_HI_LO_SUFFIX_LEN+2);
      memcpy(smallbuf,nam->cstr,nsz);
      smallbuf[nsz] = '_';
      (void)mom_hi_lo_suffix(smallbuf+nsz+1,hid,lid);
      return buf;
    }
} // end mom_radix_id_cstr


const std::string
mom_radix_id_string(MomRADIXdata*rad, uint16_t hid, uint64_t lid)
{
  if (!rad || rad==MOM_EMPTY_SLOT || rad->vtype != MomItypeEn::RADIXdata)
    return nullptr;
  auto nam = rad->rad_name;
  assert (nam != nullptr && nam->vtype == MomItypeEn::STRING);
  if (lid==0 && hid==0)
    return nam->cstr;
  auto nsz = nam->usize;
  assert (nam->cstr[nsz] == (char)0);
  char smallbuf[64];
  memset (smallbuf, 0, sizeof(smallbuf));
  if (MOM_LIKELY(nsz+MOM_HI_LO_SUFFIX_LEN+4 < sizeof(smallbuf)))
    {
      memcpy(smallbuf,nam->cstr,nsz);
      smallbuf[nsz] = '_';
      (void)mom_hi_lo_suffix(smallbuf+nsz+1,hid,lid);
      return smallbuf;
    }
  else
    {
      std::string str {nam->cstr};
      str += '_';
      char idbuf[MOM_HI_LO_SUFFIX_LEN+4];
      memset(idbuf, 0, sizeof(idbuf));
      idbuf[0] = '_';
      mom_hi_lo_suffix(idbuf+1,hid,lid);
      str += idbuf;
      return str;
    }
} // end mom_radix_id_string
