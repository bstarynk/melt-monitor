// file state.cc - managing persistent state, load & dump

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

/// from https://github.com/aminroosta/sqlite_modern_cpp
#include "sqlite_modern_cpp.h"

class MomLoader
{
  std::string _ld_dirname;
  const double _ld_startrealtime, _ld_startcputime;
  // global & user databases and their mutexes
  std::unique_ptr<sqlite::database> _ld_globdbp;
  std::unique_ptr<sqlite::database> _ld_userdbp;
  std::mutex _ld_mtxglobdb;
  std::mutex _ld_mtxuserdb;
  // object map and its mutex
  std::unordered_map<MomIdent,MomObject*,MomIdentBucketHash> _ld_objmap;
  std::mutex _ld_mtxobjmap;
  std::unique_ptr<sqlite::database> load_database(const char*dbradix);
  MomObject* load_find_object_by_id(MomIdent id)
  {
    std::lock_guard<std::mutex> gu(_ld_mtxobjmap);
    auto it = _ld_objmap.find(id);
    if (it != _ld_objmap.end()) return it->second;
    return nullptr;
  }
  long load_empty_objects_from_db(sqlite::database* pdb, bool user);
  void load_all_globdata(void);
  void load_all_objects_content(void);
  void load_all_objects_payload_make(void);
  static void load_all_objects_payload_from_db(MomLoader*,sqlite::database* pdb, bool user);
  void load_all_objects_payload_fill(void);
  void load_object_content(MomObject*pob, int thix, const std::string&strcont);
  void load_object_fill_payload(MomObject*pob, int thix, const std::string&strfill);
  void load_cold_globdata(const char*globnam, std::atomic<MomObject*>*pglob);
  static void thread_load_content_objects (MomLoader*ld, int thix, std::deque<MomObject*>*obqu, const std::function<std::string(MomObject*)>&getglobfun,const std::function<std::string(MomObject*)>&getuserfun);
  static void thread_load_fill_payload_objects (MomLoader*ld, int thix, std::deque<MomObject*>*obqu, const std::function<std::string(MomObject*)>&fillglobfun,const std::function<std::string(MomObject*)>&filluserfun);
  static void load_touch_objects_from_db(MomLoader*ld, sqlite::database* pdb, bool user);
public:
  MomLoader(const std::string&dirnam);
  static constexpr const bool IS_USER= true;
  static constexpr const bool IS_GLOBAL= false;
  void load(void);
};				// end class MomLoader

const bool MomLoader::IS_USER;
const bool MomLoader::IS_GLOBAL;

std::unique_ptr<sqlite::database>
MomLoader::load_database(const char*dbradix)
{
  std::string dbpath = _ld_dirname + "/" + dbradix + ".sqlite";
  std::string sqlpath = _ld_dirname + "/" + dbradix + ".sql";
  struct stat dbstat;
  memset(&dbstat, 0, sizeof(dbstat));
  struct stat sqlstat;
  memset(&sqlstat, 0, sizeof(sqlstat));
  if (stat(dbpath.c_str(), &dbstat)
      || !S_ISREG(dbstat.st_mode)) return nullptr;
  if (!stat(sqlpath.c_str(), &sqlstat)
      && sqlstat.st_mtime > dbstat.st_mtime)
    MOM_FATALOG("load database " << dbpath << " older than its dump " << sqlpath);
  sqlite::sqlite_config dbconfig;
  dbconfig.flags = sqlite::OpenFlags::READONLY;
  return std::make_unique<sqlite::database>(dbpath, dbconfig);
} // end MomLoader::load_database


long
MomLoader::load_empty_objects_from_db(sqlite::database* pdb, bool user)
{
  long obcnt = 0;
  MOM_DEBUGLOG(load,"load_empty_objects_from_db start " << (user?"user":"glob"));
  std::lock_guard<std::mutex> gu (*(user?(&_ld_mtxuserdb):(&_ld_mtxglobdb)));
  *pdb << (user?"SELECT ob_id /*user*/ FROM t_objects" : "SELECT ob_id /*global*/ FROM t_objects")
       >> [&](std::string idstr)
  {
    MOM_DEBUGLOG(load,"load_empty_objects_from_db " << (user?"user":"glob")
                 << " idstr=" << idstr);
    auto id = MomIdent::make_from_cstr(idstr.c_str(),true);
    auto pob = MomObject::make_object_of_id(id);
    if (!pob)
      MOM_FAILURE("load_empty_objects_from_db bad idstr=" << idstr);
    {
      std::lock_guard<std::mutex> gu(_ld_mtxobjmap);
      _ld_objmap.insert({id,pob});
    }
    if (pob->space() == MomSpace::TransientSp)
      {
        if (user)
          {
            pob->set_space(MomSpace::UserSp);
          }
        else
          {
            pob->set_space(MomSpace::GlobalSp);
          }
      }
    obcnt++;
    MOM_DEBUGLOG(load,"load_empty_objects_from_db " << (user?"user":"glob")
                 << " obcnt=" << obcnt << " pob=" << pob);
  };
  MOM_DEBUGLOG(load,"loaded " << obcnt << " empty objects from " << (user?"user":"global"));
  return obcnt;
} // end MomLoader::load_empty_objects_from_db


MomLoader::MomLoader(const std::string& dirnam)
  : _ld_dirname(dirnam),
    _ld_startrealtime(mom_elapsed_real_time()),
    _ld_startcputime(mom_process_cpu_time()),
    _ld_globdbp(nullptr), _ld_userdbp(nullptr),
    _ld_mtxglobdb(), _ld_mtxuserdb(),
    _ld_objmap(), _ld_mtxobjmap()
{
  struct stat dirstat;
  memset (&dirstat, 0, sizeof(dirstat));
  errno = 0;
  if (dirnam.empty())
    MOM_FAILURE("MomLoader empty directory");
  if (stat(dirnam.c_str(), &dirstat)
      || (!S_ISDIR(dirstat.st_mode) && (errno=ENOTDIR)!=0))
    MOM_FAILURE("MomLoader bad directory " << dirnam << " : " << strerror(errno));
  std::string globdbnam = dirnam + "/" + MOM_GLOBAL_DB + ".sqlite";
  std::string userdbnam = dirnam + "/" + MOM_USER_DB + ".sqlite";
  if (::access(globdbnam.c_str(), R_OK))
    MOM_FAILURE("MomLoader missing global db " << globdbnam << " : " << strerror(errno));
  else
    _ld_globdbp = load_database(MOM_GLOBAL_DB);
  if (!_ld_globdbp)
    MOM_FAILURE("MomLoader fail to open global db " << globdbnam);
  if (::access(userdbnam.c_str(), R_OK))
    MOM_WARNLOG("MomLoader without user db " << userdbnam << " : " << strerror(errno));
  else
    {
      _ld_userdbp = load_database(MOM_USER_DB);
      if (!_ld_userdbp)
        MOM_FAILURE("MomLoader fail to open user db " << userdbnam);
    }
} // end MomLoader::MomLoader

