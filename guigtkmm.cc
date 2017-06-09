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

class MomMainWindow;
class MomApplication : public Gtk::Application
{
  static MomApplication* _itself_;
  bool _dont_dump;
  Glib::RefPtr<Gtk::CssProvider> _css_provider;
  friend int mom_run_gtkmm_gui(int& argc, char**argv);
  friend class MomMainWindow;
  void scan_own_gc(MomGC*);
public:
  Glib::RefPtr<Gtk::CssProvider> css()
  {
    return _css_provider;
  };
  MomApplication(int& argc, char**argv, const char*name);
  ~MomApplication();
  static Glib::RefPtr<MomApplication> create(int& argc, char**argv, const char*name);
  static MomApplication* itself(void)
  {
    return _itself_;
  };
  void on_startup(void);
  void on_activate(void);
  void do_exit(void);
  void do_dump(void);
  void scan_gc(MomGC*);
};				// end class MomApplication



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
  Gtk::Statusbar _statusbar;
public:
  MomMainWindow();
  ~MomMainWindow();
  void do_window_quit(void);
  void show_status_decisec(const std::string&msg, int delay_decisec);
  void clear_statusbar(void);
  void show_status_decisec(const char*msg, int delay_decisec)
  {
    show_status_decisec(std::string(msg), delay_decisec);
  };
  void do_window_dump(void);
  void scan_gc(MomGC*);
};				// end class MomMainWindow

////////////////////////////////////////////////////////////////

MomApplication* MomApplication::_itself_;

MomApplication::MomApplication(int& argc, char**argv, const char*name)
  : Gtk::Application(argc, argv, name),
    _dont_dump(false),
    _css_provider()
{
  _itself_ = this;
};				// end MomApplication::MomApplication

MomApplication::~MomApplication()
{
  _itself_ = nullptr;
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
  _css_provider = Gtk::CssProvider::get_default();
  _css_provider->load_from_path("browsermom.css");
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


void
MomApplication::scan_own_gc(MomGC*gc)
{
#warning MomApplication::scan_own_gc incomplete
} // end MomApplication::scan_own_gc


void
MomApplication::scan_gc(MomGC*gc)
{
  scan_own_gc(gc);
  {
    auto listwindows = get_windows();
    for (auto win : listwindows)
      {
        auto mainwin = dynamic_cast<MomMainWindow*>(win);
        if (!mainwin) continue;
        mainwin->scan_gc(gc);
      }
  }
} // end MomApplication::scan_gc

////////////////


void
MomMainWindow::clear_statusbar(void)
{
  _statusbar.remove_all_messages();
} // end MomMainWindow::clear_statusbar

void
MomMainWindow::show_status_decisec(const std::string&msg, int delay_decisec)
{
  clear_statusbar();
  (void) _statusbar.push(msg);
  Glib::signal_timeout().connect_once
  (sigc::mem_fun(this,&MomMainWindow::clear_statusbar),
   delay_decisec*100+5);
} // end MomMainWindow::show_status_decisec

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
  {
    auto screen = Gdk::Screen::get_default();
    auto ctx = get_style_context();
    ctx->add_provider_for_screen(screen,MomApplication::itself()->css(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }
  add(_vbox);
  _menubar.append(_mit_app);
  _menubar.append(_mit_edit);
  _mit_app.set_submenu(_menu_app);
  _mit_edit.set_submenu(_menu_edit);
  _menu_app.append(_mit_app_quit);
  _menu_app.append(_mit_app_exit);
  _menu_app.append(_mit_app_dump);
  _mit_app_quit.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_quit));
  _mit_app_exit.signal_activate().connect(sigc::mem_fun(*MomApplication::itself(),&MomApplication::do_exit));
  _mit_app_dump.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_dump));
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
  _vbox.pack_end(_statusbar,Gtk::PACK_SHRINK);
  _txvcmd.set_vexpand(false);
  set_default_size(550,300);
  show_all_children();
};				// end MomMainWindow::MomMainWindow


