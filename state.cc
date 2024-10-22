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


// this is to help debugging, we test the pob against this id
MomIdent mom_load_spyid1; //see load_spyid1_breakpoint_at
MomIdent mom_load_spyid2; //see load_spyid2_breakpoint_at


struct MomLoaded_Content_Id_st
{
  std::string ldci_content;
  MomIdent ldci_proxyid;
};

// we don't want to move MomLoader into meltmoni.hh because it uses sqlite::
class MomLoader
{
  std::string _ld_dirname;
  const double _ld_startrealtime, _ld_startcputime;
  bool _ld_sequential;
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
  inline bool load_got_spyid1(MomObject*pob) const
  {
    if (!mom_load_spyid1 || !pob) return false;
    MOM_ASSERT(pob->vkind() == MomKind::TagObjectK, "load_got_spyid1 bad pob");
    return pob->id() == mom_load_spyid1;
  };
  inline bool load_got_spyid2(MomObject*pob) const
  {
    if (!mom_load_spyid2 || !pob) return false;
    MOM_ASSERT(pob->vkind() == MomKind::TagObjectK, "load_got_spyid2 bad pob");
    return pob->id() == mom_load_spyid2;
  };
  void load_spyid1_breakpoint_at(int lin, const char*msg) const;
  void load_spyid2_breakpoint_at(int lin, const char*msg) const;
#define LOAD_SPYID1_BREAKPOINT(Msg) load_spyid1_breakpoint_at(__LINE__, (Msg))
#define LOAD_SPYID2_BREAKPOINT(Msg) load_spyid2_breakpoint_at(__LINE__, (Msg))
  long load_empty_objects_from_db(sqlite::database* pdb, bool user);
  void load_all_globdata(void);
  void load_all_modules(void);
  void load_module_of_id(MomIdent);
  void load_all_objects_content(void);
  void load_all_objects_payload_make(void);
  static void load_all_objects_payload_from_db(MomLoader*,sqlite::database* pdb, bool user);
  void load_all_objects_payload_fill(void);
  void load_object_content(MomObject*pob, int thix, const std::string&strcont);
  void load_object_fill_payload(MomObject*pob, int thix, const std::string&strfill, const MomIdent& proxid);
  void load_cold_globdata(const char*globnam, std::atomic<MomObject*>*pglob);
  static void thread_load_content_objects (MomLoader*ld, int thix, std::deque<MomObject*>*obqu, const std::function<std::string(MomObject*)>&getglobfun,const std::function<std::string(MomObject*)>&getuserfun);
  static void thread_load_fill_payload_objects (MomLoader*ld, int thix, std::deque<MomObject*>*obqu,
      const std::function<MomLoaded_Content_Id_st(MomObject*)>&fillglobfun,
      const std::function<MomLoaded_Content_Id_st(MomObject*)>&filluserfun);
  static void load_touch_objects_from_db(MomLoader*ld, sqlite::database* pdb, bool user);
public:
  MomLoader(const std::string&dirnam);
  static constexpr const bool IS_USER= true;
  static constexpr const bool IS_GLOBAL= false;
  void load(void);
  void set_sequential(bool seq)
  {
    _ld_sequential=seq;
  };
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
    {
      struct tm sqltm= {};
      struct tm dbtm= {};
      localtime_r(&sqlstat.st_mtime, &sqltm);
      localtime_r(&dbstat.st_mtime, &dbtm);
      char sqltibuf[64];
      char dbtibuf[64];
      memset (sqltibuf, 0, sizeof(sqltibuf));
      memset (dbtibuf, 0, sizeof(dbtibuf));
      strftime(sqltibuf, sizeof(sqltibuf), "%c %Z", &sqltm);
      strftime(dbtibuf, sizeof(dbtibuf), "%c %Z", &dbtm);
      MOM_FATALOG("load database " << dbpath << " older ("
                  << dbtibuf << ") than its dump " << sqlpath
                  << " (" << sqltibuf << ")");
    }
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
    if (load_got_spyid1(pob))
      LOAD_SPYID1_BREAKPOINT("load_empty_objects_from_db id1");
    else if (load_got_spyid2(pob))
      LOAD_SPYID2_BREAKPOINT("load_empty_objects_from_db id2");
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
    _ld_sequential(false),
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
  if (!ld->_ld_sequential)
    pthread_setname_np(pthread_self(),user?"motouchuser":"mothouchglob");
  MOM_DEBUGLOG(load,"load_touch_objects_from_db start user=" << (user?"true":"false"));
  std::lock_guard<std::mutex> gu (*(user?(&ld->_ld_mtxuserdb):(&ld->_ld_mtxglobdb)));
  *pdb << (user?"SELECT ob_id, ob_mtim /*user*/ FROM t_objects" : "SELECT ob_id, ob_mtim /*global*/ FROM t_objects")
       >> [&](std::string idstr, double mtim)
  {
    auto id = MomIdent::make_from_cstr(idstr.c_str(),true);
    auto pob = ld->load_find_object_by_id(id);
    if (ld->load_got_spyid1(pob))
      ld->LOAD_SPYID1_BREAKPOINT("load_touch_objects_from_db id1");
    else if (ld->load_got_spyid2(pob))
      ld->LOAD_SPYID2_BREAKPOINT("load_touch_objects_from_db id2");
    if (pob)
      pob->touch(mtim);
  };
  MOM_DEBUGLOG(load,"load_touch_objects_from_db end user=" << (user?"true":"false"));
} // end MomLoader::load_touch_objects_from_db



