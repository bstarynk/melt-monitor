-- dump 2017 Jul 17 from mom_user dumped by monimelt-dump-state.sh .....

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


-- dump of mom_user sqlite db by dumpsqlmonimelt

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
INSERT INTO t_globdata VALUES ('anon1', '_5duwaaKJiRS_6cHHv0uehF0');
END TRANSACTION; --- for t_globdata

--- TABLE t_names @@@@@
BEGIN TRANSACTION;
INSERT INTO t_names VALUES ('_0h6QnlLjipB_7LjgRYcUBIx', 'a');
INSERT INTO t_names VALUES ('_1YO5QQwWD9S_648ErV7JiiJ', 'anon1');
INSERT INTO t_names VALUES ('_0uAMtAmkP34_1BhIutSjPN9', 'b');
INSERT INTO t_names VALUES ('_0UsE4ep7mY5_3M6VTOn9Doe', 'c');
INSERT INTO t_names VALUES ('_0V1qVtBdVnE_750Mr6HYEv2', 'd');
INSERT INTO t_names VALUES ('_0y2wxoWfbtR_59BsZol6oh5', 'e');
INSERT INTO t_names VALUES ('_13yF7zZqeUk_9vIvcjYxRqU', 'f');
INSERT INTO t_names VALUES ('_1KrQCKNTtjo_7AENFWiIXrw', 'g');
INSERT INTO t_names VALUES ('_1LYPLvcDaRx_611Ehpk7V9P', 'h');
INSERT INTO t_names VALUES ('_1SRZhQbxMWF_6NS7QIxMMN4', 'i');
INSERT INTO t_names VALUES ('_1tiNOVWIpjD_7kYkIXbupOP', 'j');
INSERT INTO t_names VALUES ('_1TQ4EET0Ael_7wCii1rLlOv', 'k');
INSERT INTO t_names VALUES ('_1xJPrAKgHGd_0fauqP3xu8Y', 'l');
INSERT INTO t_names VALUES ('_1xZpzZjza3j_58X4y4ldLP3', 'm');
INSERT INTO t_names VALUES ('_2Ed5a6FdKs2_6a9rUmfSKos', 'n');
INSERT INTO t_names VALUES ('_2fFXGqCn31C_3L7NsvxOgjQ', 'o');
INSERT INTO t_names VALUES ('_2YeTrcUT1ig_131J9mkS8sh', 'p');
INSERT INTO t_names VALUES ('_3ojMJ5jyD6s_3i5kM0DDFDT', 'q');
INSERT INTO t_names VALUES ('_3r6zyliGEa1_4VuvVlsYGiH', 'r');
INSERT INTO t_names VALUES ('_3xRwKNR8Txl_0wsckeKslWS', 's');
INSERT INTO t_names VALUES ('_40qshq4y5x8_7gVSxYkkZS7', 't');
INSERT INTO t_names VALUES ('_1su4dxploe6_3OEeK9QS68b', 'test1gen');
INSERT INTO t_names VALUES ('_4aAhx0sQzAr_9XhL9OAq5TM', 'u');
INSERT INTO t_names VALUES ('_4ioux9ZO1CL_1rL5zUwbrDz', 'v');
INSERT INTO t_names VALUES ('_4jWlc9OU9d7_7kWpO3U1cmr', 'w');
INSERT INTO t_names VALUES ('_5rqDmMLjBIY_2r8FY6iLxv4', 'x');
INSERT INTO t_names VALUES ('_5zqKICoAIK2_7LvTvNmgr8r', 'y');
INSERT INTO t_names VALUES ('_60mTYUA9RUQ_2NnbVf206gi', 'z');
END TRANSACTION; --- for t_names


--- TABLE t_objects @@@@@
BEGIN TRANSACTION;

INSERT INTO t_objects VALUES('_0UsE4ep7mY5_3M6VTOn9Doe', 1497676631.12,
'
///$c

',
'named', --- payl _0UsE4ep7mY5_3M6VTOn9Doe
'c',
'',
'');--'--
------'** end _0UsE4ep7mY5_3M6VTOn9Doe


INSERT INTO t_objects VALUES('_0V1qVtBdVnE_750Mr6HYEv2', 1497676631.13,
'
///$d

',
'named', --- payl _0V1qVtBdVnE_750Mr6HYEv2
'd',
'',
'');--'--
------'** end _0V1qVtBdVnE_750Mr6HYEv2


INSERT INTO t_objects VALUES('_0h6QnlLjipB_7LjgRYcUBIx', 1497676631.1,
'
///$a

',
'named', --- payl _0h6QnlLjipB_7LjgRYcUBIx
'a',
'',
'');--'--
------'** end _0h6QnlLjipB_7LjgRYcUBIx


