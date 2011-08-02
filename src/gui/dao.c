#include <dao.h>
#include <string.h>

#define TABLE_CONF  "config"
#define TABLE_USER  "qquser"
#define TABLE_BDY   "buddies"
#define TABLE_GRP   "groups"
#define TABLE_GMEM  "gmembers"
#define TABLE_CATE  "categories"

static const gchar table_sql[] = 
        "create table if not exists config("
        "   id integer primary key asc autoincrement, "
        "   owner, key, value);"
        "create table if not exists qquser("
        "   qqnumber primary key ,"
        "   passwd, status, last);"
        "create table if not exists buddies("
        "   id integer primary key asc autoincrement,"
        "   owner, qqnumber, vip_info, nick, markname, faceimgfile, "
        "   country, province, city, gender, face, flag, birthday_y, "
        "   birthday_m, birthday_d, blood, shengxiao, constel, phone,"
        "   mobile, email, occupation, college, homepage, personal, lnick,"
        "   cate_idx,"
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade );"
        "create table if not exists groups("
        "   gnumber primary key,"
        "   owner, name, code, flag, creator, mark, mask, opt, createtime,"
        "   gclass, glevel, face, memo, fingermemo, "
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade);"
        "create table if not exists gmembers("
        "   gnumber, qqnumber, nick, flag, card, "
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
    g_debug("Open db connection to "CONFIGDIR"gtkqq.db (%s, %d)"
                                        , __FILE__, __LINE__);
    gint retcode = sqlite3_open(CONFIGDIR"gtkqq.db", &db);
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