void
MomLoader::load(void)
{
  long nbglobob = 0;
  long nbuserob = 0;
  if (_ld_sequential)
    {
      MOM_INFORMLOG("loading sequentially from " << _ld_dirname);
      nbglobob
        = load_empty_objects_from_db(_ld_globdbp.get(),IS_GLOBAL);
      nbuserob
        = load_empty_objects_from_db(_ld_userdbp.get(),IS_USER);
    }
  else   // multi-threaded
    {
      MOM_INFORMLOG("loading with " << mom_nb_jobs << " threads from " << _ld_dirname);
      auto globthr = std::thread([&](void)
      {
        pthread_setname_np(pthread_self(), "moloademglob");
        MomAnyVal::enable_allocation();
        nbglobob
          = load_empty_objects_from_db(_ld_globdbp.get(),IS_GLOBAL);
      });
      std::thread userthr;
      if (_ld_userdbp)
        {
          userthr = std::thread([&](void)
          {
            pthread_setname_np(pthread_self(), "moloademuser");
            MomAnyVal::enable_allocation();
            nbuserob
              = load_empty_objects_from_db(_ld_userdbp.get(),IS_USER);
          });
          userthr.join();
        }
      globthr.join();
    }
  load_all_globdata();
  load_all_modules();
  load_all_objects_content();
  load_all_objects_payload_make();
  load_all_objects_payload_fill();
  if (_ld_sequential)
    {
      load_touch_objects_from_db(this,_ld_globdbp.get(),IS_GLOBAL);
      load_touch_objects_from_db(this,_ld_userdbp.get(),IS_USER);
    }
  else   // multithreaded load
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
  snprintf(realtimbuf, sizeof(realtimbuf), "%.3f", mom_elapsed_real_time() - _ld_startrealtime);
  char cpupotbuf[24], realpotbuf[24];
  snprintf(cpupotbuf, sizeof(cpupotbuf), "%.3f",
           ((mom_process_cpu_time() - _ld_startcputime)*1.0e6)
           /(nbglobob+nbuserob));
  snprintf(realpotbuf, sizeof(realpotbuf), "%.3f",
           ((mom_elapsed_real_time() - _ld_startrealtime)*1.0e6)
           /(nbglobob+nbuserob));
  MOM_INFORMLOG((_ld_sequential?"loaded sequentially ":"loaded ") << nbglobob << " global, " << nbuserob
                << " user = "
                << (nbglobob+nbuserob) << " objects from " << _ld_dirname
                << std::endl
                << "... in " << realtimbuf << " real, "  << cputimbuf << " cpu seconds"
                << " = " << realpotbuf << " real, "  << cpupotbuf << " cpu µs/ob" << std::endl);
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
      MOM_DEBUGLOG(load,"load_cold_globdata storing globnam=" << globnam
                   << " globpob=" << globpob);
      pglob->store(globpob);
      if (load_got_spyid1(globpob))
        LOAD_SPYID1_BREAKPOINT("load_cold_globdata id1");
      else if (load_got_spyid2(globpob))
        LOAD_SPYID2_BREAKPOINT("load_cold_globdata id2");
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
MomLoader::load_all_modules(void)
{
  MOM_DEBUGLOG(load,"load_all_modules start");
  int nbmod = 0;
  std::set<MomIdent> modidset;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxglobdb};
    auto globstmt = ((*_ld_globdbp)
                     << "SELECT /*globaldb*/ mod_id FROM t_modules");
    globstmt >> [&](const std::string &idstr)
    {
      nbmod++;
      MOM_DEBUGLOG(load,"load_all_modules #" << nbmod
                   << " global idstr=" << idstr);
      MomIdent modid = MomIdent::make_from_string(idstr);
      if (!modid)
        MOM_FATALOG("load_all_modules invalid global module " << idstr);
      modidset.insert(modid);
    };
  }
  if (_ld_userdbp)
    {
      std::lock_guard<std::mutex> gu{_ld_mtxuserdb};
      auto userstmt = ((*_ld_userdbp)
                       << "SELECT /*userdb*/ mod_id FROM t_modules");
      userstmt >> [&](const std::string &idstr)
      {
        nbmod++;
        MOM_DEBUGLOG(load,"load_all_modules #" << nbmod
                     << " user idstr=" << idstr);
        MomIdent modid = MomIdent::make_from_string(idstr);
        if (!modid)
          MOM_FATALOG("load_all_modules invalid user module " << idstr);
        modidset.insert(modid);
      };
    }
  MOM_ASSERT((int)modidset.size() == nbmod,
             "load_all_modules duplicate module ids");
  for (MomIdent modid : modidset)
    load_module_of_id(modid);
  if (nbmod > 0)
    MOM_INFORMLOG("loaded " << nbmod << " modules");
  MOM_DEBUGLOG(load,"load_all_modules end");
} // end  MomLoader::load_all_modules


void
MomLoader::load_module_of_id(MomIdent modid)
{
  MOM_DEBUGLOG(load, "load_module_of_id start modid=" << modid);
  MOM_ASSERT(modid, "load_module_of_id no modid");
  std::string modidstr = modid.to_string();
  std::string modsopath = _ld_dirname + "/modules/momg" + modidstr + ".so";
  std::string modccpath = _ld_dirname + "/modules/momg" + modidstr + ".cc";
  struct stat modsostat = {};
  struct stat modccstat = {};
  if (stat(modsopath.c_str(), &modsostat))
    MOM_FATALOG("load_module_of_id modid=" << modid << " missing module shared object " << modsopath);
  if (stat(modccpath.c_str(), &modccstat))
    MOM_FATALOG("load_module_of_id modid=" << modid << " missing module source code " << modccpath);
  if (modccstat.st_mtime > modsostat.st_mtime)
    {
      struct tm modcctm = {};
      struct tm modsotm = {};
      localtime_r (&modccstat.st_mtime, &modcctm);
      localtime_r (&modsostat.st_mtime, &modsotm);
      char sotibuf[64];
      char cctibuf[64];
      memset (sotibuf, 0, sizeof(sotibuf));
      memset (cctibuf, 0, sizeof(cctibuf));
      strftime(sotibuf, sizeof(sotibuf), "%c %Z", &modsotm);
      strftime(cctibuf, sizeof(cctibuf), "%c %Z", &modcctm);
      MOM_FATALOG("load_module_of_id module " << modid<< " has its source code " << modccpath
                  << " newer (" << cctibuf << ") that its shared object " << modsopath
                  << " (" << sotibuf << ")");
    }
  void *modlh = dlopen(modsopath.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!modlh)
    MOM_FATALOG("load_module_of_id module " << modid << " dlopen " << modsopath << " failure : "
                << dlerror());
  MOM_DEBUGLOG(load, "load_module_of_id end modid=" << modid);
} // end MomLoader::load_module_of_id

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
    globstmt << pob->id().to_string() >> res;
    if (load_got_spyid1(pob))
      LOAD_SPYID1_BREAKPOINT("load_all_objects_content getglobfun id1");
    else if (load_got_spyid2(pob))
      LOAD_SPYID2_BREAKPOINT("load_all_objects_content getglobfun id2");
    MOM_DEBUGLOG(load,"load_all_objects_content getglobfun pob=" << pob << " res=" << res);
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
        userstmt << pob->id().to_string() >> res;
        if (load_got_spyid1(pob))
          LOAD_SPYID1_BREAKPOINT("load_all_objects_content getuserfun id1");
        else if (load_got_spyid2(pob))
          LOAD_SPYID2_BREAKPOINT("load_all_objects_content getuserfun id2");
        MOM_DEBUGLOG(load,"load_all_objects_content getuserfun pob=" << pob << " res=" << res
                     << std::endl << "... == " << MomShowString(res) << std::endl);
        return res;
      };
    }
  if (_ld_sequential)
    {
      std::deque<MomObject*>theobjque;
      unsigned long obcnt = 0;
      std::lock_guard<std::mutex> gu{_ld_mtxobjmap};
      for (auto p : _ld_objmap)
        {
          MomObject* pob = p.second;
          MOM_ASSERT(pob != nullptr && pob->id() == p.first,
                     "MomLoader::load_all_objects_content bad id");
          theobjque.push_back(pob);
          obcnt++;
          if (load_got_spyid1(pob))
            LOAD_SPYID1_BREAKPOINT("load_all_objects_content sequential id1");
          else if (load_got_spyid2(pob))
            LOAD_SPYID2_BREAKPOINT("load_all_objects_content sequential id2");
          MOM_DEBUGLOG(load,"load_all_objects_content sequential obcnt=" << obcnt << " pob=" << pob);
        };
      thread_load_content_objects(this, 0, &theobjque, getglobfun, getuserfun);
      MOM_DEBUGLOG(load,"load_all_objects_content sequential done obcnt=" << obcnt);
    }
  else     // multi-threaded
    {
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
            if (load_got_spyid1(pob))
              LOAD_SPYID1_BREAKPOINT("load_all_objects_content multithreaded id1");
            else if (load_got_spyid2(pob))
              LOAD_SPYID2_BREAKPOINT("load_all_objects_content multithreaded id2");
            int qix = obcnt % mom_nb_jobs;
            vecobjque[qix].push_back(pob);
            obcnt++;
            MOM_DEBUGLOG(load,"load_all_objects_content obcnt=" << obcnt << " pob=" << pob << " qix#" << qix);
          }
      }
      std::vector<std::thread> vecthr(mom_nb_jobs);
      for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
        {
          vecthr[ix-1] = std::thread(thread_load_content_objects, this, ix, &vecobjque[ix-1], getglobfun, getuserfun);
          MOM_DEBUGLOG(load,
                       "load_all_objects_content thread ix=" << ix
                       << " of id:" << vecthr[ix-1].get_id());
        }
      std::this_thread::yield();
      MOM_DEBUGLOG(load,
                   "load_all_objects_content started contentload mom_nb_jobs="
                   << mom_nb_jobs);
      std::this_thread::sleep_for(std::chrono::milliseconds(50+10*mom_nb_jobs));
      for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
        {
          vecthr[ix-1].join();
          MOM_DEBUGLOG(load,"load_all_objects_content joined ix=" << ix);
        }
      usleep(10000);
    } // end multi-threaded
  globstmt.used(true);
  userstmt.used(true);
  MOM_DEBUGLOG(load,"load_all_objects_content end");
} // end MomLoader::load_all_objects_content

