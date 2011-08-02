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

#endif