gint db_clear_table(sqlite3 *db, const gchar *table
                            , const gchar *key, const gchar *value)
{
    if(db == NULL || table == NULL){
        return SQLITE_ERROR;
    }
    gchar sql[500];
    g_snprintf(sql, 500, "delete from %s where %s='%s';", table, key, value);

    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

gint db_qquser_save(sqlite3 *db, const gchar *qqnum, const gchar *passwd
                                , const gchar *status, gint last)
{
    if(db == NULL || qqnum == NULL){
        return SQLITE_ERROR;
    }
    gchar *encoded_passwd = g_base64_encode((const guchar*)passwd
                                                , (gsize)strlen(passwd));
    gchar sql[500];
    g_snprintf(sql, 500, "insert into qquser (qqnumber, last, passwd, status)"
                        "values('%s', %d, '%s', '%s');"
                        , qqnum, last, encoded_passwd, status);
    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

gint db_config_save(sqlite3 *db, const gchar *owner
                            , const gchar *key, const gchar *value)
{
    if(db == NULL || owner == NULL || key == NULL){
        return SQLITE_ERROR;
    }
    
    if(value == NULL){
        value = "";
    }

    gchar sql[500];
    g_snprintf(sql, 500, "insert into config (owner, key, value) "
                    "values ('%s', '%s', '%s');", owner, key, value);
    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

gint db_buddy_save(sqlite3 *db, const gchar *owner, QQBuddy *bdy)
{
    if(db == NULL || owner == NULL || bdy == NULL){
        return SQLITE_ERROR;
    }

    GString *sql = g_string_sized_new(5000);
    g_string_append(sql, "insert into buddies ("
                        "owner, qqnumber, vip_info, nick, markname,"
                        "faceimgfile, country, province, city, gender,"
                        "face, flag, birthday_y, birthday_m, birthday_d,"
                        "blood, shengxiao, constel, phone, mobile, email,"
                        "occupation, college, homepage, personal, lnick, "
                        "cate_idx) values (");
    g_string_append_printf(sql, "'%s',", owner);
    g_string_append_printf(sql, "'%s',", bdy -> qqnumber -> str);
    g_string_append_printf(sql, "%d,", bdy -> vip_info);
    g_string_append_printf(sql, "'%s',", bdy -> nick -> str);
    g_string_append_printf(sql, "'%s',", bdy -> markname -> str);
    g_string_append_printf(sql, "'%s',", bdy -> faceimgfile -> str);
    g_string_append_printf(sql, "'%s',", bdy -> country -> str);
    g_string_append_printf(sql, "'%s',", bdy -> province -> str);
    g_string_append_printf(sql, "'%s',", bdy -> city -> str);
    g_string_append_printf(sql, "'%s',", bdy -> gender -> str);
    g_string_append_printf(sql, "'%s',", bdy -> face -> str);
    g_string_append_printf(sql, "'%s',", bdy -> flag -> str);
    g_string_append_printf(sql, "%d,", bdy -> birthday.year);
    g_string_append_printf(sql, "%d,", bdy -> birthday.month);
    g_string_append_printf(sql, "%d,", bdy -> birthday.day);
    g_string_append_printf(sql, "%d,", bdy -> blood);
    g_string_append_printf(sql, "%d,", bdy -> shengxiao);
    g_string_append_printf(sql, "%d,", bdy -> constel);
    g_string_append_printf(sql, "'%s',", bdy -> phone -> str);
    g_string_append_printf(sql, "'%s',", bdy -> mobile -> str);
    g_string_append_printf(sql, "'%s',", bdy -> email -> str);
    g_string_append_printf(sql, "'%s',", bdy -> occupation -> str);
    g_string_append_printf(sql, "'%s',", bdy -> college -> str);
    g_string_append_printf(sql, "'%s',", bdy -> homepage -> str);
    g_string_append_printf(sql, "'%s',", bdy -> personal -> str);
    g_string_append_printf(sql, "'%s',", bdy -> lnick-> str);
    g_string_append_printf(sql, "%d);", bdy -> cate -> index);

    gchar *err = NULL;
    sqlite3_exec(db, sql -> str,  NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql -> str, err
                                        , __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }
    g_string_free(sql, TRUE);
    return SQLITE_OK;
}

gint db_group_save(sqlite3 *db, const gchar *owner, QQGroup *grp)
{
    if(db == NULL || owner == NULL || grp == NULL){
        return SQLITE_ERROR;
    }

    GString *sql = g_string_sized_new(5000);
    g_string_append(sql, "insert into groups ("
                    "gnumber, owner, name, code, flag, creator, mark, mask, "
                    "opt, createtime, gclass, glevel, face, memo, fingermemo"
                    ") values (");

    g_string_append_printf(sql, "'%s',", grp -> gnumber -> str);
    g_string_append_printf(sql, "'%s',", owner);
    g_string_append_printf(sql, "'%s', ", grp -> name -> str);
    g_string_append_printf(sql, "'%s', ", grp -> code -> str);
    g_string_append_printf(sql, "'%s', ", grp -> flag -> str);
    g_string_append_printf(sql, "'%s', ", grp -> owner -> str);
    g_string_append_printf(sql, "'%s', ", grp -> mark -> str);
    g_string_append_printf(sql, "'%s', ", grp -> mask -> str);
    g_string_append_printf(sql, "%d,", grp -> option);
    g_string_append_printf(sql, "'%s', ", grp -> createtime -> str);
    g_string_append_printf(sql, "%d,", grp -> gclass);
    g_string_append_printf(sql, "%d,", grp -> level);
    g_string_append_printf(sql, "%d,", grp -> face);
    g_string_append_printf(sql, "'%s', ", grp -> memo -> str);
    g_string_append_printf(sql, "'%s');", grp -> fingermemo -> str);

    gchar *err = NULL;
    sqlite3_exec(db, sql -> str, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql -> str
                                    , err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }
    
    gint i;
    QQGMember *gmem;
    for(i = 0; i < grp -> members -> len ; ++i){
        gmem = (QQGMember*)g_ptr_array_index(grp -> members, i);
        if(gmem == NULL){
            continue;
        }
        g_string_truncate(sql, 0);
        g_string_append(sql, "insert into gmembers("
                    "gnumber, qqnumber, nick, flag, card"
                    ") values (");
        g_string_append_printf(sql, "'%s,'", grp -> gnumber -> str);
        g_string_append_printf(sql, "'%s,'", gmem -> qqnumber -> str);
        g_string_append_printf(sql, "'%s,'", gmem -> nick -> str);
        g_string_append_printf(sql, "'%s,'", gmem -> flag -> str);
        g_string_append_printf(sql, "'%s');", gmem -> card -> str);
        sqlite3_exec(db, sql -> str, NULL, NULL, &err);
        if(err != NULL){
            g_warning("SQL:(%s) error. %s (%s, %d)", sql -> str
                                            , err, __FILE__, __LINE__);
            sqlite3_free(err);
        }
    }
    
    g_string_free(sql, TRUE);
    return SQLITE_OK;
}

gint db_category_save(sqlite3 *db, const gchar *owner, QQCategory *cate)
{
    if(db == NULL || owner == NULL || cate == NULL){
        return SQLITE_ERROR;
    }

    static const gchar *sql_fmt = "insert into categories ("
                                "owner, idx, name) values ('%s', %d, '%s')";
    gchar sql[5000];
    g_snprintf(sql, 5000, sql_fmt, owner, cate -> index, cate -> name -> str);
    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

//
// Get key:value
//
gint db_config_get(sqlite3 *db, const gchar *owner
                            , const gchar *key, gchar **value)
{
    if(db == NULL || owner == NULL || key == NULL || value == NULL){
        return SQLITE_ERROR;
    }

    gchar sql[500];
    g_snprintf(sql, 500, "select value from config where owner='%s' "
                        "and key='%s'", owner, key);

    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(db, sql, 500, &stmt, NULL) != SQLITE_OK){
        g_warning("prepare sql error. SQL(%s) (%s, %d)", sql
                        , __FILE__, __LINE__);
        return SQLITE_ERROR;
    }

    gint retcode;
    while(TRUE){
        retcode = sqlite3_step(stmt);
        switch(retcode)
        {
        case SQLITE_ROW:
            *value = g_strdup((const gchar *)sqlite3_column_text(stmt, 0));
            break;
        case SQLITE_DONE:
            retcode = SQLITE_OK;
            goto out_label;
            break;
        default:
            g_warning("sqlite3_step error!(%s, %d)", __FILE__, __LINE__);
            goto out_label;
            break;
        }
    }

out_label:
    sqlite3_finalize(stmt);
    return retcode;
}
#if 0
gint db_buddy_get(sqlite3 *db, const gchar *owner, GPtrArray **buddies);
gint db_group_get(sqlite3 *db, const gchar *owner, GPtrArray **groups);
gint db_category_get(sqlite3 *db, const gchar *owner, GPtrArray **cates);
#endif
