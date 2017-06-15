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

#include <glib.h>
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
  Glib::RefPtr<Gtk::TextTag> lookup_tag (const Glib::ustring& name)
  {
    return _app_browse_tagtable->lookup(name);
  };
  Glib::RefPtr<Gtk::TextTag> lookup_tag (const char*namestr)
  {
    return lookup_tag (Glib::ustring(namestr));
  };
  void on_parsing_css_error(const Glib::RefPtr<const Gtk::CssSection>& section, const Glib::Error& error);
  void on_startup(void);
  void on_activate(void);
  void do_exit(void);
  void do_dump(void);
  void scan_gc(MomGC*);
};				// end class MomApplication


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
};				// end MomComboBoxObjptrText
const long MomComboBoxObjptrText::_nb_named_threshold_;


class MomMainWindow : public Gtk::Window
{
public:
  static constexpr const int _default_display_depth_ = 5;
  static constexpr const int _default_display_width_ = 72;
  struct MomBrowsedObject
  {
    MomObject*_sh_ob;
    Glib::RefPtr<Gtk::TextMark> _sh_startmark;
    Glib::RefPtr<Gtk::TextMark> _sh_endmark;
    MomBrowsedObject(MomObject*ob, Glib::RefPtr<Gtk::TextMark> startmk, Glib::RefPtr<Gtk::TextMark> endmk)
      : _sh_ob(ob), _sh_startmark(startmk), _sh_endmark(endmk) {};
    ~MomBrowsedObject()
    {
      _sh_ob=nullptr;
      _sh_startmark.clear();
      _sh_endmark.clear();
    };
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
  Glib::RefPtr<Gtk::TextBuffer> _mwi_buf;
  int _mwi_dispdepth;
  int _mwi_dispwidth;
  Gtk::Paned _mwi_panedtx;
  Gtk::ScrolledWindow _mwi_scrwtop;
  Gtk::ScrolledWindow _mwi_scrwbot;
  Gtk::TextView _mwi_txvtop;
  Gtk::TextView _mwi_txvbot;
  Gtk::TextView _mwi_txvcmd;
  Gtk::Statusbar _mwi_statusbar;
  std::map<MomObject*,MomBrowsedObject,MomObjNameLess> _mwi_shownobmap;
public:
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
  void browser_insert_object_display(Gtk::TextIter& it, MomObject*ob);
  void browser_update_title_banner(void);
  void browser_insert_objptr(Gtk::TextIter& it, MomObject*ob, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth);
  void browser_insert_value(Gtk::TextIter& it, MomValue val, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth);
  void browser_insert_space(Gtk::TextIter& it, const std::vector<Glib::ustring>& tags, int depth=0);
  void browser_insert_newline(Gtk::TextIter& it, const std::vector<Glib::ustring>& tags, int depth=0);
  void do_window_dump(void);
  void do_object_show_hide(void);
  void do_object_refresh(void);
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
    if (!isalnum(*pc) && *pc != '_')
      badstr = true;
    else if (pc==str && !(isalpha(*pc) || *pc=='_'))
      badstr = true;
    else
      corrstr += pc;
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
      MOM_WARNLOG("MomComboBoxObjptrText::upgrade_for_string UNIMPLEMENTED str=" << MomShowString(str)
                  << MOM_SHOW_BACKTRACE("upgrade_for_string non-empty str"));
    }
#warning MomComboBoxObjptrText::upgrade_for_string incomplete
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
void
MomMainWindow::display_full_browser(void)
{
  int nbshownob = _mwi_shownobmap.size();
  _mwi_buf->set_text("");
  auto it = _mwi_buf->begin();
  it = _mwi_buf->insert_with_tag (it, Glib::ustring::compose(" ~ %1 objects ~ ", nbshownob), "page_title_tag");
  it = _mwi_buf->insert(it, "\n");
  for (auto itob : _mwi_shownobmap)
    {
      browser_insert_object_display(it, itob.first);
      it = _mwi_buf->insert(it, "\n");
    }
} // end MomMainWindow::display_full_browser

void
MomMainWindow::browser_insert_space(Gtk::TextIter& txit, const std::vector<Glib::ustring>& tags, int depth)
{
  int linoff = txit.get_line_offset();
  if (linoff >= _mwi_dispwidth)
    browser_insert_newline(txit,tags,depth);
  else
    txit = _mwi_buf->insert_with_tags_by_name (txit, " ", tags);
} // end MomMainWindow::browser_insert_space

void
MomMainWindow::browser_insert_newline(Gtk::TextIter& txit, const std::vector<Glib::ustring>& tags, int depth)
{
  if (depth<0) depth=0;
  constexpr const char nlspaces[]
    = "\n                                                                ";
  txit = _mwi_buf->insert_with_tags_by_name (txit, nlspaces, nlspaces + (depth % 16), tags);
} // end MomMainWindow::browser_insert_newline

void
MomMainWindow::browser_insert_object_display(Gtk::TextIter& txit, MomObject*pob)
{
  MOM_ASSERT(pob != nullptr && pob->vkind() == MomKind::TagObjectK,
             "MomMainWindow::browser_insert_object_display bad object");
  char obidbuf[32];
  memset(obidbuf, 0, sizeof(obidbuf));
  int depth = _mwi_dispdepth;
  pob->id().to_cbuf32(obidbuf);
  std::shared_lock<std::shared_mutex> lk(pob->get_shared_mutex());
  std::string obnamstr = mom_get_unsync_string_name(const_cast<MomObject*>(pob));
  auto itm = _mwi_shownobmap.find(pob);
  bool found = false;
  if (itm == _mwi_shownobmap.end())
    {
      auto begmark = _mwi_buf->create_mark(Glib::ustring::compose("begmarkob_%1", obidbuf), txit, /*left_gravity:*/ true);
      auto endmark = _mwi_buf->create_mark(Glib::ustring::compose("endmarkob_%1", obidbuf), txit, /*left_gravity:*/ false);
      auto pairitb = _mwi_shownobmap.emplace(pob,MomBrowsedObject(pob,begmark,endmark));
      itm = pairitb.first;
      found = false;
    }
  else found = true;
  MOM_DEBUGLOG(gui, "browser_insert_object_display pob="
               << MomShowObject(pob) << " depth=" << depth
               << " found=" << (found?"true":"false"));
  MomBrowsedObject& shob = itm->second;
  /// the title bar
  MOM_ASSERT(shob._sh_ob == pob, "MomMainWindow::browser_insert_object_display corrupted shob");
  if (found)
    _mwi_buf->move_mark(shob._sh_startmark, txit);
  txit = _mwi_buf->insert_with_tag (txit, " \342\201\202 " /* U+2042 ASTERISM ⁂ */, "object_title_tag");
  if (!obnamstr.empty())
    {
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obnamstr.c_str()),
              std::vector<Glib::ustring> {"object_title_tag","object_title_name_tag"});
      txit = _mwi_buf->insert_with_tag (txit, " = ", "object_title_tag");
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obidbuf),
              std::vector<Glib::ustring> {"object_title_tag","object_title_id_tag"});
    }
  else   // anonymous
    {
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              Glib::ustring(obidbuf),
              std::vector<Glib::ustring> {"object_title_tag","object_title_anon_tag"});
    }
  txit = _mwi_buf->insert_with_tags_by_name
         (txit,
          Glib::ustring::compose(" \360\235\235\231 %1 " /* U+1D759 MATHEMATICAL SANS-SERIF BOLD CAPITAL DELTA 𝝙 */,
                                 depth),
          std::vector<Glib::ustring> {"object_title_tag","object_title_depth_tag"});
  txit = _mwi_buf->insert(txit, "\n");
  /// show the modtime
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
    txit = _mwi_buf->insert_with_tag (txit, mtimbuf, "object_mtime_tag");
    txit = _mwi_buf->insert(txit, "\n");
  }
  /// show the attributes
  {
    MomDisplayCtx dctxattrs(&shob);
    std::map<MomObject*,MomValue,MomObjNameLess> mapattrs;
    pob->unsync_each_phys_attr([&](MomObject*pobattr,MomValue valattr)
    {
      mapattrs.insert({pobattr,valattr});
      return false;
    });
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
      }
    else
      {
        snprintf(atitlebuf, sizeof(atitlebuf),
                 "%s no attributes %s\n",
                 MomParser::_par_comment_start1_, MomParser::_par_comment_end1_);
      }
    txit = _mwi_buf->insert_with_tags_by_name
           (txit,atitlebuf, tagsattrindex);
    for (auto itattr : mapattrs)
      {
        MomObject*pobattr = itattr.first;
        MomValue valattr = itattr.second;
        txit = _mwi_buf->insert_with_tags_by_name
               (txit, "\342\210\231 " /* U+2219 BULLET OPERATOR ∙ */, tagsattrs);
        browser_insert_objptr(txit, pobattr, &dctxattrs, tagsattrobj, 0);
        browser_insert_space(txit, tagsattrs, 1);
        txit = _mwi_buf->insert_with_tags_by_name
               (txit, "\342\206\246" /* U+21A6 RIGHTWARDS ARROW FROM BAR ↦ */, tagsattrs);
        browser_insert_space(txit, tagsattrs, 1);
        browser_insert_value(txit, valattr, &dctxattrs, tagsattrval, 1);
        browser_insert_newline(txit, tagsattrs, 0);
      }
    txit = _mwi_buf->insert(txit, "\n");
  }
  ///  show the components
  {
    MomDisplayCtx dctxcomps(&shob);
    std::vector<Glib::ustring> tagscomps{"components_tag"};
    std::vector<Glib::ustring> tagscompindex{"components_tag", "index_comment_tag"};
    std::vector<Glib::ustring> tagscompval{"components_tag", "compval_tag"};
    char atitlebuf[72];
    memset(atitlebuf, 0, sizeof(atitlebuf));
    unsigned nbcomp = pob->unsync_nb_comps();
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
    txit = _mwi_buf->insert_with_tags_by_name
           (txit,atitlebuf, tagscompindex);
    for (unsigned ix=0; ix<nbcomp; ix++)
      {
        snprintf(atitlebuf, sizeof(atitlebuf),
                 "%s #%u %s",
                 MomParser::_par_comment_start1_, nbcomp, MomParser::_par_comment_end1_);
        txit = _mwi_buf->insert_with_tags_by_name
               (txit,atitlebuf, tagscompindex);
        browser_insert_space(txit, tagscomps, 1);
        MomValue compval = pob->unsync_unsafe_comp_at(ix);
        browser_insert_value(txit, compval, &dctxcomps, tagscompval, 1);
        browser_insert_newline(txit, tagscomps, 0);
      }
    txit = _mwi_buf->insert(txit, "\n");
  }
  /// show the payload, if any
  MomPayload* payl = pob->unsync_payload();
  if (payl)
    {
      MomDisplayCtx dctxpayl(&shob);
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
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,atitlebuf, tagspaylindex);
      browser_insert_newline(txit, tagspayl, 0);
