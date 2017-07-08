// file guigtkmm.cc - GtkMM based GUI

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

#include <glib.h>
#include <gtkmm.h>

class MomMainWindow;

class MomPaylMainWindow;

class MomApplication : public Gtk::Application
{
  static MomApplication* _itself_;
  bool _app_dont_dump;
  Glib::RefPtr<Gtk::CssProvider> _app_css_provider;
  Glib::RefPtr<Gtk::Builder> _app_ui_builder;
  Glib::RefPtr<Gtk::TextTagTable> _app_browser_tagtable;
  Glib::RefPtr<Gtk::TextTagTable> _app_command_tagtable;
  friend int mom_run_gtkmm_gui(int& argc, char**argv);
  friend class MomMainWindow;
  void scan_own_gc(MomGC*);
public:
  static constexpr const int _max_depth_ = 32;
  Glib::RefPtr<Gtk::TextTagTable> browser_tagtable(void)
  {
    return _app_browser_tagtable;
  };
  Glib::RefPtr<Gtk::TextTagTable> command_tagtable(void)
  {
    return _app_command_tagtable;
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
  Glib::RefPtr<Gtk::TextTag> lookup_browser_tag (const Glib::ustring& name)
  {
    return _app_browser_tagtable->lookup(name);
  };
  unsigned nb_browser_tags() const
  {
    return _app_browser_tagtable->get_size();
  };
  Glib::RefPtr<Gtk::TextTag> lookup_browser_tag (const char*namestr)
  {
    return lookup_browser_tag (Glib::ustring(namestr));
  };
  Glib::RefPtr<Gtk::TextTag> lookup_command_tag (const Glib::ustring& name)
  {
    return _app_command_tagtable->lookup(name);
  };
  Glib::RefPtr<Gtk::TextTag> lookup_command_tag (const char*namestr)
  {
    return lookup_command_tag (Glib::ustring(namestr));
  };
  unsigned nb_command_tags() const
  {
    return _app_command_tagtable->get_size();
  };
  void on_parsing_css_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
  void on_startup(void);
  void on_activate(void);
  void do_exit(void);
  void do_dump(void);
  void scan_gc(MomGC*);
};				// end class MomApplication


const int MomApplication::_max_depth_;

class MomShowTextIter
{
  Gtk::TextIter _shtxit;
  bool _showfull;
  unsigned _shwidth;
public:
  static constexpr const bool _FULL_= true;
  static constexpr const bool _PLAIN_= true;
  static constexpr const unsigned _MAX_WIDTH_= 40;
  explicit MomShowTextIter(Gtk::TextIter txit, bool full = false, unsigned width = 0) :
    _shtxit(txit), _showfull(full), _shwidth(width) {};
  ~MomShowTextIter() {};
  MomShowTextIter(const MomShowTextIter&) = default;
  MomShowTextIter(MomShowTextIter&&) = default;
  void output(std::ostream& os) const;
};

const bool  MomShowTextIter::_FULL_;
const bool  MomShowTextIter::_PLAIN_;

inline
std::ostream& operator << (std::ostream& out, const MomShowTextIter shtxit)
{
  shtxit.output(out);
  return out;
}

// see also https://stackoverflow.com/q/39366248/841108
class MomComboBoxObjptrText : public Gtk::ComboBoxText
{
  Glib::RefPtr<Gtk::EntryCompletion> _cbo_entcompl;
  void upgrade_for_string(const char*str);
public:
  static constexpr const long _nb_named_threshold_ = 80;
  MomComboBoxObjptrText();
  ~MomComboBoxObjptrText();
  void do_change_boxobjptr(void);
  void on_my_show(void);
  MomObject* active_object(void);
};				// end MomComboBoxObjptrText
const long MomComboBoxObjptrText::_nb_named_threshold_;


extern "C" const struct MomVtablePayload_st MOM_PAYLOADVTBL(main_window) __attribute__((section(".rodata")));


MomRegisterPayload mompy_main_window(MOM_PAYLOADVTBL(main_window));

class MomPaylMainWindow : public MomPayload
{
public:
  friend struct MomVtablePayload_st;
  friend class MomObject;
  friend class MomMainWindow;
private:
  MomMainWindow* _pymw_win;
  MomObject* _pymw_proxy;
public:
  MomPaylMainWindow(MomObject*owner, MomMainWindow*win)
    : MomPayload(&MOM_PAYLOADVTBL(main_window),owner),
      _pymw_win(win), _pymw_proxy(nullptr) {};
  virtual ~MomPaylMainWindow();
  MomMainWindow* main_win() const
  {
    return _pymw_win;
  };
  MomObject* proxy() const
  {
    return _pymw_proxy;
  };
  static MomPyv_destr_sig Destroy;
  static MomPyv_scangc_sig Scangc;
  static MomPyv_getmagic_sig Getmagic;
};				// end MomPaylMainWindow


const struct MomVtablePayload_st MOM_PAYLOADVTBL(main_window) __attribute__((section(".rodata"))) =
{
  /**   .pyv_magic=      */       MOM_PAYLOADVTBL_MAGIC,
  /**   .pyv_size=       */       sizeof(MomPaylMainWindow),
  /**   .pyv_name=       */       "main_window",
  /**   .pyv_module=     */       (const char*)nullptr,
  /**   .pyv_destroy=    */       MomPaylMainWindow::Destroy,
  /**   .pyv_scangc=     */       MomPaylMainWindow::Scangc,
  /**   .pyv_scandump=   */       nullptr,
  /**   .pyv_emitdump=   */       nullptr,
  /**   .pyv_initload=   */       nullptr,
  /**   .pyv_loadfill=   */       nullptr,
  /**   .pyv_getmagic=   */       MomPaylMainWindow::Getmagic,
  /**   .pyv_fetch=      */       nullptr,
  /**   .pyv_update=     */       nullptr,
  /**   .pyv_step=       */       nullptr,
  /**   .pyv_spare1=     */       nullptr,
  /**   .pyv_spare2=     */       nullptr,
  /**   .pyv_spare3=     */       nullptr,
};


class MomMainWindow : public Gtk::Window
{
  friend class MomPaylMainWindow;
public:
  static constexpr const int _default_display_depth_ = 5;
  static constexpr const int _min_display_depth_ = 2;
  static constexpr const int _max_display_depth_ = 10;
  static constexpr const int _default_display_width_ = 72;
  static constexpr const int _default_status_delay_deciseconds_ = 33;
  static constexpr const int _default_error_delay_milliseconds_ = 440;
  static constexpr const int _blink_period_milliseconds = 700;
  static constexpr const int _blink_delay_milliseconds = 300;
  static constexpr const bool _SCROLL_TOP_VIEW_ = true;
  static constexpr const bool _DONT_SCROLL_TOP_VIEW_ = false;
  struct MomParenOffsets
  {
    int paroff_open; // relative offset before open parenthesis-like
    int paroff_close; // relative offset after close parenthesis-like
    int paroff_xtra;  // relative offset of xtra sign, such as the *, or -1
    uint8_t paroff_openlen; // length in UTF-8 chars of opening sign
    uint8_t paroff_closelen; // length in UTF-8 chars of closing sign
    uint8_t paroff_xtralen; // length in UTF-8 chars of xtra sign
    uint8_t paroff_depth;   // depth
    bool surrounds(int off)
    {
      /// when ( ^ ) or [ ^ ] or { ^ }
      if (paroff_open <= off && off <= paroff_close)
        return true;
      /// when * conn ^ ( ... )
      if (paroff_xtra >= 0 && paroff_xtra <= off
          && off <= paroff_open && paroff_xtra < paroff_open)
        return true;
      return false;
    }
  };
  struct MomBrowsedObject
  {
    MomObject*_sh_ob;
    Glib::RefPtr<Gtk::TextMark> _sh_startmark;
    Glib::RefPtr<Gtk::TextMark> _sh_endmark;
    // the keys of these maps are UTF-8 char (not byte) offsets
    // relative to start of objects
    std::map<int,MomParenOffsets> _sh_openmap;
    std::map<int,MomParenOffsets> _sh_closemap;
    MomBrowsedObject(MomObject*ob,
                     Glib::RefPtr<Gtk::TextMark> startmk=Glib::RefPtr<Gtk::TextMark>(),
                     Glib::RefPtr<Gtk::TextMark> endmk=Glib::RefPtr<Gtk::TextMark>())
      : _sh_ob(ob), _sh_startmark(startmk), _sh_endmark(endmk),
        _sh_openmap(), _sh_closemap() {};
    ~MomBrowsedObject()
    {
      _sh_ob=nullptr;
      _sh_startmark.clear();
      _sh_endmark.clear();
    };
    void add_parens(const MomParenOffsets& po)
    {
      _sh_openmap.insert({po.paroff_open, po});
      _sh_closemap.insert({po.paroff_close, po});
    }
    void clear_parens(void)
    {
      _sh_openmap.clear();
      _sh_closemap.clear();
    }
  };
  struct MomDisplayCtx
  {
    MomBrowsedObject* _dx_shob;
    MomDisplayCtx(MomBrowsedObject*shob) : _dx_shob(shob) {};
    ~MomDisplayCtx()
    {
      _dx_shob=nullptr;
    };
  };
private:
  static int _mwi_wincount;
  const int _mwi_winrank;
  Gtk::Box _mwi_vbox;
  Gtk::MenuBar _mwi_menubar;
  Gtk::MenuItem _mwi_mit_app;
  Gtk::MenuItem _mwi_mit_edit;
  Gtk::MenuItem _mwi_mit_object;
  Gtk::Menu _mwi_menu_app;
  Gtk::Menu _mwi_menu_edit;
  Gtk::Menu _mwi_menu_object;
  Gtk::MenuItem _mwi_mit_app_quit;
  Gtk::MenuItem _mwi_mit_app_exit;
  Gtk::MenuItem _mwi_mit_app_dump;
  Gtk::MenuItem _mwi_mit_edit_copy;
  Gtk::MenuItem _mwi_mit_object_show_hide;
  Gtk::MenuItem _mwi_mit_object_refresh;
  Gtk::MenuItem _mwi_mit_object_options;
  Gtk::MenuItem _mwi_mit_txcmd_clear;
  Gtk::MenuItem _mwi_mit_txcmd_runclear;
  Gtk::MenuItem _mwi_mit_txcmd_runkeep;
  Gtk::MenuItem _mwi_mit_txcmd_cursorloc;
  Glib::RefPtr<Gtk::TextBuffer> _mwi_browserbuf;
  Glib::RefPtr<Gtk::TextBuffer> _mwi_commandbuf;
  // mark to end of title string, always followed by newline:
  Glib::RefPtr<Gtk::TextMark> _mwi_endtitlemark;
  int _mwi_dispdepth;
  int _mwi_dispwidth;
  bool _mwi_dispid;
  Gtk::Paned _mwi_panedtx;
  Gtk::ScrolledWindow _mwi_scrwtop;
  Gtk::ScrolledWindow _mwi_scrwbot;
  Gtk::TextView _mwi_txvtop;
  Gtk::TextView _mwi_txvbot;
  Gtk::Separator _mwi_sepcmd;
  Gtk::ScrolledWindow _mwi_scrcmd;
  Gtk::TextView _mwi_txvcmd;
  Gtk::Statusbar _mwi_statusbar;
  std::map<MomObject*,MomBrowsedObject,MomObjNameLess> _mwi_browsedobmap;
  Glib::RefPtr<Gtk::TextMark> _mwi_browser_startblink;
  Glib::RefPtr<Gtk::TextMark> _mwi_browser_xtrablink;
  Glib::RefPtr<Gtk::TextMark> _mwi_browser_endblink;
  double _mwi_cmduseractim;
  double _mwi_cmdlastuseractim;
  MomObject* _mwi_focusobj;
  MomObject* _mwi_winobj;
  // the key is an UTF-8 character (not byte) offset in the _mwi_commandbuf
  std::map<int,MomParenOffsets> _mwi_txcmd_openmap;
  std::map<int,MomParenOffsets> _mwi_txcmd_closemap;
  Glib::RefPtr<Gtk::TextMark> _mwi_txcmd_startblink;
  Glib::RefPtr<Gtk::TextMark> _mwi_txcmd_xtrablink;
  Glib::RefPtr<Gtk::TextMark> _mwi_txcmd_endblink;
  int _mwi_txcmd_nbmodif;
public:
  int rank() const
  {
    return _mwi_winrank;
  };
  int txcmd_nbmodif() const { return _mwi_txcmd_nbmodif; };
  MomMainWindow();
  ~MomMainWindow();
  void browser_show_object(MomObject*pob);
  void browser_hide_object(MomObject*pob);
  void do_window_quit(void);
  void show_status_decisec(const std::string&msg, int delay_decisec);
  void clear_mwi_statusbar(void);
  void show_status_decisec(const char*msg, int delay_decisec)
  {
    show_status_decisec(std::string(msg), delay_decisec);
  };
  void display_full_browser(void);
  void browser_insert_object_display(Gtk::TextIter& it, MomObject*ob, bool scrolltopview=false);
  void browser_update_title_banner(void);
  void browser_insert_objptr(Gtk::TextIter& it, MomObject*ob, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth);
  void browser_insert_value(Gtk::TextIter& it, MomValue val, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth);
  void browser_insert_space(Gtk::TextIter& it, const std::vector<Glib::ustring>& tags, int depth=0);
  void browser_insert_newline(Gtk::TextIter& it, const std::vector<Glib::ustring>& tags, int depth=0);
  void browser_set_focus_object(MomObject*);
  MomObject* browser_focused_object(void)
  {
    return _mwi_focusobj;
  };
  void do_window_dump(void);
  void do_object_show_hide(void);
  void do_object_refresh(void);
  void do_object_options(void);
  void do_browser_mark_set(const Gtk::TextIter& locit, const Glib::RefPtr<Gtk::TextMark>& mark);
  bool do_browser_blink_insert(void);
  void do_browser_unblink_insert(void);
  void do_txcmd_begin_user_action(void);
  void do_txcmd_changed(void);
  void do_txcmd_end_user_action(void);
  void do_txcmd_populate_menu(Gtk::Menu*menu);
  bool do_txcmd_complete_word(std::string word, Gtk::TextIter startxit, Gtk::TextIter endtxit);
  void do_txcmd_mark_set(const Gtk::TextIter& locit, const Glib::RefPtr<Gtk::TextMark>& mark);
  bool do_txcmd_blink_insert(void);
  void do_txcmd_unblink_insert(void);
  void do_txcmd_clear(void);
  void do_txcmd_run_then_clear(void);
  void do_txcmd_run_but_keep(void);
  void scan_gc(MomGC*);
  void do_txcmd_prettify_parse(bool apply=false);
  bool handle_txcmd_key_release(GdkEventKey*);
  void do_parse_commands(MomParser*, bool apply=false);
private:
  MomObject* browser_object_around(Gtk::TextIter txit);
  bool found_browsed_object_around_insert(MomBrowsedObject*&bob, MomParenOffsets*&po, bool&opening, bool&closing);
  Gtk::TextIter command_txiter_at_line_col(int lineno, int col)
  {
    auto cmdbuf = _mwi_txvcmd.get_buffer();
    Gtk::TextIter txit = cmdbuf->begin();
    if (lineno>1)
      txit.forward_lines(lineno-1);
    if (col>0)
      txit.forward_chars(col);
    return txit;
  };
  void add_txcmd_parens(const MomParenOffsets& po)
  {
    _mwi_txcmd_openmap.insert({po.paroff_open, po});
    _mwi_txcmd_closemap.insert({po.paroff_close, po});
  }
  void clear_txcmd_parens(void)
  {
    _mwi_txcmd_openmap.clear();
    _mwi_txcmd_closemap.clear();
  }
  MomParenOffsets* txcmd_find_parens_around_insert(void);
  void browser_insert_object_mtim_space(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob);
  void browser_insert_object_attributes(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob);
  void browser_insert_object_components(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob);
  void browser_insert_object_payload(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob);
};				// end class MomMainWindow

////////////////////////////////////////////////////////////////

MomApplication* MomApplication::_itself_;

MomApplication::MomApplication(int& argc, char**argv, const char*name)
  : Gtk::Application(argc, argv, name),
    _app_dont_dump(false),
    _app_css_provider(),
    _app_ui_builder(),
    _app_browser_tagtable(),
    _app_command_tagtable()
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
  constexpr const char* commandtagtable_id = "commandtagtable_id";
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
    _app_browser_tagtable = Glib::RefPtr<Gtk::TextTagTable>::cast_dynamic(plaintagtable);
    if (!_app_browser_tagtable)
      MOM_FATAPRINTF("failed to use UI file %s, %s is not a TextTagTable", browserui_path, browsertagtable_id);
    for (int d = 0; d<=_max_depth_; d++)
      {
        _app_browser_tagtable->add(Gtk::TextTag::create(Glib::ustring::compose("star%1_tag", d)));
        _app_browser_tagtable->add(Gtk::TextTag::create(Glib::ustring::compose("open%1_tag", d)));
        _app_browser_tagtable->add(Gtk::TextTag::create(Glib::ustring::compose("close%1_tag", d)));
      }
    auto obtitletag = lookup_browser_tag("object_title_tag");
    MOM_ASSERT(obtitletag, "on_activate nil obtitletag");
    obtitletag->set_priority(3*_max_depth_);
  }
  /// initialize the command tag table
  {
    Glib::RefPtr<Glib::Object>  cmdtagtableob = _app_ui_builder->get_object(commandtagtable_id);
    if (!cmdtagtableob)
      MOM_FATAPRINTF("failed to use UI file %s, can't find %s", browserui_path, commandtagtable_id);
    _app_command_tagtable = Glib::RefPtr<Gtk::TextTagTable>::cast_dynamic(cmdtagtableob);
    if (!_app_command_tagtable)
      MOM_FATAPRINTF("failed to use UI file %s, %s is not a TextTagTable", browserui_path, commandtagtable_id);
    for (int d = 0; d<=_max_depth_; d++)
      {
        _app_command_tagtable->add(Gtk::TextTag::create(Glib::ustring::compose("open%1_cmdtag", d)));
        _app_command_tagtable->add(Gtk::TextTag::create(Glib::ustring::compose("close%1_cmdtag", d)));
      }
    auto cmderrtag = lookup_command_tag("error_cmdtag");
    MOM_ASSERT(cmderrtag, "on_activate nil cmderrtag");
    cmderrtag->set_priority(MomApplication::itself()->nb_command_tags()-1);
  }
  auto mainwin = new MomMainWindow();
  add_window(*mainwin);
  mainwin->show_all_children();
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
  MOM_ASSERT(gc != nullptr, "null GC for MomApplication::scan_own_gc");
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
MomObject*
MomComboBoxObjptrText::active_object(void)
{
  Glib::ustring showtext = get_active_text();
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::active_object start showtext=" << MomShowString(showtext.c_str()));
  MomObject* pob = nullptr;
  if (showtext[0] < 127 && isalpha(showtext[0]))
    {
      pob = mom_find_named(showtext.c_str());
    }
  else if (showtext[0] == '_' && showtext[1] < 127 && isdigit(showtext[1]))
    {
      MomIdent idob = MomIdent::make_from_string(showtext.c_str(), MomIdent::DONT_FAIL);
      pob = MomObject::find_object_of_id(idob);
    }
  else if (showtext[0] == '@' && showtext[1] < 127 && isalpha(showtext[1]))
    {
      std::string globnamstr= showtext.substr(1);
      {
        auto globptr = MomRegisterGlobData::find_globdata(globnamstr);
        if (globptr)
          {
            pob = globptr->load();
          }
      }
    }
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::active_object result pob=" << pob);
  return pob;
} // end MomComboBoxObjptrText::active_object

