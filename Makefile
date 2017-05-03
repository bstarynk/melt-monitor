##
## file Makefile
##   Copyright (C)  2015 - 2017 Basile Starynkevitch (and FSF later)
##  MONIMELT is a monitor for MELT - see http://gcc-melt.org/
##  This file is part of GCC.
##
##  GCC is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 3, or (at your option)
##  any later version.
##
##  GCC is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##  You should have received a copy of the GNU General Public License
##  along with GCC; see the file COPYING3.   If not see
##  <http://www.gnu.org/licenses/>.
################################################################
## onion is not packaged, see https://github.com/davidmoreno/onion
CC=gcc
CXX=g++
WARNFLAGS= -Wall -Wextra -fdiagnostics-color=auto
CXXSTDFLAGS= -std=c++17
CFLAGS= -std=gnu11 $(WARNFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
CXXFLAGS= $(CXXSTDFLAGS) $(WARNFLAGS) $(PREPROFLAGS) $(OPTIMFLAGS)
INDENT= indent
ASTYLE= astyle
MD5SUM= md5sum
INDENTFLAGS= --gnu-style --no-tabs --honour-newlines
ASTYLEFLAGS= --style=gnu -s2
PACKAGES= sqlite_modern_cpp glib-2.0 sqlite3 jansson Qt5Gui
PKGCONFIG= pkg-config
PREPROFLAGS= -I. -I/usr/local/include $(shell $(PKGCONFIG) --cflags $(PACKAGES))
OPTIMFLAGS= -Og -g3
SQLITE3=sqlite3
QTMOC= moc

LIBES= -L/usr/local/lib $(shell $(PKGCONFIG) --libs $(PACKAGES)) \
	$(shell $(CXX) -print-file-name=libbacktrace.a) \
        -lpthread -lcrypt -lm -ldl

PLUGIN_SOURCES= $(sort $(wildcard momplug_*.c momplug_*.cc))
PLUGINS=  $(patsubst %.c,%.so,$(PLUGIN_SOURCES))
# modules are generated inside modules/
MODULE_SOURCES= $(sort $(wildcard modules/momg_*.cc))
# generated headers
GENERATED_HEADERS= $(sort $(wildcard _mom*.h))
MODULES=  $(patsubst %.cc,%.so,$(MODULE_SOURCES))
CSOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.c)))
CXXSOURCES= $(sort $(filter-out $(PLUGIN_SOURCES), $(wildcard [a-z]*.cc)))
OBJECTS= $(patsubst %.c,%.o,$(CSOURCES))  $(patsubst %.cc,%.o,$(CXXSOURCES)) 
RM= rm -fv
.PHONY: all checkgithooks installgithooks dumpstate restorestate tags modules plugins clean tests 
all: checkgithooks monimelt


