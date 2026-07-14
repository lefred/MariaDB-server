/* Copyright (c) 2026, MariaDB plc

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.
*/

#include "mariadb.h"
#include "sql_class.h"
#include "sql_lex.h"
#include "sql_plugin.h"
#include "sql_column_policy.h"
#include "item.h"

struct Column_policy_rewrite_args
{
  Item_field *field;
  Item *replacement;
  enum mariadb_column_policy_context context;
  int result;
};

static my_bool column_policy_rewrite_callback(THD *thd, plugin_ref plugin,
                                               void *data)
{
  Column_policy_rewrite_args *args=
    static_cast<Column_policy_rewrite_args *>(data);
  const st_mariadb_column_policy *policy=
    static_cast<const st_mariadb_column_policy *>(plugin_decl(plugin)->info);
  Item *replacement= nullptr;
  int result= policy->rewrite(thd, args->field, args->context, &replacement);

  if (result == MARIADB_COLUMN_POLICY_APPLIED && !replacement)
  {
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    result= MARIADB_COLUMN_POLICY_ERROR;
  }
  if (result != MARIADB_COLUMN_POLICY_NOT_APPLICABLE)
  {
    args->replacement= replacement;
    args->result= result;
    return true;
  }
  return false;
}

enum mariadb_column_policy_context column_policy_context(THD *thd)
{
  if (!thd->lex->current_select)
    return MARIADB_COLUMN_POLICY_CONTEXT_OTHER;

  switch (thd->lex->current_select->context_analysis_place)
  {
  case SELECT_LIST:
  case IN_RETURNING:
    return MARIADB_COLUMN_POLICY_CONTEXT_OUTPUT;
  case IN_WHERE:
  case IN_HAVING:
  case IN_ON:
    return MARIADB_COLUMN_POLICY_CONTEXT_FILTER;
  case IN_GROUP_BY:
    return MARIADB_COLUMN_POLICY_CONTEXT_GROUP;
  case IN_ORDER_BY:
    return MARIADB_COLUMN_POLICY_CONTEXT_ORDER;
  default:
    return MARIADB_COLUMN_POLICY_CONTEXT_OTHER;
  }
}

bool column_policy_rewrite_field(THD *thd, Item_field *field, Item **reference,
                                 enum mariadb_column_policy_context context)
{
  if (field->column_policy_processed() || !field->field ||
      field->field->table->s->tmp_table != NO_TMP_TABLE)
    return false;

  field->mark_column_policy_processed();
  Query_arena backup;
  Query_arena *arena= thd->activate_stmt_arena_if_needed(&backup);
  Column_policy_rewrite_args args{field, nullptr, context,
                                  MARIADB_COLUMN_POLICY_NOT_APPLICABLE};
  plugin_foreach(thd, column_policy_rewrite_callback,
                 MariaDB_COLUMN_POLICY_PLUGIN, &args);

  bool error= false;
  if (args.result == MARIADB_COLUMN_POLICY_ERROR)
    error= true;
  else if (args.result == MARIADB_COLUMN_POLICY_APPLIED)
  {
    args.replacement->name= field->name;
    *reference= args.replacement;
    error= args.replacement->fix_fields_if_needed_for_scalar(thd, reference) ||
           (*reference)->check_cols(1);
  }

  if (arena)
    thd->restore_active_arena(arena, &backup);
  return error;
}