void
MomLoader::load_all_objects_payload_make(void)
{
  MOM_DEBUGLOG(load,"load_all_objects_payload_make start");
  if (_ld_sequential)
    {
      MOM_DEBUGLOG(load,"load_all_objects_payload_make sequential starting");
      load_all_objects_payload_from_db(this, _ld_globdbp.get(),  IS_GLOBAL);
      MOM_DEBUGLOG(load,"load_all_objects_payload_make sequential done global");
      load_all_objects_payload_from_db(this, _ld_userdbp.get(),  IS_USER);
    }
  else    // multithreaded
    {
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
    }
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
    if (ld->load_got_spyid1(pob))
      ld->LOAD_SPYID1_BREAKPOINT("load_all_objects_payload_from_db id1");
    else if (ld->load_got_spyid2(pob))
      ld->LOAD_SPYID2_BREAKPOINT("load_all_objects_payload_from_db id2");
    if (!pob)
      MOM_FAILURE("no object of id:" << id);
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
  sqlite::database_binder globstmt = ((*_ld_globdbp)
                                      << "SELECT /*globaldb*/ ob_paylcontent, ob_paylproxid FROM t_objects WHERE ob_id = ?");
  std::function<MomLoaded_Content_Id_st(MomObject*)> fillglobfun =
    [&](MomObject*pob)
  {
    std::lock_guard<std::mutex> gu(_ld_mtxglobdb);
    MomLoaded_Content_Id_st resld;
    if (load_got_spyid1(pob))
      LOAD_SPYID1_BREAKPOINT("load_all_objects_payload_fill fillglobfun id1");
    else if (load_got_spyid2(pob))
      LOAD_SPYID2_BREAKPOINT("load_all_objects_payload_fill fillglobfun id2");
    globstmt << (pob->id().to_string())
             >> [&](std::string contstr, std::string proxidstr)
    {
      MOM_DEBUGLOG(load,"load_all_objects_payload_fill fillglobfun pob=" << pob << " contstr=" << contstr
                   << std::endl << "... proxidstr=" << proxidstr);
      resld.ldci_content = contstr;
      resld.ldci_proxyid = MomIdent::make_from_string(proxidstr);
    };
    return resld;
  };
  std::function<MomLoaded_Content_Id_st(MomObject*)> filluserfun;
  sqlite::database_binder userstmt = ((*(_ld_userdbp?:_ld_globdbp))
                                      << "SELECT  /*userdb*/ ob_paylcontent, ob_paylproxid FROM t_objects WHERE ob_id = ?");
  if (_ld_userdbp)
    {
      filluserfun =
        [&](MomObject*pob)
      {
        std::lock_guard<std::mutex> gu(_ld_mtxuserdb);
        MomLoaded_Content_Id_st resld;
        if (load_got_spyid1(pob))
          LOAD_SPYID1_BREAKPOINT("load_all_objects_payload_fill filluserfun id1");
        else if (load_got_spyid2(pob))
          LOAD_SPYID2_BREAKPOINT("load_all_objects_payload_fill filluserfun id2");
        userstmt << pob->id().to_string()
                 >> [&](std::string contstr, std::string proxidstr)
        {
          MOM_DEBUGLOG(load,"load_all_objects_payload_fill filluserfun pob=" << pob << " contstr=" << contstr
                       << std::endl << "... proxidstr=" << proxidstr);
          resld.ldci_content = contstr;
          resld.ldci_proxyid = MomIdent::make_from_string(proxidstr);
        };
        return resld;
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
        if (load_got_spyid1(pob))
          LOAD_SPYID1_BREAKPOINT("load_all_objects_payload_fill vector id1");
        else if (load_got_spyid2(pob))
          LOAD_SPYID2_BREAKPOINT("load_all_objects_payload_fill vector id2");
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
    const std::function<MomLoaded_Content_Id_st(MomObject*)>&fillglobfun,
    const std::function<MomLoaded_Content_Id_st(MomObject*)>&filluserfun)
{
  if (!ld->_ld_sequential)
    {
      char buf[24];
      snprintf(buf, sizeof(buf), "molopayl%d", thix);
      pthread_setname_np(pthread_self(), buf);
      MomAnyVal::enable_allocation();
    }
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
      if (ld->load_got_spyid1(pob))
        ld->LOAD_SPYID1_BREAKPOINT("thread_load_fill_payload_objects id1");
      else if (ld->load_got_spyid2(pob))
        ld->LOAD_SPYID2_BREAKPOINT("thread_load_fill_payload_objects id2");
      MomLoaded_Content_Id_st contidfill;
      if (pob->space() == MomSpace::UserSp)
        contidfill = filluserfun(pob);
      else
        contidfill = fillglobfun(pob);
      MOM_DEBUGLOG(load,"thread_load_fill_payload_objects thix=#" << thix << " pob=" << pob
                   << " contidfill cont=" << contidfill.ldci_content << std::endl
                   << ".. proxyid=" << contidfill.ldci_proxyid);
      ld->load_object_fill_payload(pob, thix, contidfill.ldci_content, contidfill.ldci_proxyid);
    };
  MOM_DEBUGLOG(load,"thread_load_fill_payload_objects done thix=#" << thix << std::endl);
} // end MomLoader::thread_load_content_objects


void MomLoader::load_object_fill_payload(MomObject*pob, int thix, const std::string&strfill, const MomIdent&proxid)
{
  MOM_DEBUGLOG(load,"load_object_fill_payload start pob=" << pob << " thix=" << thix
               << " strfill=" << strfill);
  if (load_got_spyid1(pob))
    LOAD_SPYID1_BREAKPOINT("load_object_fill_payload id1");
  else if (load_got_spyid2(pob))
    LOAD_SPYID2_BREAKPOINT("load_object_fill_payload id2");
  auto py = pob->_ob_payl;
  py->_py_vtbl->pyv_loadfill(py, pob, this, strfill.c_str());
  MomObject* proxob = load_find_object_by_id(proxid);
  if (proxob)
    py->set_proxy(proxob);
  MOM_DEBUGLOG(load,"load_object_fill_payload end pob=" << MomShowObject(pob)
               << " thix=" << thix
               << " strfill=" << strfill
               << " proxob=" << MomShowObject(proxob));
} // end MomLoader::load_object_fill_payload


void
MomLoader::thread_load_content_objects(MomLoader*ld, int thix, std::deque<MomObject*>*obpqu,
                                       const std::function<std::string(MomObject*)>&getglobfun,
                                       const std::function<std::string(MomObject*)>&getuserfun)
{
  if (!ld->_ld_sequential)
    {
      char buf[24];
      snprintf(buf, sizeof(buf), "moldcont%d", thix);
      pthread_setname_np(pthread_self(),buf);
    }
  MomAnyVal::enable_allocation();
  MOM_ASSERT(ld != nullptr,
             "MomLoader::thread_load_content_objects null ld");
  MOM_ASSERT((thix>0 && thix<=(int)mom_nb_jobs) || ld->_ld_sequential,
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
      if (ld->load_got_spyid1(pob))
        ld->LOAD_SPYID1_BREAKPOINT("thread_load_content_objects id1");
      else if (ld->load_got_spyid2(pob))
        ld->LOAD_SPYID2_BREAKPOINT("thread_load_content_objects id2");
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
  MOM_DEBUGLOG(load,"load_object_content start pob=" << pob
               << " thix=" << thix << " strcont=" << strcont << std::endl);
  std::istringstream incont(strcont);
  MomParser contpars(incont);
  char title[80];
  memset(title,0,sizeof(title));
  char idbuf[32];
  memset(idbuf, 0, sizeof(idbuf));
  pob->id().to_cbuf32(idbuf);
  if (load_got_spyid1(pob))
    LOAD_SPYID1_BREAKPOINT("load_object_content id1");
  else if (load_got_spyid2(pob))
    LOAD_SPYID2_BREAKPOINT("load_object_content id2");
  snprintf(title, sizeof(title), "*content %s*", idbuf);
  contpars.set_name(std::string{title}).set_make_from_id(true);
  contpars.next_line();
  std::lock_guard<std::recursive_mutex> gu{pob->get_recursive_mutex()};
  int nbcomp = 0;
  int nbattr = 0;
  MOM_ASSERT((thix>0 && thix<=(int)mom_nb_jobs) || _ld_sequential,
             "MomLoader::load_object_content bad thix#" << thix);
  /// we might use parse_command in a loop, but it probably is not worth the effort
  for (;;)
    {
      contpars.skip_spaces();
      if (contpars.eof())
        break;
      MOM_DEBUGLOG(load,"load_object_content pob=" << pob
                   << ", contpars@"<< contpars.location_str()
                   << ": " << MomShowString(contpars.curbytes())
                   << std::endl);
      if (contpars.got_cstring("@:"))
        {
          MOM_DEBUGLOG(load,"load_object_content attr pob=" << pob
                       << ", contpars@"<< contpars.location_str());
          bool gotattr = false;
          MomObject*pobattr = contpars.parse_objptr(&gotattr);
          MOM_DEBUGLOG(load,"load_object_content attr pob=" << pob
                       << " pobattr=" << pobattr);
          if (load_got_spyid1(pobattr))
            LOAD_SPYID1_BREAKPOINT("load_object_content pobattr id1");
          else if (load_got_spyid2(pobattr))
            LOAD_SPYID2_BREAKPOINT("load_object_content pobattr id2");
          if (!pobattr || !gotattr)
            MOM_PARSE_FAILURE(&contpars, "missing attribute after @: "
                              << MomShowString(contpars.curbytes()));
          MOM_DEBUGLOG(load,"load_object_content attr pob=" << pob << " pobattr=" << pobattr
                       << " before parse_value");
          bool gotval = false;
          MomValue valattr = contpars.parse_value(&gotval);
          MOM_DEBUGLOG(load,"load_object_content attr pob=" << pob
                       << " pobattr=" << pobattr
                       << " valattr=" << valattr);
          if (!gotval)
            MOM_PARSE_FAILURE(&contpars, "missing value for attribute " << pobattr);
          pob->unsync_put_phys_attr(pobattr, valattr);
          nbattr++;
          MOM_ASSERT(nbattr == (int) pob->unsync_nb_phys_attrs(),
                     "load_object_content nbattr mismatch pob=" << pob);
          MOM_DEBUGLOG(load, "load_object_content attr#" << nbattr
                       <<" pob=" << pob <<
                       " adding pobattr=" << pobattr << " valattr=" << valattr);
        }
      else if (contpars.got_cstring("&:"))
        {
          bool gotcomp = false;
          MomValue valcomp = contpars.parse_value(&gotcomp);
          if (!gotcomp)
            MOM_PARSE_FAILURE(&contpars, "missing component#" << nbcomp << " after &: "
                              << MomShowString(contpars.curbytes()));
          pob->unsync_append_comp(valcomp);
          nbcomp++;
          MOM_DEBUGLOG(load,"load_object_content comp#" << nbcomp
                       <<" pob=" << pob <<
                       " adding valcomp=" << valcomp);
        }
      else if (contpars.got_cstring("@MAGIC!"))
        {
          MOM_DEBUGLOG(load,"load_object_content magic pob=" << pob);
          pob->set_magic(true);
        }
      else
        MOM_PARSE_FAILURE(&contpars, "unexpected state content for pob=" << pob
                          << ": " << MomShowString(contpars.curbytes())
                          << " @" << contpars.location_str());
    }
  MOM_DEBUGLOG(load,"load_object_content end pob=" << pob << " thix=" << thix
               << " nbattr=" << nbattr << " nbcomp=" << nbcomp
               << " final contpars@" << contpars.location_str()
               << " remaining: " << MomShowString(contpars.curbytes())
               << std::endl);
} // end MomLoader::load_object_content


void
MomLoader::load_spyid1_breakpoint_at(int lin, const char*msg) const
{
  MOM_INFORMLOG("load_spyid1_breakpoint:" << msg
                << MomShowBacktraceAt(__FILE__,lin,"load_spyid1_breakpoint"));
  fflush(nullptr);
} // end MomLoader::load_spyid1_breakpoint_at

void
MomLoader::load_spyid2_breakpoint_at(int lin, const char*msg) const
{
  MOM_INFORMLOG("load_spyid2_breakpoint:" << msg
                << MomShowBacktraceAt(__FILE__,lin,"load_spyid2_breakpoint"));
  fflush(nullptr);
} // end MomLoader::load_spyid2_breakpoint_at

void
mom_load_from_directory(const char*dirname)
{
  MomLoader ld(dirname);
  MomAnyVal::enable_allocation();
  ld.load();
} // end mom_load_from_directory


void
mom_load_sequential_from_directory(const char*dirname)
{
  MomLoader ld(dirname);
  ld.set_sequential(true);
  MomAnyVal::enable_allocation();
  ld.load();
} // end mom_load_from_directory


//==============================================================
////////////////////////////////////////////////////////////////

std::string mom_random_temporary_file_suffix(void)
{
  char tempbuf[64];
  memset(tempbuf, 0, sizeof(tempbuf));
  auto rs = MomSerial63::make_random();
  char sbuf16[16];
  memset(sbuf16, 0, sizeof(sbuf16));
  rs.to_cbuf16(sbuf16);
  sbuf16[0] = '+';
  snprintf(tempbuf, sizeof(tempbuf), "%s_p%d_~", sbuf16, (int)getpid());
  return std::string{tempbuf};
} // end mom_random_temporary_suffix

// we don't want to move MomDumper into meltmoni.hh because it uses sqlite::
enum MomDumpState { dus_none, dus_scan, dus_emit };


////////////////////////////////////////////////////////////////
typedef std::function<void(MomObject*pob,int thix,double mtim,const std::string&contentstr,const MomObject::PayloadEmission& pyem)> momdumpinsertfunction_t;

class MomDumper
{
  friend void mom_dump_named_update_defer(MomDumper*du, MomObject*pob, std::string nam);
  friend void MomObject::scan_dump_module_for(MomDumper*du, const void*addr);
  std::mutex _du_mtx;
  const double _du_startrealtime;
  const double _du_startcputime;
  MomDumpState _du_state;
  std::string _du_dirname;
  std::string _du_tempsuffix;
  std::set<std::string> _du_tempset;
  std::deque<std::function<void(MomDumper*)>> _du_todoscanq;
  std::deque<std::function<void(MomDumper*)>> _du_todoemitq;
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
  std::set<MomIdent> _du_globmoduleidset;
  std::set<MomIdent> _du_usermoduleidset;
  unsigned _du_scanfunh;
  void initialize_db(sqlite::database &db, bool isglobal);
  static void dump_scan_thread(MomDumper*du, int ix);
  static void dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec, momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf);
  void dump_emit_object(MomObject*pob, int thix,momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf);
  std::function<void(MomDumper*)> pop_locked_todo_emit(void);
  void scan_for_gc(MomGC*);
public:
  void add_user_module_id(MomIdent);
  void add_glob_module_id(MomIdent);
  void todo_scan(std::function<void(MomDumper*)> todoscanfun);
  void todo_emit(std::function<void(MomDumper*)> todoemitfun);
  std::string temporary_file_path(const std::string& path);
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
  void dump_emit_modules(void);
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
    _du_todoscanq(), _du_todoemitq(),
    _du_globdbp(), _du_userdbp(),
    _du_globdbmtx(), _du_userdbmtx(),
    _du_addedcondvar(),
    _du_setobj(), _du_queobj(), _du_scancount(ATOMIC_VAR_INIT(0ul)),
    _du_predefvset(nullptr), _du_globmap{},
    _du_globmoduleidset(), _du_usermoduleidset(),
    _du_scanfunh(0)
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
  _du_tempsuffix = mom_random_temporary_file_suffix();
  auto thisdump = this;
  _du_scanfunh =
    MomGC::the_garbcoll.add_scan_function
    ([=](MomGC* thisgc)
  {
    thisdump->scan_for_gc(thisgc);
  });
} // end MomDumper::MomDumper

void
MomDumper::add_user_module_id(MomIdent modid)
{
  MOM_ASSERT(modid, "add_user_module_id bad modid");
  std::lock_guard<std::mutex> gu(_du_mtx);
  _du_usermoduleidset.insert(modid);
} // end MomDumper::add_user_module_id

void
MomDumper::add_glob_module_id(MomIdent modid)
{
  MOM_ASSERT(modid, "add_glob_module_id bad modid");
  std::lock_guard<std::mutex> gu(_du_mtx);
  _du_globmoduleidset.insert(modid);
} // end MomDumper::add_glob_module_id

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
  dbconfig.flags = sqlite::OpenFlags::CREATE | sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::NOMUTEX;
  dbconfig.encoding = sqlite::Encoding::UTF8;
  MOM_DEBUGLOG(dump, "open_databases initializing global globdbpath=" << globdbpath);
  _du_globdbp = std::make_unique<sqlite::database>(globdbpath, dbconfig);
  initialize_db(*_du_globdbp, true);
  MOM_DEBUGLOG(dump, "open_databases initializing user userdbpath=" << userdbpath);
  _du_userdbp = std::make_unique<sqlite::database>(userdbpath, dbconfig);
  initialize_db(*_du_userdbp, false);
  MOM_DEBUGLOG(dump, "open_databases done");
} // end MomDumper::open_databases