void
MomComboBoxObjptrText::upgrade_for_string(const char*str)
{
  if (!str) str="";
  auto boxentry = get_entry();
  const char*origstr = str;
  unsigned slen = strlen(str);
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string str=" << MomShowString(str)
               << MOM_SHOW_BACKTRACE("upgrade_for_string"));
  bool badstr = false;
  std::string corrstr;
  corrstr.reserve(slen+1);
  for (const char*pc = str; *pc && !badstr; pc++)
    if (*pc >= 127 && !isalnum(*pc) && *pc != '_' && !(pc==str && *pc=='@'))
      badstr = true;
    else if (pc==str && !(isalpha(*pc) || *pc=='_' || *pc=='@'))
      badstr = true;
    else
      corrstr.push_back(*pc);
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string corrstr=" << MomShowString(corrstr.c_str())
               << " badstr=" << (badstr?"true":"false"));
  if (badstr)
    {
      boxentry->set_text(corrstr.c_str());
      str = corrstr.c_str();
      slen = corrstr.size();
      MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string corrected to str=" << MomShowString(str)
                   << " origstr=" << MomShowString(origstr));
    }
  if (slen==0)
    {
      auto nbnamed = mom_nb_named();
      MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string nbnamed=" << nbnamed
                   << ' ' << ((nbnamed<=_nb_named_threshold_)?"small":"huge"));
      std::vector<std::string> namesvec;
      namesvec.reserve(nbnamed);
      mom_each_name_prefixed("",[&](const std::string&name, MomObject*pobnamed)
      {
        MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string name=" << MomShowString(name)
                     << " pobnamed=" << pobnamed << " #" << namesvec.size());
        namesvec.push_back(name);
        return false;
      });
      if (nbnamed<=_nb_named_threshold_)
        {
          // small number of named
          remove_all();
          for (const std::string& nam : namesvec)
            {
              append(nam.c_str());
            }
          MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string appended small " << namesvec.size());
        }
      else
        {
          MOM_WARNLOG("MomComboBoxObjptrText::upgrade_for_string UNIMPLEMENTED huge nbnamed=" << nbnamed
                      << MOM_SHOW_BACKTRACE("upgrade_for_string huge nbnamed"));
          // huge number of named
        }
    }
  else
    {
      // slen>0
      MOM_DEBUGLOG(gui, "upgrade_for_string str=" << MomShowString(str) << " slen=" << slen);
      if (str[0] < 127 && str[0] > 0 && isalpha(str[0]))
        {
          // complete a name
          std::vector<std::string> namesvec;
          namesvec.reserve(3+100/(slen+1));
          mom_each_name_prefixed
          (str,
           [&](const std::string& curnam,MomObject*)
          {
            namesvec.push_back(curnam);
            MOM_DEBUGLOG(gui, "upgrade_for_string name str=" << MomShowString(str)
                         << " curnam=" << MomShowString(curnam));
            return false;
          });
          remove_all();
          for (auto curnam: namesvec)
            append(curnam.c_str());
        }
      else if (str[0] == '_' && slen>=3
               && str[1] >0 && str[1] < 127 && isalnum(str[1])
               && str[2] >0 && str[2] < 127 && isalnum(str[2]))
        {
          // complete an object id
          std::vector<MomIdent> idsvec;
          MomObject::do_each_object_prefixed
          (str,
           [&](MomObject*curpob)
          {
            idsvec.push_back(curpob->id());
            MOM_DEBUGLOG(gui, "upgrade_for_string id str=" << str
                         << " curpob=" << curpob);
            return false;
          });
          remove_all();
          for (auto curid: idsvec)
            {
              char idbuf[32];
              memset (idbuf, 0, sizeof(idbuf));
              curid.to_cbuf32(idbuf);
              append(idbuf);
            };
        }
      else if (str[0] == '@')
        {
          // complete a global data name
          std::vector<std::string> globdatavec;
          MomRegisterGlobData::do_each_globdata
          ([&](const std::string&glonam, std::atomic<MomObject*>*)
          {
            if (glonam.size() >= slen-1
                && strncmp(glonam.c_str(), str+1, slen-1)==0)
              {
                globdatavec.push_back(glonam);
                MOM_DEBUGLOG(gui, "upgrade_for_string global str=" << str
                             << " glonam=" << glonam << ".");
              }
            return false;
          });
          remove_all();
          for (std::string glonam: globdatavec)
            {
              std::string strgl = std::string{"@"} + glonam;
              append(strgl.c_str());
            }
        }
    }
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::upgrade_for_string end");
} // end MomComboBoxObjptrText::upgrade_for_string



MomComboBoxObjptrText::MomComboBoxObjptrText()
  : Gtk::ComboBoxText(true /*has_entry*/),
    _cbo_entcompl()
{
  /// inspired by https://stackoverflow.com/a/39426800/841108
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::MomComboBoxObjptrText from "
               << MOM_SHOW_BACKTRACE("MomComboBoxObjptrText"));
  _cbo_entcompl = Gtk::EntryCompletion::create();
  _cbo_entcompl->set_text_column(0);
  _cbo_entcompl->set_minimum_key_length (2);
  auto boxmodel = get_model();
  _cbo_entcompl->set_model(boxmodel);
  auto boxentry = get_entry();
  boxentry->set_completion(_cbo_entcompl);
  boxentry->signal_changed().connect(sigc::mem_fun(this,&MomComboBoxObjptrText::do_change_boxobjptr));
  property_active() = 0;
  signal_show().connect(sigc::mem_fun(this,&MomComboBoxObjptrText::on_my_show));
#warning MomComboBoxObjptrText::MomComboBoxObjptrText is very incomplete
} // end MomComboBoxObjptrText::MomComboBoxObjptrText

void
MomComboBoxObjptrText::on_my_show(void)
{
  auto boxentry = get_entry();
  Glib::ustring entstr = boxentry->get_text();
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::on_my_show entstr="
               << MomShowString(entstr.c_str())
               << MOM_SHOW_BACKTRACE("on_my_show"));
  upgrade_for_string(entstr.c_str());
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::on_my_show end");
} // end MomComboBoxObjptrText::on_my_show


void
MomComboBoxObjptrText::do_change_boxobjptr(void)
{
  auto boxentry = get_entry();
  Glib::ustring entstr = boxentry->get_text();
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::do_change_boxobjptr entstr="
               << MomShowString(entstr)
               << MOM_SHOW_BACKTRACE("do_change_boxobjptr")
              );
  upgrade_for_string(entstr.c_str());
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::do_change_boxobjptr end");
#warning MomComboBoxObjptrText::do_change_boxobjptr unimplemented
} // end MomComboBoxObjptrText::do_change_boxobjptr

MomComboBoxObjptrText::~MomComboBoxObjptrText()
{
  MOM_DEBUGLOG(gui, "MomComboBoxObjptrText::~MomComboBoxObjptrText"
               << MOM_SHOW_BACKTRACE("~MomComboBoxObjptrText"));
} // end MomComboBoxObjptrText::~MomComboBoxObjptrText

////////////////
int MomMainWindow::_mwi_wincount;

void
MomMainWindow::display_full_browser(void)
{
  int nbshownob = _mwi_browsedobmap.size();
  _mwi_browserbuf->set_text("");
  auto it = _mwi_browserbuf->begin();
  if (nbshownob == 0)
    it = _mwi_browserbuf->insert_with_tag (it, " ~ no objects ~ ", "page_title_tag");
  else if (nbshownob == 1)
    it = _mwi_browserbuf->insert_with_tag (it, " ~ one object ~ ", "page_title_tag");
  else
    it = _mwi_browserbuf->insert_with_tag (it, Glib::ustring::compose(" ~ %1 objects ~ ", nbshownob), "page_title_tag");
  if (_mwi_endtitlemark)
    {
      _mwi_browserbuf->move_mark(_mwi_endtitlemark, it);
    }
  else
    {
      _mwi_endtitlemark = _mwi_browserbuf->create_mark("end_title", it, /* left_gravity: */ true);
    }
  it = _mwi_browserbuf->insert(it, "\n");
  it = _mwi_browserbuf->insert(it, "\n");
  for (auto itob : _mwi_browsedobmap)
    {
      browser_insert_object_display(it, itob.first);
      it = _mwi_browserbuf->insert(it, "\n");
    }
  MomObject* focuspob = _mwi_focusobj;
  _mwi_focusobj = nullptr; // to force the set focus to do something
  if (focuspob)
    browser_set_focus_object(focuspob);
} // end MomMainWindow::display_full_browser

void
MomMainWindow::browser_insert_space(Gtk::TextIter& txit, const std::vector<Glib::ustring>& tags, int depth)
{
  int linoff = txit.get_line_offset();
  if (linoff >= _mwi_dispwidth)
    browser_insert_newline(txit,tags,depth);
  else
    txit = _mwi_browserbuf->insert_with_tags_by_name (txit, " ", tags);
} // end MomMainWindow::browser_insert_space

void
MomMainWindow::browser_insert_newline(Gtk::TextIter& txit, const std::vector<Glib::ustring>& tags, int depth)
{
  if (depth<0)
    depth=0;
  constexpr const char nlspaces[]
    = "\n                                                                ";
  txit = _mwi_browserbuf->insert_with_tags_by_name (txit, nlspaces, nlspaces + (depth % 16) + 1, tags);
} // end MomMainWindow::browser_insert_newline



void
MomMainWindow::browser_insert_object_display(Gtk::TextIter& txit, MomObject*pob, bool scrolltopview)
{
  MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
             "MomMainWindow::browser_insert_object_display bad object");
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_insert_object_display start "
               << MomShowTextIter(txit, MomShowTextIter::_FULL_, 6)
               << " pob=" << MomShowObject(pob)
               << " scrolltopview=" << (scrolltopview?"true":"false")
               << MOM_SHOW_BACKTRACE("browser_insert_object_display"));
  char obidbuf[32];
  memset(obidbuf, 0, sizeof(obidbuf));
  int depth = _mwi_dispdepth;
  pob->id().to_cbuf32(obidbuf);
  std::shared_lock<std::shared_mutex> lk(pob->get_shared_mutex());
  std::string obnamstr = mom_get_unsync_string_name(const_cast<MomObject*>(pob));
  auto itm = _mwi_browsedobmap.find(pob);
  bool found = false;
  if (itm == _mwi_browsedobmap.end())
    {
      auto begmark = _mwi_browserbuf->create_mark(Glib::ustring::compose("begmarkob_%1", obidbuf), txit, /*left_gravity:*/ true);
      auto pairitb = _mwi_browsedobmap.emplace(pob,MomBrowsedObject(pob,begmark));
      itm = pairitb.first;
      found = false;
    }
  else
    {
      found = true;
    }
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob="
               << MomShowObject(pob) << " depth=" << depth
               << " found=" << (found?"true":"false")
               << ", txit=" << MomShowTextIter(txit, MomShowTextIter::_FULL_,7));
  MomBrowsedObject& shob = itm->second;
  shob.clear_parens();
  if (found)
    _mwi_browserbuf->move_mark(shob._sh_startmark, txit);
  /// the title bar
  MOM_ASSERT(shob._sh_ob == pob, "MomMainWindow::browser_insert_object_display corrupted shob");
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob="
               << MomShowObject(pob) << ", txit before asterism="
               << MomShowTextIter(txit, MomShowTextIter::_FULL_,7)
               << std::endl);
  txit = _mwi_browserbuf->insert_with_tag (txit, " \342\201\202 " /* U+2042 ASTERISM ⁂ */, "object_title_tag");
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob="
               << MomShowObject(pob) << ", txit after asterism="
               << MomShowTextIter(txit, MomShowTextIter::_FULL_,7)
               << std::endl);
  if (!obnamstr.empty())
    {
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obnamstr.c_str()),
              std::vector<Glib::ustring> {"object_title_tag","object_title_name_tag"});
      txit = _mwi_browserbuf->insert_with_tag (txit, " = ", "object_title_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obidbuf),
              std::vector<Glib::ustring> {"object_title_tag","object_title_id_tag"});
    }
  else   // anonymous
    {
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obidbuf),
              std::vector<Glib::ustring> {"object_title_tag","object_title_anon_tag"});
    }
  txit = _mwi_browserbuf->insert_with_tags_by_name
         (txit,
          Glib::ustring::compose(" \360\235\235\231 %1 " /* U+1D759 MATHEMATICAL SANS-SERIF BOLD CAPITAL DELTA 𝝙 */,
                                 depth),
          std::vector<Glib::ustring> {"object_title_tag","object_title_depth_tag"});
  txit = _mwi_browserbuf->insert(txit, "\n");
  if (pob == _mwi_focusobj)
    {
      Gtk::TextIter focstatxit = shob._sh_startmark->get_iter();
      Gtk::TextIter foceoltxit = focstatxit;
      foceoltxit.forward_line();
      MOM_DEBUGLOG(gui, "browser_insert_object_display focus pob=" << pob
                   << "focstatxit ="  <<  MomShowTextIter(focstatxit, MomShowTextIter::_FULL_,10)
                   << "foceoltxit ="  <<  MomShowTextIter(foceoltxit, MomShowTextIter::_FULL_,10));
      _mwi_browserbuf->apply_tag_by_name("object_title_focus_tag", focstatxit, foceoltxit);
    }
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob="
               << MomShowObject(pob) << ", txit after delta+nl="
               << MomShowTextIter(txit, MomShowTextIter::_FULL_,7)
               << std::endl);
  MOM_DEBUGLOG(gui, "browser_insert_object_display after title pob="<< MomShowObject(pob)
               << " txit=" << MomShowTextIter(txit, MomShowTextIter::_FULL_));
  /// show the modtime and the space
  browser_insert_object_mtim_space(txit, pob, shob);
  txit = _mwi_browserbuf->insert(txit, "\n");
  MOM_DEBUGLOG(gui, "browser_insert_object_display before attributes pob=" << pob);
  /// show the attributes
  browser_insert_object_attributes(txit, pob, shob);
  txit = _mwi_browserbuf->insert(txit, "\n");
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob=" << pob << " before components");
  ///  show the components
  browser_insert_object_components(txit, pob, shob);
  txit = _mwi_browserbuf->insert(txit, "\n");
  /// show the payload, if any
  MomPayload* payl = pob->unsync_payload();
  if (payl)
    {
      browser_insert_object_payload(txit,pob,shob);
      txit = _mwi_browserbuf->insert(txit, "\n");
    }
  txit = _mwi_browserbuf->insert_with_tag (txit, "\342\254\236" /* U+2B1E WHITE VERY SMALL SQUARE ⬞ */, "object_end_tag");
  if (shob._sh_endmark)
    _mwi_browserbuf->move_mark(shob._sh_endmark, txit);
  else
    shob._sh_endmark  = _mwi_browserbuf->create_mark(Glib::ustring::compose("endmarkob_%1", obidbuf), txit, /*left_gravity:*/ false);
  txit = _mwi_browserbuf->insert(txit, "\n");
  if (pob == _mwi_focusobj)
    {
      Gtk::TextIter focstatxit = shob._sh_startmark->get_iter();
      Gtk::TextIter focendtxit = shob._sh_endmark->get_iter();
      MOM_DEBUGLOG(gui, "browser_insert_object_display focus pob=" << pob
                   << "focstatxit ="  <<  MomShowTextIter(focstatxit, MomShowTextIter::_FULL_,10)
                   << "focendtxit ="  <<  MomShowTextIter(focendtxit, MomShowTextIter::_FULL_,10));
      _mwi_browserbuf->apply_tag_by_name("object_focus_tag", focstatxit, focendtxit);
    }
  if (scrolltopview)
    {
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_insert_object_display pob=" << pob
                   << " scroll top view");
      _mwi_txvtop.scroll_to(shob._sh_startmark);
    }
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_insert_object_display end final txit="
               << MomShowTextIter(txit, MomShowTextIter::_FULL_, 16)
               << "... pob=" << MomShowObject(pob) << std::endl
               << "... startmark:"
               << MomShowTextIter(shob._sh_startmark->get_iter(), MomShowTextIter::_FULL_, 12)
               << std::endl
               << "... endmark:"
               << MomShowTextIter(shob._sh_endmark->get_iter(), MomShowTextIter::_FULL_, 12)
               << std::endl);
} // end MomMainWindow::browser_insert_object_display


