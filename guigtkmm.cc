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
  bool _app_dont_dump;
  Glib::RefPtr<Gtk::CssProvider> _app_css_provider;
  Glib::RefPtr<Gtk::Builder> _app_ui_builder;
  Glib::RefPtr<Gtk::TextTagTable> _app_browse_tagtable;
  friend int mom_run_gtkmm_gui(int& argc, char**argv);
  friend class MomMainWindow;
  void scan_own_gc(MomGC*);
public:
  Glib::RefPtr<Gtk::TextTagTable> browser_tagtable(void)
  {
    return _app_browse_tagtable;
  };
  Glib::RefPtr<Gtk::CssProvider> css_provider(void)
  {
    return _app_css_provider;
  };
  MomApplication(int& argc, char**argv, const char*name);
  ~MomApplication();
  static Glib::RefPtr<MomApplication> create(int& argc, char**argv, const char*name);
  static MomApplication* itself(void)
  {
    return _itself_;
  };
  void on_parsing_css_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
  void on_startup(void);
  void on_activate(void);
  void do_exit(void);
  void do_dump(void);
  void scan_gc(MomGC*);
};				// end class MomApplication



class MomMainWindow : public Gtk::Window
{
  Gtk::Box _mwi_vbox;
  Gtk::MenuBar _mwi_menubar;
  Gtk::MenuItem _mwi_mit_app;
  Gtk::MenuItem _mwi_mit_edit;
  Gtk::Menu _mwi_menu_app;
  Gtk::Menu _mwi_menu_edit;
  Gtk::MenuItem _mwi_mit_app_quit;
  Gtk::MenuItem _mwi_mit_app_exit;
  Gtk::MenuItem _mwi_mit_app_dump;
  Gtk::MenuItem _mwi_mit_edit_copy;
  Glib::RefPtr<Gtk::TextBuffer> _mwi_buf;
  Gtk::Paned _mwi_panedtx;
  Gtk::ScrolledWindow _mwi_scrwtop;
  Gtk::ScrolledWindow _mwi_scrwbot;
  Gtk::TextView _mwi_txvtop;
  Gtk::TextView _mwi_txvbot;
  Gtk::TextView _mwi_txvcmd;
  Gtk::Statusbar _mwi_statusbar;
public:
  MomMainWindow();
  ~MomMainWindow();
  void do_window_quit(void);
  void show_status_decisec(const std::string&msg, int delay_decisec);
  void clear_mwi_statusbar(void);
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
    _app_dont_dump(false),
    _app_css_provider(),
    _app_ui_builder(),
    _app_browse_tagtable()
{
  _itself_ = this;
};				// end MomApplication::MomApplication

MomApplication::~MomApplication()
{
  _itself_ = nullptr;
};				// end MomApplication::~MomApplication

void
MomApplication::on_parsing_css_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error)
{
  auto file = section->get_file();
  MOM_WARNLOG("MomApplication parsing css error: " << error.what() << "@" << (file?file->get_path():std::string("?"))
              << " start L#" << (section->get_start_line()+1) << ",P" << section->get_start_position()
              << ", end L#" << (section->get_end_line()+1) << ",P" << section->get_end_position()
              << std::endl << MOM_SHOW_BACKTRACE("CSS Parsing error"));
} // end MomApplication::on_parsing_css_error

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
  constexpr const char* browserui_path = "browsermom.ui";
  constexpr const char* browsercss_path = "browsermom.css";
  constexpr const char* browsertagtable_id = "browsertagtable_id";
  MOM_DEBUGLOG(gui,"MomApplication::on_activate start"
               << MOM_SHOW_BACKTRACE("on_activate"));
  Gtk::Application::on_activate();
  _app_css_provider = Gtk::CssProvider::get_default();
  _app_css_provider->signal_parsing_error()
  .connect(sigc::mem_fun(*this,&MomApplication::on_parsing_css_error));
  _app_css_provider->load_from_path(browsercss_path);
  _app_ui_builder =  Gtk::Builder::create_from_file (browserui_path);
  if (!_app_ui_builder)
    MOM_FATAPRINTF("failed to parse UI file %s", browserui_path);
  {
    Glib::RefPtr<Glib::Object>  plaintagtable = _app_ui_builder->get_object(browsertagtable_id);
    if (!plaintagtable)
      MOM_FATAPRINTF("failed to use UI file %s, can't find %s", browserui_path, browsertagtable_id);
    _app_browse_tagtable = Glib::RefPtr<Gtk::TextTagTable>::cast_dynamic(plaintagtable);
    if (!_app_browse_tagtable)
      MOM_FATAPRINTF("failed to use UI file %s, %s is not a TextTagTable", browserui_path, browsertagtable_id);
  }
  auto mainwin = new MomMainWindow();
  add_window(*mainwin);
  mainwin->show();
  MOM_DEBUGLOG(gui,"MomApplication::on_activate mainwin=" << mainwin);
} // end MomApplication::on_activate

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
MomMainWindow::clear_mwi_statusbar(void)
{
  _mwi_statusbar.remove_all_messages();
} // end MomMainWindow::clear_mwi_statusbar