pid_t
MomDumper::fork_dump_database(const std::string&dbpath, const std::string&sqlpath, const std::string& basepath)
{
  std::string refpath;
  if (mom_load_dir && strcmp(mom_load_dir, "-"))
    {
      char *realloaddir = realpath(mom_load_dir, nullptr);
      if (!realloaddir)
        MOM_FATALOG("fork_dump_database: realpath " << mom_load_dir << " failure");
      refpath = std::string{realloaddir}+ "/" + basepath + ".sql";
      free (realloaddir);
    }
  else
    refpath = basepath + ".sql";
  MOM_DEBUGLOG(dump, "fork_dump_database start dbpath=" << dbpath << " sqlpath=" << sqlpath
               << " basepath=" << basepath << " refpath=" << refpath
               << MOM_SHOW_BACKTRACE("fork_dump_database"));
  std::string dumpshellscript = std::string{monimelt_directory} + '/' + "monimelt-dump-state.sh";
  pid_t p = fork();
  if (p==0)
    {
      close(STDIN_FILENO);
      int nfd = open("/dev/null", O_RDONLY);
      if (nfd>0) dup2(nfd, STDIN_FILENO);
      for (int ix=3; ix<64; ix++)
        (void) close(ix);
      nice(1);
      for (int sig=1; sig<SIGRTMIN; sig++) signal(sig, SIG_DFL);
      execlp(dumpshellscript.c_str(), dumpshellscript.c_str(),
             dbpath.c_str(), sqlpath.c_str(), basepath.c_str(),
             refpath.c_str(), nullptr);
      perror((dumpshellscript + " execlp").c_str());
      _exit(EXIT_FAILURE);
    }
  else if (p<0)
    MOM_FATALOG("MomDumper::fork_dump_database failed to fork for " << basepath
                << " (" << strerror(errno) << ")");
  return p;
} // end MomDumper::fork_dump_database


