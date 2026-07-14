#ifndef SQL_COLUMN_POLICY_INCLUDED
#define SQL_COLUMN_POLICY_INCLUDED

#include <mysql/plugin_column_policy.h>

class Item;
class Item_field;
class THD;

enum mariadb_column_policy_context column_policy_context(THD *thd);
bool column_policy_rewrite_field(THD *thd, Item_field *field, Item **reference,
                                 enum mariadb_column_policy_context context);

#endif /* SQL_COLUMN_POLICY_INCLUDED */
