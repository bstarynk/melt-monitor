// this file should be generated. It is not yet

/// forward declarations;
struct MomINT;
struct MomSTRING;
struct MomTUPLE;
struct MomSET;
struct MomNODE;
struct MomITEM;

enum class MomItypeEn : uint16_t 
{
  _NONE,
    ///
  INT,              
  STRING,           
  TUPLE,              
  SET,                
  NODE,               
  ITEM,
  __LASTVALUE,
    /// *data names are not for first class values
    ASSOVALdata,
    VECTVALdata,
    RADIXdata,
  ////
  _LAST
};


struct MomVal {
  MomItypeEn vtype;
};

struct MomINT : MomVal {
  int64_t intval;
};

struct MomHashedVal : MomVal {
  momhash_t hashv;
};

struct MomSizedVal : MomHashedVal {
  unsigned usize;
};

struct MomSTRING final : MomSizedVal {
  char cstr[MOM_FLEXIBLE_DIM];
};


struct MomSeqItemsVal : MomSizedVal {
  MomITEM* itemsarr[MOM_FLEXIBLE_DIM];
};

struct MomTUPLE final : MomSeqItemsVal {
};

struct MomSET final : MomSeqItemsVal {
};

struct MomItemEntry {
  MomITEM* ient_itm;
  MomVal* ient_val;
};

struct MomCountedVal : MomSizedVal {
  unsigned ucount;
};

struct MomASSOVALdata : MomCountedVal {
  MomItemEntry ass_entries[MOM_FLEXIBLE_DIM];
};

struct MomVECTVALdata : MomCountedVal {
  MomVal* vect_comps[MOM_FLEXIBLE_DIM];
};

struct MomRADIXdata : MomVal {
  pthread_mutex_t rad_mtx;
  unsigned rad_sizehash;
  unsigned rad_counthash;
  MomSTRING* rad_name;
  MomITEM* rad_primitem;
  MomITEM** rad_hasharr;
};

struct MomITEM final : MomHashedVal {
  pthread_mutex_t itm_mtx;
  uint16_t itm_hid;
  uint64_t itm_lid;
  time_t itm_mtime;
  MomRADIXdata* itm_radix;
  MomASSOVALdata* itm_attrs;
  MomVECTVALdata* itm_comps;
  MomITEM* itm_paylsig;
  void* itm_payldata;
};
