/*
   Copyright (c) 2000, 2012, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */


#ifdef USE_PRAGMA_INTERFACE
#pragma interface			/* gcc class implementation */
#endif

/* class for the the myisam handler */

#include <myisam.h>
#include <ft_global.h>
#include "handler.h"                            /* handler */
#include "table.h"                              /* TABLE_SHARE */

#define HA_RECOVER_DEFAULT	1	/* Automatic recover active */
#define HA_RECOVER_BACKUP	2	/* Make a backupfile on recover */
#define HA_RECOVER_FORCE	4	/* Recover even if we loose rows */
#define HA_RECOVER_QUICK	8	/* Don't check rows in data file */
#define HA_RECOVER_FULL_BACKUP 16       /* Make a copy of index file too */
#define HA_RECOVER_OFF         32	/* No automatic recover */

extern TYPELIB myisam_recover_typelib;
extern const char *myisam_recover_names[];
extern ulonglong myisam_recover_options;

C_MODE_START
ICP_RESULT index_cond_func_myisam(void *arg);
C_MODE_END

class ha_myisam: public handler
{
  MI_INFO *file;
  ulonglong int_table_flags;
  char    *data_file_name, *index_file_name;
  bool can_enable_indexes;
  int repair(THD *thd, HA_CHECK &param, bool optimize);
  void setup_vcols_for_repair(HA_CHECK *param);
  void restore_vcos_after_repair();

 public:
  ha_myisam(handlerton *hton, TABLE_SHARE *table_arg);
  ~ha_myisam() {}
  handler *clone(const char *name, MEM_ROOT *mem_root);
  const char *index_type(uint key_number);
  ulonglong table_flags() const { return int_table_flags; }
  int index_init(uint idx, bool sorted);
  int index_end();
  int rnd_end();

  ulong index_flags(uint inx, uint part, bool all_parts) const;
  uint max_supported_keys()          const { return MI_MAX_KEY; }
  uint max_supported_key_parts()     const { return HA_MAX_KEY_SEG; }
  uint max_supported_key_length()    const { return HA_MAX_KEY_LENGTH; }
  uint max_supported_key_part_length() const { return HA_MAX_KEY_LENGTH; }
  uint checksum() const;
  void change_table_ptr(TABLE *table_arg, TABLE_SHARE *share);
  int open(const char *name, int mode, uint test_if_locked);
  int close(void);
  int write_row(uchar * buf);
  int update_row(const uchar * old_data, uchar * new_data);
  int delete_row(const uchar * buf);
  int index_read_map(uchar *buf, const uchar *key, key_part_map keypart_map,
                     enum ha_rkey_function find_flag);
  int index_read_idx_map(uchar *buf, uint index, const uchar *key,
                         key_part_map keypart_map,
                         enum ha_rkey_function find_flag);
  int index_next(uchar * buf);
  int index_prev(uchar * buf);
  int index_first(uchar * buf);
  int index_last(uchar * buf);
  int index_next_same(uchar *buf, const uchar *key, uint keylen);
  int ft_init()
  {
    if (!ft_handler)
      return 1;
    ft_handler->please->reinit_search(ft_handler);
    return 0;
  }
  FT_INFO *ft_init_ext(uint flags, uint inx,String *key)
  {
    return ft_init_search(flags,file,inx,
                          (uchar *)key->ptr(), key->length(), key->charset(),
                          table->record[0]);
  }
  int ft_read(uchar *buf);
  int rnd_init(bool scan);
  int rnd_next(uchar *buf);
  int rnd_pos(uchar * buf, uchar *pos);
  int remember_rnd_pos();
  int restart_rnd_next(uchar *buf);
  void position(const uchar *record);
  int info(uint);
  int extra(enum ha_extra_function operation);
  int extra_opt(enum ha_extra_function operation, ulong cache_size);
  int reset(void);
  int external_lock(THD *thd, int lock_type);
  int delete_all_rows(void);
  int reset_auto_increment(ulonglong value);
  int disable_indexes(uint mode);
  int enable_indexes(uint mode);
  int indexes_are_disabled(void);
  void start_bulk_insert(ha_rows rows, uint flags);
  int end_bulk_insert();
  ha_rows records_in_range(uint inx, key_range *min_key, key_range *max_key);
  void update_create_info(HA_CREATE_INFO *create_info);
  int create(const char *name, TABLE *form, HA_CREATE_INFO *create_info);
  THR_LOCK_DATA **store_lock(THD *thd, THR_LOCK_DATA **to,
			     enum thr_lock_type lock_type);
  virtual void get_auto_increment(ulonglong offset, ulonglong increment,
                                  ulonglong nb_desired_values,
                                  ulonglong *first_value,
                                  ulonglong *nb_reserved_values);
  int rename_table(const char * from, const char * to);
  int delete_table(const char *name);
  int check_for_upgrade(HA_CHECK_OPT *check_opt);
  int check(THD* thd, HA_CHECK_OPT* check_opt);
  int analyze(THD* thd,HA_CHECK_OPT* check_opt);
  int repair(THD* thd, HA_CHECK_OPT* check_opt);
  bool check_and_repair(THD *thd);
  bool is_crashed() const;
  bool auto_repair(int error) const
  {
    return (myisam_recover_options != HA_RECOVER_OFF &&
            error == HA_ERR_CRASHED_ON_USAGE);
  }
  int optimize(THD* thd, HA_CHECK_OPT* check_opt);
  int assign_to_keycache(THD* thd, HA_CHECK_OPT* check_opt);
  int preload_keys(THD* thd, HA_CHECK_OPT* check_opt);
  enum_alter_inplace_result check_if_supported_inplace_alter(TABLE *new_table,
                                            Alter_inplace_info *alter_info);
  bool check_if_incompatible_data(HA_CREATE_INFO *info, uint table_changes);
#ifdef HAVE_QUERY_CACHE
  my_bool register_query_cache_table(THD *thd, char *table_key,
                                     uint key_length,
                                     qc_engine_callback
                                     *engine_callback,
                                     ulonglong *engine_data);
#endif
  MI_INFO *file_ptr(void)
  {
    return file;
  }
public:
  /**
   * Multi Range Read interface
   */
  int multi_range_read_init(RANGE_SEQ_IF *seq, void *seq_init_param,
                            uint n_ranges, uint mode, HANDLER_BUFFER *buf);
  int multi_range_read_next(range_id_t *range_info);
  ha_rows multi_range_read_info_const(uint keyno, RANGE_SEQ_IF *seq,
                                      void *seq_init_param, 
                                      uint n_ranges, uint *bufsz,
                                      uint *flags, Cost_estimate *cost);
  ha_rows multi_range_read_info(uint keyno, uint n_ranges, uint keys,
                                uint key_parts, uint *bufsz, 
                                uint *flags, Cost_estimate *cost);
  int multi_range_read_explain_info(uint mrr_mode, char *str, size_t size);

  /* Index condition pushdown implementation */
  Item *idx_cond_push(uint keyno, Item* idx_cond);
private:
  DsMrr_impl ds_mrr;
  friend ICP_RESULT index_cond_func_myisam(void *arg);
};
