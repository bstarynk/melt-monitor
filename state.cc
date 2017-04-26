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
  std::unique_ptr<sqlite::database> _ld_globdbp;
  std::unique_ptr<sqlite::database> _ld_userdbp;
  std::mutex _ld_mtxglobdb;
  std::mutex _ld_mtxuserdb;
  std::unordered_map<MomIdent,MomObject*,MomIdentBucketHash> _ld_objmap;
  std::mutex _ld_mtxobjmap;
  std::unique_ptr<sqlite::database> load_database(const char*dbradix);
  long load_empty_objects_from_db(sqlite::database* pdb, bool user);
  void load_all_globdata(void);
  void load_all_objects_content(void);
  void load_object_content(MomObject*pob, int thix, const std::string&strcont);
  void load_cold_globdata(const char*globnam, std::atomic<MomObject*>*pglob);
  static void thread_load_content_objects (MomLoader*ld, int thix, std::deque<MomObject*>*obqu, const std::function<std::string(MomObject*)>&getglobfun,const std::function<std::string(MomObject*)>&getuserfun);
#warning should add a lot more into MomLoader
public:
  MomLoader(const std::string&dirnam);
  static constexpr bool IS_USER= true;
  static constexpr bool IS_GLOBAL= false;
  void load(void);
};				// end class MomLoader


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
  std::lock_guard<std::mutex> gu (*(user?(&_ld_mtxuserdb):(&_ld_mtxglobdb)));
  *pdb << "SELECT ob_id FROM t_objects"
       >> [&](std::string idstr)
  {
    auto id = MomIdent::make_from_cstr(idstr.c_str(),true);
    auto pob = MomObject::make_object_of_id(id);
    if (pob)
      {
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
      }
  };
  return obcnt;
} // end MomLoader::load_empty_objects_from_db


MomLoader::MomLoader(const std::string& dirnam)
  : _ld_dirname(dirnam), _ld_globdbp(nullptr), _ld_userdbp(nullptr),
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
MomLoader::load(void)
{
  long nbglobob = 0;
  long nbuserob = 0;
  {
    auto globthr = std::thread([&](void)
    {
      nbglobob = load_empty_objects_from_db(_ld_globdbp.get(),IS_GLOBAL);
    });
    if (_ld_userdbp)
      {
        nbuserob = load_empty_objects_from_db(_ld_userdbp.get(),IS_USER);
      }
    globthr.join();
  }
  load_all_globdata();
  load_all_objects_content();
#warning MomLoader::load should do things in parallel
} // end MomLoader::load