void
MomMainWindow::browser_insert_object_mtim_space(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob)
{
  constexpr double one_week = 86400*7.0;
  constexpr double half_year = 86400/2*365.0;
  char mtimbuf[72];
  char mtimfract[8];
  double obmtim = pob->mtime();
  time_t mtim = obmtim;
  int e;
  snprintf(mtimfract, sizeof(mtimfract), "%.2f", frexp(obmtim,&e));
  // the tm
  struct tm obtm = {};
  localtime_r(&mtim, &obtm);
  double nowtim = mom_clock_time(CLOCK_REALTIME);
  // modified this week
  if (nowtim >= obmtim && nowtim - obmtim < one_week)
    {
      strftime(mtimbuf, sizeof(mtimbuf)-5, "mtim: %a %d, %H:%M:%S", &obtm);
      strcat(mtimbuf, mtimfract+1);
    }
  // modified half a year ago
  else if (nowtim >= obmtim && nowtim - obmtim < half_year)
    {
      strftime(mtimbuf, sizeof(mtimbuf)-5, "mtim: %a %b %d, %H:%M:%S", &obtm);
      strcat(mtimbuf, mtimfract+1);
    }
  else   // modified before, or in the future
    {
      strftime(mtimbuf, sizeof(mtimbuf)-5, "mtim: %a %b %d %Y, %H:%M:%S", &obtm);
      strcat(mtimbuf, mtimfract+1);
    }
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_insert_object_mtim_space "
               << " pob=" << MomShowObject(pob)
               << ", mtim. txit="
               << MomShowTextIter(txit, MomShowTextIter::_FULL_)
               << ", mtimbuf=" << MomShowString(mtimbuf));
  txit = _mwi_browserbuf->insert_with_tag (txit, mtimbuf, "object_mtime_tag");
  txit = _mwi_browserbuf->insert(txit, " ");
  auto spa = pob->space();
  switch (spa)
    {
    case MomSpace::TransientSp:
      txit = _mwi_browserbuf->insert_with_tag (txit, "\302\244" /*U+00A4 CURRENCY SIGN ¤ */,
             "object_space_tag");
      break;
    case MomSpace::PredefSp:
      txit = _mwi_browserbuf->insert_with_tag (txit, "\342\200\274" /*U+203C DOUBLE EXCLAMATION MARK ‼*/,
             "object_space_tag");
      break;
    case MomSpace::GlobalSp:
      txit = _mwi_browserbuf->insert_with_tag (txit, "\342\200\242" /*U+2022 BULLET •*/,
             "object_space_tag");
      break;
    case MomSpace::UserSp:
      txit = _mwi_browserbuf->insert_with_tag (txit, "\342\200\243" /*U+2023 TRIANGULAR BULLET ‣*/,
             "object_space_tag");
      break;
    }
} // end MomMainWindow::browser_insert_object_mtim_space


void
MomMainWindow::browser_insert_object_attributes(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob)
{
  MomDisplayCtx dctxattrs(&shob);
  std::map<MomObject*,MomValue,MomObjNameLess> mapattrs;
  pob->unsync_each_phys_attr([&](MomObject*pobattr,MomValue valattr)
  {
    MOM_DEBUGLOG(gui, "browser_insert_object_attributes physattr: pob=" << pob << " pobattr=" << pobattr << ", valattr=" << valattr);
    mapattrs.insert({pobattr,valattr});
    return false;
  });
  MOM_DEBUGLOG(gui, "browser_insert_object_attributes pob=" << pob << " with " << mapattrs.size() << " attributes to show");
  std::vector<Glib::ustring> tagsattrs{"attributes_tag"};
  std::vector<Glib::ustring> tagsattrindex{"attributes_tag","index_comment_tag"};
  std::vector<Glib::ustring> tagsattrobj{"attributes_tag","attrobj_tag"};
  std::vector<Glib::ustring> tagsattrval{"attributes_tag","attrval_tag"};
  unsigned nbattr = mapattrs.size();
  char atitlebuf[72];
  memset(atitlebuf, 0, sizeof(atitlebuf));
  if (nbattr > 0)
    {
      if (nbattr == 1)
        snprintf(atitlebuf, sizeof(atitlebuf),
                 "%s one attribute %s\n",
                 MomParser::_par_comment_start1_, MomParser::_par_comment_end1_);
      else
        snprintf(atitlebuf, sizeof(atitlebuf),
                 "%s %d attributes %s\n",
                 MomParser::_par_comment_start1_, nbattr, MomParser::_par_comment_end1_);

      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,atitlebuf, tagsattrindex);
      for (auto itattr : mapattrs)
        {
          MomObject*pobattr = itattr.first;
          MomValue valattr = itattr.second;
          txit = _mwi_browserbuf->insert_with_tags_by_name
                 (txit, "\342\210\231 " /* U+2219 BULLET OPERATOR ∙ */, tagsattrs);
          browser_insert_objptr(txit, pobattr, &dctxattrs, tagsattrobj, 0);
          browser_insert_space(txit, tagsattrs, 1);
          txit = _mwi_browserbuf->insert_with_tags_by_name
                 (txit, "\342\206\246" /* U+21A6 RIGHTWARDS ARROW FROM BAR ↦ */, tagsattrs);
          browser_insert_space(txit, tagsattrs, 1);
          browser_insert_value(txit, valattr, &dctxattrs, tagsattrval, 1);
          browser_insert_newline(txit, tagsattrs, 0);
        }
    }
  else  // nbattr is zero
    {
      snprintf(atitlebuf, sizeof(atitlebuf),
               "%s no attributes %s",
               MomParser::_par_comment_start1_, MomParser::_par_comment_end1_);
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,atitlebuf, tagsattrindex);
    }
} // end MomMainWindow::browser_insert_object_attributes


void
MomMainWindow::browser_insert_object_components(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob)
{
  MomDisplayCtx dctxcomps(&shob);
  std::vector<Glib::ustring> tagscomps{"components_tag"};
  std::vector<Glib::ustring> tagscompindex{"components_tag", "index_comment_tag"};
  std::vector<Glib::ustring> tagscompval{"components_tag", "compval_tag"};
  char atitlebuf[72];
  memset(atitlebuf, 0, sizeof(atitlebuf));
  unsigned nbcomp = pob->unsync_nb_comps();
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob=" << pob << " nbcomp=" << nbcomp);
  if (nbcomp == 0)
    {
      snprintf(atitlebuf, sizeof(atitlebuf),
               "%s no components %s\n",
               MomParser::_par_comment_start1_, MomParser::_par_comment_end1_);
    }
  else if (nbcomp == 1)
    {
      snprintf(atitlebuf, sizeof(atitlebuf),
               "%s one component %s\n",
               MomParser::_par_comment_start1_, MomParser::_par_comment_end1_);
    }
  else
    {
      snprintf(atitlebuf, sizeof(atitlebuf),
               "%s %u components %s\n",
               MomParser::_par_comment_start1_, nbcomp, MomParser::_par_comment_end1_);
    }
  txit = _mwi_browserbuf->insert_with_tags_by_name
         (txit,atitlebuf, tagscompindex);
  for (unsigned ix=0; ix<nbcomp; ix++)
    {
      snprintf(atitlebuf, sizeof(atitlebuf),
               "%s#%u%s",
               MomParser::_par_comment_start1_, ix, MomParser::_par_comment_end1_);
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,atitlebuf, tagscompindex);
      browser_insert_space(txit, tagscomps, 1);
      MomValue compval = pob->unsync_unsafe_comp_at(ix);
      browser_insert_value(txit, compval, &dctxcomps, tagscompval, 1);
      browser_insert_newline(txit, tagscomps, 0);
    }
} // end MomMainWindow::browser_insert_object_components


void
MomMainWindow::browser_insert_object_payload(Gtk::TextIter& txit, MomObject*pob, MomBrowsedObject& shob)
{
  MomDisplayCtx dctxpayl(&shob);
  MomPayload* payl = pob->unsync_payload();
  MOM_ASSERT(payl->_py_vtbl && payl->_py_vtbl->pyv_magic ==  MOM_PAYLOADVTBL_MAGIC,
             "browser_insert_object_display corrupted payload of pob=" << pob);
  std::vector<Glib::ustring> tagspayl{"payload_tag"};
  std::vector<Glib::ustring> tagspaylindex{"payload_tag", "index_comment_tag"};
  char atitlebuf[80];
  memset(atitlebuf, 0, sizeof(atitlebuf));
  snprintf(atitlebuf, sizeof(atitlebuf),
           "%s payload %s/%s %s",
           MomParser::_par_comment_start1_, payl->_py_vtbl->pyv_name,
           payl->_py_vtbl->pyv_module?:"_",
           MomParser::_par_comment_end1_);
  txit = _mwi_browserbuf->insert_with_tags_by_name
         (txit,atitlebuf, tagspaylindex);
  browser_insert_newline(txit, tagspayl, 0);
#warning MomMainWindow::browser_insert_object_payload should probably display the payload wisely
} // end MomMainWindow::browser_insert_object_payload

void
MomMainWindow::browser_insert_objptr(Gtk::TextIter& txit, MomObject*pob, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth)
{
  std::vector<Glib::ustring> tagscopy = tags;
  MOM_ASSERT(dcx != nullptr, "MomMainWindow::browser_insert_objptr null dcx");
  if (!pob)
    {
      tagscopy.push_back("objocc_nil_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              "__",
              tagscopy);
      return;
    }
  MOM_ASSERT(pob && pob->vkind() == MomKind::TagObjectK, "browser_insert_objptr corrupted pob");
  std::string obnamstr;
  char obidbuf[32];
  memset(obidbuf, 0, sizeof(obidbuf));
  pob->id().to_cbuf32(obidbuf);
  {
    std::shared_lock<std::shared_mutex> lk(pob->get_shared_mutex());
    obnamstr = mom_get_unsync_string_name(const_cast<MomObject*>(pob));
  }
  if (obnamstr.empty())
    {
      tagscopy.push_back("objocc_anon_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              obidbuf,
              tagscopy);
    }
  else
    {
      tagscopy.push_back("objocc_named_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              obnamstr.c_str(),
              tagscopy);
      if (depth<=2 && _mwi_dispid)
        {
          char bufcommid[48];
          memset (bufcommid, 0, sizeof(bufcommid));
          tagscopy.pop_back();
          tagscopy.insert(tagscopy.begin(), "obid_comment_tag");
          snprintf(bufcommid, sizeof(bufcommid), " |=%s|", obidbuf);
          txit = _mwi_browserbuf->insert_with_tags_by_name
                 (txit,
                  bufcommid,
                  tagscopy);
        }
    }
} // end MomMainWindow::browser_insert_objptr