void
MomDumper::close_and_dump_databases(void)
{
  auto globdbpath = temporary_file_path(MOM_GLOBAL_DB ".sqlite");
  auto userdbpath = temporary_file_path(MOM_USER_DB ".sqlite");
  auto globsqlpath = temporary_file_path(MOM_GLOBAL_DB ".sql");
  auto usersqlpath = temporary_file_path(MOM_USER_DB ".sql");
  *_du_globdbp << "END TRANSACTION /*global*/"; /* ending the mega transaction from initialize_db */
  if (_du_userdbp)
    *_du_userdbp << "END TRANSACTION /*user*/"; /* ending the mega transaction from initialize_db */
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

void MomPayload::scan_dump_module_payl(MomDumper*du) const
{
  MomIdent modid = MomIdent::make_from_cstr(_py_vtbl->pyv_module);
  MomObject* modpob = modid?MomObject::find_object_of_id(modid):nullptr;
  if (modpob)
    {
      modpob->scan_dump(du);
      if (du->is_dumped(modpob))
        {
          if (owner()->space() == MomSpace::UserSp)
            du->add_user_module_id(modpob->id());
          else
            du->add_glob_module_id(modpob->id());
        }
    }
} // end MomPayload::scan_dump_module_payl

void
MomDumper::todo_scan(std::function<void(MomDumper*)> todoscanfun)
{
  if (_du_state != dus_scan)
    MOM_FAILURE("dumper " << _du_dirname << " todo_scan in wrong dump state");
  if (!todoscanfun)
    MOM_FAILURE("dumper " << _du_dirname << " todo_scan empty function");
  {
    std::lock_guard<std::mutex> gu{_du_mtx};
    _du_todoscanq.push_back(todoscanfun);
  }
  _du_addedcondvar.notify_all();
} // end MomDumper::todo_scan

void
MomDumper::todo_emit(std::function<void(MomDumper*)> todoemitfun)
{
  if (_du_state != dus_emit && _du_state != dus_scan)
    MOM_FAILURE("dumper " << _du_dirname << " todo_emit in wrong dump state");
  {
    if (!todoemitfun)
      MOM_FAILURE("dumper " << _du_dirname << " todo_emit empty function");
    std::lock_guard<std::mutex> gu{_du_mtx};
    _du_todoemitq.push_back(todoemitfun);
  }
  _du_addedcondvar.notify_all();
} // end MomDumper::todo_emit

void
mom_dump_todo_scan(MomDumper*du, std::function<void(MomDumper*)> todofun)
{
  if (!du)
    MOM_FAILURE("mom_dump_todo_scan no dumper");
  du->todo_scan(todofun);
} // end mom_dump_todo_scan

void
mom_dump_todo_emit(MomDumper*du, std::function<void(MomDumper*)> todofun)
{
  if (!du)
    MOM_FAILURE("mom_dump_todo_emit no dumper");
  du->todo_emit(todofun);
} // end mom_dump_todo_emit

void mom_dump_named_update_defer(MomDumper*du, MomObject*pob, std::string nam)
{
  if (!du)
    MOM_FAILURE("mom_dump_named_update_defer no dumper");
  if (!pob || pob->vkind() != MomKind::TagObjectK)
    MOM_FAILURE("mom_dump_named_update_defer bad pob");
  if (nam.empty())
    MOM_FAILURE("mom_dump_named_update_defer empty name for pob=" << pob);
  // we don't bother having a prepared statement for update of t_names table
  bool user = pob->space() == MomSpace::UserSp;
  if (user)
    du->todo_emit([=](MomDumper*dumper)
    {
      std::lock_guard<std::mutex> gu_u{dumper->_du_userdbmtx};
      *dumper->_du_userdbp << "INSERT /*username*/ INTO t_names VALUES(?,?)"
                           << pob->id().to_string() << nam;
    });
  else
    du->todo_emit([=](MomDumper*dumper)
    {
      std::lock_guard<std::mutex> gu_g{dumper->_du_globdbmtx};
      *dumper->_du_globdbp << "INSERT /*globname*/ INTO t_names VALUES(?,?)"
                           << pob->id().to_string() << nam;
    });

} // end  mom_dump_named_update_defer


bool
mom_rename_file_if_changed(const std::string& srcpath, const std::string& dstpath, bool keepsamesrc)
{
  MOM_DEBUGLOG(dump,"rename_file_if_changed srcpath=" << srcpath << " dstpath=" << dstpath
               << " keepsamesrc=" << (keepsamesrc?"true":"false"));
  FILE*srcf = fopen(srcpath.c_str(), "r");
  if (!srcf)
    {
      std::string openerrmsg = strerror(errno);
      MOM_DEBUGLOG(dump,"rename_file_if_changed failedopen srcpath=" << srcpath << " dstpath=" << dstpath
                   << " keepsamesrc=" << (keepsamesrc?"true":"false")
                   << ":: " << openerrmsg
                   << MOM_SHOW_BACKTRACE("rename_file_if_changed failedopen"));
      MOM_FAILURE("rename_file_if_changed fail to open src " << srcpath
                  << " (" << openerrmsg << ")");
    }
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
      mom_rename_file_if_changed(tmpath, _du_dirname + "/" + path, true);
    }
  MOM_DEBUGLOG(dump,"rename_temporary_files end");
} // end MomDumper::rename_temporary_files

