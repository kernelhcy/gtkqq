#ifndef __GTKQQ_DAO_H
#define __GTKQQ_DAO_H
#include <glib.h>
#include <sqlite3.h>
#include <qq.h>

//
// The DAO
// Manage the connection to the database.
//

//
// Open a connection to the databases.
//
// If databases not exist, create it.
// The tables will also be created too.
//
sqlite3* db_open();

//
// Close the connection to the databases.
//
void db_close(sqlite3 *db);

//
// Set the all the row's value of col to value.
//
gint db_update_all(sqlite3 *db, const gchar *table
                        , const gchar *col, const gchar *value);
//
// Get all the users in qquser
// The result array must be free when no use.
//
gint db_get_all_users(sqlite3 *db, GPtrArray **result);
//
// Delete all the data in all tables.
// NOT delete the tables.
//
gint db_clear_db(sqlite3 *db);
//
// Delete all the data in table where key = value
//
gint db_clear_table(sqlite3 *db, const gchar *table
                            , const gchar *key, const gchar *value);

//
// Store qq user into db
//
gint db_qquser_save(sqlite3 *db, const gchar *qqnum, const gchar *passwd
					, const gchar *status, gint last, gint rempw);

//
// Store key:value into db
// Use g_free to free the value after using it.
//
gint db_config_save(sqlite3 *db, const gchar *owner
                            , const gchar *key, const gchar *value);

//
// Store buddy, group into db
//

// Just create the sql statement
void db_buddy_save_sql_append(GString *sql, const gchar *owner, QQBuddy *bdy);
gint db_buddy_save(sqlite3 *db, const gchar *owner, QQBuddy *bdy);

// Just create the sql statement
void db_group_save_sql_append(GString *sql, const gchar *owner, QQGroup *grp);
gint db_group_save(sqlite3 *db, const gchar *owner, QQGroup *grp);

//
// Get key:value
//
gint db_config_get(sqlite3 *db, const gchar *owner
                            , const gchar *key, gchar **value);
//
// Get the buddy and group.
// @param cnt : the number of the recoders found in the db
//
gint db_get_buddy(sqlite3 *db, const gchar *owner, QQBuddy *bdy, gint *cnt);
gint db_get_group(sqlite3 *db, const gchar *owner, QQGroup *grp, gint *cnt);

//
// Execute the sql
//
gint db_exec_sql(sqlite3 *db, const gchar *sql);
#endif
