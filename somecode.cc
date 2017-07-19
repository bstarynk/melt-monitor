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




extern "C" bool MOMCOD_STEPPED(start_cplusplus_outputter)
(const struct MomPayload*payl, MomObject*targpob,
 const MomValue*vecarr, unsigned veclen,int depth)
{
  auto py = static_cast<MomPaylCode*>(const_cast<MomPayload*>(payl));
  MOM_ASSERT(py && py->_py_vtbl ==  &MOM_PAYLOADVTBL(code),
             "MOMCOD_STEPPED(predefined_file_generator) invalid code payload for targpob=" << targpob);
  // targpob would be the genfile proxy of mom_predefined_file
  MomObject*ownpob = py->owner();
  // ownpob would be the code proxy of mom_predefined_file
  MOM_DEBUGLOG(gencod, "MOMCOD_STEPPED(start_cplusplus_outputter) start"
               << " targpob="  << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " args=" << MomShowVectorValues(vecarr, veclen)
               << " depth=" << depth);
  if (veclen != 5)
    {
      MOM_FAILURE("MOMCOD_STEPPED(start_cplusplus_outputter) wants five arguments but got " << MomShowVectorValues(vecarr, veclen));
    }
  MomObject*pobstrbuf = const_cast<MomObject*>(vecarr[0].as_val()->as_object());
  MomObject*pobgenfile = const_cast<MomObject*>(vecarr[1].as_val()->as_object());
  const MomNode*startnod = vecarr[2].as_val()->as_node();
  MomObject*pobctx = const_cast<MomObject*>(vecarr[3].as_val()->as_object());
  int outdepth= vecarr[4].as_tagint();
  MOM_DEBUGLOG(gencod, "MOMCOD_STEPPED(start_cplusplus_outputter)"
               << std::endl << "..."
               << " pobstrbuf=" << MomShowObject(pobstrbuf)
               << " pobgenfile=" << MomShowObject(pobgenfile)
               << " targpob=" << MomShowObject(targpob)
               << " startnod=" << MomValue(startnod)
               << " pobctx=" << MomShowObject(pobctx)
               << " outdepth=" << outdepth
               << " ¤¤¤¤¤¤" << std::endl);
  std::lock_guard<std::recursive_mutex> gustrbuf{pobstrbuf->get_recursive_mutex()};
  auto pystrbuf = pobstrbuf->unsync_runcast_payload<MomPaylStrobuf>(MOM_PAYLOADVTBL(strobuf));
  if (!pystrbuf)
    MOM_FAILURE("MOMCOD_STEPPED(start_cplusplus_outputter) "
                " pobstrbuf=" << MomShowObject(pobstrbuf)
                << " is not a strobuf");
  std::lock_guard<std::recursive_mutex> gugenfil{pobgenfile->get_recursive_mutex()};
  auto pygenfil = pobgenfile->unsync_runcast_payload<MomPaylGenfile>(MOM_PAYLOADVTBL(genfile));
  if (!pygenfil)
    MOM_FAILURE("MOMCOD_STEPPED(start_cplusplus_outputter) "
                " pobgenfile=" << MomShowObject(pobgenfile)
                << " is not a genfile");
  std::string filname = startnod->nth_son(0).as_val()->as_string()->cstr();
  static constexpr unsigned gpl_notice_size = 1024;
  char*bufgpl = (char*)calloc(gpl_notice_size, 1);
  if (MOM_UNLIKELY(!bufgpl))
    MOM_FATALOG("MOMCOD_STEPPED(start_cplusplus_outputter): "
                "failed to allocate buffer of " << gpl_notice_size
                << " for gpl notice");
  size_t sizgpl = gpl_notice_size;
  FILE*filgpl = open_memstream(&bufgpl, &sizgpl);
  if (MOM_UNLIKELY(!filgpl))
    MOM_FATAPRINTF("MOMCOD_STEPPED(start_cplusplus_outputter): "
                   "failed open_memstream");
  mom_output_gplv3_notice (filgpl, "///", "", filname.c_str());
  fflush(filgpl);
  pystrbuf->out() << bufgpl << std::endl;
  fclose(filgpl);
  free (bufgpl), bufgpl=0;
  MOM_DEBUGLOG(gencod, "MOMCOD_STEPPED(start_cplusplus_outputter) end"
               << " targpob="  << MomShowObject(targpob)
               << " ownpob=" << MomShowObject(ownpob)
               << " args=" << MomShowVectorValues(vecarr, veclen)
               << " depth=" << depth
               << std::endl);
  return true;
} // end  MOMCOD_STEPPED(start_cplusplus_outputter)
