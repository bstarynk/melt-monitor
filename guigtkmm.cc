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

class MomApplication : public Gtk::Application
{
public:
  MomApplication(int& argc, char**argv, const char*name);
  ~MomApplication();
  static Glib::RefPtr<MomApplication> create(int& argc, char**argv, const char*name);
  void on_startup(void);
  void on_activate(void);
};				// end MomApplication

class MomMainWindow : public Gtk::Window
{
  Gtk::Box _vbox;
  Gtk::MenuBar _menubar;
  Gtk::MenuItem _mit_app;
  Gtk::MenuItem _mit_edit;
  Gtk::Menu _menu_app;
  Gtk::Menu _menu_edit;
  Gtk::MenuItem _mit_app_quit;
  Gtk::MenuItem _mit_app_exit;
  Gtk::MenuItem _mit_app_dump;
  Gtk::MenuItem _mit_edit_copy;
  Glib::RefPtr<Gtk::TextBuffer> _buf;
  Gtk::Paned _panedtx;
  Gtk::ScrolledWindow _scrwtop;
  Gtk::ScrolledWindow _scrwbot;
  Gtk::TextView _txvtop;
  Gtk::TextView _txvbot;
  Gtk::TextView _txvcmd;
public:
  MomMainWindow();
  ~MomMainWindow();
};				// end class MomMainWindow
////////////////////////////////////////////////////////////////


MomApplication::MomApplication(int& argc, char**argv, const char*name)
  : Gtk::Application(argc, argv, name)
{
};				// end MomApplication::MomApplication

MomApplication::~MomApplication()
{
};				// end MomApplication::~MomApplication

void
MomApplication::on_startup(void)
{
  MOM_DEBUGLOG(gui,"MomApplication::on_startup start"
               << MOM_SHOW_BACKTRACE("on_startup"));
  Gtk::Application::on_startup();
}

void
MomApplication::on_activate(void)
{
  MOM_DEBUGLOG(gui,"MomApplication::on_activate start"
               << MOM_SHOW_BACKTRACE("on_activate"));
  Gtk::Application::on_activate();
  auto mainwin = new MomMainWindow();
  add_window(*mainwin);
  mainwin->show();
  MOM_DEBUGLOG(gui,"MomApplication::on_activate mainwin=" << mainwin);
  //  mainwin->show();
}

Glib::RefPtr<MomApplication>
MomApplication::create(int &argc, char**argv, const char*name)
{
  auto app = Glib::RefPtr<MomApplication>(new MomApplication(argc, argv, name));
  MOM_DEBUGLOG(gui, "MomApplication::create app=" << &app);
  return app;
} // end MomApplication::create




////////////////
MomMainWindow::MomMainWindow()
  : Gtk::Window(),
    _vbox(Gtk::ORIENTATION_VERTICAL),
    _menubar(),
    _mit_app("_App",true),
    _mit_edit("_Edit",true),
    _menu_app(),
    _menu_edit(),
    _mit_app_quit("_Quit",true),
    _mit_app_exit("e_Xit",true),
    _mit_app_dump("_Dump",true),
    _mit_edit_copy("_Copy",true),
    _buf(Gtk::TextBuffer::create()),
    _panedtx(Gtk::ORIENTATION_VERTICAL),
    _txvtop(_buf), _txvbot(_buf),
    _txvcmd()
{
  add(_vbox);
  _menubar.append(_mit_app);
  _menubar.append(_mit_edit);
  _mit_app.set_submenu(_menu_app);
  _mit_edit.set_submenu(_menu_edit);
  _menu_app.append(_mit_app_quit);
  _menu_app.append(_mit_app_exit);
  _menu_app.append(_mit_app_dump);
  _menu_edit.append(_mit_edit_copy);
  _vbox.set_spacing(2);
  _vbox.set_border_width(1);
  _vbox.pack_start(_menubar,Gtk::PACK_SHRINK);
  _vbox.pack_start(_panedtx,Gtk::PACK_EXPAND_WIDGET);
  _scrwtop.add(_txvtop);
  _scrwtop.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _scrwbot.add(_txvbot);
  _scrwbot.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _panedtx.add1(_scrwtop);
  _panedtx.add2(_scrwbot);
  _vbox.pack_start(_txvcmd,Gtk::PACK_EXPAND_WIDGET);
  _txvcmd.set_vexpand(false);
  set_default_size(450,300);
  show_all_children();
};				// end MomMainWindow::MomMainWindow


MomMainWindow::~MomMainWindow()
{
};				// end MomMainWindow::~MomMainWindow

int
mom_run_gtkmm_gui(int& argc, char**argv)
{
  MOM_DEBUGLOG(gui, "mom_run_gtkmm_gui argc=" << argc);
  auto app = MomApplication::create(argc, argv, "org.gcc-melt.monitor");
  MOM_INFORMPRINTF("running mom_run_gtkmm_gui");
  int runcode= app->run();
  MOM_DEBUGLOG(gui,"mom_run_gtkmm_gui runcode="<<runcode);
  if (runcode==0)
    {
      if (!mom_dump_dir)
        {
          char cwdbuf[128];
          memset (cwdbuf, 0, sizeof(cwdbuf));
          MOM_INFORMPRINTF("mom_run_gtkmm_gui dumping state in current directory %s",
                           getcwd(cwdbuf, sizeof(cwdbuf))?:".");
          mom_dump_in_directory(".");
        }
    }
  else
    MOM_INFORMPRINTF("mom_run_gtkmm_gui runcode=%d", runcode);
  return runcode;
} // end mom_run_gtkmm_gui