clean:
	$(RM) *~ *% *.o *.so */*.so *.log */*~ */*.orig *.i *.orig *.gch README.html
	$(RM) modules/*.so modules/*~ modules/*%
	$(RM) _timestamp*
	$(RM) core*
	$(RM) *memo*

checkgithooks:
	@for hf in *-githook.sh ; do \
	  [ ! -d .git -o -L .git/hooks/$$(basename $$hf "-githook.sh") ] \
	    || (echo uninstalled git hook $$hf "(run: make installgithooks)" >&2 ; exit 1) ; \
	done
installgithooks:
	for hf in *-githook.sh ; do \
	  ln -sv  "../../$$hf" .git/hooks/$$(basename $$hf "-githook.sh") ; \
	done


## we could use git rev-parse HEAD for the lastgitcommit, but it does
## not give any log comment... Notice that tr command is interpreting
## some backslash escapes itself

_timestamp.c: Makefile
	@date +'const char monimelt_timestamp[]="%c";' > _timestamp.tmp
	@(echo -n 'const char monimelt_lastgitcommit[]="' ; \
	   git log --format=oneline --abbrev=12 --abbrev-commit -q  \
	     | head -1 | tr -d '\n\r\f\"\\\\' ; \
	   echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_lastgittag[]="'; (git describe --abbrev=0 --all || echo '*notag*') | tr -d '\n\r\f\"\\\\'; echo '";') >> _timestamp.tmp
	@(echo 'const char monimelt_compilercommand[]="$(strip $(CC))";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerversion[]="' ; $(CC) -v < /dev/null 2>&1 | grep -i version | tr -d  '\n\r\f\"\\\\' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_compilerflags[]="' ; echo -n "$(strip $(CFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_optimflags[]="' ; echo -n "$(strip $(OPTIMFLAGS))" | sed 's:":\\":g' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_checksum[]="'; cat meltmoni.hh $(GENERATED_HEADERS) $(SOURCES) | $(MD5SUM) | cut -d' ' -f1 | tr -d '\n\r\f\"\\' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_directory[]="'; /bin/pwd | tr -d '\n\\"' ; echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_makefile[]="'; echo -n  $(realpath $(lastword $(MAKEFILE_LIST))); echo '";') >> _timestamp.tmp
	@(echo -n 'const char monimelt_sqliteprog[]="'; echo -n  $(shell which $(SQLITE3)); echo '";') >> _timestamp.tmp
	@mv _timestamp.tmp _timestamp.c

$(OBJECTS): meltmoni.hh.gch 

meltmoni.hh.gch: meltmoni.hh $(GENERATED_HEADERS)
	$(COMPILE.cc) $(CXXFLAGS) -c $< -o $@

%.o: %.qcc  meltmoni.hh.gch %.moc.h
	$(COMPILE.cc) $(CXXFLAGS) -c $< -o $@

.SUFFIXES: .qcc

%.moc.h: %.qcc  meltmoni.hh $(GENERATED_HEADERS)
	$(QTMOC) $(PREPROFLAGS) -o $@ $<

monimelt: $(OBJECTS) 
	@if [ -f $@ ]; then echo -n backup old executable: ' ' ; mv -v $@ $@~ ; fi
	$(MAKE) _timestamp.c _timestamp.o
	$(LINK.cc)  $(LINKFLAGS) -rdynamic $(OBJECTS) $(LIBES) -o $@  _timestamp.o
	rm _timestamp.*

indent: .indent.pro
	cp -v meltmoni.hh meltmoni.hh%
	$(ASTYLE) $(ASTYLEFLAGS) meltmoni.hh
	for f in $(wildcard [a-z]*.c) ; do \
	  echo indenting $$f ; cp $$f $$f%; \
          $(INDENT) $(INDENTFLAGS) $$f ; $(INDENT)  $(INDENTFLAGS) $$f; \
        done
	for g in $(wildcard [a-z]*.cc) ; do \
	  echo astyling $$g ; cp $$g $$g% ; \
	  $(ASTYLE)  $(ASTYLEFLAGS) $$g ; \
	done

dumpstate:
	echo "-- generated MONIMELT dump mom_global.sql ** DONT EDIT" > mom_global.sql-tmp
	$(SQLITE3) mom_global.sqlite .dump >> mom_global.sql-tmp
	mv --backup  mom_global.sql-tmp mom_global.sql
	touch -r mom_global.sqlite mom_global.sql
	echo "-- generated MONIMELT dump mom_user.sql ** DONT EDIT" > mom_user.sql-tmp
	$(SQLITE3) mom_user.sqlite .dump >> mom_user.sql-tmp
	mv --backup  mom_user.sql-tmp mom_user.sql
	touch -r mom_user.sqlite mom_user.sql

restorestate: |  mom_global.sql  mom_user.sql
	@if [ -f mom_global.sqlite ]; then \
	  echo -n makebackup global old: ' ' ; mv -v --backup mom_global.sqlite  mom_global.sqlite~ ; fi
	$(SQLITE3) mom_global.sqlite < mom_global.sql
	touch -r  mom_global.sql  mom_global.sqlite
	@if [ -f mom_user.sqlite ]; then \
	  echo -n makebackup user old: ' ' ; mv -v --backup mom_user.sqlite  mom_user.sqlite~ ; fi
	$(SQLITE3) mom_user.sqlite < mom_user.sql
	touch -r  mom_user.sql  mom_user.sqlite


modules/momg_%.so: modules/momg_%.cc $(OBJECTS)
	$(LINK.cc) -fPIC -shared $< -o $@
