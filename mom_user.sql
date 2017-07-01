-- dump 2017 Jul 01 from mom_user dumped by monimelt-dump-state.sh .....

 --   Copyright (C) 2017 Free Software Foundation, Inc.
 --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
 --  This sqlite3 dump file mom_user.sql is part of GCC.
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
INSERT INTO t_globdata VALUES('anon1','_5duwaaKJiRS_6cHHv0uehF0');
END TRANSACTION; ---- for t_globdata

------- TABLE t_names @@@@@@
BEGIN TRANSACTION; --- for t_names
INSERT INTO t_names VALUES('_0h6QnlLjipB_7LjgRYcUBIx','a');
INSERT INTO t_names VALUES('_0uAMtAmkP34_1BhIutSjPN9','b');
INSERT INTO t_names VALUES('_0UsE4ep7mY5_3M6VTOn9Doe','c');
INSERT INTO t_names VALUES('_0V1qVtBdVnE_750Mr6HYEv2','d');
INSERT INTO t_names VALUES('_0y2wxoWfbtR_59BsZol6oh5','e');
INSERT INTO t_names VALUES('_13yF7zZqeUk_9vIvcjYxRqU','f');
INSERT INTO t_names VALUES('_1KrQCKNTtjo_7AENFWiIXrw','g');
INSERT INTO t_names VALUES('_1LYPLvcDaRx_611Ehpk7V9P','h');
INSERT INTO t_names VALUES('_1SRZhQbxMWF_6NS7QIxMMN4','i');
INSERT INTO t_names VALUES('_1tiNOVWIpjD_7kYkIXbupOP','j');
INSERT INTO t_names VALUES('_1TQ4EET0Ael_7wCii1rLlOv','k');
INSERT INTO t_names VALUES('_1xJPrAKgHGd_0fauqP3xu8Y','l');
INSERT INTO t_names VALUES('_1xZpzZjza3j_58X4y4ldLP3','m');
INSERT INTO t_names VALUES('_2Ed5a6FdKs2_6a9rUmfSKos','n');
INSERT INTO t_names VALUES('_2fFXGqCn31C_3L7NsvxOgjQ','o');
INSERT INTO t_names VALUES('_2YeTrcUT1ig_131J9mkS8sh','p');
INSERT INTO t_names VALUES('_3ojMJ5jyD6s_3i5kM0DDFDT','q');
INSERT INTO t_names VALUES('_3r6zyliGEa1_4VuvVlsYGiH','r');
INSERT INTO t_names VALUES('_3xRwKNR8Txl_0wsckeKslWS','s');
INSERT INTO t_names VALUES('_40qshq4y5x8_7gVSxYkkZS7','t');
INSERT INTO t_names VALUES('_4aAhx0sQzAr_9XhL9OAq5TM','u');
INSERT INTO t_names VALUES('_4ioux9ZO1CL_1rL5zUwbrDz','v');
INSERT INTO t_names VALUES('_4jWlc9OU9d7_7kWpO3U1cmr','w');
INSERT INTO t_names VALUES('_5rqDmMLjBIY_2r8FY6iLxv4','x');
INSERT INTO t_names VALUES('_5zqKICoAIK2_7LvTvNmgr8r','y');
INSERT INTO t_names VALUES('_60mTYUA9RUQ_2NnbVf206gi','z');
END TRANSACTION; ---- for t_names

