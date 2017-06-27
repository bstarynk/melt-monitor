#! /bin/bash
##   Copyright (C) 2016 - 2017  Basile Starynkevitch and later the FSF
##   MONIMELT is a monitor for MELT - see http://gcc-melt.org/
##   This file is part of GCC.
## 
##   GCC is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License as published by
##   the Free Software Foundation; either version 3, or (at your option)
##   any later version.
## 
##   GCC is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##   You should have received a copy of the GNU General Public License
##   along with GCC; see the file COPYING3.   If not see
##   <http://www.gnu.org/licenses/>.

## Dont change the name monimelt-dump-state.sh of this script without
## care, it appears elsewhere...

## Keep this in sync with the MomDumper::initialize_db function in state.cc

echo start $0 "$@" >&2
logger --id=$$ -s -t $0 starting at $(date +%c)  "$@"
dbfile=$1
sqlfile=$2
dubase=$3
sqlref=$4

if [ ! -f "$dbfile" ]; then
    echo "$0": missing database file "$dbfile" >&2
    exit 1
fi

if file "$dbfile" | grep -qi SQLite ; then
    echo "$0:" dumping Monimelt Sqlite database $dbfile >&2
else
    echo "$0:" bad database file "$dbfile" >&2
    exit 1
fi

if [ -z "$sqlfile" ]; then
    sqlfile=$(basename $dbfile .sqlite).sql
fi

if [ -z "$dubase" ]; then
    dubase=$(basename $dbfile .sqlite)
fi
logger --id=$$ -s -t $0 dbfile: "$dbfile" sqlfile: "$sqlfile" dubase: "$dubase" sqlref: "$sqlref"

tempdump=$(basename $(tempfile -d . -p _tmp_$(basename $dubase) -s .sql))
trap 'rm -f $tempdump' EXIT INT QUIT TERM
export LANG=C LC_ALL=C

sqlbase=$(basename "$sqlfile" .sql)
# generate an initial comment, it should be at least 128 bytes
date -r "$dbfile" +"-- dump %Y %b %d from $dubase dumped by $(basename $0) ....." > $tempdump
echo >> $tempdump
date +' --   Copyright (C) %Y Free Software Foundation, Inc.' >> $tempdump
echo ' --  MONIMELT is a monitor for MELT - see http://gcc-melt.org/' >> $tempdump
echo " --  This sqlite3 dump file $dubase.sql is part of GCC." >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is free software; you can redistribute it and/or modify' >> $tempdump
echo ' --  it under the terms of the GNU General Public License as published by' >> $tempdump
echo ' --  the Free Software Foundation; either version 3, or (at your option)' >> $tempdump
echo ' --  any later version.' >> $tempdump
echo ' --' >> $tempdump
echo ' --  GCC is distributed in the hope that it will be useful,' >> $tempdump
echo ' --  but WITHOUT ANY WARRANTY; without even the implied warranty of' >> $tempdump
echo ' --  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the' >> $tempdump
echo ' --  GNU General Public License for more details.' >> $tempdump
echo ' --  You should have received a copy of the GNU General Public License' >> $tempdump
echo ' --  along with GCC; see the file COPYING3.   If not see' >> $tempdump
echo ' --  <http://www.gnu.org/licenses/>.' >> $tempdump
echo >> $tempdump


## we probably dont want to put the gitcommit in the dump
#(echo -n ' --- monimelt lastgitcommit ' ;  git log --format=oneline --abbrev=12 --abbrev-commit -q  \
#     | head -1 | tr -d '\n\r\f\"' ; echo ' ---' ) >> $tempdump

sqlite3 $dbfile >> $tempdump <<EOF
.print BEGIN TRANSACTION;   --- for schema
.schema
.print END TRANSACTION;     --- for schema
.print
.print ------- TABLE t_globdata @@@@@@
.print BEGIN TRANSACTION; --- for t_globdata
.mode insert t_globdata
SELECT * FROM t_globdata ORDER BY glob_namestr;
.print END TRANSACTION; ---- for t_globdata
.print
.print ------- TABLE t_names @@@@@@
.print BEGIN TRANSACTION; --- for t_names
.mode insert t_names
SELECT * FROM t_names ORDER BY nam_str;
.print END TRANSACTION; ---- for t_names
.print
.print ------- TABLE t_objects @@@@@@
.print BEGIN TRANSACTION; --- for t_objects
.mode insert t_objects
SELECT * FROM t_objects ORDER  BY ob_id;
.print 
.print END TRANSACTION; ---- for t_objects
.print
.print ------- END DUMP @@@@@
.print
EOF
echo  "-- monimelt-dump-state end dump $dubase" >> $tempdump

if [ -z "$sqlref" ]; then
    sqlref="$sqlfile"
fi

if [ -e "$sqlref" ]; then
    # if only the first 128 bytes changed, it is some comment
    if cmp --quiet --ignore-initial 128 "$sqlref" "$tempdump" ; then
	echo $0: unchanged Monimelt Sqlite3 dump "$sqlfile" reference "$sqlref" >&2
	if [ ! -e "$sqlfile" ]; then
	    ln -s -v "$sqlref" "$sqlfile"
	fi
	touch -r "$dbfile" "$sqlfile"
	exit 0
    fi
elif [ -e "$sqlfile" ]; then
    echo -n "backup Monimelt Sqlite3 ref dump:" >&2
    mv -v --backup=existing "$sqlfile" "$sqlfile~" >&2    
fi
## we need that the .sql file has the same date as the .sqlite file
mv "$tempdump" "$sqlfile"
touch -r "$dbfile" "$sqlfile"
#eof monimelt-dump-state.sh
