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
  // targpob would be the genfile proxy of mom_predefined_file
  MomObject*ownpob = py->owner();
  // ownpob would be the code proxy of mom_predefined_file
  MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) start"
               << " targpob="  << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
               << " args=" << MomShowVectorValues(vecarr, veclen));
  // attrob could be start_generation
  if (attrob == MOMP_start_generation && veclen == 1)
    {
      MomValue vstart = vecarr[0];
      MomObject*pobstart = const_cast<MomObject*>(vstart.to_val()->as_object());
      if (!pobstart)
        MOM_FAILURE("MOMCOD_UPDATED(predefined_file_generator) bad vstart=" << vstart);
      // pobstart would be the strobuf created by generated_strbuf_object
      {
        std::lock_guard<std::recursive_mutex> gustart{pobstart->get_recursive_mutex()};
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) start_generation pobstart=" << MomShowObject(pobstart)
                     << " of paylname=" << pobstart->unsync_paylname()
                     << " with ownpob=" << MomShowObject(ownpob)
                     << " and targpob=" << MomShowObject(targpob));
        pobstart->unsync_append_comp(MomString::make_from_string("// generated _mom_predefined.h\n"));
        auto pystartbuf = pobstart->unsync_runcast_payload<MomPaylStrobuf>(MOM_PAYLOADVTBL(strobuf));
        if (!pystartbuf)
          {
            MOM_WARNLOG("MOMCOD_UPDATED(predefined_file_generator) pobstart=" << MomShowObject(pobstart) << " has not a strobuf payload but " <<  pobstart->unsync_paylname());
            return false;
          }
        std::lock_guard<std::recursive_mutex> gutarg{targpob->get_recursive_mutex()};
        auto pytargenfil = targpob->unsync_runcast_payload<MomPaylGenfile>(MOM_PAYLOADVTBL(genfile));
        if (!pytargenfil)
          {
            MOM_WARNLOG("MOMCOD_UPDATED(predefined_file_generator) targpob=" << MomShowObject(targpob) <<
                        " has no genfile payload but " << targpob->unsync_paylname());
            return false;
          }
        std::lock_guard<std::recursive_mutex> guown{ownpob->get_recursive_mutex()};
        MomValue comp0own = ownpob->unsync_get_nth_comp(0);
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) ownpob=" << MomShowObject(ownpob)
                     << " with comp0own=" << comp0own);
        MomObject*pobcomp0own = const_cast<MomObject*>(comp0own->as_object());
        MomValue vnod = MomNode::make_from_values(pobcomp0own,
                        MomString::make_from_string(pytargenfil->genfile_pathstr()));
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) vnod=" << vnod
                     << " appended to pobstart=" << MomShowObject(pobstart));
        pobstart->unsync_append_comp(vnod);
        MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(predefined_file_generator) ending successfully pobstart="
                     << MomShowObject(pobstart) << " ¤¤¤¤¤¤" << std::endl);
        return true;
      }
    }
  return false;
} // end MOMCOD_UPDATED(predefined_file_generator)

extern "C" bool MOMCOD_UPDATED(start_cplusplus_outputter)
(const struct MomPayload*payl, MomObject*targpob,const MomObject*attrob,
 const MomValue*vecarr, unsigned veclen)
{
  auto py = static_cast<MomPaylCode*>(const_cast<MomPayload*>(payl));
  MOM_ASSERT(py && py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MOMCOD_UPDATED(predefined_file_generator) invalid code payload for targpob=" << targpob);
  // targpob would be the genfile proxy of mom_predefined_file
  MomObject*ownpob = py->owner();
  // ownpob would be the code proxy of mom_predefined_file
  MOM_DEBUGLOG(gencod, "MOMCOD_UPDATED(start_cplusplus_outputter) start"
               << " targpob="  << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " attrob=" << MomShowObject(const_cast<MomObject*>(attrob))
               << " args=" << MomShowVectorValues(vecarr, veclen));
  return false;
} // end  MOMCOD_UPDATED(start_cplusplus_outputter)