void
MomMainWindow::browser_insert_value(Gtk::TextIter& txit, MomValue val, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags,  int depth)
{
  std::vector<Glib::ustring> tagscopy = tags;
  if (depth<0) depth=0;
  if (val.is_empty())
    {
      tagscopy.push_back("value_nil_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              "__",
              tagscopy);
      return;
    }
  else if (val.is_tagint())
    {
      auto iv = val.as_tagint();
      char numbuf[32];
      memset(numbuf, 0, sizeof(numbuf));
      snprintf(numbuf, sizeof(numbuf), "%lld", (long long) iv);
      tagscopy.push_back("value_numerical_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              numbuf,
              tagscopy);
      return;
    }
  MOM_ASSERT(val.is_val(), "browser_insert_value corrupted val");
  if (val.is_transient())
    {
      tagscopy.push_back("value_transient_tag");
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit,
              "°",
              tagscopy);
    };
  const MomAnyVal* vv = val.as_val();
  switch(vv->vkind())
    {
    //////
    case MomKind::TagObjectK: /// object occurence
    {
      auto obv = reinterpret_cast<const MomObject*>(vv);
      MOM_ASSERT(obv != nullptr && obv->vkind() == MomKind::TagObjectK,
                 "corrupted object in browser_insert_value");
      browser_insert_objptr(txit, const_cast<MomObject*>(obv), dcx, tags, depth);
    }
    break;
    //////
    case MomKind::TagIntSqK:  /// integer sequence
    {
      int openoff = -1, closeoff = -1;
      std::vector<Glib::ustring> tagsindex = tags;
      auto isv = reinterpret_cast<const MomIntSq*>(vv);
      unsigned sz = isv->sizew();
      tagscopy.push_back("value_numberseq_tag");
      tagsindex.push_back("index_comment_tag");
      std::vector<Glib::ustring> tagspairing = tagscopy;
      tagspairing.push_back("open_tag");
      tagspairing.push_back(Glib::ustring::compose("open%1_tag", depth));
      openoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, "(#", tagspairing);
      for (unsigned ix=0; ix<sz; ix++)
        {
          char numbuf[32];
          memset(numbuf, 0, sizeof(numbuf));
          if (ix>0)
            {
              browser_insert_space(txit, tagscopy, depth+1);
              if (ix % 10 == 0 && ix+4 < sz)
                {
                  browser_insert_space(txit, tagscopy, depth+1);
                  snprintf(numbuf, sizeof(numbuf), "|%d:|", ix);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          snprintf(numbuf, sizeof(numbuf), "%lld", (long long)isv->unsafe_at(ix));
          txit =
            _mwi_browserbuf->insert_with_tags_by_name  (txit,  numbuf, tagscopy);
        }
      tagspairing.pop_back();
      tagspairing.pop_back();
      tagspairing.push_back("close_tag");
      tagspairing.push_back(Glib::ustring::compose("close%1_tag", depth));
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit,   "#)", tagspairing);
      closeoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      MOM_DEBUGLOG(gui, "browser_insert_value intsq=" << val << " openoff=" << openoff
                   << " closeoff=" << closeoff);
      MOM_ASSERT(openoff>=0, "browser_insert_value intsq no openoff");
      MOM_ASSERT(closeoff>=openoff, "browser_insert_value intsq bad closeoff");
      MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                          .paroff_xtra= -1, .paroff_openlen=2, .paroff_closelen=2,
                          .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
      dcx->_dx_shob->add_parens(po);
    }
    break;
    //////
    case MomKind::TagDoubleSqK:  /// double sequence
    {
      int openoff = -1, closeoff = -1;
      std::vector<Glib::ustring> tagsindex = tags;
      auto dsv = reinterpret_cast<const MomDoubleSq*>(vv);
      unsigned sz = dsv->sizew();
      tagscopy.push_back("value_numberseq_tag");
      tagsindex.push_back("index_comment_tag");
      openoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, "(:", tagscopy);
      for (unsigned ix=0; ix<sz; ix++)
        {
          char numbuf[48];
          memset(numbuf, 0, sizeof(numbuf));
          if (ix>0)
            {
              browser_insert_space(txit, tagscopy, depth+1);
              if (ix % 10 == 0 && ix+4 < sz)
                {
                  browser_insert_space(txit, tagscopy, depth+1);
                  snprintf (numbuf, sizeof(numbuf), "|%d:|", ix);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          snprintf(numbuf, sizeof(numbuf), "%.15g", dsv->unsafe_at(ix));
          txit =
            _mwi_browserbuf->insert_with_tags_by_name (txit, numbuf, tagscopy);
        }
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, ":)",  tagscopy);
      closeoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      MOM_DEBUGLOG(gui, "browser_insert_value doublesq=" << val << " openoff=" << openoff
                   << " closeoff=" << closeoff);
      MOM_ASSERT(openoff>=0, "browser_insert_value doublesq no openoff");
      MOM_ASSERT(closeoff>=openoff, "browser_insert_value doublesq bad closeoff");
      MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                          .paroff_xtra= -1, .paroff_openlen=2, .paroff_closelen=2,
                          .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
      dcx->_dx_shob->add_parens(po);
    }
    break;
    ////
    case MomKind::TagStringK: /// string value
    {
      std::vector<Glib::ustring> tagsescape = tags;
      auto strv = reinterpret_cast<const MomString*>(vv);
      std::string str = strv->string();
      txit = _mwi_browserbuf->insert_with_tags_by_name	(txit, "\"", tags);
      tagscopy.push_back("value_string_tag");
      tagsescape.push_back("value_stresc_tag");
      const char *s = str.c_str();
      int len = strlen(s);
      const char* end = s+len;
      assert (s && g_utf8_validate (s, len, nullptr));
      gunichar uc = 0;
      for (const char *pc = s; pc < end; pc = g_utf8_next_char (pc), uc = 0)
        {
          int linoff = txit.get_line_offset();
          if ((linoff +2 >=  _mwi_dispwidth && pc + 10 < end)
              || (isspace(*pc) && linoff + 10 >=  2*_mwi_dispwidth/3 && pc + 10 < end))
            {
              txit = _mwi_browserbuf->insert_with_tags_by_name (txit, "\"", tags);
              browser_insert_newline(txit, tags, depth);
              txit = _mwi_browserbuf->insert_with_tags_by_name (txit, "&+&", tagsescape);
              txit = _mwi_browserbuf->insert_with_tags_by_name (txit, " \"", tags);
            }
          uc = g_utf8_get_char (pc);
          switch (uc)
            {
            case 0:
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\0", tagsescape);
              break;
            case '\"':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\\"", tagsescape);
              break;
            case '\'':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\\'", tagsescape);
              break;
            case '\a':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\a", tagsescape);
              break;
            case '\b':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\b", tagsescape);
              break;
            case '\f':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\f", tagsescape);
              break;
            case '\n':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\n", tagsescape);
              break;
            case '\r':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\r", tagsescape);
              break;
            case '\t':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\t", tagsescape);
              break;
            case '\v':
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\v", tagsescape);
              break;
            case '\033' /*ESCAPE*/:
              txit =
                _mwi_browserbuf->insert_with_tags_by_name (txit, "\\e", tagsescape);
              break;
            default:
            {
              char buf[16];
              memset (buf, 0, sizeof(buf));
              if ((uc>=' ' && uc<127) || g_unichar_isprint(uc))
                {
                  g_unichar_to_utf8(uc, buf);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name (txit, buf, tagscopy);
                }
              else if (uc<0xffff)
                {
                  snprintf (buf, sizeof (buf), "\\u%04x", (int) uc);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name (txit, buf, tagsescape);
                }
              else
                {
                  snprintf (buf, sizeof (buf), "\\U%08x", (int) uc);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name (txit, buf, tagsescape);
                }
            }
            break;
            }
        }
      txit = _mwi_browserbuf->insert_with_tags_by_name
             (txit, "\"", tags);
    }
    break;
    case MomKind::TagSetK:
    case MomKind::TagTupleK:
    {
      auto seqv = reinterpret_cast<const MomAnyObjSeq*>(vv);
      int openoff= -1, closeoff= -1;
      bool istuple = (vv->vkind() == MomKind::TagTupleK);
      unsigned sz = seqv->sizew();
      std::vector<Glib::ustring> tagsindex = tags;
      tagscopy.push_back("value_sequence_tag");
      std::vector<Glib::ustring> tagspairing = tagscopy;
      tagsindex.push_back("index_comment_tag");
      tagspairing.push_back("open_tag");
      tagspairing.push_back(Glib::ustring::compose("open%1_tag", depth));
      openoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, (istuple?"[":"{"), tagspairing);
      for (unsigned ix=0; ix<sz; ix++)
        {
          if (ix>0)
            {
              browser_insert_space(txit, tagscopy, depth+1);
              if (ix % 10 == 0 && ix+4 < sz)
                {
                  char numbuf[24];
                  memset(numbuf, 0, sizeof(numbuf));
                  browser_insert_space(txit, tagscopy, depth+1);
                  snprintf(numbuf, sizeof(numbuf), "|%d:|", ix);
                  txit =
                    _mwi_browserbuf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          browser_insert_objptr(txit, seqv->unsafe_at(ix), dcx,
                                tagscopy, depth+1);
        };
      tagspairing.pop_back();
      tagspairing.pop_back();
      tagspairing.push_back("close_tag");
      tagspairing.push_back(Glib::ustring::compose("close%1_tag", depth));
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, (istuple?"]":"}"), tagspairing);
      closeoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      MOM_DEBUGLOG(gui, "browser_insert_value sequence=" << val << " openoff=" << openoff
                   << " closeoff=" << closeoff);
      MOM_ASSERT(openoff>=0, "browser_insert_value sequence no openoff");
      MOM_ASSERT(closeoff>=openoff, "browser_insert_value sequence bad closeoff");
      MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                          .paroff_xtra= -1, .paroff_openlen=1, .paroff_closelen=1,
                          .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
      dcx->_dx_shob->add_parens(po);
    }
    break;
    //////
    case MomKind::TagNodeK:  /// node value
    {
      int staroff= -1, openoff= -1, closeoff= -1;
      auto nodv = reinterpret_cast<const MomNode*>(vv);
      unsigned sz = nodv->sizew();
      std::vector<Glib::ustring> tagsindex = tags;
      tagscopy.push_back("value_node_tag");
      std::vector<Glib::ustring> tagspairing = tagscopy;
      tagsindex.push_back("index_comment_tag");
      tagspairing.push_back("star_tag");
      tagspairing.push_back(Glib::ustring::compose("star%1_tag", depth));
      staroff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, "*", tagspairing);
      tagspairing.pop_back();
      tagspairing.pop_back();
      browser_insert_objptr(txit, nodv->conn(), dcx, tagscopy, depth);
      browser_insert_space(txit, tagscopy, depth);
      tagspairing.push_back("open_tag");
      tagspairing.push_back(Glib::ustring::compose("open%1_tag", depth));
      openoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, "(", tagspairing);
      if (depth >= _mwi_dispdepth)
        {
          txit =
            _mwi_browserbuf->insert_with_tags_by_name (txit,
                // U+2026 HORIZONTAL ELLIPSIS …
                "|\342\200\246|",
                tagscopy);
        }
      else
        {
          for (unsigned ix=0; ix<sz; ix++)
            {
              if (ix>0)
                {
                  browser_insert_space(txit, tagscopy, depth+1);
                  if (ix % 10 == 0 && ix+4 < sz)
                    {
                      char numbuf[24];
                      memset(numbuf, 0, sizeof(numbuf));
                      browser_insert_space(txit, tagscopy, depth+1);
                      snprintf(numbuf, sizeof(numbuf), "|%d:|", ix);
                      txit =
                        _mwi_browserbuf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                    }
                }
              browser_insert_value(txit, nodv->unsafe_at(ix), dcx,
                                   tagscopy, depth+1);
            };
        }
      tagspairing.pop_back();
      tagspairing.pop_back();
      tagspairing.push_back("close_tag");
      tagspairing.push_back(Glib::ustring::compose("close%1_tag", depth));
      txit =
        _mwi_browserbuf->insert_with_tags_by_name (txit, ")", tagspairing);
      closeoff = (txit.get_offset() - dcx->_dx_shob->_sh_startmark->get_iter().get_offset());
      MOM_DEBUGLOG(gui, "browser_insert_value node=" << val
                   << " staroff=" << staroff
                   << " openoff=" << openoff
                   << " closeoff=" << closeoff);
      MOM_ASSERT(staroff>=0, "browser_insert_value sequence no openoff");
      MOM_ASSERT(openoff>staroff, "browser_insert_value sequence no openoff");
      MOM_ASSERT(closeoff>openoff, "browser_insert_value sequence bad closeoff");
      MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                          .paroff_xtra= staroff, .paroff_openlen=1, .paroff_closelen=1,
                          .paroff_xtralen= 1, .paroff_depth=(uint8_t)depth};
      dcx->_dx_shob->add_parens(po);
    }
    break;
    //////
    case MomKind::TagIntK:
    case MomKind::TagNoneK:
    case MomKind::Tag_LastK:
      MOM_FATAPRINTF("MomMainWindow::browser_insert_value impossible tag %d",
                     (int) vv->vkind());
    }
} // end MomMainWindow::browser_insert_value

MomObject*
MomMainWindow::browser_object_around(Gtk::TextIter txit)
{
  // perhaps should use std::binary_search
  for (auto it:  _mwi_browsedobmap)
    {
      MomObject* curpob = it.first;
      MomBrowsedObject& curbob = it.second;
      if (curbob._sh_startmark && curbob._sh_endmark)
        {
          Gtk::TextIter obstartit = curbob._sh_startmark->get_iter();
          Gtk::TextIter obendit =  curbob._sh_endmark->get_iter();
          if (txit.in_range(obstartit, obendit))
            return curpob;
        }
    }
  return nullptr;
} // end MomMainWindow::browser_object_around

bool
MomMainWindow::do_browser_blink_insert(void)
{
  MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert start");
  auto blinktag = MomApplication::itself()->lookup_browser_tag("blink_tag");
  blinktag->set_priority(MomApplication::itself()->nb_browser_tags()-1);
  MomBrowsedObject* pbob = nullptr;
  MomParenOffsets* ppo=nullptr;
  bool opening=false, closing=false;
  bool found = found_browsed_object_around_insert(pbob, ppo,opening,closing);
  MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert end found=" << found
               << " pbob=" << pbob << " ppo=" << ppo
               << " opening=" << opening << " closing=" << closing);
  if (found && ppo != nullptr && pbob != nullptr)
    {
      MomObject*pob = pbob->_sh_ob;
      Gtk::TextIter startxit = pbob->_sh_startmark->get_iter();
      Gtk::TextIter opentxit = startxit;
      Gtk::TextIter closetxit = startxit;
      int offopen = ppo->paroff_open;
      int offclose = ppo->paroff_close;
      int offxtra = ppo->paroff_xtra;
      int openlen = ppo->paroff_openlen;
      int closelen = ppo->paroff_closelen;
      int xtralen = ppo->paroff_xtralen;
      MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert in pob=" << pob << " offopen=" << offopen
                   << " openlen=" << openlen
                   << " offclose=" << offclose << " closelen=" << closelen
                   << " offxtra=" << offxtra << " xtralen=" << xtralen);
      opentxit.forward_chars(offopen);
      closetxit.forward_chars(offclose);
      _mwi_browser_startblink = _mwi_browserbuf->create_mark("start_blink_browser", opentxit);
      if (openlen>0)
        {
          Gtk::TextIter endopentxit = opentxit;
          endopentxit.forward_chars(openlen);
          MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert in pob=" << pob << " opentxit=" << MomShowTextIter(opentxit)
                       << " endopentxit=" << MomShowTextIter(endopentxit));
          _mwi_browserbuf->apply_tag(blinktag, opentxit, endopentxit);
        }
      _mwi_browser_endblink = _mwi_browserbuf->create_mark("end_blink_browser", closetxit);
      if (closelen>0)
        {
          Gtk::TextIter begclosetxit = closetxit;
          begclosetxit.backward_chars(closelen);
          MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert in pob=" << pob << " begclosetxit=" << MomShowTextIter(begclosetxit)
                       << " closetxit=" << MomShowTextIter(closetxit));
          _mwi_browserbuf->apply_tag(blinktag, begclosetxit, closetxit);
        }
      if (xtralen>0)
        {
          Gtk::TextIter xtratxit= startxit;
          xtratxit.forward_chars(offxtra);
          Gtk::TextIter endxtratxit = xtratxit;
          endxtratxit.forward_chars(xtralen);
          MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert in pob=" << pob << " xtratxit=" << MomShowTextIter(xtratxit)
                       << " endxtratxit=" << MomShowTextIter(endxtratxit));
          _mwi_browserbuf->apply_tag(blinktag, xtratxit, endxtratxit);
          _mwi_browser_xtrablink = _mwi_browserbuf->create_mark("xtra_blink_browser", xtratxit);
        }
      Glib::signal_timeout().connect_once
      (sigc::mem_fun(this,&MomMainWindow::do_browser_unblink_insert),
       _blink_delay_milliseconds);
    }
  else
    {
      MOM_DEBUGLOG(blinkgui, "do_browser_blink_insert outside");
      _mwi_browser_startblink.clear();
      _mwi_browser_xtrablink.clear();
      _mwi_browser_endblink.clear();
    }
  return true;
} // end MomMainWindow::do_browser_blink_insert

void
MomMainWindow::do_browser_unblink_insert(void)
{
  MOM_DEBUGLOG(blinkgui, "do_browser_unblink_insert start");
  auto blinktag = MomApplication::itself()->lookup_browser_tag("blink_tag");
  if (_mwi_browser_startblink && _mwi_browser_endblink)
    {
      Gtk::TextIter startxit = _mwi_browser_startblink->get_iter();
      Gtk::TextIter endtxit = _mwi_browser_endblink->get_iter();
      MOM_DEBUGLOG(blinkgui, "do_browser_unblink_insert startxit=" << MomShowTextIter(startxit)
                   << " endtxit=" << MomShowTextIter(endtxit));
      _mwi_browserbuf->remove_tag(blinktag, startxit, endtxit);
    }
  if (_mwi_browser_xtrablink)
    {
      Gtk::TextIter xtratxit = _mwi_browser_xtrablink->get_iter();
      Gtk::TextIter bolxtratxit = xtratxit;
      bolxtratxit.backward_line();
      xtratxit.forward_line();
      xtratxit.forward_char();
      MOM_DEBUGLOG(blinkgui, "do_browser_unblink_insert xtraxit=" << MomShowTextIter(xtratxit)
                   << " bolxtratxit=" << MomShowTextIter(bolxtratxit));
      _mwi_browserbuf->remove_tag(blinktag, bolxtratxit, xtratxit);
    }
  _mwi_browser_startblink.clear();
  _mwi_browser_xtrablink.clear();
  _mwi_browser_endblink.clear();
} // end MomMainWindow::do_browser_unblink_insert

void
MomMainWindow::do_browser_mark_set(const Gtk::TextIter& locit,
                                   const Glib::RefPtr<Gtk::TextMark>& mark)
{
  if (mark != _mwi_browserbuf->get_insert())
    return;
  do_browser_unblink_insert();
  MomObject*pob = browser_object_around(locit);
  MOM_DEBUGLOG(gui, "do_browser_mark_set insertmark locit=" << MomShowTextIter(locit)
               << " around pob=" << MomShowObject(pob));
  if (pob)
    do_browser_blink_insert();
} // end MomMainWindow::do_browser_mark_set

void
MomMainWindow::do_txcmd_mark_set(const Gtk::TextIter& locit,
                                 const Glib::RefPtr<Gtk::TextMark>& mark)
{
  if (mark != _mwi_commandbuf->get_insert())
    return;
  MOM_DEBUGLOG(gui, "do_txcmd_mark_set insertmark locit=" << MomShowTextIter(locit)
               << " oldinsertmark at=" << MomShowTextIter(mark->get_iter()));
  do_txcmd_unblink_insert();
  do_txcmd_blink_insert();
  MOM_DEBUGLOG(gui, "do_txcmd_mark_set insertmark done");
} // end MomMainWindow::do_txcmd_mark_set

