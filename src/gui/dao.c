#include <dao.h>

#define TABLE_CONF  "config"
#define TABLE_USER  "qquser"
#define TABLE_BDY   "buddies"
#define TABLE_GRP   "groups"
#define TABLE_GMEM  "gmembers"
#define TABLE_CATE  "categories"

static const gchar table_sql[] = 
        "create table if not exists config("
        "   id integer primary key asc autoincrement, "
        "   key, value);"
        "create table if not exists qquser("
        "   qqnumber primary key ,"
        "   last);"
        "create table if not exists buddies("
        "   id integer primary key asc autoincrement,"
        "   owner, uin, qqnumber, vip_info, nick, markname, faceimgfile, "
        "   country, province, city, gender, face, flag, birthday_y, "
        "   birthday_m, birthday_d, blood, shengxiao, constel, phone,"
        "   mobile, email, occupation, college, homepage, personal, lnick,"
        "   cate_idx,"
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade );"
        "create table if not exists groups("
        "   gnumber primary key,"
        "   owner, name, code, flag, creater, mark, mask, opt, createtime,"
        "   gclass, glevel, face, memo, fingermemo, "
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade);"
        "create table if not exists gmemebers("
        "   gnumber, qqnumber, nick, flag, status, card, "
        "   foreign key(gnumber) references groups(gnumber) "
        "   on delete cascade);"
        "create table if not exists categories("
        "   id integer primary key asc autoincrement,"
        "   owner, idx, name,"
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade );";

//
// Test if we have tables in the databases
//
static gboolean test_table_exist(sqlite3 *db)
{
    gchar *err = NULL;
    sqlite3_exec(db, "select * from qquser", NULL, NULL, &err);
    if(err != NULL){
        // no such table.
        sqlite3_free(err);
        return FALSE;
    }
    return TRUE;
}

static gint create_tables(sqlite3 *db)
{
    // Try to drop all tables
    sqlite3_exec(db, "drop table if exists "TABLE_CONF, NULL, NULL, NULL);
    sqlite3_exec(db, "drop table if exists "TABLE_CATE, NULL, NULL, NULL);
    sqlite3_exec(db, "drop table if exists "TABLE_GMEM, NULL, NULL, NULL);
    sqlite3_exec(db, "drop table if exists "TABLE_GRP, NULL, NULL, NULL);
    sqlite3_exec(db, "drop table if exists "TABLE_BDY, NULL, NULL, NULL);
    sqlite3_exec(db, "drop table if exists "TABLE_USER, NULL, NULL, NULL);

    g_debug("Create all tables (%s, %d)", __FILE__, __LINE__);
    gchar *err = NULL;
    //create all tables
    sqlite3_exec(db, table_sql, NULL, NULL, &err);
    if(err != NULL){
        g_error("create tables error. %s (%s, %d)", err
                            , __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

sqlite3* db_open()
{
    sqlite3 *db;
    g_debug("Open db connection to file://"CONFIGDIR"gtkqq.db (%s, %d)"
                                        , __FILE__, __LINE__);
    //gint retcode = sqlite3_open("file://"CONFIGDIR"gtkqq.db", &db);
    gint retcode = sqlite3_open("gtkqq.db", &db);
    if(retcode != SQLITE_OK){
        if(retcode == SQLITE_NOMEM){
            g_error("Open database error. no memory. (%s, %d)", __FILE__, __LINE__);
            return NULL;
        }
        g_error("Open database error. %s (%s, %d)", sqlite3_errmsg(db)
                                        , __FILE__, __LINE__);
        sqlite3_close(db);
        return NULL;
    }
    if(!test_table_exist(db)){
        if(create_tables(db) != SQLITE_OK){
            g_error("Create tables error! (%s, %d)", __FILE__, __LINE__);
            sqlite3_close(db);
            return NULL;
        }
    }
    return db;
}

void db_close(sqlite3 *db)
{
    sqlite3_close(db);
}