MomMainWindow::~MomMainWindow()
{
};				// end MomMainWindow::~MomMainWindow

void
MomMainWindow::do_window_quit(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_window_quit");
  Gtk::MessageDialog dialog(*this, "Quit ... ?",
                            false /* use_markup */, Gtk::MESSAGE_QUESTION,
                            Gtk::BUTTONS_OK_CANCEL);
  dialog.set_secondary_text( "Quit without saving state.");
  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK)
    {
      MomApplication::itself()->_dont_dump = true;
      MomApplication::itself()->quit();
    }
} // end MomMainWindow::do_quit

void
MomMainWindow::do_window_dump(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_window_dump");
  long nbob = 0;
  unsigned constexpr showdumpdelay = 32;
  std::ostringstream outs;
  if (!mom_dump_dir)
    {
      char cwdbuf[128];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      if (!getcwd(cwdbuf, sizeof(cwdbuf)))
        cwdbuf[0] = '.';
      MOM_INFORMPRINTF("MomMainWindow::do_window_dump dumping state in current directory %s",
                       cwdbuf);
      nbob = mom_dump_in_directory(".");
      outs << "dumped " << nbob << " objects in current dir. " << cwdbuf;
    }
  else
    {
      MOM_INFORMPRINTF("MomMainWindow::do_window_dump dumping state in %s", mom_dump_dir);
      nbob = mom_dump_in_directory(mom_dump_dir);
      outs << "dumped " << nbob << " objects in directory " << mom_dump_dir;
    }
  outs.flush();
  show_status_decisec(outs.str(), showdumpdelay);
} // end MomMainWindow::do_window_dump


void
MomMainWindow::scan_gc(MomGC*gc)
{
#warning MomMainWindow::scan_gc incomplete
} // end MomApplication::scan_gc

void
MomApplication::do_exit(void)
{
  MOM_DEBUGLOG(gui, "MomApplication::do_exit");
  quit();
} // end MomApplication::do_exit

void
MomApplication::do_dump(void)
{
  MOM_DEBUGLOG(gui, "MomApplication::do_dump");
  if (!mom_dump_dir)
    {
      char cwdbuf[128];
      memset (cwdbuf, 0, sizeof(cwdbuf));
      MOM_INFORMPRINTF("MomApplication::do_dump dumping state in current directory %s",
                       getcwd(cwdbuf, sizeof(cwdbuf))?:".");
      mom_dump_in_directory(".");
    }
  else
    {
      MOM_INFORMPRINTF("MomApplication::do_dump dumping state in %s", mom_dump_dir);
      mom_dump_in_directory(mom_dump_dir);
    }
} // end MomApplication::do_dump

int
mom_run_gtkmm_gui(int& argc, char**argv)
{
  MOM_DEBUGLOG(gui, "mom_run_gtkmm_gui argc=" << argc);
  auto app = MomApplication::create(argc, argv, "org.gcc-melt.monitor");
  MOM_INFORMPRINTF("running mom_run_gtkmm_gui");
  int runcode= app->run();
  MOM_DEBUGLOG(gui,"mom_run_gtkmm_gui runcode="<<runcode);
  if (runcode==0 && !app->_dont_dump)
    {
      if (!mom_dump_dir)
        {
          char cwdbuf[128];
          memset (cwdbuf, 0, sizeof(cwdbuf));
          MOM_INFORMPRINTF("mom_run_gtkmm_gui dumping state in current directory %s",
                           getcwd(cwdbuf, sizeof(cwdbuf))?:".");
          mom_dump_in_directory(".");
        }
      else
        {
          MOM_INFORMPRINTF("mom_run_gtkmm_gui dumping state in %s", mom_dump_dir);
          mom_dump_in_directory(mom_dump_dir);
        }
    }
  else
    MOM_INFORMPRINTF("mom_run_gtkmm_gui runcode=%d dontdump %s", runcode, app->_dont_dump?"true":"false");
  return runcode;
} // end mom_run_gtkmm_gui