void
MomLoader::load_touch_objects_from_db(MomLoader*ld, sqlite::database* pdb, bool user)
{
  pthread_setname_np(pthread_self(),user?"motouchuser":"mothouchglob");
  MOM_DEBUGLOG(load,"load_touch_objects_from_db start user=" << (user?"true":"false"));
  std::lock_guard<std::mutex> gu (*(user?(&ld->_ld_mtxuserdb):(&ld->_ld_mtxglobdb)));
  *pdb << (user?"SELECT ob_id, ob_mtim /*user*/ FROM t_objects" : "SELECT ob_id, ob_mtim /*global*/ FROM t_objects")
       >> [&](std::string idstr, double mtim)
  {
    auto id = MomIdent::make_from_cstr(idstr.c_str(),true);
    auto pob = ld->load_find_object_by_id(id);
    if (pob)
      pob->_ob_mtime = mtim;
  };
  MOM_DEBUGLOG(load,"load_touch_objects_from_db end user=" << (user?"true":"false"));
} // end MomLoader::load_touch_objects_from_db

void
MomLoader::load(void)
{
  long nbglobob = 0;
  long nbuserob = 0;
  {
    auto globthr = std::thread([&](void)
    {
      pthread_setname_np(pthread_self(), "moloademglob");
      nbglobob
        = load_empty_objects_from_db(_ld_globdbp.get(),IS_GLOBAL);
    });
    std::thread userthr;
    if (_ld_userdbp)
      {
        userthr = std::thread([&](void)
        {
          pthread_setname_np(pthread_self(), "moloademuser");
          nbuserob
            = load_empty_objects_from_db(_ld_userdbp.get(),IS_USER);
        });
        userthr.join();
      }
    globthr.join();
  }
  load_all_globdata();
  load_all_objects_content();
  load_all_objects_payload_make();
  load_all_objects_payload_fill();
  {
    auto ld = this;
    auto globthr = std::thread(load_touch_objects_from_db,ld,_ld_globdbp.get(),IS_GLOBAL);
    std::thread userthr;
    if (_ld_userdbp)
      {
        userthr = std::thread(load_touch_objects_from_db,ld,_ld_userdbp.get(),IS_USER);
        userthr.join();
      }
    globthr.join();
  }
  char cputimbuf[24], realtimbuf[24];
  snprintf(cputimbuf, sizeof(cputimbuf), "%.3f", mom_process_cpu_time() - _ld_startcputime);
  snprintf(realtimbuf, sizeof(realtimbuf), "%.2f", mom_elapsed_real_time() - _ld_startrealtime);
  MOM_INFORMLOG("loaded " << nbglobob << " global, " << nbuserob << " user objects from " << _ld_dirname
                << " in " << realtimbuf << " real, "  << cputimbuf << " cpu seconds" << std::endl);
} // end MomLoader::load



void
MomLoader::load_cold_globdata(const char*globnam, std::atomic<MomObject*>*pglob)
{
  MOM_DEBUGLOG(load,"load_cold_globdata start globnam=" << globnam);
  MomObject*globpob = nullptr;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxglobdb};
    *_ld_globdbp << "SELECT glob_oid /*global*/ FROM t_globdata WHERE glob_namestr = ?;"
                 << globnam >> [&](const std::string &idstr)
    {
      globpob = MomObject::find_object_of_id(MomIdent::make_from_cstr(idstr.c_str()));
      MOM_DEBUGLOG(load,"load_cold_globdata global globnam=" << globnam << " globpob=" << globpob);
    };
  }
  if (!globpob && _ld_userdbp)
    {
      std::lock_guard<std::mutex> gu{_ld_mtxuserdb};
      *_ld_userdbp << "SELECT glob_oid /*user*/ FROM t_globdata WHERE glob_namestr = ?;"
                   << globnam >> [&](const std::string &idstr)
      {
        globpob = MomObject::find_object_of_id(MomIdent::make_from_cstr(idstr.c_str()));
        MOM_DEBUGLOG(load,"load_cold_globdata user globnam=" << globnam << " globpob=" << globpob);
      };
    }
  if (globpob)
    {
      pglob->store(globpob);
    }
} // end MomLoader::load_cold_globdata

void
MomLoader::load_all_globdata(void)
{
#define MOM_HAS_GLOBDATA(Nam) \
  load_cold_globdata(#Nam,&MOM_GLOBDATA_VAR(Nam));
#include "_mom_globdata.h"
} // end MomLoader::load_all_globdata


void
MomLoader::load_all_objects_content(void)
{
  MOM_DEBUGLOG(load,"load_all_objects_content start");
  /// prepare the object loading statements
  auto globstmt = ((*_ld_globdbp)
                   << "SELECT /*globaldb*/ ob_content FROM t_objects WHERE ob_id = ?");
  std::function<std::string(MomObject*)> getglobfun =
    [&](MomObject*pob)
  {
    std::lock_guard<std::mutex> gu(_ld_mtxglobdb);
    MOM_DEBUGLOG(load,"load_all_objects_content getglobfun start pob=" << pob);
    std::string res;
    //MOM_BACKTRACELOG("load_all_objects_content @@getglobfun pob=" << pob);
    globstmt.reset();
    globstmt << pob->id().to_string() >> res;
    MOM_DEBUGLOG(load,"load_all_objects_content getglobfun pob=" << pob << " res=" << res);
    globstmt.reset();
    return res;
  };
  std::function<std::string(MomObject*)> getuserfun;
  auto userstmt = ((*(_ld_userdbp?:_ld_globdbp))
                   << "SELECT  /*userdb*/ ob_content FROM t_objects WHERE ob_id = ?");
  if (_ld_userdbp)
    {
      getuserfun =
        [&](MomObject*pob)
      {
        std::lock_guard<std::mutex> gu(_ld_mtxuserdb);
        MOM_DEBUGLOG(load,"load_all_objects_content getuserfun start pob=" << pob);
        std::string res;
        userstmt.reset();
        userstmt << pob->id().to_string() >> res;
        MOM_DEBUGLOG(load,"load_all_objects_content getuserfun pob=" << pob << " res=" << res);
        userstmt.reset();
        return res;
      };
    }
  /// build a vector of queue of objects to be loaded
  std::vector<std::deque<MomObject*>> vecobjque(mom_nb_jobs);
  unsigned long obcnt = 0;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxobjmap};
    MOM_DEBUGLOG(load,"load_all_objects_content objmapsize=" << _ld_objmap.size());
    for (auto p : _ld_objmap)
      {
        MomObject* pob = p.second;
        MOM_ASSERT(pob != nullptr && pob->id() == p.first,
                   "MomLoader::load_all_objects_content bad id");
        int qix = obcnt % mom_nb_jobs;
        vecobjque[qix].push_back(pob);
        obcnt++;
        MOM_DEBUGLOG(load,"load_all_objects_content obcnt=" << obcnt << " pob=" << pob << " qix#" << qix);
      }
  }
  std::vector<std::thread> vecthr(mom_nb_jobs);
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1] = std::thread(thread_load_content_objects, this, ix, &vecobjque[ix-1], getglobfun, getuserfun);
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
  globstmt.used(true);
  userstmt.used(true);
  MOM_DEBUGLOG(load,"load_all_objects_content end");
} // end MomLoader::load_all_objects_content