------- TABLE t_objects @@@@@@
BEGIN TRANSACTION; --- for t_objects
INSERT INTO t_objects VALUES('_0UsE4ep7mY5_3M6VTOn9Doe',1497676631.12,'
///$c

','named','c','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_0V1qVtBdVnE_750Mr6HYEv2',1497676631.13,'
///$d

','named','d','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_0h6QnlLjipB_7LjgRYcUBIx',1497676631.1,'
///$a

','named','a','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_0uAMtAmkP34_1BhIutSjPN9',1497676631.11,'
///$b

','named','b','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_0y2wxoWfbtR_59BsZol6oh5',1497676631.14,'
///$e

','named','e','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_13yF7zZqeUk_9vIvcjYxRqU',1497676631.15,'
///$f

','named','f','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1KrQCKNTtjo_7AENFWiIXrw',1497676631.16,'
///$g

','named','g','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1LYPLvcDaRx_611Ehpk7V9P',1497676631.17,'
///$h

','named','h','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1SRZhQbxMWF_6NS7QIxMMN4',1497676631.18,'
///$i

','named','i','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1TQ4EET0Ael_7wCii1rLlOv',1497676631.2,'
///$k

','named','k','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1tiNOVWIpjD_7kYkIXbupOP',1497676631.19,'
///$j

','named','j','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1xJPrAKgHGd_0fauqP3xu8Y',1497676631.21,'
///$l

','named','l','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_1xZpzZjza3j_58X4y4ldLP3',1497676631.22,'
///$m

','named','m','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_2Ed5a6FdKs2_6a9rUmfSKos',1497676631.23,'
///$n

','named','n','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_2YeTrcUT1ig_131J9mkS8sh',1497676631.25,'
///$p

','named','p','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_2fFXGqCn31C_3L7NsvxOgjQ',1497676631.24,'
///$o

','named','o','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3ojMJ5jyD6s_3i5kM0DDFDT',1497676631.26,'
///$q

','named','q','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3r6zyliGEa1_4VuvVlsYGiH',1497676631.27,'
///$r

','named','r','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_3xRwKNR8Txl_0wsckeKslWS',1497676631.28,'
///$s

','named','s','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_40qshq4y5x8_7gVSxYkkZS7',1497676631.29,'
///$t

','named','t','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4aAhx0sQzAr_9XhL9OAq5TM',1497676631.3,'
///$u

','named','u','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4ioux9ZO1CL_1rL5zUwbrDz',1497676631.31,'
///$v

','named','v','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_4jWlc9OU9d7_7kWpO3U1cmr',1497676631.32,'
///$w

','named','w','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_5duwaaKJiRS_6cHHv0uehF0',1498798000.0,'
@: _0vgCFjXblkx_4zCMhMAWjVK 12345
@: _2mYaTh9kH4I_7ENiXcymRmy {_0h6QnlLjipB_7LjgRYcUBIx _0uAMtAmkP34_1BhIutSjPN9 _0y2wxoWfbtR_59BsZol6oh5
 _0UsE4ep7mY5_3M6VTOn9Doe _0V1qVtBdVnE_750Mr6HYEv2 _13yF7zZqeUk_9vIvcjYxRqU _1tiNOVWIpjD_7kYkIXbupOP
 _1xJPrAKgHGd_0fauqP3xu8Y _1xZpzZjza3j_58X4y4ldLP3 _1KrQCKNTtjo_7AENFWiIXrw _1LYPLvcDaRx_611Ehpk7V9P
 _1SRZhQbxMWF_6NS7QIxMMN4 _1TQ4EET0Ael_7wCii1rLlOv _2fFXGqCn31C_3L7NsvxOgjQ _2Ed5a6FdKs2_6a9rUmfSKos
 _2YeTrcUT1ig_131J9mkS8sh _3ojMJ5jyD6s_3i5kM0DDFDT _3r6zyliGEa1_4VuvVlsYGiH _3xRwKNR8Txl_0wsckeKslWS
 _40qshq4y5x8_7gVSxYkkZS7 _4aAhx0sQzAr_9XhL9OAq5TM _4ioux9ZO1CL_1rL5zUwbrDz _4jWlc9OU9d7_7kWpO3U1cmr
 _5rqDmMLjBIY_2r8FY6iLxv4 _5zqKICoAIK2_7LvTvNmgr8r _60mTYUA9RUQ_2NnbVf206gi}
@: _7T9OwSFlgov_0wVJaK1eZbn _5zqKICoAIK2_7LvTvNmgr8r
&: "some-string"
&: (#1 2 3#)
&: (:3.14159 2.71828:)
&: *_7D8xcWnEiys_8oqOVSkCxkA(_7QeFDN33m7B_3dQv2OqqbH6 "node-child" *_0V1qVtBdVnE_750Mr6HYEv2(
  _4ioux9ZO1CL_1rL5zUwbrDz [_3ojMJ5jyD6s_3i5kM0DDFDT _3r6zyliGEa1_4VuvVlsYGiH] {_0h6QnlLjipB_7LjgRYcUBIx
   _0uAMtAmkP34_1BhIutSjPN9} *_3r6zyliGEa1_4VuvVlsYGiH("inside")))

','','','');
INSERT INTO t_objects VALUES('_5rqDmMLjBIY_2r8FY6iLxv4',1497676631.33,'
///$x

','named','x','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_5zqKICoAIK2_7LvTvNmgr8r',1497676631.34,'
///$y

','named','y','@NAMEDPROXY: __');
INSERT INTO t_objects VALUES('_60mTYUA9RUQ_2NnbVf206gi',1497676631.35,'
///$z

','named','z','@NAMEDPROXY: __');

END TRANSACTION; ---- for t_objects

------- END DUMP @@@@@

-- monimelt-dump-state end dump mom_user
