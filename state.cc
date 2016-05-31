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
MomLoader::first_pass(void)
{
  if (_ld_magic != MAGIC)
    MOM_FATAPRINTF("corrupted loader @%p", (void*)this);
  long nbitems = 0;
  do
    {
      getline();
      assert (_ld_linbuf != nullptr);
      if (_ld_linbuf[0] == '*' && isalpha(_ld_linbuf[1]))
        {
          const char*pc = _ld_linbuf+1;
          while (isalnum(*pc) || *pc=='_') pc++;
          std::string nam {_ld_linbuf+1, (unsigned long)(pc-(_ld_linbuf+1))};
          MomITEM*itm = mom_make_item_from_string(nam);
          if (MOM_UNLIKELY(itm==nullptr))
            MOM_FATAPRINTF("failed to make item %s from state file %s line %d",
                           nam.c_str(), _ld_path.c_str(), _ld_lineno);
          _ld_setitems.insert(itm);
	  nbitems++;
        }
    }
  while(!feof(_ld_file));
  MOM_INFORMPRINTF("first pass load of %s made %ld items",
		   _ld_path.c_str(), nbitems);
} // end MomLoader::first_pass


void MomLoader::second_pass(void)
{
  if (_ld_magic != MAGIC)
    MOM_FATAPRINTF("corrupted loader @%p", (void*)this);
  _ld_stack.clear();
  do
    {
      getline();
      assert (_ld_linbuf != nullptr);
      if (_ld_linbuf[0] == '#' || _ld_linbuf[0] == '\n')
	continue;
      char *eol = strchr (_ld_linbuf, '\n');
      if (eol)
        *eol = (char) 0;
      /// 123 is pushing a raw integer
      /// -234_ is pushing a boxed integer
      /// 12.0 is pushing a raw double
      /// -12.3e-5_ is pushing a boxed double
      if (isdigit (_ld_linbuf[0])
          || ((_ld_linbuf[0] == '+' || _ld_linbuf[0] == '-')
	      && isdigit (_ld_linbuf[1])))
        {
          char *end = NULL;
          if (strchr (_ld_linbuf, '.'))
            {
              double x = strtod (_ld_linbuf, &end);
              bool boxed = end && (*end == '_');
	    }
          else
            {
              long long ll = strtoll (_ld_linbuf, &end, 0);
              bool boxed = end && (*end == '_');
	    }
	}
	  
    }
  while(!feof(_ld_file));
      
#warning MomLoader::second_pass unimplemented
} // end MomLoader::second_pass