void
MomLoader::load_all_objects_payload_make(void)
{
  MOM_DEBUGLOG(load,"load_all_objects_payload_make start");
  std::thread thrglob(load_all_objects_payload_from_db, this,  _ld_globdbp.get(),  IS_GLOBAL);
  std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
  if (_ld_userdbp)
    {
      std::this_thread::yield();
      std::thread thruser(load_all_objects_payload_from_db, this,  _ld_userdbp.get(),  IS_USER);
      std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
      thruser.join();
    }
  thrglob.join();
  MOM_DEBUGLOG(load,"load_all_objects_payload_make end");
} // end MomLoader::load_all_objects_payload_make


void
MomLoader::load_all_objects_payload_from_db(MomLoader*ld, sqlite::database* pdb, bool user)
{
  MOM_DEBUGLOG(load, "start load_all_objects_payload_from_db user=" << (user?"true":"false"));
  std::lock_guard<std::mutex> gu(*(user?&ld->_ld_mtxuserdb:&ld->_ld_mtxglobdb));
  *pdb << (user?"SELECT /*user*/ ob_id, ob_paylkind, ob_paylinit"
           " FROM t_objects WHERE ob_paylkind != ''"
           :"SELECT /*global*/ ob_id, ob_paylkind, ob_paylinit"
           " FROM t_objects WHERE ob_paylkind != ''")
       >> [=] (const std::string& idstr, const std::string& paykind, const std::string&paylinit)
  {
    MOM_DEBUGLOG(load, "load_all_objects_payload_from_db idstr=" << idstr
                 << " paykind=" << paykind
                 << " paylinit=" << paylinit);
    auto id = MomIdent::make_from_string(idstr, MomIdent::DO_FAIL);
    auto pob = ld->load_find_object_by_id(id);
    if (!pob) MOM_FAILURE("no object of id:" << id);
    MOM_DEBUGLOG(load, "load_all_objects_payload_from_db pob=" << pob << " paykind=" << paykind
                 << std::endl << "..paylinit=" << paylinit);
    const MomVtablePayload_st* pyvt = MomRegisterPayload::find_payloadv(paykind);
    if (!pyvt)
      MOM_FAILURE("object " << id << " has bad payload kind " << paykind);
    if (!pyvt->pyv_initload)
      MOM_FAILURE("object " << id << " withot initload payload " << pyvt->pyv_name);
    MOM_DEBUGLOG(load, "load_all_objects_payload_from_db before initload pob=" << pob
                 << " paylinit=" << paylinit);
    pob->_ob_payl = pyvt->pyv_initload(pob,ld,paylinit.c_str());
    MOM_DEBUGLOG(load, "load_all_objects_payload_from_db after initload pob=" << pob);
  };
  MOM_DEBUGLOG(load, "end load_all_objects_payload_from_db user=" << (user?"true":"false"));
} // end MomLoader::load_all_objects_payload_from_db




void
MomLoader::load_all_objects_payload_fill(void)
{
  MOM_DEBUGLOG(load,"load_all_objects_payload_fill start");
  /// prepare the payload filling statements
  auto globstmt = ((*_ld_globdbp)
                   << "SELECT /*globaldb*/ ob_paylcontent FROM t_objects WHERE ob_id = ?");
  std::function<std::string(MomObject*)> fillglobfun =
    [&](MomObject*pob)
  {
    std::lock_guard<std::mutex> gu(_ld_mtxglobdb);
    std::string res;
    globstmt << pob->id().to_string() >> res;
    globstmt.reset();
    MOM_DEBUGLOG(load,"load_all_objects_payload_fill fillglobfun pob=" << pob << " res=" << res);
    return res;
  };
  std::function<std::string(MomObject*)> filluserfun;
  auto userstmt = ((*(_ld_userdbp?:_ld_globdbp))
                   << "SELECT  /*userdb*/ ob_paylcontent FROM t_objects WHERE ob_id = ?");
  if (_ld_userdbp)
    {
      filluserfun =
        [&](MomObject*pob)
      {
        std::lock_guard<std::mutex> gu(_ld_mtxuserdb);
        std::string res;
        userstmt << pob->id().to_string() >> res;
        userstmt.reset();
        MOM_DEBUGLOG(load,"load_all_objects_payload_fill filluserfun pob=" << pob << " res=" << res);
        return res;
      };
    }
  /// build a vector of queue of objects to be filled
  std::vector<std::deque<MomObject*>> vecobjque(mom_nb_jobs);
  unsigned long obcnt = 0;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxobjmap};
    for (auto p : _ld_objmap)
      {
        MomObject* pob = p.second;
        MOM_ASSERT(pob != nullptr && pob->id() == p.first,
                   "MomLoader::load_all_objects_payload_fill bad id");
        if (!pob->_ob_payl)
          continue;
        if (!pob->_ob_payl->_py_vtbl->pyv_loadfill)
          {
            MOM_DEBUGLOG(load,"load_all_objects_payload_fill skip pob=" << pob);
            continue;
          }
        int qix = obcnt % mom_nb_jobs;
        vecobjque[qix].push_back(pob);
        obcnt++;
        MOM_DEBUGLOG(load,"load_all_objects_payload_fill pob=" << pob << " qix=" << qix
                     << " obcnt=" << obcnt);
      }
  }
  std::vector<std::thread> vecthr(mom_nb_jobs);
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1] = std::thread(thread_load_fill_payload_objects, this, ix, &vecobjque[ix-1], fillglobfun, filluserfun);
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
  globstmt.used(true);
  userstmt.used(true);
} // end MomLoader::load_all_objects_payload_fill


void
MomLoader::thread_load_fill_payload_objects(MomLoader*ld, int thix, std::deque<MomObject*>*obpqu,
    const std::function<std::string(MomObject*)>&fillglobfun,
    const std::function<std::string(MomObject*)>&filluserfun)
{
  char buf[24];
  snprintf(buf, sizeof(buf), "molopayl%d", thix);
  pthread_setname_np(pthread_self(), buf);
  MOM_ASSERT(ld != nullptr,
             "MomLoader::thread_load_fill_payload_objects null ld");
  MOM_ASSERT(thix>0 && thix<=(int)mom_nb_jobs,
             "MomLoader::thread_load_fill_payload_objects bad thix=" << thix);
  MOM_ASSERT(obpqu != nullptr, "MomLoader::thread_load_fill_payload_objects null obpqu");
  MOM_DEBUGLOG(load,"thread_load_fill_payload_objects thix=#" << thix);
  while (!obpqu->empty())
    {
      MomObject*pob = obpqu->front();
      obpqu->pop_front();
      MOM_DEBUGLOG(load,"thread_load_fill_payload_objects thix=#" << thix << " pob=" << pob);
      MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
                 "MomLoader::thread_load_fill_payload_objects bad pob");
      MOM_ASSERT(pob->_ob_payl != nullptr,
                 "MomLoader::thread_load_fill_payload_objects no payload for pob:" <<pob);
      std::string strfill;
      if (pob->space() == MomSpace::UserSp)
        strfill = filluserfun(pob);
      else
        strfill = fillglobfun(pob);
      MOM_DEBUGLOG(load,"thread_load_fill_payload_objects thix=#" << thix << " pob=" << pob
                   << " strfill=" << strfill);
      ld->load_object_fill_payload(pob, thix, strfill);
    };
  MOM_DEBUGLOG(load,"thread_load_fill_payload_objects done thix=#" << thix << std::endl);
} // end MomLoader::thread_load_content_objects