INSERT INTO t_objects VALUES('_0uAMtAmkP34_1BhIutSjPN9', 1497676631.11,
'
///$b

',
'named', --- payl _0uAMtAmkP34_1BhIutSjPN9
'b',
'',
'');--'--
------'** end _0uAMtAmkP34_1BhIutSjPN9


INSERT INTO t_objects VALUES('_0y2wxoWfbtR_59BsZol6oh5', 1497676631.14,
'
///$e

',
'named', --- payl _0y2wxoWfbtR_59BsZol6oh5
'e',
'',
'');--'--
------'** end _0y2wxoWfbtR_59BsZol6oh5


INSERT INTO t_objects VALUES('_13yF7zZqeUk_9vIvcjYxRqU', 1497676631.15,
'
///$f

',
'named', --- payl _13yF7zZqeUk_9vIvcjYxRqU
'f',
'',
'');--'--
------'** end _13yF7zZqeUk_9vIvcjYxRqU


INSERT INTO t_objects VALUES('_1KrQCKNTtjo_7AENFWiIXrw', 1497676631.16,
'
///$g

',
'named', --- payl _1KrQCKNTtjo_7AENFWiIXrw
'g',
'',
'');--'--
------'** end _1KrQCKNTtjo_7AENFWiIXrw


INSERT INTO t_objects VALUES('_1LYPLvcDaRx_611Ehpk7V9P', 1497676631.17,
'
///$h

',
'named', --- payl _1LYPLvcDaRx_611Ehpk7V9P
'h',
'',
'');--'--
------'** end _1LYPLvcDaRx_611Ehpk7V9P


INSERT INTO t_objects VALUES('_1SRZhQbxMWF_6NS7QIxMMN4', 1497676631.18,
'
///$i

',
'named', --- payl _1SRZhQbxMWF_6NS7QIxMMN4
'i',
'',
'');--'--
------'** end _1SRZhQbxMWF_6NS7QIxMMN4


INSERT INTO t_objects VALUES('_1TQ4EET0Ael_7wCii1rLlOv', 1497676631.2,
'
///$k

',
'named', --- payl _1TQ4EET0Ael_7wCii1rLlOv
'k',
'',
'');--'--
------'** end _1TQ4EET0Ael_7wCii1rLlOv


INSERT INTO t_objects VALUES('_1TaJYE9mf5h_41UzXi1vTBb', 1500301533.05,
'
@: _3xRwKNR8Txl_0wsckeKslWS " the code proxy of test1gen"

',
'code', --- payl _1TaJYE9mf5h_41UzXi1vTBb
'@CODEBASE: test1generator @CODEUPDATE!
',
'',
'');--'--
------'** end _1TaJYE9mf5h_41UzXi1vTBb


INSERT INTO t_objects VALUES('_1YO5QQwWD9S_648ErV7JiiJ', 1499315419.12,
'
///$anon1

',
'named', --- payl _1YO5QQwWD9S_648ErV7JiiJ
'anon1',
'',
'');--'--
------'** end _1YO5QQwWD9S_648ErV7JiiJ


INSERT INTO t_objects VALUES('_1su4dxploe6_3OEeK9QS68b', 1499689282.34,
'
///$test1gen
@: _3xRwKNR8Txl_0wsckeKslWS _4SCyZfyLX4u_38OYdt3f0wJ

',
'named', --- payl _1su4dxploe6_3OEeK9QS68b
'test1gen',
'',
'_4SCyZfyLX4u_38OYdt3f0wJ');--'--
------'** end _1su4dxploe6_3OEeK9QS68b


INSERT INTO t_objects VALUES('_1tiNOVWIpjD_7kYkIXbupOP', 1497676631.19,
'
///$j

',
'named', --- payl _1tiNOVWIpjD_7kYkIXbupOP
'j',
'',
'');--'--
------'** end _1tiNOVWIpjD_7kYkIXbupOP


INSERT INTO t_objects VALUES('_1xJPrAKgHGd_0fauqP3xu8Y', 1497676631.21,
'
///$l

',
'named', --- payl _1xJPrAKgHGd_0fauqP3xu8Y
'l',
'',
'');--'--
------'** end _1xJPrAKgHGd_0fauqP3xu8Y


INSERT INTO t_objects VALUES('_1xZpzZjza3j_58X4y4ldLP3', 1497676631.22,
'
///$m

',
'named', --- payl _1xZpzZjza3j_58X4y4ldLP3
'm',
'',
'');--'--
------'** end _1xZpzZjza3j_58X4y4ldLP3


