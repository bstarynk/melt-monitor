// file webonion.cc - managing web interface with onion library

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

#include <onion/onion.hpp>
#include <onion/response.hpp>
#include <onion/url.hpp>
#include <onion/shortcuts.hpp>

char mom_web_host[80];
int mom_web_port;
void mom_run_web_onion(const char*webopt)
{
  MOM_DEBUGLOG(webonion, "mom_run_web_onion webopt=" << webopt);
  int pos=0;
  if (sscanf(webopt, "%78[A-Za-z0-9.]:%d%n", mom_web_host, &mom_web_port, &pos)>=2 && pos>0)
    {
      MOM_DEBUGLOG(webonion, "mom_run_web_onion webhost " << mom_web_host
                   << " webport=" << mom_web_port);
    }
  else if (sscanf(webopt, ":%d%n", &mom_web_port, &pos)>=1 && pos>0 && mom_web_port>0)
    {
      strcpy(mom_web_host, "localhost");
      MOM_DEBUGLOG(webonion, "mom_run_web_onion no webhost "
                   << " webport=" << mom_web_port);
    }
  else MOM_FATAPRINTF("mom_run_web_onion invalid option %s", webopt);
  Onion::Onion myonion;
  myonion.setHostname(mom_web_host);
  myonion.setPort(mom_web_port);
  myonion.setRootHandler(Onion::ExportLocal("webroot/"));
  myonion.setMaxThreads(mom_nb_jobs);
  MOM_INFORMPRINTF("start HTTP listening %s", webopt);
  myonion.listen();
  MOM_INFORMPRINTF("end HTTP listening %s", webopt);
} // end  mom_run_web_onion