void MomLoader::load_object_fill_payload(MomObject*pob, int thix, const std::string&strfill)
{
  MOM_DEBUGLOG(load,"load_object_fill_payload start pob=" << pob << " thix=" << thix
               << " strfill=" << strfill);
  auto py = pob->_ob_payl;
  py->_py_vtbl->pyv_loadfill(py, pob, this, strfill.c_str());
  MOM_DEBUGLOG(load,"load_object_fill_payload end pob=" << pob << " thix=" << thix
               << " strfill=" << strfill);
} // end MomLoader::load_object_fill_payload


void
MomLoader::thread_load_content_objects(MomLoader*ld, int thix, std::deque<MomObject*>*obpqu,
                                       const std::function<std::string(MomObject*)>&getglobfun,
                                       const std::function<std::string(MomObject*)>&getuserfun)
{
  char buf[24];
  snprintf(buf, sizeof(buf), "moldcont%d", thix);
  pthread_setname_np(pthread_self(),buf);
  MOM_ASSERT(ld != nullptr,
             "MomLoader::thread_load_content_objects null ld");
  MOM_ASSERT(thix>0 && thix<=(int)mom_nb_jobs,
             "MomLoader::thread_load_content_objects bad thix=" << thix);
  MOM_ASSERT(obpqu != nullptr, "MomLoader::thread_load_content_objects null obpqu");
  MOM_DEBUGLOG(load,"thread_load_content_objects thix=#" << thix << " qusiz=" << obpqu->size());
  while (!obpqu->empty())
    {
      MomObject*pob = obpqu->front();
      obpqu->pop_front();
      MOM_DEBUGLOG(load,"thread_load_content_objects thix=#" << thix << " pob=" << pob
                   << " space#" << (int)pob->space()
                   << ":"
                   << (((pob->space() == MomSpace::UserSp)?"user":"global")));
      MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
                 "MomLoader::thread_load_content_objects bad pob");
      std::string strcont;
      if (pob->space() == MomSpace::UserSp)
        {
          MOM_DEBUGLOG(load,"thread_load_content_objects thix#" << thix
                       << " before getuserfun on pob=" << pob);
          strcont = getuserfun(pob);
          MOM_DEBUGLOG(load,"thread_load_content_objects thix#" << thix
                       << " after getuserfun on pob=" << pob);
        }
      else
        {
          MOM_DEBUGLOG(load,"thread_load_content_objects thix#" << thix
                       << " before getglobfun on pob=" << pob);
          strcont = getglobfun(pob);
          MOM_DEBUGLOG(load,"thread_load_content_objects thix#" << thix
                       << " after getglobfun on pob=" << pob);
        }
      MOM_DEBUGLOG(load,"thread_load_content_objects thix=#" << thix << " pob=" << pob
                   << " strcont=" << strcont);
      if (!strcont.empty())
        ld->load_object_content(pob, thix, strcont);
    };
  MOM_DEBUGLOG(load,"thread_load_content_objects done thix=#" << thix << std::endl);
} // end MomLoader::thread_load_content_objects



void
MomLoader::load_object_content(MomObject*pob, int thix, const std::string&strcont)
{
  MOM_DEBUGLOG(load,"load_object_content start pob=" << pob << " thix=" << thix << " strcont=" << strcont);
  std::istringstream incont(strcont);
  MomParser contpars(incont);
  char title[80];
  memset(title,0,sizeof(title));
  char idbuf[32];
  memset(idbuf, 0, sizeof(idbuf));
  pob->id().to_cbuf32(idbuf);
  snprintf(title, sizeof(title), "*content %s*", idbuf);
  contpars.set_name(std::string{title}).set_make_from_id(true);
  contpars.next_line();
  int nbcomp = 0;
  int nbattr = 0;
  MOM_ASSERT(thix>0 && thix<=(int)mom_nb_jobs, "MomLoader::load_object_content bad thix#" << thix);
  for (;;)
    {
      contpars.skip_spaces();
      if (contpars.eof()) break;
      if (contpars.hasdelim("@@"))
        {
          bool gotattr = false;
          MomObject*pobattr = contpars.parse_objptr(&gotattr);
          if (!pobattr || !gotattr)
            MOM_PARSE_FAILURE(&contpars, "missing attribute after @@");
          bool gotval = false;
          MomValue valattr = contpars.parse_value(&gotval);
          if (!gotval)
            MOM_PARSE_FAILURE(&contpars, "missing value for attribute " << pobattr);
          pob->locked_modify([=](MomObject* pthisob)
          {
            pthisob->unsync_put_phys_attr(pobattr, valattr);
            return;
          });
          nbattr++;
        }
      else if (contpars.hasdelim("&&"))
        {
          bool gotcomp = false;
          MomValue valcomp = contpars.parse_value(&gotcomp);
          if (!gotcomp)
            MOM_PARSE_FAILURE(&contpars, "missing component#" << nbcomp);
          pob->locked_modify([=](MomObject* pthisob)
          {
            pthisob->unsync_append_comp(valcomp);
            return;
          });
          nbcomp++;
        }
    }
  MOM_DEBUGLOG(load,"load_object_content end pob=" << pob << " thix=" << thix
               << " nbattr=" << nbattr << " nbcomp=" << nbcomp);
} // end MomLoader::load_object_content


void
mom_load_from_directory(const char*dirname)
{
  MomLoader ld(dirname);
  ld.load();
} // end mom_load_from_directory


//==============================================================
////////////////////////////////////////////////////////////////

enum MomDumpState { dus_none, dus_scan, dus_emit };

typedef std::function<void(MomObject*pob,int thix,double mtim,const std::string&contentstr,const MomObject::PayloadEmission& pyem)> momdumpinsertfunction_t;
class MomDumper
{
  std::mutex _du_mtx;
  const double _du_startrealtime;
  const double _du_startcputime;
  MomDumpState _du_state;
  std::string _du_dirname;
  std::string _du_tempsuffix;
  std::set<std::string> _du_tempset;
  std::unique_ptr<sqlite::database> _du_globdbp;
  std::unique_ptr<sqlite::database> _du_userdbp;
  std::mutex _du_globdbmtx;
  std::mutex _du_userdbmtx;
  std::condition_variable _du_addedcondvar;
  std::unordered_set<const MomObject*,MomObjptrHash> _du_setobj;
  std::deque<const MomObject*> _du_queobj;
  std::atomic_ulong _du_scancount;
  const MomSet* _du_predefvset;
  std::map<std::string,MomObject*> _du_globmap;
  void initialize_db(sqlite::database &db);
  static void dump_scan_thread(MomDumper*du, int ix);
  static void dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec, momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf);
  void dump_emit_object(MomObject*pob, int thix,momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf);
