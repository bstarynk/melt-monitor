-- dump 2017 May 29 from mom_global dumped by monimelt-dump-state.sh .....

 --   Copyright (C) 2017 Free Software Foundation, Inc.
 --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
 --  This sqlite3 dump file mom_global.sql is part of GCC.
 --
 --  GCC is free software; you can redistribute it and/or modify
 --  it under the terms of the GNU General Public License as published by
 --  the Free Software Foundation; either version 3, or (at your option)
 --  any later version.
 --
 --  GCC is distributed in the hope that it will be useful,
 --  but WITHOUT ANY WARRANTY; without even the implied warranty of
 --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 --  GNU General Public License for more details.
 --  You should have received a copy of the GNU General Public License
 --  along with GCC; see the file COPYING3.   If not see
 --  <http://www.gnu.org/licenses/>.

BEGIN TRANSACTION; --- for schema
CREATE TABLE t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim REAL NOT NULL,
  ob_content TEXT NOT NULL,
  ob_paylkind VARCHAR(30) NOT NULL,
  ob_paylinit TEXT NOT NULL,
  ob_paylcontent TEXT NOT NULL);
CREATE TABLE t_names
 (nam_oid VARCHAR(30) NOT NULL UNIQUE,
  nam_str TEXT PRIMARY KEY ASC NOT NULL UNIQUE);
CREATE TABLE t_globdata
 (glob_namestr VARCHAR(80) NOT NULL UNIQUE,
  glob_oid  VARCHAR(30) NOT NULL);
END TRANSACTION; --- for schema

------- TABLE t_globdata @@@@@@
BEGIN TRANSACTION; --- for t_globdata
END TRANSACTION; ---- for t_globdata

------- TABLE t_names @@@@@@
BEGIN TRANSACTION; --- for t_names
INSERT INTO t_names VALUES('_7QeFDN33m7B_3dQv2OqqbH6','get');
INSERT INTO t_names VALUES('_1jJjA6LcXiX_1V4ZcXlje09','name');
INSERT INTO t_names VALUES('_6ss8POQNnku_8e8woNsmvN9','outputter');
INSERT INTO t_names VALUES('_4w339hT5dXd_1tUZFvU4fWx','proxy');
INSERT INTO t_names VALUES('_3bMUJDWlMMQ_2j2bm7EeIbv','put');
INSERT INTO t_names VALUES('_2mYaTh9kH4I_7ENiXcymRmy','set');
INSERT INTO t_names VALUES('_3SFtXTTy3kj_89fO24X2HFo','size');
END TRANSACTION; ---- for t_names

------- TABLE t_objects @@@@@@
BEGIN TRANSACTION; --- for t_objects
INSERT INTO t_objects VALUES('_1jJjA6LcXiX_1V4ZcXlje09',1493382726.98,'
///$name
@MAGIC!

','named','name','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_2mYaTh9kH4I_7ENiXcymRmy',1493382726.98,'
///$set
@MAGIC!

','named','set','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3SFtXTTy3kj_89fO24X2HFo',1495430034.89,'
///$size
@MAGIC!

','named','size','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3bMUJDWlMMQ_2j2bm7EeIbv',1495430034.89,'
///$put
@MAGIC!

','named','put','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4w339hT5dXd_1tUZFvU4fWx',1495430034.89,'
///$proxy
@MAGIC!

','named','proxy','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_6ss8POQNnku_8e8woNsmvN9',1496020871.11,'
///$outputter
@MAGIC!

','named','outputter','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_7QeFDN33m7B_3dQv2OqqbH6',1493382726.98,'
///$get
@MAGIC!

','named','get','@NAMEDPROXY: __');

END TRANSACTION; ---- for t_objects

-- monimelt-dump-state end dump mom_global
