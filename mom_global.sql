-- generated MONIMELT dump mom_global.sql ** DONT EDIT
PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE t_objects
 (ob_id VARCHAR(30) PRIMARY KEY ASC NOT NULL UNIQUE,
  ob_mtim REAL NOT NULL,
  ob_content TEXT NOT NULL,
  ob_paylkind VARCHAR(30) NOT NULL,
  ob_paylinit TEXT NOT NULL,
  ob_paylcontent TEXT NOT NULL);
INSERT INTO "t_objects" VALUES('_7QeFDN33m7B_3dQv2OqqbH6',1493382726.98,'

','','','');
INSERT INTO "t_objects" VALUES('_2mYaTh9kH4I_7ENiXcymRmy',1493382726.98,'

','','','');
INSERT INTO "t_objects" VALUES('_1jJjA6LcXiX_1V4ZcXlje09',1493382726.98,'

','','','');
CREATE TABLE t_names
 (nam_oid VARCHAR(30) NOT NULL UNIQUE,
  nam_str TEXT PRIMARY KEY ASC NOT NULL UNIQUE);
CREATE TABLE t_globdata
 (glob_namestr VARCHAR(80) NOT NULL UNIQUE,
  glob_oid  VARCHAR(30) NOT NULL);
COMMIT;
