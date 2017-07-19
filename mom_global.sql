-- dump 2017 Jul 19 from mom_global dumped by monimelt-dump-state.sh .....

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


-- dump of mom_global sqlite db by dumpsqlmonimelt

--- the data base schema @@@@@
BEGIN TRANSACTION;
CREATE TABLE t_globdata
 (glob_namestr VARCHAR(80) NOT NULL UNIQUE,
  glob_oid  VARCHAR(30) NOT NULL);
CREATE TABLE t_modules
 (mod_id VARCHAR(30) NOT NULL UNIQUE);
CREATE TABLE t_names
 (nam_oid VARCHAR(30) NOT NULL UNIQUE,
  nam_str TEXT PRIMARY KEY ASC NOT NULL UNIQUE);
CREATE TABLE t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim REAL NOT NULL,
  ob_content TEXT NOT NULL,
  ob_paylkind VARCHAR(30) NOT NULL,
  ob_paylinit TEXT NOT NULL,
  ob_paylcontent TEXT NOT NULL,
  ob_paylproxid VARCHAR(30) NOT NULL);
END TRANSACTION; -- schema

--- TABLE t_globdata @@@@@
BEGIN TRANSACTION;
END TRANSACTION; --- for t_globdata

--- TABLE t_names @@@@@
BEGIN TRANSACTION;
INSERT INTO t_names VALUES ('_7hbSpcNUdHi_4QezAVr6Bgj', 'chunk');
INSERT INTO t_names VALUES ('_8f7GaaT5WJK_01z8k1JztS6', 'dollar');
INSERT INTO t_names VALUES ('_4cCZc3Izgsr_7LsCkWW7flp', 'doublesq');
INSERT INTO t_names VALUES ('_33imugvOze0_0wdPqjmME0U', 'embed');
INSERT INTO t_names VALUES ('_2QeF5dreQwf_51c3fAJEYqJ', 'emit');
INSERT INTO t_names VALUES ('_7DYTrPqfMqY_1sSbTYnbhTB', 'emit_predefined_content');
INSERT INTO t_names VALUES ('_7QeFDN33m7B_3dQv2OqqbH6', 'get');
INSERT INTO t_names VALUES ('_0vgCFjXblkx_4zCMhMAWjVK', 'int');
INSERT INTO t_names VALUES ('_45cnnX4v29t_9gZkfMOZj2Z', 'intsq');
INSERT INTO t_names VALUES ('_1cstd9Sn6qZ_8NZPx7Zvm7T', 'mom_predefined_file');
INSERT INTO t_names VALUES ('_1jJjA6LcXiX_1V4ZcXlje09', 'name');
INSERT INTO t_names VALUES ('_7D8xcWnEiys_8oqOVSkCxkA', 'node');
INSERT INTO t_names VALUES ('_1JSykdLcLdl_8rWxtDBaPGN', 'none');
INSERT INTO t_names VALUES ('_7T9OwSFlgov_0wVJaK1eZbn', 'object');
INSERT INTO t_names VALUES ('_6ss8POQNnku_8e8woNsmvN9', 'outputter');
INSERT INTO t_names VALUES ('_4w339hT5dXd_1tUZFvU4fWx', 'proxy');
INSERT INTO t_names VALUES ('_3bMUJDWlMMQ_2j2bm7EeIbv', 'put');
INSERT INTO t_names VALUES ('_2mYaTh9kH4I_7ENiXcymRmy', 'set');
INSERT INTO t_names VALUES ('_3SFtXTTy3kj_89fO24X2HFo', 'size');
INSERT INTO t_names VALUES ('_2SnSoZOZXSr_9jjK01p1ahH', 'start_cplusplus_emission');
INSERT INTO t_names VALUES ('_268RKjuQT02_2Kh0wgKnhLT', 'start_generation');
INSERT INTO t_names VALUES ('_6ZpMq408eTN_8t3ZHpgCk3R', 'starter');
INSERT INTO t_names VALUES ('_4T8am97muLl_5969SR22Ecq', 'string');
INSERT INTO t_names VALUES ('_5Wnb8RZdglo_9jkLPwTffMm', 'the_system');
INSERT INTO t_names VALUES ('_6TmLNh9vtVY_0pwkHRtJ44k', 'tuple');
END TRANSACTION; --- for t_names