void
MomDumper::initialize_db(sqlite::database &db, bool isglobal)
{
  /// keep this in sync with monimelt-dump-state.sh script
  /// and with dumpsqlmonimelt.cc file
  db << R"!*(
CREATE TABLE IF NOT EXISTS t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim REAL NOT NULL,
  ob_content TEXT NOT NULL,
  ob_paylkind VARCHAR(30) NOT NULL,
  ob_paylinit TEXT NOT NULL,
  ob_paylcontent TEXT NOT NULL,
  ob_paylproxid VARCHAR(30) NOT NULL)
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
    db << R"!*(
CREATE TABLE IF NOT EXISTS t_modules
 (mod_id VARCHAR(30) NOT NULL UNIQUE)
)!*";
    db << (isglobal ? "BEGIN TRANSACTION /*global*/" : "BEGIN TRANSACTION /*user*/");
    // the corresponding end of transaction is in close_and_dump_databases
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
  MOM_DEBUGLOG(dump, "scan_globdata start");
  MomRegisterGlobData::do_each_globdata
    ([&,du]
     (const std::string&, std::atomic<MomObject*>*pglob) {
      MomObject*pob = pglob->load();
      MOM_DEBUGLOG(dump, "scan_globdata pob=" << pob);
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
  MomRegisterGlobData::do_each_globdata
    ([&,du](const std::string&nam, std::atomic<MomObject*>*pglob) {
      MomObject*pob = pglob->load();
      MOM_DEBUGLOG(dump, "dump_emit_globdata nam=" << nam << " pob=" << pob << ", sp#" << (pob?((int)pob->space()):0));
      if (pob  && pob->space() != MomSpace::TransientSp
	  && du->is_dumped(pob)) {
	if (pob->space()!=MomSpace::UserSp) {
	  MOM_DEBUGLOG(dump, "dump_emit_globdata global globdata "
		       << " nam=" << nam << " pob=" << pob);
	  globstmt << nam << pob->id().to_string();
	  MOM_DEBUGLOG(dump, "dump_emit_globdata globstmt="<< globstmt.sql());
	  globstmt.execute();
	}
	else {
	  MOM_DEBUGLOG(dump, "dump_emit_globdata user globdata "
		       << " nam=" << nam << " pob=" << pob);
	  globstmt << nam << pob->id().to_string();
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
  MOM_DEBUGLOG(dump, "dump_emit_globdata end");
} // end MomDumper::dump_emit_globdata


void
MomDumper::dump_emit_modules(void) {
  MOM_DEBUGLOG(dump, "dump_emit_modules start;" << MOM_SHOW_BACKTRACE("dump_emit_modules"));
  auto du = this;
  std::lock_guard<std::mutex> gu_d{_du_mtx};
  MOM_DEBUGLOG(dump, "dump_emit_modules " << du->_du_globmoduleidset.size() << " global modules");
  {
    std::lock_guard<std::mutex> gu_g{_du_globdbmtx};
    sqlite::database_binder globstmt = (*_du_globdbp) << "INSERT /*globaldb*/ INTO t_modules VALUES(?)";
    for (auto gmodid: du->_du_globmoduleidset) {
      MOM_DEBUGLOG(dump, "dump_emit_modules glob gmodid=" << gmodid);
      globstmt << gmodid.to_string();
    }
    globstmt.used(true);
  }
  MOM_DEBUGLOG(dump, "dump_emit_modules after globals");
  {
    MOM_DEBUGLOG(dump, "dump_emit_modules " << du->_du_usermoduleidset.size() << " user modules");
    std::lock_guard<std::mutex> gu_u{_du_userdbmtx};
    sqlite::database_binder userstmt = (*_du_userdbp) << "INSERT /*userdb*/ INTO t_modules VALUES(?)";
    for (auto umodid: du->_du_usermoduleidset) {
      MOM_DEBUGLOG(dump, "dump_emit_modules user modid=" << umodid);
      userstmt << umodid.to_string();
    }
    userstmt.used(true);
  }
  MOM_DEBUGLOG(dump, "dump_emit_modules end");
} // end MomDumper::dump_emit_modules



void
MomDumper::scan_inside_object(MomObject*pob) {
  MOM_ASSERT(pob != nullptr, "MomDumper::scan_inside_object null pob");
  pob->scan_dump_content(this);
} // end MomDumper::scan_inside_object


void 
MomDumper::dump_scan_thread(MomDumper*du, int ix)
{
  if (ix>0) {
    char buf[16];
    snprintf(buf, sizeof(buf), "moduscan%d", ix);
    pthread_setname_np(pthread_self(),buf);
    MomAnyVal::enable_allocation();
  }
  MOM_ASSERT(du != nullptr, "MomDumper::dump_scan_thread null dumper");
  MOM_ASSERT(du->_du_state == dus_scan,
	     "MomDumper::dump_scan_thread bad state#" << (int)du->_du_state);
  MOM_ASSERT(ix>=0 && ix<=(int)mom_nb_jobs, "MomDumper::dump_scan_thread bad ix="<< ix);
  bool endedscan = false;
  while (!endedscan) {
    const MomObject* ob1 = nullptr;
    const MomObject* ob2 = nullptr;
    endedscan = true;
    std::function<void(MomDumper*)> todofun;
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
      if (!du->_du_todoscanq.empty()) {
	todofun = du->_du_todoscanq.front();
	du->_du_todoscanq.pop_front();
      }
    }
    if (ob1) {
      ob1->scan_dump_content(du);
      std::atomic_fetch_add(&du->_du_scancount,1UL);
      endedscan = false;
    }
    if (ob2) {
      ob2->scan_dump_content(du);
      std::atomic_fetch_add(&du->_du_scancount,1UL);
      endedscan = false;
    }
    if (todofun)
      todofun(du);
    if (endedscan) {
      std::unique_lock<std::mutex> lk(du->_du_mtx);
      du->_du_addedcondvar.wait_for(lk,std::chrono::milliseconds(80));
      endedscan = du->_du_queobj.empty() && du->_du_todoscanq.empty();
    }
  }
} // end MomDumper::dump_scan_thread




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
    std::lock_guard<std::recursive_mutex> gu{pob->_ob_recmtx};
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
    obmtime = pob->mtime();
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


std::function<void(MomDumper*)>
MomDumper::pop_locked_todo_emit(void)
{
  MOM_ASSERT(_du_state == dus_emit, "bad state in pop_locked_todo_emit");
  std::function<void(MomDumper*)> todofun;
  {
    std::lock_guard<std::mutex> gu{_du_mtx};
    if (!_du_todoemitq.empty()) {
      todofun =_du_todoemitq.front();
      _du_todoemitq.pop_front();
    }
  }
  return todofun;
} // end  MomDumper::pop_locked_todo_emit

void
MomDumper::dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec, momdumpinsertfunction_t* dumpglobf, momdumpinsertfunction_t* dumpuserf)
{
  if (ix>0) {
    char buf[16];
    snprintf(buf, sizeof(buf), "moduemit%d", ix);
    pthread_setname_np(pthread_self(),buf);
    MomAnyVal::enable_allocation();
  }
  MOM_DEBUGLOG(dump,"dump_emit_thread start ix#" << ix);
  std::sort(pvec->begin(), pvec->end(), MomObjptrLess{});
  unsigned long todocount=0;
  std::function<void(MomDumper*)> todofun;
  for (MomObject*pob : *pvec) {
    MOM_DEBUGLOG(dump,"dump_emit_thread emitting pob=" << pob << " ix#" << ix);
    du->dump_emit_object(pob, ix, dumpglobf,dumpuserf);
    while ((todofun=du->pop_locked_todo_emit())) {
      todocount++;
      MOM_DEBUGLOG(dump,"dump_emit_thread before todo#" << todocount << " ix#" << ix);
      todofun(du);
    }
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
  auto globstmt = (*_du_globdbp) << "INSERT /*globaldb*/ INTO t_objects VALUES(?,?,?,?,?,?,?)";
  MOM_DEBUGLOG(dump,"dump_emit_loop globstmt=" << globstmt.sql());
  dumpglobf = [&] (MomObject*pob,int thix,double mtim,
		   const std::string&contentstr, const MomObject::PayloadEmission& pyem) {
    std::lock_guard<std::mutex> gu(_du_globdbmtx);
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpglobf thix#" << thix << " pob=" << pob << " mtim=" << mtim
		 << " contentstr=" << contentstr
		 << " pyem=(kind:" << pyem.pye_kind << ", init=" << pyem.pye_init
		 << ", content=" << pyem.pye_content
		 << ", proxyid=" << pyem.pye_proxyid << ")"
		 << std::endl << " globstmt=" << globstmt.sql());
    char mtimbuf[40];
    memset(mtimbuf, 0, sizeof(mtimbuf));
    snprintf(mtimbuf, sizeof(mtimbuf), "%.2f", mtim);
    globstmt << pob->id().to_string() << mtimbuf << contentstr
	     << pyem.pye_kind << pyem.pye_init << pyem.pye_content << pyem.pye_proxyid;
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf pob=" << pob << " globstmt=" << globstmt.sql());
    globstmt.execute();
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpglobf did thix#" << thix << " pob=" << pob);
  };
  auto userstmt = *_du_userdbp  << "INSERT /*userdb*/ INTO t_objects VALUES(?,?,?,?,?,?,?)";
  MOM_DEBUGLOG(dump,"dump_emit_loop userstmt=" << userstmt.sql());
  dumpuserf = [&] (MomObject*pob,int thix,double mtim,
		   const std::string&contentstr, const MomObject::PayloadEmission& pyem) {
    std::lock_guard<std::mutex> gu(_du_userdbmtx);
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf thix#" << thix << " pob=" << pob << " mtim=" << mtim
		 << " contentstr=" << contentstr
		 << " pyem=(kind:" << pyem.pye_kind << ", init=" << pyem.pye_init
		 << ", content=" << pyem.pye_content
		 << ", proxyid=" << pyem.pye_proxyid
		 << ")"
		 << std::endl << " userstmt=" << userstmt.sql());
    char mtimbuf[40];
    memset(mtimbuf, 0, sizeof(mtimbuf));
    snprintf(mtimbuf, sizeof(mtimbuf), "%.2f", mtim);
    userstmt << pob->id().to_string() << mtimbuf << contentstr
    << pyem.pye_kind << pyem.pye_init << pyem.pye_content << pyem.pye_proxyid;
    MOM_DEBUGLOG(dump,"dump_emit_loop dumpuserf pob=" << pob << " userstmt=" << userstmt.sql());
    userstmt.execute();
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

void
MomDumper::scan_for_gc(MomGC*gc) {
    std::lock_guard<std::mutex> gu(_du_mtx);
    for (auto pob: _du_setobj)
      gc->scan_object(const_cast<MomObject*>(pob));
} // end MomDumper::scan_for_gc


MomDumper::~MomDumper() {  
  MOM_DEBUGLOG(dump,"~MomDumper " << _du_dirname);
  std::lock_guard<std::mutex> gu(_du_mtx);
  _du_setobj.clear();
  MomGC::the_garbcoll.remove_scan_handle(_du_scanfunh);
  _du_scanfunh = 0;
} // end of MomDumper::~MomDumper



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
MomObject::scan_dump_module_for(MomDumper*du, const void*addr)
{
  MOM_ASSERT(du && du->_du_state == dus_scan, "MomDumper::scan_dump_module_for not scanning");
  if (!du->is_dumped(this) || space() == MomSpace::TransientSp || !addr)
    return;
  bool glob = (space() == MomSpace::GlobalSp);
  Dl_info dlinf = {};
  if (dladdr(addr, &dlinf)) {
    MOM_DEBUGLOG(dump, "scan_dump_module_for this=" << MomShowObject(this) << " addr=" << addr
		 << " dli_fname=" << (dlinf.dli_fname?:"*null*")
		 << " dli_sname=" << (dlinf.dli_sname?:"*null*"));
    if (dlinf.dli_fname) {
      const char*obf = strstr(dlinf.dli_fname, "/momg_");
      if (obf)
	obf++;
      else obf = strstr(dlinf.dli_fname, "momg_");
      if (!obf) return;
      MomIdent modid = MomIdent::make_from_cstr(obf+strlen("momg_"));
      MomObject* modpob = nullptr;
      if (modid)
	modpob = MomObject::find_object_of_id(modid);
      MOM_DEBUGLOG(dump, "scan_dump_module_for this=" << MomShowObject(this) << " addr=" << addr
		   << " modid=" << modid << " modpob=" << MomShowObject(modpob));
      if (modpob)
	const_cast<const MomObject*>(modpob)->scan_dump(du);
      if (du->is_dumped(modpob)) {
	if (glob) {
	  MOM_DEBUGLOG(dump, "scan_dump_module_for this=" << MomShowObject(this) << " addr=" << addr
		       << " global modpob=" << modpob);
	  du->add_glob_module_id(modpob->id());
	}
	else {
	  MOM_DEBUGLOG(dump, "scan_dump_module_for this=" << MomShowObject(this) << " addr=" << addr
		       << " user modpob=" << modpob);
	  du->add_user_module_id(modpob->id());
	}
      }
      else
	  MOM_DEBUGLOG(dump, "scan_dump_module_for this=" << MomShowObject(this) << " addr=" << addr
		       << " nondumped modpob=" << modpob);
	
    }
  };
} // end MomObject::scan_dump_module_for

void
MomObject::scan_dump_content(MomDumper*du) const
{
  std::lock_guard<std::recursive_mutex> gu{this->_ob_recmtx};
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
  std::string nam = mom_get_unsync_string_name(this);
  if (!nam.empty()) {
    em.out() << "///$" << nam;
    em.emit_newline(0);
  }
  if (is_magic()) {
    em.out() << "@MAGIC!";
    em.emit_newline(0);
  }
  MomObjptrSet setattrs;
  for (auto &p: _ob_attrs) {
    const MomObject*pobattr = p.first;
    if (!pobattr)
      continue;
    if (!du->is_dumped(pobattr))
      continue;
    const MomValue valattr = p.second;
    if (!valattr || valattr.is_transient())
      continue;
    setattrs.insert(pobattr);
  }
  for (auto pobcstattr: setattrs) {
    auto pobattr = const_cast<MomObject*>(pobcstattr);
    if (!du->is_dumped(pobattr))
      continue;
    auto valattr = _ob_attrs.find(pobattr)->second;
    em.out() << "@: ";
    em.emit_objptr(pobcstattr);
    em.emit_space(1);
    em.emit_value(valattr, 0, MomEmitter::_DONTSKIP_VALUE_);
    em.emit_newline(0);
  }
  for (const MomValue vcomp : _ob_comps) {
    em.out() << "&: ";
    em.emit_value(vcomp, 0, MomEmitter::_DONTSKIP_VALUE_);
    em.emit_newline(0);
  }
} // end MomObject::unsync_emit_dump_content

bool mom_dump_is_dumpable_object(MomDumper*du, MomObject*pob)
{
  if (!du) return false;
  if (!pob) return false;
  return du->is_dumped(pob);
} // end  mom_dump_is_dumpable_object

std::string
mom_dump_temporary_file_path(MomDumper*du, const std::string&path)
{
  if (!du) MOM_FAILURE("missing dumper for  mom_dump_temporary_file_path path=" << path);
  return du->temporary_file_path(path);
} // end  mom_dump_temporary_file_path

void 
MomObject::unsync_emit_dump_payload(MomDumper*du, MomObject::PayloadEmission&pyem) const
{
  auto vt = _ob_payl->_py_vtbl;
  MOM_ASSERT(vt != nullptr && vt->pyv_magic == MOM_PAYLOADVTBL_MAGIC && vt->pyv_name,
	     "unsync_emit_dump_payload bad payload for " << this);
  if (vt->pyv_emitdump) {
    MOM_DEBUGLOG(dump, "unsync_emit_dump_payload this=" << this << " /pynam=" << vt->pyv_name);
    std::ostringstream outinit;
    MomDumpEmitter emitinit(outinit, du);
    std::ostringstream outcontent;
    MomDumpEmitter emitcontent(outcontent, du);
    _ob_payl->_py_vtbl->pyv_emitdump(_ob_payl,const_cast<MomObject*>(this),du,&emitinit,&emitcontent);
    pyem.pye_kind = _ob_payl->_py_vtbl->pyv_name;
    if (_ob_payl->_py_vtbl->pyv_module != nullptr) {
      MomIdent modid = MomIdent::make_from_cstr(_ob_payl->_py_vtbl->pyv_module);
      MomObject* modpob = modid?MomObject::find_object_of_id(modid):nullptr;
      if (du->is_dumped(modpob)) 
	pyem.pye_module = modpob->id().to_string();
    }
    outinit.flush();
    pyem.pye_init = outinit.str();
    outcontent.flush();
    pyem.pye_content = outcontent.str();
    MomObject* proxob = _ob_payl->proxy();
    if (proxob && du->is_dumped(proxob)) 
      pyem.pye_proxyid = proxob->id().to_string();
    else
      pyem.pye_proxyid = "";
    MOM_DEBUGLOG(dump, "unsync_emit_dump_payload done this=" <<this
		 << " /pynam=" << vt->pyv_name <<std::endl
		 << ".. init=" << pyem.pye_init <<std::endl
		 << ".. content=" << pyem.pye_content <<std::endl
		 << ".. proxid=" <<  pyem.pye_proxyid <<std::endl);
  }
  else    
    MOM_DEBUGLOG(dump, "unsync_emit_dump_payload nondumpable payload this=" << this
		 << " /pynam=" << vt->pyv_name);
} // end MomObject::unsync_emit_dump_payload

long
mom_dump_in_directory(const char*dirname)
{
  MomDumper dumper(dirname);
  dumper.open_databases();
  dumper.scan_predefined();
  dumper.scan_globdata();
  dumper.dump_scan_loop();	// multi-threaded
  dumper.dump_emit_loop();	// multi-threaded
  dumper.dump_emit_globdata();
  dumper.dump_emit_modules();
  dumper.close_and_dump_databases(); // forks two sqlite3 processes
  dumper.rename_temporary_files();
  long nbob = dumper.nb_objects();
  double rt = dumper.dump_real_time();
  double cpu = dumper.dump_cpu_time();
  MOM_INFORMPRINTF("dumped %ld objects in directory %s in %.3f real, %.4f cpu seconds\n"
		   ".. [%.3f real µs, %.3f cpu µs /ob]",
		   nbob, dirname, rt, cpu, (rt*1.0e6)/nbob, (cpu*1.0e6)/nbob);
  return nbob;
} // end mom_dump_in_directory
