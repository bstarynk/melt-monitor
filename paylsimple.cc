// file paylsimple.cc - simple payloads

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

class MomPaylNamed : public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  typedef std::string stringty;
private:
  const stringty _nam_str;
  MomObject* _nam_proxy;
public:
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_scandump_sig Scandump;
  static MomPyv_emitdump_sig Emitdump;
  static MomPyv_initload_sig Initload;
  static MomPyv_loadfill_sig Loadfill;
  static MomPyv_getmagic_sig Getmagic;
}; // end class MomPaylNamed

extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(named) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylNamed),
  /**   .pyv_name=       */       __BASE_FILE__,
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylNamed::Destroy,
  /**   .pyv_scangc=     */       MomPaylNamed::Scangc,
  /**   .pyv_scandump=   */       MomPaylNamed::Scandump,
  /**   .pyv_emitdump=   */       MomPaylNamed::Emitdump,
  /**   .pyv_initload=   */       MomPaylNamed::Initload,
  /**   .pyv_loadfill=   */       MomPaylNamed::Loadfill,
  /**   .pyv_getmagic=   */       MomPaylNamed::Getmagic,
  /**   .pyv_fetch=      */       nullptr,
  /**   .pyv_update=     */       nullptr,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};

void
MomPaylNamed::Destroy (struct MomPayload*payl,MomObject*own)
{
  auto py = static_cast<MomPaylNamed*>(payl);
  py->_nam_str.~stringty();
  py->_nam_proxy = nullptr;
} // end MomPaylNamed::Destroy


void
MomPaylNamed::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
} // end MomPaylNamed::Scangc


void
MomPaylNamed::Scandump(const struct MomPayload*payl,MomObject*own,MomDumper*du)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
  if (py->_nam_proxy)
    py->_nam_proxy->scan_dump(du);
} // end MomPaylNamed::Scandump


void
MomPaylNamed::Emitdump(const struct MomPayload*payl,MomObject*own,MomDumper*du, MomEmitter*empaylinit, MomEmitter*empaylcont)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
} // end MomPaylNamed::Emitdump


MomPayload*
MomPaylNamed::Initload(MomObject*own,MomLoader*ld,const char*inits)
{
} // end MomPaylNamed::Initload



void  MomPaylNamed::Loadfill(struct MomPayload*payl,MomObject*own,MomLoader*ld,const char*fills)
{
  auto py = static_cast< MomPaylNamed*>(payl);
} // end MomPaylNamed::Loadfill


MomValue
MomPaylNamed::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylNamed*>(payl);
} // end   MomPaylNamed::Getmagic
