// file somecode.cc - some code

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

extern "C" bool MOMCOD_UPDATED(test1generator)
(const struct MomPayload*payl, MomObject*targpob,const MomObject*attrob,
 const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<MomPaylCode*>(const_cast<MomPayload*>(payl));
  MOM_ASSERT(py && py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MOMCOD_UPDATED(test1generator) invalid code payload for targpob=" << targpob);
  MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(test1generator) targpob=" << MomShowObject(targpob)
	       << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob)));
  MOM_WARNLOG("incomplete MOMCOD_UPDATED(test1generator) targob="<< targpob
              << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
              << MOM_SHOW_BACKTRACE("from MOMCOD_UPDATED(test1generator)"));
#warning incomplete  MOMCOD_UPDATED(test1generator)
} // end  MOMCOD_UPDATED(test1generator)