--- TABLE t_objects @@@@@
BEGIN TRANSACTION;

INSERT INTO t_objects VALUES('_0LK4TzFd6u1_0JFUsrQ4odG', 1497705092.87,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _0LK4TzFd6u1_0JFUsrQ4odG


INSERT INTO t_objects VALUES('_0vgCFjXblkx_4zCMhMAWjVK', 1497636292.47,
'
///$int
@MAGIC!

',
'named', --- payl _0vgCFjXblkx_4zCMhMAWjVK
'int',
'',
'');--'--
------'** end _0vgCFjXblkx_4zCMhMAWjVK


INSERT INTO t_objects VALUES('_1AXXUcWe8Pc_4UIYaHPzd9G', 1500453602.86,
'
@: _3xRwKNR8Txl_0wsckeKslWS "the genfile proxy of mom_predefined_file"

',
'genfile', --- payl _1AXXUcWe8Pc_4UIYaHPzd9G
'_mom_predefined.h',
'',
'_3Kk21ebU2OC_1GJWVglZfFM');--'--
------'** end _1AXXUcWe8Pc_4UIYaHPzd9G


INSERT INTO t_objects VALUES('_1JSykdLcLdl_8rWxtDBaPGN', 1497636292.47,
'
///$none
@MAGIC!

',
'named', --- payl _1JSykdLcLdl_8rWxtDBaPGN
'none',
'',
'');--'--
------'** end _1JSykdLcLdl_8rWxtDBaPGN


INSERT INTO t_objects VALUES('_1cstd9Sn6qZ_8NZPx7Zvm7T', 1500453299.5,
'
///$mom_predefined_file
@: _3xRwKNR8Txl_0wsckeKslWS _1AXXUcWe8Pc_4UIYaHPzd9G

',
'named', --- payl _1cstd9Sn6qZ_8NZPx7Zvm7T
'mom_predefined_file',
'',
'_1AXXUcWe8Pc_4UIYaHPzd9G');--'--
------'** end _1cstd9Sn6qZ_8NZPx7Zvm7T


INSERT INTO t_objects VALUES('_1jJjA6LcXiX_1V4ZcXlje09', 1493382726.98,
'
///$name
@MAGIC!

',
'named', --- payl _1jJjA6LcXiX_1V4ZcXlje09
'name',
'',
'');--'--
------'** end _1jJjA6LcXiX_1V4ZcXlje09


INSERT INTO t_objects VALUES('_268RKjuQT02_2Kh0wgKnhLT', 1500010547.63,
'
///$start_generation

',
'named', --- payl _268RKjuQT02_2Kh0wgKnhLT
'start_generation',
'',
'');--'--
------'** end _268RKjuQT02_2Kh0wgKnhLT


INSERT INTO t_objects VALUES('_2QeF5dreQwf_51c3fAJEYqJ', 1499613379.2,
'
///$emit

',
'named', --- payl _2QeF5dreQwf_51c3fAJEYqJ
'emit',
'',
'');--'--
------'** end _2QeF5dreQwf_51c3fAJEYqJ


INSERT INTO t_objects VALUES('_2SnSoZOZXSr_9jjK01p1ahH', 1500369880.54,
'
///$start_cplusplus_emission
@: _6ss8POQNnku_8e8woNsmvN9 _6Jns5AFaIAO_8vsgtP9xyZj

',
'named', --- payl _2SnSoZOZXSr_9jjK01p1ahH
'start_cplusplus_emission',
'',
'');--'--
------'** end _2SnSoZOZXSr_9jjK01p1ahH


INSERT INTO t_objects VALUES('_2mYaTh9kH4I_7ENiXcymRmy', 1493382726.98,
'
///$set
@MAGIC!

',
'named', --- payl _2mYaTh9kH4I_7ENiXcymRmy
'set',
'',
'');--'--
------'** end _2mYaTh9kH4I_7ENiXcymRmy


INSERT INTO t_objects VALUES('_33imugvOze0_0wdPqjmME0U', 1497705092.87,
'
///$embed

',
'named', --- payl _33imugvOze0_0wdPqjmME0U
'embed',
'',
'');--'--
------'** end _33imugvOze0_0wdPqjmME0U


INSERT INTO t_objects VALUES('_3A2Up6FQUMF_6qcLJJNfjaD', 1500454839.91,
'
@: _3xRwKNR8Txl_0wsckeKslWS "the code proxy of emit_predefined_content"

',
'code', --- payl _3A2Up6FQUMF_6qcLJJNfjaD
'@CODEBASE: emit_predefined_full @CODESTEP!
',
'',
'');--'--
------'** end _3A2Up6FQUMF_6qcLJJNfjaD


INSERT INTO t_objects VALUES('_3Kk21ebU2OC_1GJWVglZfFM', 1500453726.49,
'
@: _3xRwKNR8Txl_0wsckeKslWS "the code proxy of mom_predefined_file"
&: _2SnSoZOZXSr_9jjK01p1ahH
&: _7DYTrPqfMqY_1sSbTYnbhTB

',
'code', --- payl _3Kk21ebU2OC_1GJWVglZfFM
'@CODEBASE: predefined_file_generator @CODEUPDATE!
',
'',
'');--'--
------'** end _3Kk21ebU2OC_1GJWVglZfFM


INSERT INTO t_objects VALUES('_3SFtXTTy3kj_89fO24X2HFo', 1495430034.89,
'
///$size
@MAGIC!

',
'named', --- payl _3SFtXTTy3kj_89fO24X2HFo
'size',
'',
'');--'--
------'** end _3SFtXTTy3kj_89fO24X2HFo


INSERT INTO t_objects VALUES('_3bMUJDWlMMQ_2j2bm7EeIbv', 1495430034.89,
'
///$put
@MAGIC!

',
'named', --- payl _3bMUJDWlMMQ_2j2bm7EeIbv
'put',
'',
'');--'--
------'** end _3bMUJDWlMMQ_2j2bm7EeIbv


INSERT INTO t_objects VALUES('_45cnnX4v29t_9gZkfMOZj2Z', 1497636412.76,
'
///$intsq
@MAGIC!

',
'named', --- payl _45cnnX4v29t_9gZkfMOZj2Z
'intsq',
'',
'');--'--
------'** end _45cnnX4v29t_9gZkfMOZj2Z


INSERT INTO t_objects VALUES('_4T8am97muLl_5969SR22Ecq', 1497636412.76,
'
///$string
@MAGIC!

',
'named', --- payl _4T8am97muLl_5969SR22Ecq
'string',
'',
'');--'--
------'** end _4T8am97muLl_5969SR22Ecq


INSERT INTO t_objects VALUES('_4cCZc3Izgsr_7LsCkWW7flp', 1497636292.47,
'
///$doublesq
@MAGIC!

',
'named', --- payl _4cCZc3Izgsr_7LsCkWW7flp
'doublesq',
'',
'');--'--
------'** end _4cCZc3Izgsr_7LsCkWW7flp


INSERT INTO t_objects VALUES('_4w339hT5dXd_1tUZFvU4fWx', 1495430034.89,
'
///$proxy
@MAGIC!

',
'named', --- payl _4w339hT5dXd_1tUZFvU4fWx
'proxy',
'',
'');--'--
------'** end _4w339hT5dXd_1tUZFvU4fWx


INSERT INTO t_objects VALUES('_5Wnb8RZdglo_9jkLPwTffMm', 1500351215.97,
'
///$the_system
@: _3xRwKNR8Txl_0wsckeKslWS [_1cstd9Sn6qZ_8NZPx7Zvm7T]

',
'named', --- payl _5Wnb8RZdglo_9jkLPwTffMm
'the_system',
'',
'');--'--
------'** end _5Wnb8RZdglo_9jkLPwTffMm


INSERT INTO t_objects VALUES('_6Jns5AFaIAO_8vsgtP9xyZj', 1500406793.05,
'
@: _3xRwKNR8Txl_0wsckeKslWS "the outputter of start_cplusplus_emission"

',
'code', --- payl _6Jns5AFaIAO_8vsgtP9xyZj
'@CODEBASE: start_cplusplus_outputter @CODESTEP!
',
'',
'');--'--
------'** end _6Jns5AFaIAO_8vsgtP9xyZj


INSERT INTO t_objects VALUES('_6TmLNh9vtVY_0pwkHRtJ44k', 1497636292.47,
'
///$tuple
@MAGIC!

',
'named', --- payl _6TmLNh9vtVY_0pwkHRtJ44k
'tuple',
'',
'');--'--
------'** end _6TmLNh9vtVY_0pwkHRtJ44k


INSERT INTO t_objects VALUES('_6ZpMq408eTN_8t3ZHpgCk3R', 1497636292.47,
'
///$starter
@MAGIC!

',
'named', --- payl _6ZpMq408eTN_8t3ZHpgCk3R
'starter',
'',
'');--'--
------'** end _6ZpMq408eTN_8t3ZHpgCk3R


INSERT INTO t_objects VALUES('_6ss8POQNnku_8e8woNsmvN9', 1496020871.11,
'
///$outputter
@MAGIC!

',
'named', --- payl _6ss8POQNnku_8e8woNsmvN9
'outputter',
'',
'');--'--
------'** end _6ss8POQNnku_8e8woNsmvN9


INSERT INTO t_objects VALUES('_785pDebNo9y_1ugTc398Np4', 1497705092.87,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _785pDebNo9y_1ugTc398Np4


INSERT INTO t_objects VALUES('_7D8xcWnEiys_8oqOVSkCxkA', 1497636412.76,
'
///$node
@MAGIC!

',
'named', --- payl _7D8xcWnEiys_8oqOVSkCxkA
'node',
'',
'');--'--
------'** end _7D8xcWnEiys_8oqOVSkCxkA


INSERT INTO t_objects VALUES('_7DYTrPqfMqY_1sSbTYnbhTB', 1500453884.23,
'
///$emit_predefined_content

',
'named', --- payl _7DYTrPqfMqY_1sSbTYnbhTB
'emit_predefined_content',
'',
'_3A2Up6FQUMF_6qcLJJNfjaD');--'--
------'** end _7DYTrPqfMqY_1sSbTYnbhTB


INSERT INTO t_objects VALUES('_7QeFDN33m7B_3dQv2OqqbH6', 1493382726.98,
'
///$get
@MAGIC!

',
'named', --- payl _7QeFDN33m7B_3dQv2OqqbH6
'get',
'',
'');--'--
------'** end _7QeFDN33m7B_3dQv2OqqbH6


INSERT INTO t_objects VALUES('_7T9OwSFlgov_0wVJaK1eZbn', 1497636292.47,
'
///$object
@MAGIC!

',
'named', --- payl _7T9OwSFlgov_0wVJaK1eZbn
'object',
'',
'');--'--
------'** end _7T9OwSFlgov_0wVJaK1eZbn


INSERT INTO t_objects VALUES('_7hbSpcNUdHi_4QezAVr6Bgj', 1497636292.47,
'
///$chunk

',
'named', --- payl _7hbSpcNUdHi_4QezAVr6Bgj
'chunk',
'',
'');--'--
------'** end _7hbSpcNUdHi_4QezAVr6Bgj


INSERT INTO t_objects VALUES('_7ld89d4lLTB_3596gc6hLFI', 1497636292.47,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _7ld89d4lLTB_3596gc6hLFI


INSERT INTO t_objects VALUES('_8ZPtpM4JscH_1ViGLydV6mU', 1497705092.87,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _8ZPtpM4JscH_1ViGLydV6mU


INSERT INTO t_objects VALUES('_8f7GaaT5WJK_01z8k1JztS6', 1497636292.47,
'
///$dollar

',
'named', --- payl _8f7GaaT5WJK_01z8k1JztS6
'dollar',
'',
'');--'--
------'** end _8f7GaaT5WJK_01z8k1JztS6


INSERT INTO t_objects VALUES('_8vdOBDVvgy8_5Hq3NM5whLJ', 1497636292.47,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _8vdOBDVvgy8_5Hq3NM5whLJ


INSERT INTO t_objects VALUES('_9Cr5XlTL8lI_7ALESzYlXD7', 1497636292.47,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _9Cr5XlTL8lI_7ALESzYlXD7




END TRANSACTION; --- for t_objects



--- TABLE t_modules @@@@@
BEGIN TRANSACTION;



END TRANSACTION; --- for t_modules


-- end dump of mom_global by dumpsqlmonimelt
-- monimelt-dump-state end dump mom_global
