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
  MomObject*ownpob = py->owner();
  MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(test1generator) targpob=" << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
               << " args=" << MomShowVectorValues(vecarr, veclen));
  MOM_WARNLOG("Incomplete MOMCOD_UPDATED(test1generator) targob="<< MomShowObject(targpob)
              << " ownpob=" << MomShowObject(ownpob)
              << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
              << " args=" << MomShowVectorValues(vecarr, veclen));
  if (attrob == MOMP_start_generation && veclen == 1)
    {
      MomValue vstart = vecarr[0];
      MomObject*pobstart = const_cast<MomObject*>(vstart.to_val()->as_object());
      if (!pobstart)
        MOM_FAILURE("MOMCOD_UPDATED(test1generator) bad vstart=" << vstart);
      {
        std::lock_guard<std::recursive_mutex> gu{pobstart->get_recursive_mutex()};
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(test1generator) start_generation pobstart=" << MomShowObject(pobstart)
                     << " of paylname=" << pobstart->unsync_paylname()
                     << " with ownpob=" << MomShowObject(ownpob)
                     << " and targpob=" << MomShowObject(targpob));
        pobstart->unsync_append_comp(MomString::make_from_string("# generated test1gen.out\n"));
        /*** bad code below, becase the_system has no outputter
             See code of  MomPaylStrobuf::output_value_to_buffer
             pobstart->unsync_append_comp
               (MomNode::make_from_values(MOMP_the_system,
                      MomString::make_from_string("foobar"),
        	MomValue((intptr_t)35)));
        ***/
      }
    }
#warning incomplete  MOMCOD_UPDATED(test1generator)
  return false;
} // end  MOMCOD_UPDATED(test1generator)


extern "C" bool MOMCOD_UPDATED(predefined_file_generator)
(const struct MomPayload*payl, MomObject*targpob,const MomObject*attrob,
 const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<MomPaylCode*>(const_cast<MomPayload*>(payl));
  MOM_ASSERT(py && py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MOMCOD_UPDATED(predefined_file_generator) invalid code payload for targpob=" << targpob);
  MomObject*ownpob = py->owner();
  MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) targpob="
               << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
               << " args=" << MomShowVectorValues(vecarr, veclen));
  if (attrob == MOMP_start_generation && veclen == 1)
    {
      MomValue vstart = vecarr[0];
      MomObject*pobstart = const_cast<MomObject*>(vstart.to_val()->as_object());
      if (!pobstart)
        MOM_FAILURE("MOMCOD_UPDATED(predefined_file_generator) bad vstart=" << vstart);
      {
        std::lock_guard<std::recursive_mutex> gu{pobstart->get_recursive_mutex()};
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) start_generation pobstart=" << MomShowObject(pobstart)
                     << " of paylname=" << pobstart->unsync_paylname()
                     << " with ownpob=" << MomShowObject(ownpob)
                     << " and targpob=" << MomShowObject(targpob));
        pobstart->unsync_append_comp(MomString::make_from_string("// generated _mom_predefined.h\n"));
        return true;
      }
    }
  return false;
} // end MOMCOD_UPDATED(predefined_file_generator)