bool
MomMainWindow::found_browsed_object_around_insert(MomBrowsedObject*&pbob, MomParenOffsets*&po,
    bool&opening, bool&closing)
{
  Gtk::TextIter insertxit = _mwi_browserbuf->get_insert()->get_iter();
  MomObject* pob = browser_object_around(insertxit);
  opening = false;
  closing = false;
  if (!pob)
    {
      pbob = nullptr;
      po = nullptr;
      MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
                   << " outside of object");
      return false;
    }
  auto showit = _mwi_browsedobmap.find(pob);
  MOM_ASSERT(showit != _mwi_browsedobmap.end(), "found_browsed_object_at_insert bad showit");
  MomBrowsedObject& showbob = showit->second;
  pbob = &showbob;
  po = nullptr;
  int markoff = insertxit.get_offset() - showbob._sh_startmark->get_iter().get_offset();
  MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
               << " inside pob=" << pob << " markoff=" << markoff);
  {
    auto openit = showbob._sh_openmap.find(markoff);
    if (openit != showbob._sh_openmap.end())
      {
        po = &openit->second;
        opening = true;
        MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
                     << " opening pob=" << pob << std::endl
                     << ".. paroff: open=" << (int)(po->paroff_open) << " openlen=" << (int)(po->paroff_openlen)
                     << " close=" << (int)(po->paroff_close) << " closelen=" << (int)(po->paroff_closelen) << std::endl
                     << ".. xtra=" << (int)(po->paroff_xtra) << " xtralen=" << (int)(po->paroff_xtralen)
                     << " depth=" << (int)(po->paroff_depth)
                    );
        return true;
      }
  }
  {
    auto closeit = showbob._sh_closemap.find(markoff);
    if (closeit != showbob._sh_closemap.end())
      {
        po = &closeit->second;
        closing = true;
        MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
                     << " closing pob=" << pob << std::endl
                     << ".. paroff: open=" << (int)(po->paroff_open) << " openlen=" << (int)(po->paroff_openlen)
                     << " close=" << (int)(po->paroff_close) << " closelen=" << (int)(po->paroff_closelen) << std::endl
                     << ".. xtra=" << (int)(po->paroff_xtra) << " xtralen=" << (int)(po->paroff_xtralen)
                     << " depth=" << (int)(po->paroff_depth)
                    );
        return true;
      }
  }
  ///
  for (auto afterit = showbob._sh_openmap.lower_bound(markoff);
       afterit != showbob._sh_openmap.end();
       afterit --)
    {
      MomParenOffsets*afterpo = nullptr;
      if (afterit != showbob._sh_openmap.end()
          && ((afterpo=&afterit->second),
              (afterpo->surrounds(markoff))))
        {
          po = afterpo;
          MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
                       << " after, pob=" << pob << std::endl
                       << ".. paroff: open=" << (int)(po->paroff_open) << " openlen=" << (int)(po->paroff_openlen)
                       << " close=" << (int)(po->paroff_close) << " closelen=" << (int)(po->paroff_closelen) << std::endl
                       << ".. xtra=" << (int)(po->paroff_xtra) << " xtralen=" << (int)(po->paroff_xtralen)
                       << " depth=" << (int)(po->paroff_depth)
                      );
          return true;
        }
      if (afterit == showbob._sh_openmap.begin())
        break;
    };
  //
  for (auto beforeit = showbob._sh_closemap.upper_bound(markoff);
       beforeit != showbob._sh_closemap.end();
       beforeit ++)
    {
      MomParenOffsets*beforepo = nullptr;
      if (beforeit != showbob._sh_closemap.end()
          && ((beforepo=&beforeit->second),
              (beforepo->surrounds(markoff))))
        {
          po = beforepo;
          MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
                       << " before, pob=" << pob << std::endl
                       << ".. paroff: open=" << (int)(po->paroff_open) << " openlen=" << (int)(po->paroff_openlen)
                       << " close=" << (int)(po->paroff_close) << " closelen=" << (int)(po->paroff_closelen) << std::endl
                       << ".. xtra=" << (int)(po->paroff_xtra) << " xtralen=" << (int)(po->paroff_xtralen)
                       << " depth=" << (int)(po->paroff_depth)
                      );
          return true;
        }
    }
  MOM_DEBUGLOG(blinkgui, "found_browsed_object_around_insert insertxit=" << MomShowTextIter(insertxit)
               << " none, pob=" << pob);
  return true;
} // end MomMainWindow::found_browsed_object_around_insert


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
    _mwi_winrank(++_mwi_wincount),
    _mwi_vbox(Gtk::ORIENTATION_VERTICAL),
    _mwi_menubar(),
    _mwi_mit_app("_App",true),
    _mwi_mit_edit("_Edit",true),
    _mwi_mit_object("_Object",true),
    _mwi_menu_app(),
    _mwi_menu_edit(),
    _mwi_menu_object(),
    _mwi_mit_app_quit("_Quit",true),
    _mwi_mit_app_exit("e_Xit",true),
    _mwi_mit_app_dump("_Dump",true),
    _mwi_mit_edit_copy("_Copy",true),
    _mwi_mit_object_show_hide("_Show/hide",true),
    _mwi_mit_object_refresh("_Refresh",true),
    _mwi_mit_object_options("_Options",true),
    _mwi_mit_txcmd_clear("_Clear",true),
    _mwi_mit_txcmd_runclear("_Run then clear",true),
    _mwi_mit_txcmd_runkeep("run but _Keep",true),
    _mwi_mit_txcmd_cursorloc("@"),
    _mwi_browserbuf(Gtk::TextBuffer::create(MomApplication::itself()->browser_tagtable())),
    _mwi_commandbuf(Gtk::TextBuffer::create(MomApplication::itself()->command_tagtable())),
    _mwi_dispdepth(_default_display_depth_),
    _mwi_dispwidth(_default_display_width_),
    _mwi_dispid(false),
    _mwi_panedtx(Gtk::ORIENTATION_VERTICAL),
    _mwi_txvtop(_mwi_browserbuf), _mwi_txvbot(_mwi_browserbuf),
    _mwi_sepcmd(Gtk::ORIENTATION_HORIZONTAL),
    _mwi_scrcmd(),
    _mwi_txvcmd(_mwi_commandbuf),
    _mwi_statusbar(),
    _mwi_browsedobmap(),
    _mwi_cmduseractim(0.0),
    _mwi_cmdlastuseractim(0.0),
    _mwi_focusobj(nullptr),
    _mwi_winobj(nullptr),
    _mwi_txcmd_nbmodif(0)
{
  {
    auto screen = Gdk::Screen::get_default();
    auto ctx = get_style_context();
    ctx->add_provider_for_screen(screen,MomApplication::itself()->css_provider(), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  }
  add(_mwi_vbox);
  _mwi_menubar.append(_mwi_mit_app);
  _mwi_menubar.append(_mwi_mit_edit);
  _mwi_menubar.append(_mwi_mit_object);
  _mwi_mit_app.set_submenu(_mwi_menu_app);
  _mwi_mit_edit.set_submenu(_mwi_menu_edit);
  _mwi_menu_app.append(_mwi_mit_app_quit);
  _mwi_menu_app.append(_mwi_mit_app_exit);
  _mwi_menu_app.append(_mwi_mit_app_dump);
  _mwi_mit_app_quit.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_quit));
  _mwi_mit_app_exit.signal_activate().connect(sigc::mem_fun(*MomApplication::itself(),&MomApplication::do_exit));
  _mwi_mit_app_dump.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_window_dump));
  _mwi_menu_edit.append(_mwi_mit_edit_copy);
  _mwi_mit_object.set_submenu(_mwi_menu_object);
  _mwi_menu_object.append(_mwi_mit_object_show_hide);
  _mwi_menu_object.append(_mwi_mit_object_refresh);
  _mwi_menu_object.append(_mwi_mit_object_options);
  _mwi_mit_object_show_hide.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_show_hide));
  _mwi_mit_object_refresh.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_refresh));
  _mwi_mit_object_options.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_options));
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
  _mwi_scrcmd.add(_mwi_txvcmd);
  _mwi_scrcmd.set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_ALWAYS);
  _mwi_panedtx.add1(_mwi_scrwtop);
  _mwi_panedtx.add2(_mwi_scrwbot);
  _mwi_panedtx.set_wide_handle(true);
  _mwi_browserbuf->signal_mark_set().connect(sigc::mem_fun(*this,&MomMainWindow::do_browser_mark_set));
  {
    _mwi_vbox.pack_start(_mwi_sepcmd,Gtk::PACK_SHRINK);
    _mwi_vbox.pack_start(_mwi_scrcmd,Gtk::PACK_SHRINK);
    _mwi_txvcmd.set_vexpand(false);
    auto ctx = _mwi_txvcmd.get_style_context();
    ctx->add_class("commandwin_cl");
  }
  _mwi_vbox.pack_end(_mwi_statusbar,Gtk::PACK_SHRINK);
  //_mwi_txvcmd.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_options));
  {
    auto cmdbuf = _mwi_txvcmd.get_buffer();
    cmdbuf->signal_begin_user_action().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_begin_user_action));
    cmdbuf->signal_changed().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_changed));
    cmdbuf->signal_end_user_action().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_end_user_action));
  }
  _mwi_txvcmd.signal_populate_popup().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_populate_menu));
  _mwi_txvcmd.add_events(Gdk::KEY_RELEASE_MASK);
  _mwi_txvcmd.signal_key_press_event().connect(sigc::mem_fun(this,&MomMainWindow::handle_txcmd_key_release), false);
  _mwi_mit_txcmd_clear.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_clear));
  _mwi_mit_txcmd_runclear.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_run_then_clear));
  _mwi_mit_txcmd_runkeep.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_txcmd_run_but_keep));
  _mwi_commandbuf->signal_mark_set().connect(sigc::mem_fun(*this,&MomMainWindow::do_txcmd_mark_set));
  set_default_size(630,480);
  property_title().set_value(Glib::ustring::compose("mom window #%1", _mwi_winrank));
  _mwi_winobj = MomObject::make_object();
  _mwi_winobj->unsync_make_payload<MomPaylMainWindow>(this);
  Glib::signal_timeout().connect
  (sigc::mem_fun(this,&MomMainWindow::do_browser_blink_insert),
   _blink_period_milliseconds);
  display_full_browser();
  show_all_children();
};				// end MomMainWindow::MomMainWindow


MomMainWindow::~MomMainWindow()
{
  _mwi_browsedobmap.clear();
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
MomMainWindow::do_object_show_hide(void)
{
  enum { REPEAT=0, ShowOb=1, HideOb=2 };
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide");
  Gtk::Dialog showdialog("Show/Hide object", *this, true/*modal*/);
  Gtk::Box* showcontbox = showdialog.get_content_area();
  Gtk::Label showlabel("show/hide:");
  MomComboBoxObjptrText showcombox;
  showcontbox->pack_end(showlabel,Gtk::PACK_EXPAND_PADDING,3);
  showcontbox->pack_end(showcombox,Gtk::PACK_EXPAND_WIDGET,3);
  showdialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  showdialog.add_button("Show", ShowOb);
  showdialog.add_button("Hide", HideOb);
  showdialog.set_default_response(Gtk::RESPONSE_CANCEL);
  showcombox.signal_changed()
  .connect([&](void)
  {
    Glib::ustring showtext = showcombox.get_active_text();
    MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide showcombox changed showtext="
                 << MomShowString(showtext.c_str()));
    MomObject* pob = showcombox.active_object();
    MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide showcombox changed pob=" << pob);
    if (pob)
      {
        if (_mwi_browsedobmap.find(pob) != _mwi_browsedobmap.end())
          {
            MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide showcombox default hiding pob=" << MomShowObject(pob));
            showdialog.set_default_response(HideOb);
          }
        else
          {
            MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide showcombox default showing pob=" << MomShowObject(pob));
            showdialog.set_default_response(ShowOb);
          }
        showdialog.show_all_children();
      };
  });
  showdialog.show_all_children();
  int result =  Gtk::RESPONSE_CANCEL;
  do
    {
      result = showdialog.run();
      Glib::ustring showtext = showcombox.get_active_text();
      MomObject* pob = nullptr;
      if (showtext[0] < 127 && isalpha(showtext[0]))
        {
          pob = mom_find_named(showtext.c_str());
        }
      else if (showtext[0] == '_' && showtext[1] < 127 && isdigit(showtext[1]))
        {
          MomIdent idob = MomIdent::make_from_string(showtext.c_str(), MomIdent::DONT_FAIL);
          pob = MomObject::find_object_of_id(idob);
        }
      else if (showtext[0] == '@' && showtext[1] < 127 && isalpha(showtext[1]))
        {
          std::string globnamstr= showtext.substr(1);
          bool foundglobname = false;
          {
            auto globptr = MomRegisterGlobData::find_globdata(globnamstr);
            if (globptr)
              {
                foundglobname = true;
                pob = globptr->load();
              }
          }
          if (!foundglobname)
            result = REPEAT;
        }
      MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide result=" << result
                   << " showtext=" << MomShowString(showtext.c_str()) << " pob=" << pob);
      switch (result)
        {
        case ShowOb:
          MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide show showtext=" << MomShowString(showtext.c_str()));
          MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide show pob=" << pob);
          if (pob)
            {
              browser_show_object(pob);
              browser_set_focus_object(pob);
            }
          else
            result = REPEAT;
          break;
        case HideOb:
          MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide hide showtext=" << MomShowString(showtext));
          if (pob)
            browser_hide_object(pob);
          else
            result = REPEAT;
          break;
        case Gtk::RESPONSE_CANCEL:
          MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide cancel");
          break;
        }
    }
  while (result == REPEAT);
  showdialog.hide();
#warning incomplete MomMainWindow::do_object_show_hide
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide end");
} // end MomMainWindow::do_object_show_hide

void
MomMainWindow::do_object_options(void)
{
  int res=0;
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_options start");
  Gtk::Dialog optiondialog("Show options", *this, true/*modal*/);
  Gtk::Box* optcontbox = optiondialog.get_content_area();
  Gtk::Box subbox(Gtk::ORIENTATION_HORIZONTAL,3);
  optcontbox->pack_end(subbox,Gtk::PACK_EXPAND_WIDGET,3);
  Gtk::Label depthlabel("depth");
  subbox.pack_end(depthlabel,Gtk::PACK_SHRINK,2);
  Gtk::Scale depthscale(Gtk::ORIENTATION_HORIZONTAL);
  depthscale.set_range(_min_display_depth_, _max_display_depth_);
  depthscale.set_increments(1.0,3.0);
  depthscale.set_value(_mwi_dispdepth);
  for (int d = _min_display_depth_; d<= _max_display_depth_; d++)
    depthscale.add_mark(d,Gtk::POS_BOTTOM,Glib::ustring::compose("%1", d));
  subbox.pack_end(depthscale,Gtk::PACK_EXPAND_WIDGET,3);
  Gtk::Separator sep(Gtk::ORIENTATION_VERTICAL);
  subbox.pack_end(sep,Gtk::PACK_SHRINK,3);
  Gtk::CheckButton displayidbut("display ids");
  displayidbut.set_active(_mwi_dispid);
  subbox.pack_end(displayidbut,Gtk::PACK_EXPAND_WIDGET,3);
  optiondialog.add_button("Apply", Gtk::RESPONSE_APPLY);
  optiondialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  optiondialog.set_default_size(440,-1);
  optiondialog.show_all_children();
  res = optiondialog.run();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_options res=" << res);
  if (res ==  Gtk::RESPONSE_APPLY)
    {
      double depthval = depthscale.get_value();
      int newdepth = depthval;
      if (newdepth<_min_display_depth_)
        newdepth = _min_display_depth_;
      else if (newdepth>_max_display_depth_)
        newdepth = _max_display_depth_;
      bool dispids = displayidbut.get_active();
      MOM_DEBUGLOG(gui, "MomMainWindow::do_object_options raw depthval=" << depthval
                   << ", newdepth=" << newdepth
                   << " dispids=" << (dispids?"true":"false"));
      _mwi_dispdepth = newdepth;
      _mwi_dispid = dispids;
    }
  optiondialog.hide();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_options end");
} // end MomMainWindow::do_object_options

bool
MomMainWindow::handle_txcmd_key_release(GdkEventKey*evk)
{
  // see <gdk/gdkkeysyms.h> for names of keysyms
  // perhaps should also handle GDK_KEY_KP_Enter
  if (evk->keyval == GDK_KEY_Return)
    {
      auto modifiers = Gtk::AccelGroup::get_default_mod_mask();
      bool withctrl = (evk->state & modifiers) == GDK_CONTROL_MASK;
      bool withshift = (evk->state & modifiers) == GDK_SHIFT_MASK;
      MOM_DEBUGLOG(gui, "handle_txcmd_key_release got return key"
                   << ' ' <<(withshift?"shifted":"unshifted")
                   << ' ' << (withctrl?"control":"nocontrol"));
      if (withshift)
        {
          MOM_DEBUGLOG(gui, "handle_txcmd_key_release dealing with shift return");
          do_txcmd_run_then_clear();
          MOM_DEBUGLOG(gui, "handle_txcmd_key_release done with shift return");
          return true;		// so retain, don't forward this event
        }
      else if (withctrl)
        {
          MOM_DEBUGLOG(gui, "handle_txcmd_key_release dealing with ctrl return");
          do_txcmd_run_but_keep();
          MOM_DEBUGLOG(gui, "handle_txcmd_key_release done with ctrl return");
          return true;// so retain, don't forward this event
        }
      MOM_DEBUGLOG(gui, "handle_txcmd_key_release done & propagate plain return");
    }
  else if (evk->keyval == GDK_KEY_Tab)
    {
      auto modifiers = Gtk::AccelGroup::get_default_mod_mask();
      bool withctrl = (evk->state & modifiers) == GDK_CONTROL_MASK;
      bool withshift = (evk->state & modifiers) == GDK_SHIFT_MASK;
      Gtk::TextIter insertxit = _mwi_commandbuf->get_insert()->get_iter();
      MOM_DEBUGLOG(gui, "handle_txcmd_key_release got tab key"
                   << ' ' <<(withshift?"shifted":"unshifted")
                   << ' ' << (withctrl?"control":"nocontrol")
                   << " insertxit=" << MomShowTextIter(insertxit, MomShowTextIter::_FULL_,8) << std::endl
                   << "... starts_word=" << (insertxit.starts_word()?"true":"false")
                   << " ends_word=" << (insertxit.ends_word()?"true":"false")
                   << " inside_word=" << (insertxit.inside_word()?"true":"false")
                  );
      Gtk::TextIter startinstxit = insertxit;
      Gtk::TextIter endinstxit = insertxit;
      if (insertxit.ends_word() || insertxit.inside_word())
        {
          Gtk::TextIter befinstxit = insertxit;
          Gtk::TextIter afterinstxit = insertxit;
          bool again = false;
          again = !insertxit.is_start();
          while (again)
            {
              befinstxit.backward_char();
              int c=0;
              {
                Glib::ustring span = befinstxit.get_text(insertxit);
                if (span.size()>0)
                  c = span[0];
              }
              if (c<=0 || c>=127 || !(isalnum(c) || c=='_'))
                break;
              startinstxit = befinstxit;
              again = !(befinstxit.starts_line() || befinstxit.is_start());
            }
          again = !insertxit.is_end();
          while (again)
            {
              if (afterinstxit.ends_line())
                break;
              afterinstxit.forward_char();
              int c = 0;
              {
                Glib::ustring span = insertxit.get_text(afterinstxit);
                auto spansize = span.size();
                if (spansize>0)
                  c = span[spansize-1];
              }
              if (c<=0 || c>=127 || !(isalnum(c) || c=='_'))
                break;
              endinstxit = afterinstxit;
            }
          Glib::ustring wordustr = startinstxit.get_text(endinstxit);
          MOM_DEBUGLOG(gui, "handle_txcmd_key_release tab wordustr=" << MomShowString(wordustr.c_str())
                       << " startinstxit=" << MomShowTextIter(startinstxit)
                       << " endinstxit=" << MomShowTextIter(endinstxit));
          std::string strword = wordustr;
          bool completed = do_txcmd_complete_word (strword, startinstxit, endinstxit);
          if (!completed)
            {
              _mwi_txvcmd.error_bell();
              return true;
            }
          else
            {
              MOM_DEBUGLOG(gui, "handle_txcmd_key_release tab completed");
              do_txcmd_prettify_parse();
              do_txcmd_blink_insert();
              MOM_DEBUGLOG(gui, "handle_txcmd_key_release tab ending completed");
              return true;
            };
        }
      _mwi_txvcmd.error_bell();
      return true; // never propagate tabulation
    }
  return false; // propagate the event
}		// end MomMainWindow::handle_txcmd_key_release

