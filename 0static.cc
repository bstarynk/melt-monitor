// file 0static.cc - early static constructed data
// the data should be constructed early!

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

std::mutex MomRegisterGlobData::_gd_mtx_;
std::map<std::string,std::atomic<MomObject*>*> MomRegisterGlobData::_gd_dict_;

std::mutex  MomRegisterPayload::_pd_mtx_;
std::map<std::string,const MomVtablePayload_st*>  MomRegisterPayload::_pd_dict_;

const char* mom_dump_dir;
const char* mom_web_option;
const char*mom_load_dir = ".";

MomObject::MomBucketObj MomObject::_ob_bucketarr_[MomObject::_obmaxbucket_];
std::atomic<unsigned> MomObject::_ob_nbclearedbuckets_;
std::atomic<unsigned> MomObject::_ob_nbsweepedbuckets_;
std::mutex MomObject::_predefmtx_;
MomObjptrSet MomObject::_predefset_;
