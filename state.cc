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
  std::unordered_map<MomIdent,MomObject*,MomIdentBucketHash> _ld_objmap;
  std::unique_ptr<sqlite::database> load_database(const char*dbradix);
  long load_empty_objects_from_db(sqlite::database& db);
#warning should add a lot more into MomLoader
public:
  MomLoader(const std::string&dirnam);
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


long MomLoader::load_empty_objects_from_db(sqlite::database& db)
{
  long obcnt = 0;
  db << "SELECT ob_id FROM t_objects"
     >> [&](std::string idstr)
  {
    auto id = MomIdent::make_from_cstr(idstr.c_str(),true);
    auto pob = MomObject::make_object_of_id(id);
    if (pob)
      {
        _ld_objmap.insert({id,pob});
        obcnt++;
      }
  };
  return obcnt;
} // end MomLoader::load_empty_objects_from_db


MomLoader::MomLoader(const std::string& dirnam)
  : _ld_dirname(dirnam), _ld_globdbp(nullptr), _ld_userdbp(nullptr)
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




//==============================================================
////////////////////////////////////////////////////////////////
class MomDumper
{
  std::string _du_dirname;
  std::string _du_tempsuffix;
  std::set<std::string> _du_tempset;
  std::unique_ptr<sqlite::database> _du_globdbp;
  std::unique_ptr<sqlite::database> _du_userdbp;
  void initialize_db(sqlite::database &db);
public:
  std::string temporary_file_path(const std::string& path);
  MomDumper(const std::string&dirnam);
  void open_databases(void);
  ~MomDumper();
#warning should add a lot more into MomDumper
};				// end class MomDumper


MomDumper::MomDumper(const std::string&dirnam)
  : _du_dirname(dirnam), _du_tempsuffix()
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
