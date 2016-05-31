// file values.cc - values

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


const char *
mom_itype_str (const MomVal *v)
{
  if (!v)
    return "*nil*";
  else if (v == MOM_EMPTY_SLOT)
    return "*emptyslot*";
  switch (v->vtype)
    {
    case MomItypeEn::_NONE:
      return "*none*";
    case MomItypeEn::INT:
      return "INT";
    case MomItypeEn::STRING:
      return "STRING";
    case MomItypeEn::TUPLE:
      return "TUPLE";
    case MomItypeEn::SET:
      return "SET";
    case MomItypeEn::NODE:
      return "NODE";
    case MomItypeEn::ITEM:
      return "ITEM";
    //
    case MomItypeEn::ASSOVALdata:
      return "ASSOVALdata";
    case MomItypeEn::VECTVALdata:
      return "VECTVALdata";
    case MomItypeEn::RADIXdata:
      return "RADIXdata";
    //
    default:
    {
      char tybuf[24];
      memset(tybuf, 0, sizeof(tybuf));
      snprintf(tybuf, sizeof(tybuf), "*?Ityp#%u?", (unsigned)v->vtype);
      return GC_STRDUP(tybuf);
    }
    }
}

MomSTRING*
mom_make_string(const char*cstr, int len)
{
  if (!cstr || cstr==MOM_EMPTY_SLOT) return NULL;
  if (len<0) len = strlen(cstr);
  if (MOM_UNLIKELY(len >= MOM_SIZE_MAX))
    MOM_FATAPRINTF("too big string (%d) starting with .50%s",
                   len, cstr);
  momhash_t h = mom_cstring_hash_len(cstr,len);
  assert (h != 0);
  void *p = mom_gc_alloc_scalar(sizeof(MomSTRING)+len);
  MomSTRING*s = new(p) MomSTRING;
  s->vtype = MomItypeEn::STRING;
  s->usize = len;
  s->hashv = h;
  memcpy(s->cstr, cstr, len);
  return s;
}