bool
MomMainWindow::do_txcmd_complete_word(std::string word, Gtk::TextIter startxit, Gtk::TextIter endtxit)
{
  MOM_DEBUGLOG(gui, "do_txcmd_complete_word start word=" << MomShowString(word)
               << " startxit=" << MomShowTextIter(startxit)
               << " endtxit=" << MomShowTextIter(endtxit));
  std::vector<std::string> complvec;
  complvec.reserve(12);
  if (isalpha(word[0]))
    {
      mom_each_name_prefixed(word.c_str(),[&](const std::string&name, MomObject*pobnamed)
      {
        MOM_DEBUGLOG(gui, "do_txcmd_complete_word name=" << MomShowString(name)
                     << " pobnamed=" << pobnamed << " #" << complvec.size());
        complvec.push_back(name);
        return false;
      });
    }
  else if (word.size()>=3 && word[0] == '_' && isdigit(word[1]) && isalnum(word[2]))
    {
      MomObject::do_each_object_prefixed
      (word.c_str(),
       [&](MomObject*curpob)
      {
        MOM_DEBUGLOG(gui, "do_txcmd_complete_word id curpob=" << curpob << " #" << complvec.size());
        complvec.push_back(curpob->id().to_string());
        return false;
      });
    }
  MOM_DEBUGLOG(gui, "do_txcmd_complete_word word=" << MomShowString(word)
               << " got " << complvec.size() << " completions");
  if (complvec.size() == 0)   // no completion possible, failing
    {
      return false;
    }
  else if (complvec.size() == 1)   // single completion, replace and succeed
    {
      Gtk::TextIter newtxit = _mwi_commandbuf->erase(startxit,endtxit);
      newtxit = _mwi_commandbuf->insert(newtxit, complvec[0]);
      MOM_DEBUGLOG(gui, "do_txcmd_complete_word replaced with " << complvec[0] << " newtxit=" << MomShowTextIter(newtxit));
      return true;
    }
  else  			// several completions, show a menu
    {
      MOM_WARNLOG("do_txcmd_complete_word got " << complvec.size() << " completions for " << word);
#warning do_txcmd_complete_word unimplemented for several completions
    }
  return false; // completion failed
} // end MomMainWindow::do_txcmd_complete_word

void
MomMainWindow::do_txcmd_begin_user_action(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_begin_user_action");
  _mwi_cmduseractim = _mwi_cmdlastuseractim = mom_clock_time(CLOCK_REALTIME);
} // end MomMainWindow::do_txcmd_begin_user_action

void
MomMainWindow::do_txcmd_changed(void)
{
  if (_mwi_cmduseractim <= 0.0)
    return;
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_changed start");
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_changed end");
} // end MomMainWindow::do_txcmd_changed

void
MomMainWindow::do_txcmd_populate_menu(Gtk::Menu*menu)
{
  menu->prepend(_mwi_mit_txcmd_runkeep);
  menu->prepend(_mwi_mit_txcmd_runclear);
  menu->prepend(_mwi_mit_txcmd_clear);
  Gtk::TextIter insertxit = _mwi_commandbuf->get_insert()->get_iter();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_populate_menu start insertxit=" << MomShowTextIter(insertxit));
  int lineno=insertxit.get_line();
  int colno=insertxit.get_line_offset();
  int offset=insertxit.get_offset();
  auto tagsvec = insertxit.get_tags();
  _mwi_mit_txcmd_cursorloc.set_label
  (Glib::ustring::compose("@L%1C%2o%3", lineno, colno, offset));
  Gtk::Menu* cursorsubmenu = Gtk::manage(new Gtk::Menu);
  _mwi_mit_txcmd_cursorloc.set_submenu(*cursorsubmenu);
  _mwi_mit_txcmd_cursorloc.signal_deselect().connect
  ([=]
  {
    MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_populate_menu cursorloc deselect");
    delete cursorsubmenu;
  });
  cursorsubmenu->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
  int startnb=0, endnb=0, gotnb=0;
  for (auto tagref : tagsvec)
    {
      if (!tagref) continue;
      auto tagnamprop = tagref->property_name();
      auto tagnamustr = tagnamprop.get_value();
      if (tagnamustr.empty()) continue;
      MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_populate_menu curtag=" << tagnamustr);
      if (insertxit.starts_tag(tagref))
        {
          auto startmit = Gtk::manage(new Gtk::MenuItem(Glib::ustring::compose("+%1", tagnamustr)));
          startmit->set_sensitive(false);
          cursorsubmenu->append(*startmit);
          startnb++;
        }
      else if (insertxit.ends_tag(tagref))
        {
          auto endmit = Gtk::manage(new Gtk::MenuItem(Glib::ustring::compose("-%1", tagnamustr)));
          endmit->set_sensitive(false);
          cursorsubmenu->append(*endmit);
          endnb++;
        }
      else
        {
          auto gotmit = Gtk::manage(new Gtk::MenuItem(Glib::ustring::compose("|%1", tagnamustr)));
          gotmit->set_sensitive(false);
          cursorsubmenu->append(*gotmit);
          gotnb++;
        }
    }
  // _mwi_mit_txcmd_cursorloc.set_sensitive(false);
  _mwi_mit_txcmd_cursorloc.set_right_justified();
  menu->append(_mwi_mit_txcmd_cursorloc);
  menu->show_all_children();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_populate_menu end startnb=" << startnb
               << " endnb=" << endnb << " gotnb=" << gotnb
               << " insertxit=" << MomShowTextIter(insertxit));
} // end MomMainWindow::do_txcmd_populate_menu

void
MomMainWindow::do_txcmd_clear(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_clear");
  _mwi_commandbuf->set_text("");
  show_status_decisec("cleared command", 10);
} // end MomMainWindow::do_txcmd_clear


void
MomMainWindow::do_txcmd_run_but_keep(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_run_but_keep");
  do_txcmd_prettify_parse(true);
} // end MomMainWindow::do_txcmd_run_but_keep



void
MomMainWindow::do_txcmd_run_then_clear(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_run_then_clear");
  do_txcmd_prettify_parse(true);
  _mwi_commandbuf->set_text("");
} // end MomMainWindow::do_txcmd_clear


void
MomMainWindow::do_txcmd_end_user_action(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_end_user_action start");
  do_txcmd_prettify_parse();
  _mwi_cmduseractim = 0.0;
  do_txcmd_unblink_insert();
  do_txcmd_blink_insert();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_end_user_action done");
} // end MomMainWindow::do_txcmd_end_user_action


MomMainWindow::MomParenOffsets*
MomMainWindow::txcmd_find_parens_around_insert(void)
{
  MomMainWindow::MomParenOffsets* po=nullptr;
  Gtk::TextIter insertxit = _mwi_commandbuf->get_insert()->get_iter();
  int markoff = insertxit.get_offset();
  MOM_DEBUGLOG(blinkgui, "MomMainWindow::txcmd_find_parens_around_insert insertxit=" << MomShowTextIter(insertxit));
  if (!po)
    {
      auto openit = _mwi_txcmd_openmap.find(markoff);
      if (openit != _mwi_txcmd_openmap.end())
        {
          po = &openit->second;
        };
    }
  if (!po)
    {
      auto closeit = _mwi_txcmd_closemap.find(markoff);
      if (closeit != _mwi_txcmd_closemap.end())
        {
          po = &closeit->second;
        };
    }
  for (auto afterit = _mwi_txcmd_openmap.lower_bound(markoff);
       po==nullptr && afterit != _mwi_txcmd_openmap.end();
       afterit --)
    {
      MomParenOffsets*afterpo = nullptr;
      if (afterit != _mwi_txcmd_openmap.end()
          && ((afterpo=&afterit->second),
              (afterpo->surrounds(markoff))))
        po = afterpo;
      if (afterit == _mwi_txcmd_openmap.begin())
        break;
    }
  for (auto beforeit = _mwi_txcmd_closemap.upper_bound(markoff);
       po==nullptr && beforeit != _mwi_txcmd_closemap.end();
       beforeit ++)
    {
      MomParenOffsets*beforepo = nullptr;
      if (beforeit != _mwi_txcmd_closemap.end()
          && ((beforepo=&beforeit->second),
              (beforepo->surrounds(markoff))))
        po = beforepo;
    }
  if (po)
    {
      MOM_DEBUGLOG(blinkgui, "txcmd_find_parens_around_insert insertxit=" << MomShowTextIter(insertxit)
                   << " found po: open=" << (int)(po->paroff_open) << " openlen=" << (int)(po->paroff_openlen)
                   << " close=" << (int)(po->paroff_close) << " closelen=" << (int)(po->paroff_closelen) << std::endl
                   << ".. xtra=" << (int)(po->paroff_xtra) << " xtralen=" << (int)(po->paroff_xtralen));
    }
  else
    {
      MOM_DEBUGLOG(blinkgui, "txcmd_find_parens_around_insert insertxit=" << MomShowTextIter(insertxit) << " notfound-po");
    };
  return po;
} // end txcmd_find_parens_around_insert

bool
MomMainWindow::do_txcmd_blink_insert(void)
{
  MOM_DEBUGLOG(blinkgui, "MomMainWindow::do_txcmd_blink_insert");
  auto blinktag = MomApplication::itself()->lookup_command_tag("blink_cmdtag");
  blinktag->set_priority(MomApplication::itself()->nb_command_tags()-1);
  MomParenOffsets*  po = txcmd_find_parens_around_insert();
  _mwi_txcmd_startblink.clear();
  _mwi_txcmd_xtrablink.clear();
  _mwi_txcmd_endblink.clear();
  if (po)
    {
      int openoff = po->paroff_open;
      int closeoff = po->paroff_close;
      int xtraoff = po->paroff_xtra;
      int openlen = po->paroff_openlen;
      int closelen = po->paroff_closelen;
      int xtralen = po->paroff_xtralen;
      MOM_DEBUGLOG(blinkgui, "do_command_blink_insert openoff=" << openoff
                   << " openlen=" << openlen
                   << " closeoff=" << closeoff << " closelen=" << closelen
                   << " xtraoff=" << xtraoff << " xtralen=" << xtralen);
      Gtk::TextIter startxit = _mwi_commandbuf->begin();
      if (openoff>=0 && openlen>0)
        {
          Gtk::TextIter opentxit= startxit;
          opentxit.forward_chars(openoff);
          _mwi_txcmd_startblink = _mwi_commandbuf->create_mark("start_blink_command", opentxit);
          Gtk::TextIter openendtxit = opentxit;
          openendtxit.forward_chars(openlen);
          _mwi_commandbuf->apply_tag(blinktag, opentxit, openendtxit);
          MOM_DEBUGLOG(blinkgui, "do_command_blink_insert opentxit="
                       << MomShowTextIter(opentxit)
                       << " openendtxit=" << MomShowTextIter(openendtxit));
        }
      if (closeoff>=0 && closeoff>openoff && closelen>0)
        {
          Gtk::TextIter closetxit= startxit;
          closetxit.forward_chars(closeoff);
          _mwi_txcmd_endblink = _mwi_commandbuf->create_mark("end_blink_command", closetxit);
          Gtk::TextIter closebegtxit = closetxit;
          closebegtxit.backward_chars(closelen);
          _mwi_commandbuf->apply_tag(blinktag, closebegtxit, closetxit);
          MOM_DEBUGLOG(blinkgui, "do_command_blink_insert closebegtxit="
                       << MomShowTextIter(closebegtxit)
                       << " closetxit=" << MomShowTextIter(closetxit));
        }
      if (xtraoff>=0 && xtraoff<openoff && xtralen>0)
        {
          Gtk::TextIter xtratxit= startxit;
          xtratxit.forward_chars(xtraoff);
          _mwi_txcmd_xtrablink = _mwi_commandbuf->create_mark("xtra_blink_command", xtratxit);
          Gtk::TextIter endxtratxit = xtratxit;
          endxtratxit.forward_chars(xtralen);
          _mwi_commandbuf->apply_tag(blinktag, xtratxit, endxtratxit);
          MOM_DEBUGLOG(blinkgui, "do_command_blink_insert xtratxit="
                       << MomShowTextIter(xtratxit)
                       << " endxtratxit=" << MomShowTextIter(endxtratxit));
        }
      Glib::signal_timeout().connect_once
      (sigc::mem_fun(this,&MomMainWindow::do_browser_unblink_insert),
       _blink_delay_milliseconds);
    }
  else
    {
      MOM_DEBUGLOG(blinkgui, "do_command_blink_insert outside");
    }
  return true;
} // end MomMainWindow::do_txcmd_blink_insert


void
MomMainWindow::do_txcmd_unblink_insert(void)
{
  auto blinktag = MomApplication::itself()->lookup_command_tag("blink_cmdtag");
  MOM_DEBUGLOG(blinkgui, "MomMainWindow::do_txcmd_unblink_insert");
  if (_mwi_txcmd_startblink && _mwi_txcmd_endblink)
    {
      Gtk::TextIter startxit = _mwi_txcmd_startblink->get_iter();
      Gtk::TextIter endtxit = _mwi_txcmd_endblink->get_iter();
      MOM_DEBUGLOG(blinkgui, "do_command_unblink_insert startxit=" << MomShowTextIter(startxit)
                   << " endtxit=" << MomShowTextIter(endtxit));
      _mwi_commandbuf->remove_tag(blinktag, startxit, endtxit);
    }
  if (_mwi_txcmd_xtrablink)
    {
      Gtk::TextIter xtratxit = _mwi_txcmd_xtrablink->get_iter();
      Gtk::TextIter bolxtratxit = xtratxit;
      bolxtratxit.backward_line();
      xtratxit.forward_line();
      xtratxit.forward_char();
      MOM_DEBUGLOG(blinkgui, "do_txcmd_unblink_insert xtraxit=" << MomShowTextIter(xtratxit)
                   << " bolxtratxit=" << MomShowTextIter(bolxtratxit));
      _mwi_commandbuf->remove_tag(blinktag, bolxtratxit, xtratxit);
    }
  _mwi_txcmd_startblink.clear();
  _mwi_txcmd_xtrablink.clear();
  _mwi_txcmd_endblink.clear();
} // end MomMainWindow::do_txcmd_unblink_insert