public:
  std::string temporary_file_path(const std::string& path);
  static bool rename_file_if_changed(const std::string& srcpath, const std::string& dstpath, bool keepsamesrc=false); // return true if same files
  void rename_temporary_files(void);
  MomDumper(const std::string&dirnam);
  void open_databases(void);
  long nb_objects(void)
  {
    std::lock_guard<std::mutex> gu(_du_mtx);
    return _du_setobj.size();
  }
  void close_and_dump_databases(void);
  pid_t fork_dump_database(const std::string&dbpath, const std::string&sqlpath, const std::string& basepath);
  void scan_predefined(void);
  void scan_globdata(void);
  void dump_scan_loop(void);
  void dump_emit_loop(void);
  void dump_emit_globdata(void);
  void scan_inside_object(MomObject*pob);
  void add_scanned_object(const MomObject*pob)
  {
    MOM_ASSERT(pob != nullptr, "MomDumper::add_scanned_object null ptr");
    MOM_ASSERT(_du_state == dus_scan, "MomDumper::add_scanned_object not scanning");
    bool added = false;
    {
      std::lock_guard<std::mutex> gu{_du_mtx};
      if (_du_setobj.find(pob) == _du_setobj.end())
        {
          if (pob->space() == MomSpace::TransientSp)
            return;
          _du_setobj.insert(pob);
          _du_queobj.push_back(pob);
          MOM_DEBUGLOG(dump, "MomDumper::add_scanned_object added pob=" << pob);
          added = true;
        }
    }
    if (added)
      _du_addedcondvar.notify_all();
  }
  bool is_dumped(const MomObject*pob)
  {
    if (!pob) return false;
    std::lock_guard<std::mutex> gu{_du_mtx};
    return (_du_setobj.find(pob) != _du_setobj.end());
  }
  ~MomDumper();
  double dump_real_time()
  {
    return mom_elapsed_real_time() - _du_startrealtime;
  };
  double dump_cpu_time()
  {
    return mom_process_cpu_time() - _du_startcputime;
  };
};				// end class MomDumper

class MomDumpEmitter final : public MomEmitter
{
  MomDumper*_de_dumper;
public:
  MomDumpEmitter(std::ostringstream& outs, MomDumper*dum)
    : MomEmitter(outs), _de_dumper(dum)
  {
    set_line_width(88).show_transient(false);
  };
  virtual bool skippable_object(const MomObject*pob) const
  {
    return !_de_dumper->is_dumped(pob);
  }
};				// end MomDumpEmitter

MomDumper::MomDumper(const std::string&dirnam)
  : _du_mtx(),
    _du_startrealtime(mom_elapsed_real_time()),
    _du_startcputime(mom_process_cpu_time()),
    _du_state{dus_none}, _du_dirname(dirnam), _du_tempsuffix(), _du_tempset(),
    _du_globdbp(), _du_userdbp(),
    _du_globdbmtx(), _du_userdbmtx(),
    _du_addedcondvar(),
    _du_setobj(), _du_queobj(), _du_scancount(ATOMIC_VAR_INIT(0ul)), _du_predefvset(nullptr), _du_globmap{}
{
  struct stat dirstat;
  memset (&dirstat, 0, sizeof(dirstat));
  errno = 0;
  if (dirnam.empty())
    MOM_FAILURE("MomDumper empty directory");
  if (access(dirnam.c_str(), F_OK) && errno == ENOENT)
    {
      if (mkdir(dirnam.c_str(), 0750))
        MOM_FAILURE("MomDumper fail to mkdir " << dirnam << " : " << strerror(errno));
      else
        MOM_INFORMPRINTF("made dump directory %s", dirnam.c_str());
    }
  if (stat(dirnam.c_str(), &dirstat)
      || (!S_ISDIR(dirstat.st_mode) && (errno=ENOTDIR)!=0))
    MOM_FAILURE("MomDumper bad directory " << dirnam << " : " << strerror(errno));
  {
    char tempbuf[64];
    memset(tempbuf, 0, sizeof(tempbuf));
    auto rs = MomSerial63::make_random();
    char sbuf16[16];
    memset(sbuf16, 0, sizeof(sbuf16));
    rs.to_cbuf16(sbuf16);
    sbuf16[0] = '+';
    snprintf(tempbuf, sizeof(tempbuf), "%s_p%d-", sbuf16, (int)getpid());
    _du_tempsuffix.assign(tempbuf);
  }
} // end MomDumper::MomDumper


std::string
MomDumper::temporary_file_path(const std::string& path)
{
  if (path.empty() || path[0] == '.' || path[0] == '/' || path.find("..") != std::string::npos)
    MOM_FAILURE("MomDumper (in " << _du_dirname << " directory) with bad temporal path " << path);
  _du_tempset.insert(path);
  return _du_dirname + "/" + path + _du_tempsuffix;
} // end MomDumper::temporary_file_path

void
MomDumper::open_databases(void)
{
  auto globdbpath = temporary_file_path(MOM_GLOBAL_DB ".sqlite");
  auto userdbpath = temporary_file_path(MOM_USER_DB ".sqlite");
  sqlite::sqlite_config dbconfig;
  dbconfig.flags = sqlite::OpenFlags::CREATE | sqlite::OpenFlags::READWRITE| sqlite::OpenFlags::NOMUTEX;
  dbconfig.encoding = sqlite::Encoding::UTF8;
  _du_globdbp = std::make_unique<sqlite::database>(globdbpath, dbconfig);
  _du_userdbp = std::make_unique<sqlite::database>(userdbpath, dbconfig);
  initialize_db(*_du_globdbp);
  initialize_db(*_du_userdbp);
} // end MomDumper::open_databases


pid_t
MomDumper::fork_dump_database(const std::string&dbpath, const std::string&sqlpath, const std::string& basepath)
{
  FILE *f = fopen(sqlpath.c_str(), "w");
  if (!f)
    MOM_FATALOG("MomDumper::fork_dump_database failed to open " << sqlpath
                << " (" << strerror(errno) << ")");
  fprintf(f, "-- generated MONIMELT dump %s ** DONT EDIT\n", basepath.c_str());
  fflush(f);
  fflush(nullptr);
  pid_t p = fork();
  if (p==0)
    {
      close(STDIN_FILENO);
      int nfd = open("/dev/null", O_RDONLY);
      if (nfd>0) dup2(nfd, STDIN_FILENO);
      for (int ix=3; ix<128; ix++) if (ix != fileno(f)) (void) close(ix);
      dup2(fileno(f), STDOUT_FILENO);
      nice(1);
      for (int sig=1; sig<SIGRTMIN; sig++) signal(sig, SIG_DFL);
      execlp(monimelt_sqliteprog, monimelt_sqliteprog, dbpath.c_str(), ".dump", nullptr);
      perror("execlp sqlite3");
      _exit(EXIT_FAILURE);
    }
  else if (p<0)
    MOM_FATALOG("MomDumper::fork_dump_database failed to fork for " << basepath
                << " (" << strerror(errno) << ")");
  fclose(f);
  return p;
} // end MomDumper::fork_dump_database