INSERT INTO t_objects VALUES('_2Ed5a6FdKs2_6a9rUmfSKos', 1497676631.23,
'
///$n

',
'named', --- payl _2Ed5a6FdKs2_6a9rUmfSKos
'n',
'',
'');--'--
------'** end _2Ed5a6FdKs2_6a9rUmfSKos


INSERT INTO t_objects VALUES('_2YeTrcUT1ig_131J9mkS8sh', 1497676631.25,
'
///$p

',
'named', --- payl _2YeTrcUT1ig_131J9mkS8sh
'p',
'',
'');--'--
------'** end _2YeTrcUT1ig_131J9mkS8sh


INSERT INTO t_objects VALUES('_2fFXGqCn31C_3L7NsvxOgjQ', 1497676631.24,
'
///$o

',
'named', --- payl _2fFXGqCn31C_3L7NsvxOgjQ
'o',
'',
'');--'--
------'** end _2fFXGqCn31C_3L7NsvxOgjQ


INSERT INTO t_objects VALUES('_3iDyCLgx9oR_6tNgi2Tey82', 1498988862.1,
'

',
'', '', '', '' -- nopayl
);--'--
------'** end _3iDyCLgx9oR_6tNgi2Tey82


INSERT INTO t_objects VALUES('_3ojMJ5jyD6s_3i5kM0DDFDT', 1497676631.26,
'
///$q

',
'named', --- payl _3ojMJ5jyD6s_3i5kM0DDFDT
'q',
'',
'');--'--
------'** end _3ojMJ5jyD6s_3i5kM0DDFDT


INSERT INTO t_objects VALUES('_3r6zyliGEa1_4VuvVlsYGiH', 1497676631.27,
'
///$r

',
'named', --- payl _3r6zyliGEa1_4VuvVlsYGiH
'r',
'',
'');--'--
------'** end _3r6zyliGEa1_4VuvVlsYGiH


INSERT INTO t_objects VALUES('_3xRwKNR8Txl_0wsckeKslWS', 1497676631.28,
'
///$s

',
'named', --- payl _3xRwKNR8Txl_0wsckeKslWS
's',
'',
'');--'--
------'** end _3xRwKNR8Txl_0wsckeKslWS


INSERT INTO t_objects VALUES('_40qshq4y5x8_7gVSxYkkZS7', 1497676631.29,
'
///$t

',
'named', --- payl _40qshq4y5x8_7gVSxYkkZS7
't',
'',
'');--'--
------'** end _40qshq4y5x8_7gVSxYkkZS7


INSERT INTO t_objects VALUES('_4SCyZfyLX4u_38OYdt3f0wJ', 1499690053.01,
'
@: _3xRwKNR8Txl_0wsckeKslWS "the genfile proxy of test1gen"

',
'genfile', --- payl _4SCyZfyLX4u_38OYdt3f0wJ
'_test1gen.out',
'',
'_1TaJYE9mf5h_41UzXi1vTBb');--'--
------'** end _4SCyZfyLX4u_38OYdt3f0wJ


INSERT INTO t_objects VALUES('_4aAhx0sQzAr_9XhL9OAq5TM', 1497676631.3,
'
///$u

',
'named', --- payl _4aAhx0sQzAr_9XhL9OAq5TM
'u',
'',
'');--'--
------'** end _4aAhx0sQzAr_9XhL9OAq5TM


INSERT INTO t_objects VALUES('_4ioux9ZO1CL_1rL5zUwbrDz', 1497676631.31,
'
///$v

',
'named', --- payl _4ioux9ZO1CL_1rL5zUwbrDz
'v',
'',
'');--'--
------'** end _4ioux9ZO1CL_1rL5zUwbrDz


INSERT INTO t_objects VALUES('_4jWlc9OU9d7_7kWpO3U1cmr', 1497676631.32,
'
///$w

',
'named', --- payl _4jWlc9OU9d7_7kWpO3U1cmr
'w',
'',
'');--'--
------'** end _4jWlc9OU9d7_7kWpO3U1cmr