void
MomMainWindow::do_txcmd_prettify_parse(bool apply)
{
  auto cmdbuf = _mwi_commandbuf;
  cmdbuf->remove_all_tags(cmdbuf->begin(), cmdbuf->end());
  std::string strcmd = cmdbuf->get_text();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse strcmd=" << MomShowString(strcmd));
  std::istringstream inscmd(strcmd);
  MomSimpleParser cmdpars(inscmd);
  char cmdnambuf[24];
  memset (cmdnambuf, 0, sizeof(cmdnambuf));
  snprintf(cmdnambuf, sizeof(cmdnambuf), "*cmd#%d*", _mwi_winrank);
  clear_txcmd_parens();
  cmdpars
  .set_name(cmdnambuf)
  .disable_exhaustion(true)
  .set_no_build(!apply)
  .set_debug(MOM_IS_DEBUGGING(gui))
  .set_signal_value(true)
  .set_has_chunk(true)
  ;
  cmdpars
  .set_named_fetch_fun
  ([&](MomSimpleParser*thisparser,
       const std::string&namstr, long namoff, unsigned namlincnt, int namcolpos)
  {
    MomObject* ob = thisparser->simple_named_object(namstr, namoff, namlincnt, namcolpos);
    Gtk::TextIter txit = command_txiter_at_line_col(namlincnt, namcolpos);
    Gtk::TextIter endtxit = txit;
    endtxit.forward_chars(namstr.size()+1);
    if (ob)
      {
        MOM_DEBUGLOG(gui, "prettify known name " << namstr
                     << " lineno=" << thisparser->lineno()
                     << " colpos=" << thisparser->colpos()
                     << " txit=" << MomShowTextIter(txit, MomShowTextIter::_FULL_,10)
                     << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10)
                     << " ob=" << ob);
        cmdbuf->apply_tag_by_name("knownname_cmdtag",
                                  txit, endtxit);
      }
    else
      {
        MOM_DEBUGLOG(gui, "prettify new name " << namstr
                     << " lineno=" << thisparser->lineno()
                     << " colpos=" << thisparser->colpos()
                     << " txit=" << MomShowTextIter(txit, MomShowTextIter::_FULL_,10)
                     << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10)
                     << " ob=" << ob);
        cmdbuf->apply_tag_by_name("newname_cmdtag",
                                  txit, endtxit);
      }
    return ob;
  } /*end named_fetch λ */)
  //
  .set_parsedval_nullfun
  ([&](MomSimpleParser*thisparser MOM_UNUSED,
       long offset MOM_UNUSED, unsigned linecnt, int colpos)
  {
    Gtk::TextIter txit = command_txiter_at_line_col(linecnt, colpos);
    Gtk::TextIter endtxit = txit;
    endtxit.forward_chars(2);
    MOM_DEBUGLOG(gui, "prettify null cmd txit=" << MomShowTextIter(txit)
                 << " endtxit=" << MomShowTextIter(endtxit));
    cmdbuf->apply_tag_by_name("null_cmdtag",
                              txit, endtxit);
  }/* end parsedval_null λ*/)
  //
  .set_parsedval_strfun
  ([&]  (MomSimpleParser*, const std::string&str,
         long inioffset MOM_UNUSED, unsigned inilinecnt, int inicolpos,
         long endoffset MOM_UNUSED, unsigned endlinecnt, int endcolpos)
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos);
    MOM_DEBUGLOG(gui, "prettify string cmd str=" << MomShowString(str)
                 << " inioffset=" << inioffset
                 << " inilinecnt=" << inilinecnt
                 << " inicolpos=" << inicolpos
                 << " initxit=" << MomShowTextIter(initxit, MomShowTextIter::_FULL_,10) << std::endl
                 << "... endoffset=" << endoffset
                 << " endlinecnt=" << endlinecnt
                 << " endcolpos=" << endcolpos
                 << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10));
    cmdbuf->apply_tag_by_name("string_cmdtag",
                              initxit, endtxit);
  }/*end parsedval_str λ*/)
  //
  .set_parsedval_intfun
  ([&](MomSimpleParser*, intptr_t num MOM_UNUSED,
       long offset, unsigned linecnt, int colpos, int endcolpos)
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(linecnt, colpos);
    Gtk::TextIter endtxit = initxit;
    endtxit.forward_chars(endcolpos-colpos+1);
    MOM_DEBUGLOG(gui, "prettify int num=" << num
                 << " offset=" << offset << " lincnt=" << linecnt
                 << " colpos=" << colpos << " endcolpos=" << endcolpos
                 << " cmd initxit=" << MomShowTextIter(initxit, MomShowTextIter::_FULL_,10)
                 << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10));
    cmdbuf->apply_tag_by_name("int_cmdtag",
                              initxit, endtxit);
  }/*end parsedval_int λ*/)
  //
  .set_parsedval_intsqfun
  ([&](MomSimpleParser*, const MomIntSq*intsq,
       long inioffset, unsigned inilinecnt, int inicolpos,
       long endoffset, unsigned endlinecnt, int endcolpos,
       int depth
      )
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos);
    Gtk::TextIter inipartxit = initxit;
    inipartxit.forward_chars(2);
    Gtk::TextIter endpartxit = endtxit;
    endpartxit.backward_chars(2);
    MOM_DEBUGLOG(gui, "prettify intsq=" << MomValue(intsq)
                 << " inioffset=" << inioffset
                 << " inilinecnt=" << inilinecnt
                 << " inicolpos=" << inicolpos
                 << " initxit=" << MomShowTextIter(initxit)
                 << " endoffset=" << endoffset
                 << " endlinecnt=" << endlinecnt
                 << " endcolpos=" << endcolpos
                 << " endtxit=" << MomShowTextIter(endtxit)
                 << " depth=" << depth);
    cmdbuf->apply_tag_by_name("numseq_cmdtag",
                              initxit, endtxit);
    cmdbuf->apply_tag_by_name("open_cmdtag",
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name("close_cmdtag",
                              endpartxit, endtxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("open%1_cmdtag", depth),
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("close%1_cmdtag", depth),
                              endpartxit, endtxit);
    int openoff=initxit.get_offset();
    int closeoff=endtxit.get_offset();
    MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                        .paroff_xtra= -1, .paroff_openlen=2, .paroff_closelen=2,
                        .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
    add_txcmd_parens(po);
  } /*end parsedval_intsq  λ*/)
  //
  .set_parsedval_doublesqfun
  ([&](MomSimpleParser*, const MomDoubleSq*dblsq,
       long inioffset, unsigned inilinecnt, int inicolpos,
       long endoffset, unsigned endlinecnt, int endcolpos,
       int depth
      )
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos);
    Gtk::TextIter inipartxit = initxit;
    inipartxit.forward_chars(2);
    Gtk::TextIter endpartxit = endtxit;
    endpartxit.backward_chars(2);
    MOM_DEBUGLOG(gui, "prettify doublesq=" << MomValue(dblsq)
                 << " inioffset=" << inioffset
                 << " inilinecnt=" << inilinecnt
                 << " inicolpos=" << inicolpos
                 << " initxit=" << MomShowTextIter(initxit)
                 << " endoffset=" << endoffset
                 << " endlinecnt=" << endlinecnt
                 << " endcolpos=" << endcolpos
                 << " endtxit=" << MomShowTextIter(endtxit)
                 << " depth=" << depth);
    cmdbuf->apply_tag_by_name("numseq_cmdtag",
                              initxit, endtxit);
    cmdbuf->apply_tag_by_name("open_cmdtag",
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name("close_cmdtag",
                              endpartxit, endtxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("open%1_cmdtag", depth),
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("close%1_cmdtag", depth),
                              endpartxit, endtxit);
    int openoff=initxit.get_offset();
    int closeoff=endtxit.get_offset();
    MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                        .paroff_xtra= -1, .paroff_openlen=2, .paroff_closelen=2,
                        .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
    add_txcmd_parens(po);
  } /*end parsedval_doublesq  λ*/)
  //
  .set_parsedval_seqfun
  ([&](MomSimpleParser*,
       const MomAnyObjSeq*seq, bool istuple,
       long inioffset, unsigned inilinecnt, int inicolpos,
       long endoffset, unsigned endlinecnt, int endcolpos,
       int depth
      )
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos+1);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos+1);
    Gtk::TextIter inipartxit = initxit;
    inipartxit.forward_chars(1);
    Gtk::TextIter endpartxit = endtxit;
    endpartxit.backward_chars(1);
    MOM_DEBUGLOG(gui, "prettify cmd " << (istuple?"tuple":"set")
                 << " initxit=" << MomShowTextIter(initxit, MomShowTextIter::_FULL_,10)
                 << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10));
    cmdbuf->apply_tag_by_name(istuple?"tuple_cmdtag":"set_cmdtag",
                              initxit, endtxit);
    cmdbuf->apply_tag_by_name("open_cmdtag",
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name("close_cmdtag",
                              endpartxit, endtxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("open%1_cmdtag", depth),
                              initxit, inipartxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("close%1_cmdtag", depth),
                              endpartxit, endtxit);
    int openoff=initxit.get_offset();
    int closeoff=endtxit.get_offset();
    MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                        .paroff_xtra= -1, .paroff_openlen=1, .paroff_closelen=1,
                        .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
    add_txcmd_parens(po);
  }/*end parsedval_seq λ*/ )
  //
  .set_parsedval_valnodefun
  ([&](MomSimpleParser*,const MomNode *nod,
       long inioffset, unsigned inilinecnt, int inicolpos,
       long leftoffset, unsigned leftlinecnt, int leftcolpos,
       long endoffset, unsigned endlinecnt, int endcolpos, int depth
      )
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos+1);
    Gtk::TextIter leftxit = command_txiter_at_line_col(leftlinecnt, leftcolpos+1);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos+1);
    Gtk::TextIter inistarfintxit = initxit;
    inistarfintxit.forward_chars(1);
    Gtk::TextIter leftpartxit = leftxit;
    leftpartxit.forward_chars(1);
    Gtk::TextIter endpartxit = endtxit;
    endpartxit.backward_chars(1);
    MOM_DEBUGLOG(gui, "prettify cmd node "
                 << " initxit=" << MomShowTextIter(initxit, MomShowTextIter::_FULL_,10)
                 << " leftxit=" << MomShowTextIter(leftxit, MomShowTextIter::_FULL_,10)
                 << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10));
    cmdbuf->apply_tag_by_name("node_cmdtag",
                              initxit, endtxit);
    cmdbuf->apply_tag_by_name("star_cmdtag",
                              initxit, inistarfintxit);
    cmdbuf->apply_tag_by_name("open_cmdtag",
                              leftxit, leftpartxit);
    cmdbuf->apply_tag_by_name("close_cmdtag",
                              endpartxit, endtxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("open%1_cmdtag", depth),
                              leftxit, leftpartxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("close%1_cmdtag", depth),
                              endpartxit, endtxit);
    int openoff=leftxit.get_offset();
    int closeoff=endtxit.get_offset();
    int xtraoff=initxit.get_offset();
    MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                        .paroff_xtra= xtraoff, .paroff_openlen=1, .paroff_closelen=1,
                        .paroff_xtralen= 1, .paroff_depth=(uint8_t)depth};
    add_txcmd_parens(po);
  }/*end parsedval_valnode λ*/ )
  //
  .set_parsedval_valchunkfun
  ([&](MomSimpleParser*,const MomValue chkval,
       long inioffset, unsigned inilinecnt, int inicolpos,
       long endoffset, unsigned endlinecnt, int endcolpos, int depth)
  {
    Gtk::TextIter initxit = command_txiter_at_line_col(inilinecnt, inicolpos+1);
    Gtk::TextIter endtxit = command_txiter_at_line_col(endlinecnt, endcolpos+1);
    MOM_DEBUGLOG(gui, "prettify cmd chunk "
                 << " initxit=" << MomShowTextIter(initxit, MomShowTextIter::_FULL_,10)
                 << " endtxit=" << MomShowTextIter(endtxit, MomShowTextIter::_FULL_,10));
    Gtk::TextIter iniparfintxit = initxit;
    iniparfintxit.forward_chars(2);
    Gtk::TextIter endparbegtxit = endtxit;
    endparbegtxit.backward_chars(2);
    cmdbuf->apply_tag_by_name("chunk_cmdtag",
                              initxit, endtxit);
    cmdbuf->apply_tag_by_name("open_cmdtag",
                              initxit, iniparfintxit);
    cmdbuf->apply_tag_by_name("close_cmdtag",
                              endparbegtxit, endtxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("open%1_cmdtag", depth),
                              initxit, iniparfintxit);
    cmdbuf->apply_tag_by_name(Glib::ustring::compose("close%1_cmdtag", depth),
                              endparbegtxit, endtxit);
    int openoff=initxit.get_offset();
    int closeoff=endtxit.get_offset();
    MomParenOffsets po {.paroff_open=openoff, .paroff_close= closeoff,
                        .paroff_xtra= -1, .paroff_openlen=2, .paroff_closelen=2,
                        .paroff_xtralen= 0, .paroff_depth=(uint8_t)depth};
    add_txcmd_parens(po);
  } /*end parsedval_valchunk  λ*/ )
  //
  .set_getfocusedobjectfun
  ([&](MomSimpleParser*) {
    MOM_DEBUGLOG(gui, "do_txcmd_prettify_parse getfocusedobjectfun λ _mwi_focusobj=" << _mwi_focusobj);
     return _mwi_focusobj;
   } /* end getfocusedobjectfun λ*/)
  //
  .set_putfocusedobjectfun
    ([&](MomSimpleParser*, MomObject*pobnewfocus) {
       MOM_DEBUGLOG(gui, "do_txcmd_prettify_parse putfocusedobjectfun λ pobnewfocus="
		    << MomShowObject(pobnewfocus));
       _mwi_focusobj = pobnewfocus;
     } /* end putfocusedobjectfun λ*/)
    //
    .set_updatedfocusedobjectfun
    ([&](MomSimpleParser*, MomObject*pobnewfocus) {
       MOM_DEBUGLOG(gui, "do_txcmd_prettify_parse updatedfocusedobjectfun λ pobnewfocus="
		    << MomShowObject(pobnewfocus));
       if (pobnewfocus)
	 pobnewfocus->touch();
       MOM_ASSERT(pobnewfocus == _mwi_focusobj,
		  "do_txcmd_prettify_parse updatedfocusedobjectfun pobnewfocus=" << pobnewfocus
		  << " different of _mwi_focusobj=" << _mwi_focusobj);
       browser_show_object(pobnewfocus);
     } /* end updatedfocusedobjectfun  λ*/)
  ;
  try
    {
      cmdpars.next_line().skip_spaces();
      MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse before do_parse_commands");
      do_parse_commands(&cmdpars, apply);
      MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse do_parse_commands done");
    }
  catch (MomParser::Mom_parse_failure pfail)
    {
      auto curlin = cmdpars.lineno();
      auto colpos = cmdpars.colpos();
      std::string failmsg = pfail.what();
      MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse  do_parse_commands failed:" << failmsg
                   << std::endl << "... curlin=" << curlin << " colpos=" << colpos);
      Gtk::TextIter errtxit = command_txiter_at_line_col(curlin, colpos);
      Gtk::TextIter endtxit = _mwi_commandbuf->end();
      cmdbuf->apply_tag_by_name("error_cmdtag",errtxit,endtxit);
      double cmduseractim = _mwi_cmduseractim;
      Glib::signal_timeout().connect_once
      ([=](void)
      {
        if (!apply && _mwi_cmdlastuseractim>cmduseractim)
          return;
        MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse show error failmsg=" << MomShowString(failmsg));
        show_status_decisec(failmsg, _default_status_delay_deciseconds_);
      },
      _default_error_delay_milliseconds_+3);
    }
  MOM_DEBUGLOG(gui, "MomMainWindow::do_txcmd_prettify_parse done");
} // end MomMainWindow::do_txcmd_prettify_parse



void
MomMainWindow::do_parse_commands(MomParser*pars, bool apply)
{
  MOM_ASSERT(pars != nullptr, "do_parse_commands: null parser");
  pars->skip_spaces();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_parse_commands start @ " << pars->location_str()
               << " " << MomShowString(pars->curbytes())
               << " apply=" << (apply?"true":"false"));
  pars->set_no_build(!apply);
  _mwi_txcmd_nbmodif = 0;
  while (!pars->eof())
    {
      pars->skip_spaces();
      if (pars->eof()) {
	MOM_DEBUGLOG(gui, "do_parse_commands eof");
        break;
      }
      MOM_DEBUGLOG(gui, "do_parse_commands before parse_command curbytes=" << MomShowString(pars->curbytes())
                   << " @" << pars->location_str());
      bool gotcommand=false;      
      pars->parse_command(&gotcommand);
      MOM_DEBUGLOG(gui, "do_parse_commands after parse_command curbytes=" << MomShowString(pars->curbytes())
                   << " @" << pars->location_str()
		   << " gotcommand=" << (gotcommand?"true":"false")
		   << ' ' << (pars->eol()?"eol":"noneol")
		   << ' ' << (pars->eof()?"eof":"noneof"));
      if (!gotcommand) {
	pars->skip_spaces();
	if (pars->eof()) {
	  MOM_DEBUGLOG(gui, "do_parse_commands eof break");
	  break;
	}
	else {
	  MOM_DEBUGLOG(gui, "do_parse_commands parse_command failed"
		       << " @" << pars->location_str());
	  MOM_PARSE_FAILURE(pars, "do_parse_commands failed to parse command @" << pars->location_str());
	}
      }
    };
  MOM_DEBUGLOG(gui, "do_parse_commands done parsing @" << pars->location_str());
  if (apply)
    {
      char modifbuf[40];
      memset (modifbuf, 0, sizeof(modifbuf));
      snprintf(modifbuf, sizeof(modifbuf), "done %d modifications",
               _mwi_txcmd_nbmodif);
      show_status_decisec(modifbuf, _default_status_delay_deciseconds_);
    }
  MOM_DEBUGLOG(gui, "do_parse_commands done  @ " << pars->location_str()
               << " " << MomShowString(pars->curbytes())
               << " apply=" << (apply?"true":"false"));
} // end MomMainWindow::do_parse_commands


void
MomMainWindow::browser_update_title_banner(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner start");
  auto it = _mwi_browserbuf->begin();
  Gtk::TextIter begit =  it;
  auto titletag = MomApplication::itself()->lookup_browser_tag("page_title_tag");
  Gtk::TextIter endit = it;
  while(endit.has_tag(titletag))
    endit.forward_char();
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner "
               << "begit=" << MomShowTextIter(begit)
               << ", endit=" << MomShowTextIter(endit));
  _mwi_browserbuf->erase(begit,endit);
  begit =  _mwi_browserbuf->begin();
  int nbshownob = _mwi_browsedobmap.size();
  if (nbshownob == 0)
    it = _mwi_browserbuf->insert_with_tag (begit, " ~ no objects ~ ", titletag);
  else if (nbshownob == 1)
    it = _mwi_browserbuf->insert_with_tag (begit, " ~ one object ~ ", titletag);
  else
    it = _mwi_browserbuf->insert_with_tag (begit, Glib::ustring::compose(" ~ %1 objects ~ ", nbshownob),
                                           titletag);
  _mwi_browserbuf->move_mark(_mwi_endtitlemark, it);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner end nbshownob=" << nbshownob);
} // end MomMainWindow::browser_update_title_banner


void
MomMainWindow::do_object_refresh(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_refresh start");
  display_full_browser();
  int nbshownob = _mwi_browsedobmap.size();
  char msg[40];
  memset(msg, 0, sizeof(msg));
  if (nbshownob == 0)
    strcpy(msg, "no object");
  else if (nbshownob == 1)
    strcpy(msg, "one object");
  else
    snprintf(msg, sizeof(msg), "%d objects", nbshownob);
  show_status_decisec(msg, _default_status_delay_deciseconds_);
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_refresh end");
} // end MomMainWindow::do_object_refresh