void
MomDumper::close_and_dump_databases(void)
{
  auto globdbpath = temporary_file_path(MOM_GLOBAL_DB ".sqlite");
  auto userdbpath = temporary_file_path(MOM_USER_DB ".sqlite");
  auto globsqlpath = temporary_file_path(MOM_GLOBAL_DB ".sql");
  auto usersqlpath = temporary_file_path(MOM_USER_DB ".sql");
  *_du_globdbp << "END /*global*/ TRANSACTION";
  if (_du_userdbp)
    *_du_userdbp << "END /*user*/ TRANSACTION";
  _du_globdbp.reset();
  _du_userdbp.reset();
  pid_t globpid = fork_dump_database(globdbpath, globsqlpath, MOM_GLOBAL_DB);
  std::this_thread::sleep_for(std::chrono::milliseconds(15+2*mom_nb_jobs));
  pid_t userpid = fork_dump_database(userdbpath, usersqlpath, MOM_USER_DB);
  std::this_thread::sleep_for(std::chrono::milliseconds(35+4*mom_nb_jobs));
  int globstatus = 0;
  int userstatus = 0;
  if (waitpid(globpid, &globstatus, 0) < 0 || !WIFEXITED(globstatus) || WEXITSTATUS(globstatus)>0)
    MOM_FAILURE("MomDumper::close_and_dump_databases in " << _du_dirname << " failed to dump global");
  if (waitpid(userpid, &userstatus, 0) < 0 || !WIFEXITED(userstatus) || WEXITSTATUS(userstatus)>0)
    MOM_FAILURE("MomDumper::close_and_dump_databases in " << _du_dirname << " failed to dump user");
} // end MomDumper::close_and_dump_databases


bool
MomDumper::rename_file_if_changed(const std::string& srcpath, const std::string& dstpath, bool keepsamesrc)
{
  MOM_DEBUGLOG(dump,"rename_file_if_changed srcpath=" << srcpath << " dstpath=" << dstpath
               << " keepsamesrc=" << (keepsamesrc?"true":"false"));
  FILE*srcf = fopen(srcpath.c_str(), "r");
  if (!srcf)
    MOM_FAILURE("rename_file_if_changed fail to open src " << srcpath
                << " (" << strerror(errno) << ")");
  if (access(dstpath.c_str(), F_OK) && errno==ENOENT)
    {
      fclose(srcf);
      if (::rename(srcpath.c_str(), dstpath.c_str()))
        MOM_FATALOG("rename_file_if_changed failed to rename " << srcpath << " -> " << dstpath
                    << " (" << strerror(errno) << ")");
      return false;
    }
  FILE*dstf = fopen(dstpath.c_str(), "r");
  struct stat srcstat = { };
  struct stat dststat = { };
  if (fstat (fileno (srcf), &srcstat))
    MOM_FATALOG("rename_file_if_changed fstat#" << fileno(srcf) << ":" << srcpath << " failed"
                << " (" << strerror(errno) << ")");
  if (fstat (fileno (dstf), &dststat))
    MOM_FATALOG("rename_file_if_changed fstat#" << fileno(dstf) << ":" << dstpath << " failed"
                << " (" << strerror(errno) << ")");
  bool samefilecontent = srcstat.st_size == dststat.st_size;
  while (samefilecontent)
    {
      int srcc = fgetc (srcf);
      int dstc = fgetc (dstf);
      if (srcc != dstc)
        samefilecontent = false;
      else if (srcc == EOF) break;
    }
  fclose(srcf), srcf=nullptr;
  fclose(dstf), dstf=nullptr;
  if (samefilecontent)
    {
      if (!keepsamesrc)
        remove(srcpath.c_str());
      return true;
    }
  std::string backupath = dstpath + "~";
  (void) ::rename(dstpath.c_str(), backupath.c_str());
  if (::rename(srcpath.c_str(), dstpath.c_str()))
    MOM_FATALOG("rename_file_if_changed " << srcpath
                << " -> " << dstpath << " failed"
                << " (" << strerror(errno) << ")");
  return false;
} // end of MomDumper::rename_file_if_changed

void
MomDumper::rename_temporary_files(void)
{
  MOM_DEBUGLOG(dump,"rename_temporary_files start");
  for (std::string path : _du_tempset)
    {
      std::string tmpath = _du_dirname + "/" + path + _du_tempsuffix;
      MOM_DEBUGLOG(dump, "rename_temporary_files path=" << path << " tmpath=" << path);
      rename_file_if_changed(tmpath, _du_dirname + "/" + path);
    }
  MOM_DEBUGLOG(dump,"rename_temporary_files end");
} // end MomDumper::rename_temporary_files

void
MomDumper::initialize_db(sqlite::database &db)
{
  db << R"!*(
CREATE TABLE IF NOT EXISTS t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim REAL NOT NULL,
  ob_content TEXT NOT NULL,
  ob_paylkind VARCHAR(30) NOT NULL,
  ob_paylinit TEXT NOT NULL,
  ob_paylcontent TEXT NOT NULL)
)!*";
    db << R"!*(
CREATE TABLE IF NOT EXISTS t_names
 (nam_oid VARCHAR(30) NOT NULL UNIQUE,
  nam_str TEXT PRIMARY KEY ASC NOT NULL UNIQUE)
)!*";
    db << R"!*(
CREATE TABLE IF NOT EXISTS t_globdata
 (glob_namestr VARCHAR(80) NOT NULL UNIQUE,
  glob_oid  VARCHAR(30) NOT NULL)
)!*";
    db << "BEGIN TRANSACTION";
  } // end MomDumper::initialize_db




void
MomDumper::scan_predefined(void) {
  _du_state = dus_scan;
  auto predset = MomObject::predefined_set();
  for (MomObject* pob : *predset) {
    add_scanned_object(pob);
  }
  std::lock_guard<std::mutex> gu{_du_mtx};
  _du_predefvset = predset;
} // end MomDumper::scan_predefined




void
MomDumper::scan_globdata(void) {
  auto du = this;
  MomRegisterGlobData::every_globdata([&,du](const std::string&, std::atomic<MomObject*>*pglob) {
      MomObject*pob = pglob->load();
      if (pob)
	du->add_scanned_object(pob);
      return false;
    });
} // end MomDumper::scan_globdata

void
MomDumper::dump_emit_globdata(void) {
  auto du = this;
  std::lock_guard<std::mutex> gu_g{_du_globdbmtx};
  std::lock_guard<std::mutex> gu_u{_du_userdbmtx};
  sqlite::database_binder globstmt = (*_du_globdbp) << "INSERT /*globaldb*/ INTO t_globdata VALUES(?,?)";
  sqlite::database_binder userstmt = (*_du_userdbp) << "INSERT /*userdb*/ INTO t_globdata VALUES(?,?)";
  MOM_DEBUGLOG(dump, "dump_emit_globdata start");
  MomRegisterGlobData::every_globdata([&,du](const std::string&nam, std::atomic<MomObject*>*pglob) {
      MomObject*pob = pglob->load();
      MOM_DEBUGLOG(dump, "dump_emit_globdata nam=" << nam << " pob=" << pob << ", sp#" << (pob?((int)pob->space()):0));
      if (pob && du->is_dumped(pob)) {
	if (pob->space()!=MomSpace::UserSp) {
	  globstmt.reset();
	  globstmt << nam << pob->id().to_string();
	  MOM_DEBUGLOG(dump, "dump_emit_globdata globstmt="<< globstmt.sql());
	  globstmt.execute();
	}
	else {
	  userstmt.reset();
	  userstmt << nam << pob->id().to_string();
	  MOM_DEBUGLOG(dump, "dump_emit_globdata userstmt="<< userstmt.sql());
	  userstmt.execute();
	}
      }
      MOM_DEBUGLOG(dump, "dump_emit_globdata done nam=" << nam << " pob=" << pob);
      return false;
    });
  globstmt.used(true);
  userstmt.used(true);
  MOM_DEBUGLOG(dump, "dump_emit_globdata done");
} // end MomDumper::dump_emit_globdata