void
MomMainWindow::show_status_decisec(const std::string&msg, int delay_decisec)
{
  clear_mwi_statusbar();
  (void) _mwi_statusbar.push(msg);
  Glib::signal_timeout().connect_once
  (sigc::mem_fun(this,&MomMainWindow::clear_mwi_statusbar),
   delay_decisec*100+5);
} // end MomMainWindow::show_status_decisec

MomMainWindow::MomMainWindow()
  : Gtk::Window(),
    _mwi_vbox(Gtk::ORIENTATION_VERTICAL),
    _mwi_menubar(),
    _mwi_mit_app("_App",true),
    _mwi_mit_edit("_Edit",true),
    _mwi_menu_app(),
    _mwi_menu_edit(),
    _mwi_mit_app_quit("_Quit",true),
    _mwi_mit_app_exit("e_Xit",true),
    _mwi_mit_app_dump("_Dump",true),
    _mwi_mit_edit_copy("_Copy",true),
    _mwi_buf(Gtk::TextBuffer::create(MomApplication::itself()->browser_tagtable())),
    _mwi_panedtx(Gtk::ORIENTATION_VERTICAL),
    _mwi_txvtop(_mwi_buf), _mwi_txvbot(_mwi_buf),
    _mwi_txvcmd()
{
  {
    auto screen = Gdk::Screen::get_default();
    auto ctx = get_style_context();
    ctx->add_provider_for_screen(screen,MomApplication::itself()->css_provider(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }
  add(_mwi_vbox);
  _mwi_menubar.append(_mwi_mit_app);
  _mwi_menubar.append(_mwi_mit_edit);
  _mwi_mit_app.set_submenu(_mwi_menu_app);
  _mwi_mit_edit.set_submenu(_mwi_menu_edit);
  _mwi_menu_app.append(_mwi_mit_app_quit);
  _mwi_menu_app.append(_mwi_mit_app_exit);
  _mwi_menu_app.append(_mwi_mit_app_dump);
  _mwi_mit_app_quit.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_quit));
  _mwi_mit_app_exit.signal_activate().connect(sigc::mem_fun(*MomApplication::itself(),&MomApplication::do_exit));
  _mwi_mit_app_dump.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_dump));
  _mwi_menu_edit.append(_mwi_mit_edit_copy);
  _mwi_vbox.set_spacing(2);
  _mwi_vbox.set_border_width(1);
  _mwi_vbox.pack_start(_mwi_menubar,Gtk::PACK_SHRINK);
  _mwi_vbox.pack_start(_mwi_panedtx,Gtk::PACK_EXPAND_WIDGET);
  _mwi_txvtop.set_editable(false);
  _mwi_txvbot.set_editable(false);
  _mwi_scrwtop.add(_mwi_txvtop);
  _mwi_scrwtop.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _mwi_scrwbot.add(_mwi_txvbot);
  _mwi_scrwbot.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _mwi_panedtx.add1(_mwi_scrwtop);
  _mwi_panedtx.add2(_mwi_scrwbot);
  {
    _mwi_vbox.pack_start(_mwi_txvcmd,Gtk::PACK_EXPAND_WIDGET);
    _mwi_txvcmd.set_vexpand(false);
    auto ctx = _mwi_txvcmd.get_style_context();
    ctx->add_class("commandwin_cl");
  }
  _mwi_vbox.pack_end(_mwi_statusbar,Gtk::PACK_SHRINK);
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
      MomApplication::itself()->_app_dont_dump = true;
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
  if (runcode==0 && !app->_app_dont_dump)
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
    MOM_INFORMPRINTF("mom_run_gtkmm_gui runcode=%d dontdump %s", runcode, app->_app_dont_dump?"true":"false");
  return runcode;
} // end mom_run_gtkmm_gui
