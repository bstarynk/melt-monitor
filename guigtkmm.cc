// file guigtkmm.cc - GtkMM based GUI

/**   Copyright (C)  2015 - 2017  Basile Starynkevitch and later the FSF
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

#include <gtkmm.h>

int
mom_run_gtkmm_gui(int& argc, char**argv)
{
  auto app = Gtk::Application::create(argc, argv, "org.gcc-melt.monitor");
  Gtk::Window window;
  MOM_INFORMPRINTF("running mom_run_gtkmm_gui");
  window.set_default_size(200,300);
  return app->run(window);
} // end mom_run_gtkmm_gui