void
MomDumper::scan_inside_object(MomObject*pob) {
  MOM_ASSERT(pob != nullptr, "MomDumper::scan_inside_object null pob");
  pob->scan_dump_content(this);
} // end MomDumper::scan_inside_object


void 
MomDumper::dump_scan_thread(MomDumper*du, int ix)
{
  MOM_ASSERT(du != nullptr, "MomDumper::dump_scan_thread null dumper");
  MOM_ASSERT(du->_du_state == dus_scan,
	     "MomDumper::dump_scan_thread bad state#" << (int)du->_du_state);
  MOM_ASSERT(ix>0 && ix<=(int)mom_nb_jobs, "MomDumper::dump_scan_thread bad ix="<< ix);
  bool endedscan = false;
  while (!endedscan) {
    const MomObject* ob1 = nullptr;
    const MomObject* ob2 = nullptr;
    {
      std::lock_guard<std::mutex> gu{du->_du_mtx};
      if (!du->_du_queobj.empty()) {
	ob1 = du->_du_queobj.front();
	du->_du_queobj.pop_front();
      }
      if (!du->_du_queobj.empty()) {
	ob2 = du->_du_queobj.front();
	du->_du_queobj.pop_front();
      }
    }
    if (ob1) {
      ob1->scan_dump_content(du);
      std::atomic_fetch_add(&du->_du_scancount,1UL);
    }
    if (ob2) {
      ob2->scan_dump_content(du);
      std::atomic_fetch_add(&du->_du_scancount,1UL);
    }
    if (!ob1 && !ob2) {
      std::unique_lock<std::mutex> lk(du->_du_mtx);
      du->_du_addedcondvar.wait_for(lk,std::chrono::milliseconds(80));
      endedscan = du->_du_queobj.empty();
    }
  }
} // end dump_scan_thread

void
MomDumper::dump_scan_loop(void) {
  MOM_ASSERT(_du_state == dus_scan, "MomDumper::dump_scan_loop bad state");
  std::vector<std::thread> vecthr(mom_nb_jobs);
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1] = std::thread(dump_scan_thread, this, ix);
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
} // end MomDumper::dump_scan_loop


void
MomDumper::dump_emit_object(MomObject*pob, int thix,momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf)
{
  MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
	     "MomDumper::dump_emit_object bad pob");
  MOM_ASSERT(thix > 0 && thix <= (int)mom_nb_jobs, "MomDumper::dump_emit_object bad thix:" << thix);
  MOM_DEBUGLOG(dump,"dump_emit_object pob=" << pob << " thix=" << thix
	       << " dumpglobf=" << dumpglobf
	       << " dumpuserf=" << dumpuserf);
  bool isglobal = false;
  bool isuser = false;
  std::string contentstr;
  MomObject::PayloadEmission pyem;
  double obmtime=0.0;
  {
    std::shared_lock<std::shared_mutex> gu{pob->_ob_shmtx};
    auto sp = pob->space();
    if (sp == MomSpace::PredefSp || sp == MomSpace::GlobalSp)
      isglobal = true;
    else if (sp == MomSpace::UserSp)
      isuser = true;
    else {
      MOM_BACKTRACELOG("MomDumper::dump_emit_object pob=" << pob
		       << " with strange space#" << (int)sp);
      return;
    }
    obmtime = pob->_ob_mtime;
    {
      std::ostringstream outcontent;
      MomDumpEmitter emitcontent(outcontent, this);
      pob->unsync_emit_dump_content(this, emitcontent);
      outcontent << std::endl;
      contentstr = outcontent.str();
    }
    if (pob->_ob_payl) {
     pob->unsync_emit_dump_payload(this,pyem);
    }
  }
  if (isglobal) {
    (*dumpglobf)(pob,thix,obmtime,contentstr,pyem);
  }
  else if (isuser) {
    (*dumpuserf)(pob,thix,obmtime,contentstr,pyem);
  }
  MOM_DEBUGLOG(dump,"dump_emit_object end pob=" << pob << " thix=" << thix);
} // end MomDumper::dump_emit_object



void
MomDumper::dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec, momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf)
{
  MOM_DEBUGLOG(dump,"dump_emit_thread start ix#" << ix);
  std::sort(pvec->begin(), pvec->end(), MomObjptrLess{});
  for (MomObject*pob : *pvec) {
    MOM_DEBUGLOG(dump,"dump_emit_thread emitting pob=" << pob << " ix#" << ix);
    du->dump_emit_object(pob, ix, dumpglobf,dumpuserf);
    MOM_DEBUGLOG(dump,"dump_emit_thread did emit pob=" << pob << " ix#" << ix);
  }
  MOM_DEBUGLOG(dump,"dump_emit_thread end ix#" << ix);
} // end MomDumper::dump_emit_thread


