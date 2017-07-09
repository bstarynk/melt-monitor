// file dumpsqlmonimelt.cc - dump an sqlite file, standalone program

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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif /*_GNU_SOURCE*/

#if __cplusplus < 201412L
#error expecting C++17 standard
#endif

#include <features.h>           // GNU things
#include <unistd.h>
#include <string.h>

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>





/// from https://github.com/aminroosta/sqlite_modern_cpp
#include "sqlite_modern_cpp.h"

/**
query to retrieve the CREATE TABLE statements
without ending semicolon is :
  SELECT sql FROM sqlite_master WHERE type='table' ORDER BY name;

query to retrieve the names of tables is
  SELECT name FROM sqlite_master WHERE type='table' ORDER by name;

query to retrieve the CREATE INDEX statements
without ending semicolon is :
  SELECT sql FROM sqlite_master WHERE type='index' ORDER BY name;
**/

class ShowQuoted
{
  std::string str;
public:
  ShowQuoted(const std::string&s) : str(s) {};
  ShowQuoted(const char*s) : str(s) {};
  void output(std::ostream&out) const;
  ~ShowQuoted() {};
};

inline std::ostream&operator << (std::ostream&out, const ShowQuoted&qu)
{
  qu.output(out);
  return out;
};

void ShowQuoted::output(std::ostream&out) const
{
  for (char c : str)
    {
      if (c=='\'')
        out << "''";
      else
        out << c;
    }
}

void dump_schema(sqlite::database& db)
{
  std::cout << "--- the data base schema @@@@@" << std::endl;
  std::cout << "BEGIN TRANSACTION;" << std::endl;
  db << "SELECT sql FROM sqlite_master WHERE type='table' ORDER BY name;"
     >> [&](std::string stmtxt)
  {
    if (!stmtxt.empty())
      std::cout << stmtxt << ";" << std::endl;
  };
  db << "SELECT sql FROM sqlite_master WHERE type='index' ORDER BY name;"
     >> [&](std::string stmtxt)
  {
    if (!stmtxt.empty())
      std::cout << stmtxt << ";" << std::endl;
  };
  std::cout << "END TRANSACTION; -- schema\n" << std::endl;
} // end dump_schema


void dump_globdata(sqlite::database& db)
{
  std::cout << "--- TABLE t_globdata @@@@@" << std::endl;
  std::cout << "BEGIN TRANSACTION;" << std::endl;
  db << "SELECT (glob_namestr, glob_oid) FROM t_globdata"
     " ORDER BY glob_namestr"
     >> [&](std::string nam, std::string oid)
  {
    std::cout << "INSERT INTO t_globdata VALUES ('" << nam << "', '" << oid << "');\n";
  };
  std::cout << "END TRANSACTION; --- for t_globdata\n" << std::endl;
} // end dump_globdata

void dump_names(sqlite::database& db)
{
  std::cout << "--- TABLE t_names @@@@@" << std::endl;
  std::cout << "BEGIN TRANSACTION;" << std::endl;
  db << "SELECT (nam_oid, nam_str) FROM t_names"
     " ORDER BY nam_str"
     >> [&](std::string oid, std::string nam)
  {
    std::cout << "INSERT INTO t_names VALUES ('" << oid << "', '" << nam << "');\n";
  };
  std::cout << "END TRANSACTION; --- for t_names\n" << std::endl;
} // end dump_names

void dump_objects(sqlite::database& db)
{
  std::cout << "\n" "--- TABLE t_objects @@@@@" << std::endl;
  std::cout << "BEGIN TRANSACTION;\n" << std::endl;
  db << "SELECT ob_id, ob_mtim, ob_content,"
     "  ob_paylkind, ob_paylinit, ob_paylcontent"
     " FROM t_objects ORDER BY ob_id"
     >> [&](std::string oid, std::string mtim, std::string content,
            std::string paylkind, std::string paylinit, std::string paylcontent)
  {
    std::cout << "INSERT INTO t_objects VALUES('" << oid
              << "', " << mtim << ",\n'" << ShowQuoted(content)
              << "',\n";
    if (paylkind.empty() && paylinit.empty() && paylcontent.empty())
      std::cout << "'', '', ''\n";
    else
      std::cout << "'" << paylkind << "',\n"
                << "'" << ShowQuoted(paylinit) << ",\n"
                << "'" << ShowQuoted(paylcontent) << '\n';
    std::cout << ");\n" << "--'* end " << oid<< "\n\n"<< std::endl;
  };
  std::cout << "\n\n" "END TRANSACTION; --- for t_objects\n\n" << std::endl;
} // end dump_objects

int main(int argc, char**argv)
{
  if (argc != 2)
    {
      std::clog << argv[0] << " expects exactly one argument, an *.sqlite file" << std::endl;
      exit(EXIT_FAILURE);
    }
  if (access(argv[1],R_OK))
    {
      perror(argv[1]);
      exit(EXIT_FAILURE);
    };
  sqlite::sqlite_config dbconfig;
  dbconfig.flags = sqlite::OpenFlags::READONLY;
  dbconfig.encoding = sqlite::Encoding::UTF8;
  sqlite::database db(argv[1],dbconfig);
  dump_schema(db);
  dump_globdata(db);
  dump_names(db);
  dump_objects(db);
  std::cout << "-- end dump of " << basename(argv[1]) << std::endl;
  return 0;
} // end main