#warning MomMainWindow::browser_insert_object_display should probably display the payload wisely
    }
  txit = _mwi_buf->insert(txit, "\n");
  _mwi_buf->move_mark(shob._sh_endmark, txit);
} // end MomMainWindow::browser_insert_object_display



void
MomMainWindow::browser_insert_objptr(Gtk::TextIter& txit, MomObject*pob, MomDisplayCtx*dcx, const std::vector<Glib::ustring>& tags, int depth)
{
  std::vector<Glib::ustring> tagscopy = tags;
  if (!pob)
    {
      tagscopy.push_back("objocc_nil_tag");
      txit = _mwi_buf->insert_with_tags_by_name
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
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              obidbuf,
              tagscopy);
    }
  else
    {
      tagscopy.push_back("objocc_named_tag");
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              obnamstr.c_str(),
              tagscopy);
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
      txit = _mwi_buf->insert_with_tags_by_name
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
      txit = _mwi_buf->insert_with_tags_by_name
             (txit,
              numbuf,
              tagscopy);
      return;
    }
  MOM_ASSERT(val.is_val(), "browser_insert_value corrupted val");
  if (val.is_transient())
    {
      tagscopy.push_back("value_transient_tag");
      txit = _mwi_buf->insert_with_tags_by_name
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
      std::vector<Glib::ustring> tagsindex = tags;
      auto isv = reinterpret_cast<const MomIntSq*>(vv);
      unsigned sz = isv->sizew();
      tagscopy.push_back("value_numberseq_tag");
      tagsindex.push_back("index_comment_tag");
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, "(#", tagscopy);
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
                    _mwi_buf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          snprintf(numbuf, sizeof(numbuf), "%lld", (long long)isv->unsafe_at(ix));
          txit =
            _mwi_buf->insert_with_tags_by_name  (txit,  numbuf, tagscopy);
        }
      txit =
        _mwi_buf->insert_with_tags_by_name (txit,   "#)", tagscopy);
    }
    break;
    //////
    case MomKind::TagDoubleSqK:  /// double sequence
    {
      std::vector<Glib::ustring> tagsindex = tags;
      auto dsv = reinterpret_cast<const MomDoubleSq*>(vv);
      unsigned sz = dsv->sizew();
      tagscopy.push_back("value_numberseq_tag");
      tagsindex.push_back("index_comment_tag");
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, "(:", tagscopy);
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
                    _mwi_buf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          snprintf(numbuf, sizeof(numbuf), "%.15g", dsv->unsafe_at(ix));
          txit =
            _mwi_buf->insert_with_tags_by_name (txit, numbuf, tagscopy);
        }
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, ":)",  tagscopy);
    }
    break;
    ////
    case MomKind::TagStringK: /// string value
    {
      std::vector<Glib::ustring> tagsescape = tags;
      auto strv = reinterpret_cast<const MomString*>(vv);
      unsigned sz = strv->sizew();
      std::string str = strv->string();
      txit = _mwi_buf->insert_with_tags_by_name	(txit, "\"", tags);
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
              txit = _mwi_buf->insert_with_tags_by_name (txit, "\"", tags);
              browser_insert_newline(txit, tags, depth);
              txit = _mwi_buf->insert_with_tags_by_name (txit, "&+&", tagsescape);
              txit = _mwi_buf->insert_with_tags_by_name (txit, " \"", tags);
            }
          uc = g_utf8_get_char (pc);
          switch (uc)
            {
            case 0:
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\0", tagsescape);
              break;
            case '\"':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\\"", tagsescape);
              break;
            case '\'':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\\'", tagsescape);
              break;
            case '\a':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\a", tagsescape);
              break;
            case '\b':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\b", tagsescape);
              break;
            case '\f':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\f", tagsescape);
              break;
            case '\n':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\n", tagsescape);
              break;
            case '\r':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\r", tagsescape);
              break;
            case '\t':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\t", tagsescape);
              break;
            case '\v':
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\v", tagsescape);
              break;
            case '\033' /*ESCAPE*/:
              txit =
                _mwi_buf->insert_with_tags_by_name (txit, "\\e", tagsescape);
              break;
            default:
            {
              char buf[16];
              memset (buf, 0, sizeof(buf));
              if ((uc>=' ' && uc<127) || g_unichar_isprint(uc))
                {
                  g_unichar_to_utf8(uc, buf);
                  txit =
                    _mwi_buf->insert_with_tags_by_name (txit, buf, tagscopy);
                }
              else if (uc<0xffff)
                {
                  snprintf (buf, sizeof (buf), "\\u%04x", (int) uc);
                  txit =
                    _mwi_buf->insert_with_tags_by_name (txit, buf, tagsescape);
                }
              else
                {
                  snprintf (buf, sizeof (buf), "\\U%08x", (int) uc);
                  txit =
                    _mwi_buf->insert_with_tags_by_name (txit, buf, tagsescape);
                }
            }
            break;
            }
        }
      txit = _mwi_buf->insert_with_tags_by_name
             (txit, "\"", tags);
    }
    break;
