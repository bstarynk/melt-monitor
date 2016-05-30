// file state.cc - managing state

/**   Copyright (C)  2015 - 2016  Basile Starynkevitch and later the FSF
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


MomLoader::MomLoader(const char*path)
  : _ld_stack(), _ld_magic(MAGIC), _ld_file(nullptr), _ld_path(),
    _ld_linbuf(nullptr), _ld_linsiz(0), _ld_linlen(0), _ld_lineno(0)
{
  if (!path || !path[0]) MOM_FATAPRINTF("empty path for loader");
  FILE*f = fopen(path,"r");
  if (!f)
    MOM_FATAPRINTF("failed to open path %s for loader (%m)",
                   path);
  _ld_file = f;
  _ld_path = path;
  const int inisiz=256;
  _ld_linbuf = (char*)malloc(inisiz);
  if (MOM_UNLIKELY(_ld_linbuf==nullptr))
    MOM_FATAPRINTF("failed to allocate linebuffer of %d bytes (%m)",
                   inisiz);
  memset (_ld_linbuf, 0, inisiz);
  _ld_linsiz=inisiz;
}

MomLoader::~MomLoader()
{
  if (_ld_magic != MAGIC)
    MOM_FATAPRINTF("corrupted loader @%p", (void*)this);
  if (_ld_linbuf) free(_ld_linbuf), _ld_linbuf=nullptr;
  _ld_linsiz= _ld_linlen=0;
  if (_ld_file)
    fclose(_ld_file), _ld_file=nullptr;
}

void
MomLoader::first_pass()
{
  if (_ld_magic != MAGIC)
    MOM_FATAPRINTF("corrupted loader @%p", (void*)this);
  do
    {
      getline();
      assert (_ld_linbuf != nullptr);
      if (_ld_linbuf[0] == '*' && isalpha(_ld_linbuf[1]))
        {
          const char*pc = _ld_linbuf+1;
          while (isalnum(*pc) || *pc=='_') pc++;
          std::string nam {_ld_linbuf+1, (unsigned long)(pc-(_ld_linbuf+1))};
        }
    }
  while(!feof(_ld_file));
}