void
MomLoader::load_cold_globdata(const char*globnam, std::atomic<MomObject*>*pglob)
{
  MomObject*globpob = nullptr;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxglobdb};
    *_ld_globdbp << "SELECT glob_oid FROM t_globals WHERE glob_namestr = ?;"
                 << globnam >> [&](const std::string &idstr)
    {
      globpob = MomObject::find_object_of_id(MomIdent::make_from_cstr(idstr.c_str()));
    };
  }
  if (!globpob && _ld_userdbp)
    {
      std::lock_guard<std::mutex> gu{_ld_mtxuserdb};
      *_ld_userdbp << "SELECT glob_oid FROM t_globals WHERE glob_namestr = ?;"
                   << globnam >> [&](const std::string &idstr)
      {
        globpob = MomObject::find_object_of_id(MomIdent::make_from_cstr(idstr.c_str()));
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
  /// prepare the object loading statements
  auto globstmt = ((*_ld_globdbp) << "SELECT ob_content FROM t_objects WHERE ob_id = ?;");
  std::function<std::string(MomObject*)> getglobfun =
    [&](MomObject*pob)
  {
    std::lock_guard<std::mutex> gu(_ld_mtxglobdb);
    std::string res;
    globstmt << pob->id().to_string() >> res;
    return res;
  };
  std::function<std::string(MomObject*)> getuserfun;
  auto userstmt = ((*(_ld_userdbp?:_ld_globdbp)) << "SELECT ob_content FROM t_objects WHERE ob_id = ?;");
  if (_ld_userdbp)
    {
      getuserfun =
        [&](MomObject*pob)
      {
        std::lock_guard<std::mutex> gu(_ld_mtxuserdb);
        std::string res;
        userstmt << pob->id().to_string() >> res;
        return res;
      };
    }
  /// build a vector of queue of objects to be loaded
  std::vector<std::deque<MomObject*>> vecobjque(mom_nb_jobs);
  unsigned long obcnt = 0;
  {
    std::lock_guard<std::mutex> gu{_ld_mtxobjmap};
    for (auto p : _ld_objmap)
      {
        MomObject* pob = p.second;
        MOM_ASSERT(pob != nullptr && pob->id() == p.first,
                   "MomLoader::load_all_objects_content bad id");
        vecobjque[obcnt % mom_nb_jobs].push_back(pob);
        obcnt++;
      }
  }
  std::vector<std::thread> vecthr(mom_nb_jobs);
  for (int ix=1; ix<=mom_nb_jobs; ix++)
    vecthr[ix-1] = std::thread(thread_load_content_objects, this, ix, &vecobjque[ix], getglobfun, getuserfun);
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(5+2*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
} // end MomLoader::load_all_objects_content


void
MomLoader::thread_load_content_objects(MomLoader*ld, int thix, std::deque<MomObject*>*obpqu,
                                       const std::function<std::string(MomObject*)>&getglobfun,
                                       const std::function<std::string(MomObject*)>&getuserfun)
{
  MOM_ASSERT(ld != nullptr,
             "MomLoader::thread_load_content_objects null ld");
  MOM_ASSERT(thix>0 && thix<=(int)mom_nb_jobs,
             "MomLoader::thread_load_content_objects bad thix=" << thix);
  MOM_ASSERT(obpqu != nullptr, "MomLoader::thread_load_content_objects null obpqu");
  while (!obpqu->empty())
    {
      MomObject*pob = obpqu->front();
      obpqu->pop_front();
      MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
                 "MomLoader::thread_load_content_objects bad pob");
      std::string strcont;
      if (pob->space() == MomSpace::UserSp)
        strcont = getuserfun(pob);
      else
        strcont = getglobfun(pob);
      if (!strcont.empty())
        ld->load_object_content(pob, thix, strcont);
    };
} // end MomLoader::thread_load_content_objects



void
MomLoader::load_object_content(MomObject*pob, int thix, const std::string&strcont)
{
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
          /// should add the attribute
        }
      else if (contpars.hasdelim("&&"))
        {
          bool gotcomp = false;
          MomValue valattr = contpars.parse_value(&gotcomp);
          if (!gotcomp)
            MOM_PARSE_FAILURE(&contpars, "missing component#" << nbcomp);
          nbcomp++;
          /// should append the value
        }
    }
#warning incomplete MomLoader::load_object_content
  /// should create a parser
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

class MomDumper
{
  std::mutex _du_mtx;
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
  static void dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec);
  void dump_emit_object(MomObject*pob, int thix);
public:
  std::string temporary_file_path(const std::string& path);
  MomDumper(const std::string&dirnam);
  void open_databases(void);
  void scan_predefined(void);
  void scan_globdata(void);
  void dump_scan_loop(void);
  void dump_emit_loop(void);
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
          added = true;
        }
    }
    if (added)
      _du_addedcondvar.notify_all();
  }
  bool is_dumped(const MomObject*pob)
  {
    std::lock_guard<std::mutex> gu{_du_mtx};
    return (_du_setobj.find(pob) != _du_setobj.end());
  }
  ~MomDumper();
#warning should add a lot more into MomDumper
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
  : _du_mtx(), _du_state{dus_none}, _du_dirname(dirnam), _du_tempsuffix(), _du_tempset(),
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
    snprintf(tempbuf, sizeof(tempbuf), "%sp%d-", sbuf16, (int)getpid());
    _du_tempsuffix.assign(tempbuf);
  }
} // end MomDumper::MomDumper


std::string
MomDumper::temporary_file_path(const std::string& path)
{
  if (path.empty() || path[0] == '.' || path[0] == '/' || path.find("..") != std::string::npos)
    MOM_FAILURE("MomDumper (in " << _du_dirname << " directory) with bad temporal path " << path);
  _du_tempset.insert(path);
  return path + _du_tempsuffix;
} // end MomDumper::temporary_file_path