#warning MomMainWindow::browser_insert_value should probably care of parenthesis and be able to match them
    case MomKind::TagSetK:
    case MomKind::TagTupleK:
    {
      auto seqv = reinterpret_cast<const MomAnyObjSeq*>(vv);
      bool istuple = (vv->vkind() == MomKind::TagTupleK);
      unsigned sz = seqv->sizew();
      std::vector<Glib::ustring> tagsindex = tags;
      tagscopy.push_back("value_sequence_tag");
      tagsindex.push_back("index_comment_tag");
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, (istuple?"[":"{"), tagscopy);
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
                    _mwi_buf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                }
            }
          browser_insert_objptr(txit, seqv->unsafe_at(ix), dcx,
                                tagscopy, depth+1);
        };
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, (istuple?"]":"}"), tagscopy);
    }
    break;
    //////
    case MomKind::TagNodeK:  /// node value
    {
      auto nodv = reinterpret_cast<const MomNode*>(vv);
      unsigned sz = nodv->sizew();
      std::vector<Glib::ustring> tagsindex = tags;
      tagscopy.push_back("value_node_tag");
      tagsindex.push_back("index_comment_tag");
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, "*", tagscopy);
      browser_insert_objptr(txit, nodv->conn(), dcx, tagscopy, depth);
      browser_insert_space(txit, tagscopy, depth);
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, "(", tagscopy);
      if (depth >= _mwi_dispdepth)
        {
          txit =
            _mwi_buf->insert_with_tags_by_name (txit,
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
                        _mwi_buf->insert_with_tags_by_name  (txit,  numbuf, tagsindex);
                    }
                }
              browser_insert_value(txit, nodv->unsafe_at(ix), dcx,
                                   tagscopy, depth+1);
            };
        }
      txit =
        _mwi_buf->insert_with_tags_by_name (txit, ")", tagscopy);
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
    _mwi_buf(Gtk::TextBuffer::create(MomApplication::itself()->browser_tagtable())),
    _mwi_dispdepth(_default_display_depth_),
    _mwi_dispwidth(_default_display_width_),
    _mwi_panedtx(Gtk::ORIENTATION_VERTICAL),
    _mwi_txvtop(_mwi_buf), _mwi_txvbot(_mwi_buf),
    _mwi_txvcmd(),
    _mwi_statusbar(),
    _mwi_shownobmap()
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
  _mwi_mit_object_show_hide.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_show_hide));
  _mwi_mit_object_refresh.signal_activate().connect(sigc::mem_fun(this,&MomMainWindow::do_object_refresh));
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
    _mwi_vbox.pack_start(_mwi_txvcmd,Gtk::PACK_SHRINK);
    _mwi_txvcmd.set_vexpand(false);
    auto ctx = _mwi_txvcmd.get_style_context();
    ctx->add_class("commandwin_cl");
  }
  _mwi_vbox.pack_end(_mwi_statusbar,Gtk::PACK_SHRINK);
  set_default_size(550,300);
  display_full_browser();
  show_all_children();
};				// end MomMainWindow::MomMainWindow