void
MomDumper::dump_emit_loop(void) {
  MOM_ASSERT(_du_state == dus_scan, "MomDumper::dump_emit_loop bad start state");
  _du_state = dus_emit;
  MOM_DEBUGLOG(dump,"dump_emit_loop start");
  momdumpinsertfunction_t dumpglobf;
  momdumpinsertfunction_t dumpuserf;
  auto globstmt = (*_du_globdbp) << "INSERT /*globaldb*/ INTO t_objects VALUES(?,?,?,?,?,?)";
  MOM_DEBUGLOG(dump,"dump_emit_loop globstmt=" << globstmt.sql());
  dumpglobf = [&] (MomObject*pob,int thix,double mtim,
		   const std::string&contentstr, const MomObject::PayloadEmission& pyem) {
    std::lock_guard<std::mutex> gu(_du_globdbmtx);
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpglobf thix#" << thix << " pob=" << pob << " mtim=" << mtim
		 << " contentstr=" << contentstr
		 << " pyem=(kind:" << pyem.pye_kind << ", init=" << pyem.pye_init
		 << ", content=" << pyem.pye_content << ")"
		 << std::endl << " globstmt=" << globstmt.sql());
    char mtimbuf[40];
    memset(mtimbuf, 0, sizeof(mtimbuf));
    snprintf(mtimbuf, sizeof(mtimbuf), "%.2f", mtim);
    globstmt.reset();
    globstmt << pob->id().to_string() << mtimbuf << contentstr
    << pyem.pye_kind << pyem.pye_init << pyem.pye_content;
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf pob=" << pob << " globstmt=" << globstmt.sql());
    globstmt.execute();
    globstmt.reset();
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpglobf did thix#" << thix << " pob=" << pob);
  };
  auto userstmt = *_du_userdbp  << "INSERT /*userdb*/ INTO t_objects VALUES(?,?,?,?,?,?)";
  MOM_DEBUGLOG(dump,"dump_emit_loop userstmt=" << userstmt.sql());
  dumpuserf = [&] (MomObject*pob,int thix,double mtim,
		   const std::string&contentstr, const MomObject::PayloadEmission& pyem) {
    std::lock_guard<std::mutex> gu(_du_userdbmtx);
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf thix#" << thix << " pob=" << pob << " mtim=" << mtim
		 << " contentstr=" << contentstr
		 << " pyem=(kind:" << pyem.pye_kind << ", init=" << pyem.pye_init
		 << ", content=" << pyem.pye_content << ")"
		 << std::endl << " userstmt=" << userstmt.sql());
    char mtimbuf[40];
    memset(mtimbuf, 0, sizeof(mtimbuf));
    snprintf(mtimbuf, sizeof(mtimbuf), "%.2f", mtim);
    userstmt.reset();
    userstmt << pob->id().to_string() << mtimbuf << contentstr
    << pyem.pye_kind << pyem.pye_init << pyem.pye_content;
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf pob=" << pob << " userstmt=" << userstmt.sql());
    userstmt.execute();
    userstmt.reset();
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf did thix#" << thix << " pob=" << pob);
  };
  std::vector<std::vector<MomObject*>> vecobjob(mom_nb_jobs);
  std::vector<std::thread> vecthr(mom_nb_jobs);
  {
      std::lock_guard<std::mutex> gu{_du_mtx};
      auto nbobj = _du_setobj.size();
      for (unsigned ix=0; ix<mom_nb_jobs; ix++)
	vecobjob[ix].reserve(nbobj/mom_nb_jobs+5);
      unsigned long cnt = 0;
      for (const MomObject*pob : _du_setobj) {
	MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
		   "MomDumper::dump_emit_loop bad pob");
	vecobjob[cnt % mom_nb_jobs].push_back(const_cast<MomObject*>(pob));
	cnt++;
      }
  }
  MOM_DEBUGLOG(dump,"dump_emit_loop before dump_emit_loop");
  for (unsigned ix=1; ix<=mom_nb_jobs; ix++) {
    vecthr[ix-1] = std::thread(dump_emit_thread, this, ix, &vecobjob[ix-1], &dumpglobf, &dumpuserf);
  }
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(50+20*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
  /// we don't want the destructor of statements to run any Sqlite request
  globstmt.used(true);
  userstmt.used(true);
  MOM_DEBUGLOG(dump,"dump_emit_loop done");
} // end MomDumper::dump_emit_loop


MomDumper::~MomDumper() {
  MOM_DEBUGLOG(dump,"~MomDumper " << _du_dirname);
}

void
MomAnyObjSeq::scan_dump(MomDumper*du) const
{
  for (MomObject* pob : *this)
    du->add_scanned_object(pob);
} // end MomAnyObjSeq::scan_dump


void
MomNode::scan_dump(MomDumper*du) const
{
  du->add_scanned_object(conn());
  if (du->is_dumped(conn()))
    for (const MomValue v: *this) {
      v.scan_dump(du);
    }
} // end MomNode::scan_dump

void
MomObject::scan_dump(MomDumper*du) const
{
  MOM_ASSERT(vkind() == MomKind::TagObjectK,
	     "MomObject::scan_dump bad object@" << (const void*)this);
  if (space()==MomSpace::TransientSp) return;
  du->add_scanned_object(this);
} // end MomObject::scan_dump


void
MomObject::scan_dump_content(MomDumper*du) const
{
  std::shared_lock<std::shared_mutex> gu{_ob_shmtx};
  MOM_ASSERT(vkind() == MomKind::TagObjectK,
	     "MomObject::scan_dump_content bad object@" << (const void*)this);
  if (space()==MomSpace::TransientSp) return;
  for (auto &p: _ob_attrs) {
    const MomObject*pobattr = p.first;
    if (!pobattr)
      continue;
    du->add_scanned_object(pobattr);
    if (!du->is_dumped(pobattr))
      continue;
    const MomValue valattr = p.second;
    valattr.scan_dump(du);
  }
  for (const MomValue valcomp : _ob_comps) {
    valcomp.scan_dump(du);
  }
  if (_ob_payl)
    _ob_payl->scan_dump_payl(du);
} // end MomObject::scan_dump_content


void
MomObject::unsync_emit_dump_content(MomDumper*du, MomEmitter&em) const
{
  MOM_ASSERT(vkind() == MomKind::TagObjectK,
	     "MomObject::unsync_emit_dump_content bad object@" << (const void*)this);
  if (space()==MomSpace::TransientSp) return;
  em.emit_newline(0);
  for (auto &p: _ob_attrs) {
    const MomObject*pobattr = p.first;
    if (!pobattr)
      continue;
    if (!du->is_dumped(pobattr))
      continue;
    const MomValue valattr = p.second;
    if (!valattr || valattr.is_transient())
      continue;
    em.out() << "@@ ";
    em.emit_objptr(pobattr);
    em.emit_space(1);
    em.emit_value(valattr);
    em.emit_newline(0);
  }
  for (const MomValue vcomp : _ob_comps) {
    em.out() << "&& ";
    em.emit_value(vcomp);
    em.emit_newline(0);
  }
} // end MomObject::unsync_emit_dump_content

void 
MomObject::unsync_emit_dump_payload(MomDumper*du, MomObject::PayloadEmission&pyem) const
{  
  if (_ob_payl->_py_vtbl->pyv_emitdump) {
    std::ostringstream outinit;
    MomDumpEmitter emitinit(outinit, du);
    std::ostringstream outcontent;
    MomDumpEmitter emitcontent(outcontent, du);
    _ob_payl->_py_vtbl->pyv_emitdump(_ob_payl,const_cast<MomObject*>(this),du,&emitinit,&emitcontent);
    pyem.pye_kind = _ob_payl->_py_vtbl->pyv_name;
    pyem.pye_module = _ob_payl->_py_vtbl->pyv_module;
    outinit<<std::flush;
    pyem.pye_init = outinit.str();
    outcontent<<std::endl;
    pyem.pye_content = outcontent.str();
  }
} // end MomObject::unsync_emit_dump_payload

void
mom_dump_in_directory(const char*dirname)
{
  MomDumper dumper(dirname);
  dumper.open_databases();
  dumper.scan_predefined();
  dumper.scan_globdata();
  dumper.dump_scan_loop();	// multi-threaded
  dumper.dump_emit_loop();	// multi-threaded
  dumper.dump_emit_globdata();
  dumper.close_and_dump_databases(); // forks two sqlite3 processes
  dumper.rename_temporary_files();
  long nbob = dumper.nb_objects();
  double rt = dumper.dump_real_time();
  double cpu = dumper.dump_cpu_time();
  MOM_INFORMPRINTF("dumped %ld objects in directory %s in %.2f real, %.3f cpu seconds",
		   nbob, dirname, rt, cpu);
} // end mom_dump_in_directory
