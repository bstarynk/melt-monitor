// file seqobjv.cc - sequence of object  values - sets and tuples

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


template<unsigned hinit, unsigned k1, unsigned k2, unsigned k3, unsigned k4>
MomHash
MomAnyObjSeq::compute_hash_seq(MomObject*const* obarr, unsigned sz)
{
  if (MOM_UNLIKELY(sz>0 && obarr==nullptr))
    MOM_FAILURE("MomAnyObjSeq::compute_hash null obarr for sz=" << sz);
  MomHash h1 = hinit;
  MomHash h2 = sz ^ (hinit + k2 + (k2 + k3) / ((sz&0xffff)+2) + k4);
  for (unsigned ix=0; ix<sz; ix++)
    {
      const MomObject* curob = obarr[ix];
      if (curob==nullptr)
        MOM_FAILURE("MomAnyObjSeq::compute_hash null object at ix=" << ix);
      MomHash hob = curob->hash();
      if (ix % 2 == 0)
        h1 = (k1 * h1) ^ (hob * k2 + ix);
      else
        h2 = (k3 * h2 + 11 * (ix&0xffff)) ^ (hob * k4);
    }
  MomHash h = h1 ^ h2;
  if (MOM_UNLIKELY(h==0))
    return 3*(h1 & 0xffffff) + 5*(h2 & 0xfffff) + (sz & 0xffff) + hinit/1000 + 8;
  return h;
} // end of MomAnyObjSeq::compute_hash

MomHash
MomSet::compute_hash(MomObject*const* obarr, unsigned sz)
{
  return compute_hash_seq<MomSet::hinit,
         MomSet::k1,
         MomSet::k2,
         MomSet::k3,
         MomSet::k4>(obarr, sz);
};



MomHash
MomTuple::compute_hash(MomObject*const* obarr, unsigned sz)
{
  return  compute_hash_seq<MomTuple::hinit,
          MomTuple::k1,
          MomTuple::k2,
          MomTuple::k3,
          MomTuple::k4>(obarr, sz);
};
