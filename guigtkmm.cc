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

class MomMainWindow : public Gtk::ApplicationWindow
{
  Gtk::Box _vbox;
  Glib::RefPtr<Gtk::TextBuffer> _buf;
  Gtk::Paned _panedtx;
  Gtk::ScrolledWindow _scrwtop;
  Gtk::ScrolledWindow _scrwbot;
  Gtk::TextView _txvtop;
  Gtk::TextView _txvbot;
  Gtk::TextView _txvcmd;
public:
  MomMainWindow(const Glib::RefPtr<Gtk::Application>&app);
  ~MomMainWindow();
};				// end class MomMainWindow

MomMainWindow::MomMainWindow(const Glib::RefPtr<Gtk::Application>&app)
  : Gtk::ApplicationWindow(app),
    _vbox(Gtk::ORIENTATION_VERTICAL),
    _buf(Gtk::TextBuffer::create()),
    _panedtx(Gtk::ORIENTATION_VERTICAL),
    _txvtop(_buf), _txvbot(_buf),
    _txvcmd()
{
  add(_vbox);
  _vbox.pack_start(_panedtx,Gtk::PACK_SHRINK);
  _scrwtop.add(_txvtop);
  _scrwtop.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _scrwbot.add(_txvbot);
  _scrwbot.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _panedtx.add1(_scrwtop);
  _panedtx.add2(_scrwbot);
  _vbox.pack_start(_txvcmd,Gtk::PACK_SHRINK);
  show_all_children();
};				// end MomMainWindow::MomMainWindow


MomMainWindow::~MomMainWindow()
{
};				// end MomMainWindow::~MomMainWindow

int
mom_run_gtkmm_gui(int& argc, char**argv)
{
  auto app = Gtk::Application::create(argc, argv, "org.gcc-melt.monitor");
  MomMainWindow window(app);
  MOM_INFORMPRINTF("running mom_run_gtkmm_gui");
  window.set_default_size(200,300);
  return app->run(window);
} // end mom_run_gtkmm_gui