void
MomMainWindow::browser_show_object(MomObject*pob)
{
  if (pob==nullptr || pob->vkind() != MomKind::TagObjectK)
    MOM_FATAPRINTF("MomMainWindow::browser_show_object invalid pob @%p", (void*)pob);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object start pob.id=" << pob->id());
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object start shown pob=" << MomShowObject(pob));
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object start raw pob=" << (pob));
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object start nbshown=" << _mwi_browsedobmap.size());
  auto shmbegit = _mwi_browsedobmap.begin();
  auto shmendit = _mwi_browsedobmap.end();
  auto oldshowit = _mwi_browsedobmap.find(pob);
  MomObject*begpob = nullptr;
  MomObject*endpob = nullptr;
  if (shmbegit == shmendit)
    {
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object first object pob=" << MomShowObject(pob));
      Gtk::TextIter txit = _mwi_browserbuf->end();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object empty txit="
                   << MomShowTextIter(txit));
      browser_insert_object_display(txit, pob, _SCROLL_TOP_VIEW_);
      browser_update_title_banner();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object after first object pob=" << MomShowObject(pob));
    }
  else if (oldshowit != _mwi_browsedobmap.end())
    {
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object redisplay object pob=" << MomShowObject(pob));
      MomBrowsedObject& oldshowbob = oldshowit->second;
      MOM_ASSERT(oldshowbob._sh_startmark, "browser_show_object missing start mark for pob=" << pob);
      MOM_ASSERT(oldshowbob._sh_endmark, "browser_show_object missing end mark for pob=" << pob);
      Gtk::TextIter oldstatxit = oldshowbob._sh_startmark->get_iter();
      Gtk::TextIter oldendtxit = oldshowbob._sh_endmark->get_iter();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object redisplay pob=" << pob
                   << " oldstatxit=" << MomShowTextIter(oldstatxit, MomShowTextIter::_FULL_)
                   << " oldendtxit=" << MomShowTextIter(oldendtxit, MomShowTextIter::_FULL_));
      _mwi_browserbuf->erase(oldstatxit,oldendtxit);
      Gtk::TextIter redisptxit = oldshowbob._sh_startmark->get_iter();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object redisplay pob=" << pob
                   << " redisptxit="  << MomShowTextIter(redisptxit, MomShowTextIter::_FULL_));
      browser_insert_object_display(redisptxit, pob);
      browser_update_title_banner();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object after redisplay object pob=" << MomShowObject(pob));
    }
  else
    {
      begpob = shmbegit->first;
      auto shmlastit = shmendit;
      shmlastit--;
      endpob = shmlastit->first;
      bool afterbeg = MomObjNameLess{} (begpob, pob);
      bool beforend = MomObjNameLess{} (pob, endpob);
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object begpob="
                   << MomShowObject(begpob) << ", endpob="  << MomShowObject(endpob)
                   << ", pob=" << MomShowObject(pob)
                   << ", afterbeg=" << (afterbeg?"true":"false")
                   << ", beforend=" << (beforend?"true":"false"));
      if (!afterbeg)
        {
          MOM_ASSERT(shmbegit != shmendit,
                     "browser_show_object shmbegit is not != shmendit for pob=" << pob);
          Gtk::TextIter endtitltxit = _mwi_endtitlemark->get_iter();
          endtitltxit.forward_line(); // after the newline
          MOM_DEBUGLOG(gui, "browser_show_object beforebeg endtitltxit="
                       << MomShowTextIter(endtitltxit, MomShowTextIter::_FULL_, 12));
          Gtk::TextIter txit = endtitltxit;
          MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object before begin txit="
                       << MomShowTextIter(txit, MomShowTextIter::_FULL_, 32)
                       << ", pob=" << MomShowObject(pob));
          txit = _mwi_browserbuf->insert(txit, "\n");
          browser_insert_object_display(txit, pob);
        }
      else if (!beforend)
        {
          MomBrowsedObject& lastbob = shmlastit->second;
          Gtk::TextIter txit = lastbob._sh_endmark->get_iter();
          txit.forward_char();
          MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object after end txit="
                       << MomShowTextIter(txit, MomShowTextIter::_FULL_)
                       << ", pob=" << MomShowObject(pob));
          browser_insert_object_display(txit, pob);
        }
      else
        {
          MomObject* lowerpob = nullptr;
          MomObject* upperpob = nullptr;
          MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle pob="
                       << MomShowObject(pob));
          auto shmlowit = _mwi_browsedobmap.lower_bound(pob);
          if (shmlowit == _mwi_browsedobmap.end())
            MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle shmlowit at end");
          else
            {
              MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle shmlowit /" << MomShowObject(shmlowit->first));
              shmlowit--;
              if (shmlowit == _mwi_browsedobmap.end())
                MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle shmlowit now at end");
              else
                MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle now shmlowit /" << MomShowObject(shmlowit->first));
            }
          auto shmuppit = _mwi_browsedobmap.upper_bound(pob);
          if (shmuppit == _mwi_browsedobmap.end())
            MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle shmuppit at end");
          else
            MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle shmuppit /" << MomShowObject(shmuppit->first));
          lowerpob = (shmlowit!=_mwi_browsedobmap.end())?(shmlowit->first):nullptr;
          upperpob = (shmuppit!=_mwi_browsedobmap.end())?(shmuppit->first):nullptr;
          MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object inside"
                       << " lowerpob=" << MomShowObject(lowerpob)
                       << ", pob=" << MomShowObject(pob)
                       << ", upperpob=" << MomShowObject(upperpob));
          if (lowerpob && upperpob
              && MomObjNameLess{} (lowerpob, pob) && MomObjNameLess{} (pob, upperpob)
              && shmlowit != _mwi_browsedobmap.end()
              && shmuppit != _mwi_browsedobmap.end())
            {
              MomBrowsedObject& lowerbob = shmlowit->second;
              Gtk::TextIter lowendtxit = lowerbob._sh_endmark->get_iter();
              lowendtxit.forward_char();
              MomBrowsedObject& upperbob = shmuppit->second;
              Gtk::TextIter uppstatxit = upperbob._sh_startmark->get_iter();
              MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object middle after lowerpob="
                           << MomShowObject(lowerpob)
                           << " lowendtxit="
                           << MomShowTextIter(lowendtxit, MomShowTextIter::_FULL_)
                           << " uppstatxit="
                           << MomShowTextIter(uppstatxit, MomShowTextIter::_FULL_)
                           << ", pob=" << MomShowObject(pob)
                           << " before upperpob=" << MomShowObject(upperpob));
              browser_insert_object_display(lowendtxit, pob, _SCROLL_TOP_VIEW_);
            }
          else
            MOM_WARNLOG("MomMainWindow::browser_show_object non empty unimplemented for pob="  << pob
                        << " nbshown=" << _mwi_browsedobmap.size());
        }
    }
  browser_update_title_banner();
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object end pob=" << pob
               << " nbshown=" << _mwi_browsedobmap.size());
} // end MomMainWindow::browser_show_object


void
MomMainWindow::browser_hide_object(MomObject*pob)
{
  if (pob==nullptr || pob->vkind() != MomKind::TagObjectK)
    MOM_FATAPRINTF("MomMainWindow::browser_hide_object invalid pob @%p", (void*)pob);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object start pob=" << MomShowObject(pob));
  if (pob == _mwi_focusobj)
    _mwi_focusobj = nullptr;
  auto shmit = _mwi_browsedobmap.find(pob);
  if (shmit == _mwi_browsedobmap.end())
    {
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object did not find pob=" << MomShowObject(pob));
      MOM_WARNLOG("MomMainWindow::browser_hide_object cannot hide undisplayed pob=" << MomShowObject(pob));
      std::string outmsg;
      std::ostringstream out (outmsg);
      out << "cannot hide undisplayed " << pob << std::flush;
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object fail outmsg=" << MomShowString(outmsg));
      show_status_decisec(outmsg, _default_status_delay_deciseconds_);
    }
  else
    {
      MomBrowsedObject& bob = shmit->second;
      Gtk::TextIter statxit = bob._sh_startmark->get_iter();
      Gtk::TextIter endtxit = bob._sh_endmark->get_iter();
      endtxit.forward_char();
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object pob=" << MomShowObject(pob)
                   << " statxit=" << MomShowTextIter(statxit, MomShowTextIter::_FULL_)
                   << ", endtxit="  << MomShowTextIter(endtxit, MomShowTextIter::_FULL_)
                   << " spanning " << (endtxit.get_offset() - statxit.get_offset())
                   << " chars");
      MOM_ASSERT(endtxit.get_offset() > statxit.get_offset(), "browser_hide_object"
                 << " statxit=" << MomShowTextIter(statxit, MomShowTextIter::_FULL_)
                 << " not before endtxit="  << MomShowTextIter(endtxit, MomShowTextIter::_FULL_));
      _mwi_browserbuf->erase(statxit,endtxit);
      _mwi_browserbuf->delete_mark(bob._sh_startmark);
      _mwi_browserbuf->delete_mark(bob._sh_endmark);
      _mwi_browsedobmap.erase(shmit);
      browser_update_title_banner();
      std::string outmsg;
      std::ostringstream out (outmsg);
      out << "hide " << pob << std::flush;
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object good outmsg=" << MomShowString(outmsg) << " for pob=" << pob);
      show_status_decisec(outmsg, _default_status_delay_deciseconds_);
    }
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object end pob=" << pob);
} // end MomMainWindow::browser_hide_object

void
MomMainWindow::browser_set_focus_object(MomObject*pob)
{
  if (pob == _mwi_focusobj) return;
  MomObject* oldfocpob = _mwi_focusobj;
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_set_focus_object start oldfocpob=" << oldfocpob
               << " pob=" << pob);
  if (oldfocpob != nullptr)
    {
      auto oldfocshowit = _mwi_browsedobmap.find(oldfocpob);
      if (oldfocshowit == _mwi_browsedobmap.end())
        {
          MOM_WARNLOG("browser_set_focus_object old focus " << _mwi_focusobj
                      << " was hidden " << MOM_SHOW_BACKTRACE("browser_set_focus_object old"));
        }
      else
        {
          MomBrowsedObject& oldfocbrob = oldfocshowit->second;
          Gtk::TextIter oldfocstatxit = oldfocbrob._sh_startmark->get_iter();
          Gtk::TextIter oldfocendtxit = oldfocbrob._sh_endmark->get_iter();
          MOM_DEBUGLOG(gui, "browser_set_focus_object oldfocpob=" << MomShowObject(oldfocpob)
                       << " oldfocstatxit=" <<  MomShowTextIter(oldfocstatxit, MomShowTextIter::_FULL_,10)
                       << " oldfocendtxit=" <<  MomShowTextIter(oldfocendtxit, MomShowTextIter::_FULL_,10));
          _mwi_browserbuf->remove_tag_by_name("object_focus_tag", oldfocstatxit, oldfocendtxit);
          Gtk::TextIter oldfoceoltxit = oldfocstatxit;
          oldfoceoltxit.forward_line();
          MOM_DEBUGLOG(gui, "browser_set_focus_object oldfocpob=" << oldfocpob
                       << " oldfoceoltxit="  <<  MomShowTextIter(oldfoceoltxit, MomShowTextIter::_FULL_,10));
          _mwi_browserbuf->remove_tag_by_name("object_title_focus_tag", oldfocstatxit, oldfoceoltxit);
        }
    };
  if (pob != nullptr)
    {
      auto newfocshowit = _mwi_browsedobmap.find(pob);
      if (newfocshowit == _mwi_browsedobmap.end())
        {
          MOM_WARNLOG("browser_set_focus_object new focus " << pob
                      << " is hidden " << MOM_SHOW_BACKTRACE("browser_set_focus_object new"));
        }
      else
        {
          MomBrowsedObject& newfocbrob = newfocshowit->second;
          Gtk::TextIter newfocstatxit = newfocbrob._sh_startmark->get_iter();
          Gtk::TextIter newfocendtxit = newfocbrob._sh_endmark->get_iter();
          MOM_DEBUGLOG(gui, "browser_set_focus_object pob=" << MomShowObject(pob)
                       << " newfocstatxit=" <<  MomShowTextIter(newfocstatxit, MomShowTextIter::_FULL_,10)
                       << " newfocendtxit=" <<  MomShowTextIter(newfocendtxit, MomShowTextIter::_FULL_,10));
          _mwi_browserbuf->apply_tag_by_name("object_focus_tag", newfocstatxit, newfocendtxit);
          Gtk::TextIter newfoceoltxit = newfocstatxit;
          newfoceoltxit.forward_line();
          MOM_DEBUGLOG(gui, "browser_set_focus_object pob=" << pob
                       << " newfoceoltxit="  <<  MomShowTextIter(newfoceoltxit, MomShowTextIter::_FULL_,10));
          _mwi_browserbuf->apply_tag_by_name("object_title_focus_tag", newfocstatxit, newfoceoltxit);
        }
    }
  _mwi_focusobj = pob;
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_set_focus_object end oldfocpob=" << oldfocpob
               << " pob=" << pob);
} // end MomMainWindow::browser_set_focus_object


void
MomMainWindow::scan_gc(MomGC*gc)
{
  if (_mwi_winobj)
    gc->scan_object(_mwi_winobj);
  for (auto it: _mwi_browsedobmap)
    {
      gc->scan_object(it.first);
    }
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


void
MomShowTextIter::output(std::ostream&outs) const
{
  outs << "txit/" << _shtxit.get_offset()
       << "L" << _shtxit.get_line() << "C" << _shtxit.get_line_offset();
  if (_showfull)
    {
      auto tagsvec = _shtxit.get_tags();
      for (auto tagref : tagsvec)
        {
          if (!tagref) continue;
          auto tagnamprop = tagref->property_name();
          outs << ":";
          if (_shtxit.starts_tag(tagref))
            outs<<'+';
          else if (_shtxit.ends_tag(tagref))
            outs<<'-';
          outs << tagnamprop.get_value();
        }
      outs << "!";
    }
  if (_shwidth > 0)
    {
      unsigned width = _shwidth;
      if (width > _MAX_WIDTH_)
        width = _MAX_WIDTH_;
      Gtk::TextIter beforit = _shtxit;
      Gtk::TextIter afterit = _shtxit;
      beforit.backward_chars(width);
      afterit.forward_chars(width);
      outs << " *has " << width << " around ..."
           << MomShowString(beforit.get_text(_shtxit).c_str()) << "_^_"
           << MomShowString(_shtxit.get_text(afterit).c_str()) ;
    }
} // end MomShowTextIter::output



////////////////////////////////////////////////////////////////

MomPaylMainWindow::~MomPaylMainWindow()
{
  if (_pymw_win) _pymw_win->_mwi_winobj = nullptr;
} // end MomPaylMainWindow::~MomPaylMainWindow

void
MomPaylMainWindow::Destroy(struct MomPayload*payl,MomObject*own)
{
  auto py = static_cast<MomPaylMainWindow*>(payl);
  delete py;
} // end MomPaylMainWindow::Destroy


MomValue
MomPaylMainWindow::Getmagic (const struct MomPayload*payl,const MomObject*own,const MomObject*attrob)
{
  auto py = static_cast<const MomPaylMainWindow*>(payl);
  MomObject*proxob = nullptr;
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(main_window),
             "MomPaylMainWindow::Getmagic invalid main_win payload for own=" << own);
  if (attrob == MOMP_int)
    return py->_pymw_win?py->_pymw_win->rank():0;
  else if (attrob == MOMP_proxy)
    return py->_pymw_proxy;
  else if ((proxob=py->_pymw_proxy) != nullptr)
    {
      std::shared_lock<std::shared_mutex> lk(proxob->get_shared_mutex(py));
      return proxob->unsync_get_magic_attr(attrob);
    }
  return nullptr;
} // end of MomPaylMainWindow::Getmagic

void
MomPaylMainWindow::Scangc(const struct MomPayload*payl,MomObject*own,MomGC*gc)
{
  auto py = static_cast<const MomPaylMainWindow*>(payl);
  MOM_ASSERT(py->_py_vtbl ==  &MOM_PAYLOADVTBL(main_window),
             "invalid main_win payload for own=" << own);
  if (py->_pymw_win)
    py->_pymw_win->scan_gc(gc);
  if (py->_pymw_proxy)
    gc->scan_object(py->_pymw_proxy);
} // end MomPaylMainWindow::Scangc

////////////////////////////////////////////////////////////////
extern "C"
void
mom_gtk_error_handler (const gchar *log_domain,
                       GLogLevelFlags log_level,
                       const gchar *message,
                       gpointer user_data MOM_UNUSED)
{
  std::string loglevstr;
  if (log_level & G_LOG_FLAG_RECURSION) loglevstr += " RECURSION";
  if (log_level & G_LOG_FLAG_FATAL) loglevstr += " FATAL";
  if (log_level & G_LOG_LEVEL_ERROR) loglevstr += " ERROR";
  if (log_level & G_LOG_LEVEL_CRITICAL) loglevstr += " CRITICAL";
  if (log_level & G_LOG_LEVEL_WARNING) loglevstr += " WARNING";
  if (log_level & G_LOG_LEVEL_MESSAGE) loglevstr += " MESSAGE";
  if (log_level & G_LOG_LEVEL_INFO) loglevstr += " INFO";
  MOM_WARNLOG("mom_gtk_error_handler level" << loglevstr
              << " domain " << (log_domain?:"??")
              << " message " << message
              << MOM_SHOW_BACKTRACE("gtk_error_handler"));
} // end mom_gtk_error_handler



int
mom_run_gtkmm_gui(int& argc, char**argv)
{
  MOM_DEBUGLOG(gui, "mom_run_gtkmm_gui argc=" << argc);
  g_log_set_handler ("Gtk",
                     (GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
                                       | G_LOG_FLAG_RECURSION),
                     mom_gtk_error_handler, NULL);
  g_log_set_handler ("GLib",
                     (GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
                                       | G_LOG_FLAG_RECURSION),
                     mom_gtk_error_handler, NULL);
  g_log_set_handler ("GIo",
                     (GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
                                       | G_LOG_FLAG_RECURSION),
                     mom_gtk_error_handler, NULL);
  g_log_set_handler (nullptr,
                     (GLogLevelFlags) (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_FLAG_FATAL
                                       | G_LOG_FLAG_RECURSION),
                     mom_gtk_error_handler, NULL);
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
