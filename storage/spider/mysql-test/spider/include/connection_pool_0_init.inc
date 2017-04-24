--disable_warnings
--disable_query_log
--disable_result_log
--source ../t/test_init.inc
--enable_result_log
--enable_query_log
--enable_warnings
--let $MASTER_1_COMMENT_2_1_BACKUP= $MASTER_1_COMMENT_2_1
let $MASTER_1_COMMENT_2_1=
  COMMENT='srv "s_2_1", table "tbl_a"';
--let $CHILD2_1_DROP_TABLES_BACKUP= $CHILD2_1_DROP_TABLES
let $CHILD2_1_DROP_TABLES=
  DROP TABLE IF EXISTS tbl_a;
--let $CHILD2_1_CREATE_TABLES_BACKUP= $CHILD2_1_CREATE_TABLES
let $CHILD2_1_CREATE_TABLES=
  CREATE TABLE tbl_a (
    value int NOT NULL
  ) $CHILD2_1_ENGINE $CHILD2_1_CHARSET;
--let $CHILD2_1_SELECT_TABLES_BACKUP= $CHILD2_1_SELECT_TABLES
let $CHILD2_1_SELECT_TABLES=
  SELECT value FROM tbl_a ORDER BY value;
let $CHILD2_1_SELECT_ARGUMENT1=
  SELECT argument FROM mysql.general_log WHERE argument LIKE '%select %';
--connection master_1
SET @old_max_connections = @@global.spider_max_connections;
SET GLOBAL spider_max_connections=1;
SET @old_conn_wait_timeout = @@global.spider_conn_wait_timeout;
SET GLOBAL spider_conn_wait_timeout=1;
--connection default
SET @old_conn_recycle_mode = @@session.spider_conn_recycle_mode;
SET spider_conn_recycle_mode=0;
--let $OUTPUT_CHILD_GROUP2_BACKUP= $OUTPUT_CHILD_GROUP2
--let $OUTPUT_CHILD_GROUP2= 1
--let $USE_GENERAL_LOG_BACKUP= $USE_GENERAL_LOG
--let $USE_GENERAL_LOG= 1