void
MomDumper::open_databases(void)
{
  auto globdbpath = temporary_file_path(_du_dirname + "/" +  MOM_GLOBAL_DB + ".sqlite");
  auto userdbpath = temporary_file_path(_du_dirname + "/" +  MOM_USER_DB + ".sqlite");
  sqlite::sqlite_config dbconfig;
  dbconfig.flags = sqlite::OpenFlags::CREATE | sqlite::OpenFlags::READWRITE| sqlite::OpenFlags::NOMUTEX;
  dbconfig.encoding = sqlite::Encoding::UTF8;
  _du_globdbp = std::make_unique<sqlite::database>(globdbpath, dbconfig);
  _du_userdbp = std::make_unique<sqlite::database>(userdbpath, dbconfig);
  initialize_db(*_du_globdbp);
  initialize_db(*_du_userdbp);
} // end MomDumper::open_databases

void
MomDumper::initialize_db(sqlite::database &db)
{
  db << R"!*(
CREATE TABLE IF NOT EXISTS t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim INT8 NOT NULL,
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
CREATE TABLE IF NOT EXISTS t_globals
 (glob_namestr VARCHAR(80) NOT NULL UNIQUE,
  glob_oid  VARCHAR(30) NOT NULL)
)!*";
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
#define MOM_HAS_GLOBDATA(Nam) do {			\
  MomObject* gpob##Nam = MOM_LOAD_GLOBDATA(Nam);	\
  if (gpob##Nam != nullptr) {				\
    add_scanned_object(gpob##Nam);			\
    std::lock_guard<std::mutex> gu{_du_mtx};		\
    _du_globmap.insert({std::string{#Nam},gpob##Nam});	\
  }							\
  } while(0);
#include "_mom_globdata.h"
} // end MomDumper::scan_globdata

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
MomDumper::dump_emit_object(MomObject*pob, int thix)
{
  MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
	     "MomDumper::dump_emit_object bad pob");
  MOM_ASSERT(thix > 0 && thix <= (int)mom_nb_jobs, "MomDumper::dump_emit_object bad thix:" << thix);
  bool isglobal = false;
  bool isuser = false;
  std::string contentstr;
  MomObject::PayloadEmission pyem;
  double obmtime=0.0;
  bool haspayload = false;
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
     haspayload = true;
    }
  }
  // lock the relevant database mutex and insert into it
  if (isglobal) {
    std::lock_guard<std::mutex> glg(_du_globdbmtx);
    //_du_globdbp
  }
  else if (isuser) {
    std::lock_guard<std::mutex> glg(_du_userdbmtx);
    //_du_userdbp
  }
#warning MomDumper::dump_emit_object incomplete
} // end MomDumper::dump_emit_object



void
MomDumper::dump_emit_thread(MomDumper*du, int ix, std::vector<MomObject*>* pvec)
{
  std::sort(pvec->begin(), pvec->end(), MomObjptrLess{});
  for (MomObject*pob : *pvec) {
    du->dump_emit_object(pob, ix);
  }
#warning MomDumper::dump_emit_thread very incomplete
} // end MomDumper::dump_emit_thread

void
MomDumper::dump_emit_loop(void) {
  MOM_ASSERT(_du_state == dus_scan, "MomDumper::dump_emit_loop bad start state");
  _du_state = dus_emit;
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
  for (unsigned ix=1; ix<=mom_nb_jobs; ix++) {
#warning something wrong here in MomDumper::dump_emit_loop
    vecthr[ix-1] = std::thread(dump_emit_thread, this, ix, &vecobjob[ix-1]);
  }
  std::this_thread::yield();
  std::this_thread::sleep_for(std::chrono::milliseconds(50+20*mom_nb_jobs));
  for (int ix=1; ix<=(int)mom_nb_jobs; ix++)
    vecthr[ix-1].join();
} // end MomDumper::dump_emit_loop


MomDumper::~MomDumper() {
  #warning incomplete MomDumper destructor
  MOM_FATALOG("incomplete MomDumper destructor");
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
#warning missing MomObject::unsync_emit_dump_payload
} // end MomObject::unsync_emit_dump_content

void
mom_dump_in_directory(const char*dirname)
{
  MomDumper dumper(dirname);
  dumper.open_databases();
  dumper.scan_predefined();
  dumper.scan_globdata();
  dumper.dump_scan_loop();	// multi-threaded
  dumper.dump_emit_loop();	// multi-threaded
#warning incomplete mom_dump_in_directory
} // end mom_dump_in_directory