INSERT INTO t_objects VALUES('_5duwaaKJiRS_6cHHv0uehF0', 1500010711.46,
'
@: _0vgCFjXblkx_4zCMhMAWjVK 12345
@: _2mYaTh9kH4I_7ENiXcymRmy {_0h6QnlLjipB_7LjgRYcUBIx _0uAMtAmkP34_1BhIutSjPN9 _0y2wxoWfbtR_59BsZol6oh5
 _0UsE4ep7mY5_3M6VTOn9Doe _0V1qVtBdVnE_750Mr6HYEv2 _13yF7zZqeUk_9vIvcjYxRqU _1tiNOVWIpjD_7kYkIXbupOP
 _1xJPrAKgHGd_0fauqP3xu8Y _1xZpzZjza3j_58X4y4ldLP3 _1KrQCKNTtjo_7AENFWiIXrw _1LYPLvcDaRx_611Ehpk7V9P
 _1SRZhQbxMWF_6NS7QIxMMN4 _1TQ4EET0Ael_7wCii1rLlOv _2fFXGqCn31C_3L7NsvxOgjQ _2Ed5a6FdKs2_6a9rUmfSKos
 _2YeTrcUT1ig_131J9mkS8sh _3ojMJ5jyD6s_3i5kM0DDFDT _3r6zyliGEa1_4VuvVlsYGiH _3xRwKNR8Txl_0wsckeKslWS
 _40qshq4y5x8_7gVSxYkkZS7 _4aAhx0sQzAr_9XhL9OAq5TM _4ioux9ZO1CL_1rL5zUwbrDz _4jWlc9OU9d7_7kWpO3U1cmr
 _5rqDmMLjBIY_2r8FY6iLxv4 _5zqKICoAIK2_7LvTvNmgr8r _60mTYUA9RUQ_2NnbVf206gi}
@: _2Ed5a6FdKs2_6a9rUmfSKos *_5rqDmMLjBIY_2r8FY6iLxv4(1 2 3 *_5zqKICoAIK2_7LvTvNmgr8r(_0h6QnlLjipB_7LjgRYcUBIx
  [_0uAMtAmkP34_1BhIutSjPN9 _0UsE4ep7mY5_3M6VTOn9Doe] _0V1qVtBdVnE_750Mr6HYEv2 "efg") _5zqKICoAIK2_7LvTvNmgr8r)
@: _4ioux9ZO1CL_1rL5zUwbrDz _60mTYUA9RUQ_2NnbVf206gi
@: _4jWlc9OU9d7_7kWpO3U1cmr [_268RKjuQT02_2Kh0wgKnhLT]
@: _5rqDmMLjBIY_2r8FY6iLxv4 _3iDyCLgx9oR_6tNgi2Tey82
@: _7T9OwSFlgov_0wVJaK1eZbn _5zqKICoAIK2_7LvTvNmgr8r
&: "some-string"
&: _1YO5QQwWD9S_648ErV7JiiJ
&: (#1 2 3#)
&: (:3.14159 2.71828:)
&: *_7D8xcWnEiys_8oqOVSkCxkA(_7QeFDN33m7B_3dQv2OqqbH6 "node-child" *_0V1qVtBdVnE_750Mr6HYEv2(
  _4ioux9ZO1CL_1rL5zUwbrDz [_3ojMJ5jyD6s_3i5kM0DDFDT _3r6zyliGEa1_4VuvVlsYGiH] "BLACK HEART SUIT is \u2665"
  {_0h6QnlLjipB_7LjgRYcUBIx _0uAMtAmkP34_1BhIutSjPN9} *_3r6zyliGEa1_4VuvVlsYGiH("inside" 
   "HAMMER \u2692 PICK")))
&: _1su4dxploe6_3OEeK9QS68b

',
'', '', '', '' -- nopayl
);--'--
------'** end _5duwaaKJiRS_6cHHv0uehF0


INSERT INTO t_objects VALUES('_5rqDmMLjBIY_2r8FY6iLxv4', 1497676631.33,
'
///$x

',
'named', --- payl _5rqDmMLjBIY_2r8FY6iLxv4
'x',
'',
'');--'--
------'** end _5rqDmMLjBIY_2r8FY6iLxv4


INSERT INTO t_objects VALUES('_5zqKICoAIK2_7LvTvNmgr8r', 1497676631.34,
'
///$y

',
'named', --- payl _5zqKICoAIK2_7LvTvNmgr8r
'y',
'',
'');--'--
------'** end _5zqKICoAIK2_7LvTvNmgr8r


INSERT INTO t_objects VALUES('_60mTYUA9RUQ_2NnbVf206gi', 1497676631.35,
'
///$z

',
'named', --- payl _60mTYUA9RUQ_2NnbVf206gi
'z',
'',
'');--'--
------'** end _60mTYUA9RUQ_2NnbVf206gi




END TRANSACTION; --- for t_objects



--- TABLE t_modules @@@@@
BEGIN TRANSACTION;



END TRANSACTION; --- for t_modules


-- end dump of mom_user by dumpsqlmonimelt
-- monimelt-dump-state end dump mom_user
