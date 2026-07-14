#ifndef MARIADB_PLUGIN_COLUMN_POLICY_INCLUDED
#define MARIADB_PLUGIN_COLUMN_POLICY_INCLUDED

/* Copyright (c) 2026, MariaDB plc

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.
*/

#ifdef __cplusplus

#include <mysql/plugin.h>

class Item;
class Item_field;
class THD;

#define MariaDB_COLUMN_POLICY_INTERFACE_VERSION (MYSQL_VERSION_ID << 8)

enum mariadb_column_policy_context
{
  MARIADB_COLUMN_POLICY_CONTEXT_OTHER= 0,
  MARIADB_COLUMN_POLICY_CONTEXT_OUTPUT,
  MARIADB_COLUMN_POLICY_CONTEXT_FILTER,
  MARIADB_COLUMN_POLICY_CONTEXT_GROUP,
  MARIADB_COLUMN_POLICY_CONTEXT_ORDER
};

enum mariadb_column_policy_result
{
  MARIADB_COLUMN_POLICY_ERROR= -1,
  MARIADB_COLUMN_POLICY_NOT_APPLICABLE= 0,
  MARIADB_COLUMN_POLICY_APPLIED= 1
};

/**
  Server-internal C++ interface for column expression policy plugins.

  The callback runs after privilege checks and name resolution. If a policy
  applies, it allocates an expression in thd->mem_root and stores it in
  replacement. The server activates the persistent statement arena before
  invoking the callback and fixes the returned expression before continuing.
*/
struct st_mariadb_column_policy
{
  int interface_version;
  int (*rewrite)(THD *thd, Item_field *original,
                 enum mariadb_column_policy_context context,
                 Item **replacement);
};

#endif /* __cplusplus */

#endif /* MARIADB_PLUGIN_COLUMN_POLICY_INCLUDED */
