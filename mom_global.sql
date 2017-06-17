-- dump 2017 Jun 17 from mom_global dumped by monimelt-dump-state.sh .....

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
INSERT INTO t_names VALUES('_4cCZc3Izgsr_7LsCkWW7flp','doublesq');
INSERT INTO t_names VALUES('_7QeFDN33m7B_3dQv2OqqbH6','get');
INSERT INTO t_names VALUES('_0vgCFjXblkx_4zCMhMAWjVK','int');
INSERT INTO t_names VALUES('_45cnnX4v29t_9gZkfMOZj2Z','intsq');
INSERT INTO t_names VALUES('_1jJjA6LcXiX_1V4ZcXlje09','name');
INSERT INTO t_names VALUES('_7D8xcWnEiys_8oqOVSkCxkA','node');
INSERT INTO t_names VALUES('_1JSykdLcLdl_8rWxtDBaPGN','none');
INSERT INTO t_names VALUES('_7T9OwSFlgov_0wVJaK1eZbn','object');
INSERT INTO t_names VALUES('_6ss8POQNnku_8e8woNsmvN9','outputter');
INSERT INTO t_names VALUES('_4w339hT5dXd_1tUZFvU4fWx','proxy');
INSERT INTO t_names VALUES('_3bMUJDWlMMQ_2j2bm7EeIbv','put');
INSERT INTO t_names VALUES('_2mYaTh9kH4I_7ENiXcymRmy','set');
INSERT INTO t_names VALUES('_3SFtXTTy3kj_89fO24X2HFo','size');
INSERT INTO t_names VALUES('_4T8am97muLl_5969SR22Ecq','string');
INSERT INTO t_names VALUES('_6TmLNh9vtVY_0pwkHRtJ44k','tuple');
--- °°°°
INSERT INTO t_names VALUES('_7hbSpcNUdHi_4QezAVr6Bgj','chunk');
INSERT INTO t_names VALUES('_8f7GaaT5WJK_01z8k1JztS6', 'dollar');
INSERT INTO t_names VALUES('_33imugvOze0_0wdPqjmME0U','embed');
END TRANSACTION; ---- for t_names

------- TABLE t_objects @@@@@@
BEGIN TRANSACTION; --- for t_objects
INSERT INTO t_objects VALUES('_0LK4TzFd6u1_0JFUsrQ4odG',1497705092.87,'

','','','');
INSERT INTO t_objects VALUES('_0vgCFjXblkx_4zCMhMAWjVK',1497636292.47,'
///$int
@MAGIC!

','named','int','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1JSykdLcLdl_8rWxtDBaPGN',1497636292.47,'
///$none
@MAGIC!

','named','none','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1jJjA6LcXiX_1V4ZcXlje09',1493382726.98,'
///$name
@MAGIC!

','named','name','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_2mYaTh9kH4I_7ENiXcymRmy',1493382726.98,'
///$set
@MAGIC!

','named','set','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_33imugvOze0_0wdPqjmME0U',1497705092.87,'
///°°°$embed
','named','embed','');
INSERT INTO t_objects VALUES('_3SFtXTTy3kj_89fO24X2HFo',1495430034.89,'
///$size
@MAGIC!

','named','size','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3bMUJDWlMMQ_2j2bm7EeIbv',1495430034.89,'
///$put
@MAGIC!

','named','put','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_45cnnX4v29t_9gZkfMOZj2Z',1497636412.76,'
///$intsq
@MAGIC!

','named','intsq','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4T8am97muLl_5969SR22Ecq',1497636412.76,'
///$string
@MAGIC!

','named','string','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4cCZc3Izgsr_7LsCkWW7flp',1497636292.47,'
///$doublesq
@MAGIC!

','named','doublesq','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4w339hT5dXd_1tUZFvU4fWx',1495430034.89,'
///$proxy
@MAGIC!

','named','proxy','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_6TmLNh9vtVY_0pwkHRtJ44k',1497636292.47,'
///$tuple
@MAGIC!

','named','tuple','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_6ZpMq408eTN_8t3ZHpgCk3R',1497636292.47,'

','','','');
INSERT INTO t_objects VALUES('_6ss8POQNnku_8e8woNsmvN9',1496020871.11,'
///$outputter
@MAGIC!

','named','outputter','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_785pDebNo9y_1ugTc398Np4',1497705092.87,'

','','','');
INSERT INTO t_objects VALUES('_7D8xcWnEiys_8oqOVSkCxkA',1497636412.76,'
///$node
@MAGIC!

','named','node','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_7QeFDN33m7B_3dQv2OqqbH6',1493382726.98,'
///$get
@MAGIC!

','named','get','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_7T9OwSFlgov_0wVJaK1eZbn',1497636292.47,'
///$object
@MAGIC!

','named','object','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_7hbSpcNUdHi_4QezAVr6Bgj',1497636292.47,'
///°°°$chunk
','named','chunk','');
INSERT INTO t_objects VALUES('_7ld89d4lLTB_3596gc6hLFI',1497636292.47,'

','','','');
INSERT INTO t_objects VALUES('_8ZPtpM4JscH_1ViGLydV6mU',1497705092.87,'

','','','');
INSERT INTO t_objects VALUES('_8f7GaaT5WJK_01z8k1JztS6',1497636292.47,'
///°°°$dollar
','named','dollar','');
INSERT INTO t_objects VALUES('_8vdOBDVvgy8_5Hq3NM5whLJ',1497636292.47,'

','','','');
INSERT INTO t_objects VALUES('_9Cr5XlTL8lI_7ALESzYlXD7',1497636292.47,'

','','','');

END TRANSACTION; ---- for t_objects

------- END DUMP @@@@@

-- monimelt-dump-state end dump mom_global
