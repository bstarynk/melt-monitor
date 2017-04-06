// file parsemit.cc - our parser and emitter

/**   Copyright (C)  2017  Basile Starynkevitch and later the FSF
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

MomValue
MomParser::parse_value(bool *pgotval)
{
  auto inipos = _painp.tellg();
  if (_painp.eof()) goto failure;
failure:
  if (pgotval)
    *pgotval = false;
  _painp.seekg(inipos);
  return nullptr;
} // end of MomParser::parse_value


void
MomEmitter::emit_value(const MomValue v, int depth)
{
} // end of MomEmitter::emit_value