MomMainWindow::~MomMainWindow()
{
  _mwi_shownobmap.clear();
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
  showdialog.show_all_children();
  int result =  Gtk::RESPONSE_CANCEL;
  do
    {
      result = showdialog.run();
      Glib::ustring showtext = showcombox.get_active_text();
      MomObject* pob = nullptr;
      if (isalpha(showtext[0]))
        {
          pob = mom_find_named(showtext.c_str());
        }
      else if (showtext[0] == '_' && isdigit(showtext[1]))
        {
          MomIdent idob = MomIdent::make_from_string(showtext.c_str(), MomIdent::DONT_FAIL);
          pob = MomObject::find_object_of_id(idob);
        }
      MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide result=" << result
                   << " showtext=" << MomShowString(showtext.c_str()) << " pob=" << pob);
      switch (result)
        {
        case ShowOb:
          MOM_DEBUGLOG(gui, "MomMainWindow::do_object_show_hide show showtext=" << MomShowString(showtext));
          if (pob)
            browser_show_object(pob);
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
MomMainWindow::browser_update_title_banner(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner start");
  auto it = _mwi_buf->begin();
  Gtk::TextIter begit =  it;
  auto titletag = MomApplication::itself()->lookup_tag("page_title_tag");
  Gtk::TextIter endit = it;
  while(endit.has_tag(titletag))
    endit.forward_char();
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner"
               << " begit/" << begit.get_offset()
               << "L" << begit.get_line() << "C" << begit.get_line_offset()
               << " endit/" << endit.get_offset()
               << "L" << endit.get_line() << "C" << endit.get_line_offset());
  _mwi_buf->erase(begit,endit);
  begit =  _mwi_buf->begin();
  int nbshownob = _mwi_shownobmap.size();
  it = _mwi_buf->insert_with_tag (begit, Glib::ustring::compose(" ~ %1 objects ~ ", nbshownob),
                                  titletag);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_update_title_banner end nbshownob=" << nbshownob);
} // end MomMainWindow::browser_update_title_banner


void
MomMainWindow::do_object_refresh(void)
{
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_refresh start");
  display_full_browser();
  MOM_DEBUGLOG(gui, "MomMainWindow::do_object_refresh end");
} // end MomMainWindow::do_object_refresh

void
MomMainWindow::browser_show_object(MomObject*pob)
{
  if (pob==nullptr || pob->vkind() != MomKind::TagObjectK)
    MOM_FATAPRINTF("MomMainWindow::browser_show_object invalid pob @%p", (void*)pob);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object start pob=" << MomShowObject(pob)
               << " nbshown=" << _mwi_shownobmap.size());
  auto shmbegit = _mwi_shownobmap.begin();
  auto shmendit = _mwi_shownobmap.end();
  MomObject*begpob = nullptr;
  MomObject*endpob = nullptr;
  if (shmbegit == shmendit)
    {
      MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object first object pob=" << pob);
      Gtk::TextIter txit = _mwi_buf->end();
      browser_insert_object_display(txit, pob);
      browser_update_title_banner();
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
      if (!afterbeg) {
	MomBrowsedObject& firstbob = shmbegit->second;
      }
      else 
      MOM_WARNLOG("MomMainWindow::browser_show_object non empty unimplemented for pob="  << pob
                  << " nbshown=" << _mwi_shownobmap.size());
    }
  browser_update_title_banner();
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_show_object end pob=" << pob
               << " nbshown=" << _mwi_shownobmap.size());
} // end MomMainWindow::browser_show_object

void
MomMainWindow::browser_hide_object(MomObject*pob)
{
  if (pob==nullptr || pob->vkind() != MomKind::TagObjectK)
    MOM_FATAPRINTF("MomMainWindow::browser_hide_object invalid pob @%p", (void*)pob);
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object start pob=" << MomShowObject(pob));
  MOM_DEBUGLOG(gui, "MomMainWindow::browser_hide_object end pob=" << pob);
} // end MomMainWindow::browser_hide_object

void
MomMainWindow::scan_gc(MomGC*gc)
{
  for (auto it: _mwi_shownobmap)
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
