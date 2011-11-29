#include <dao.h>
#include <string.h>
#include <gqqconfig.h>
#include <glib/gprintf.h>
#include <glib.h>

#define TABLE_CONF  "config"
#define TABLE_USER  "qquser"
#define TABLE_BDY   "buddies"
#define TABLE_GRP   "groups"
#define TABLE_GMEM  "gmembers"

static const gchar table_sql[] = 
        "create table if not exists config("
        "   id integer primary key asc autoincrement, "
        "   owner, key, value);"
        "create table if not exists qquser("
        "   qqnumber primary key ,"
        "   passwd, status, last, rempw);"
        "create table if not exists buddies("
        "   qqnumber primary key,"
        "   owner,vip_info, nick, markname, "
        "   country, province, city, gender, face, flag, birthday_y, "
        "   birthday_m, birthday_d, blood, shengxiao, constel, phone,"
        "   mobile, email, occupation, college, homepage, personal, lnick,"
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade );"
        "create table if not exists groups("
        "   gnumber primary key,"
        "   owner, name, flag, creator, mark, mask, opt, createtime,"
        "   gclass, glevel, face, memo, fingermemo, "
        "   foreign key(owner) references qquser(qqnumber) "
        "   on delete cascade);"
        "create table if not exists gmembers("
        "   id primary key,"
        "   gnumber, qqnumber, nick, flag, card, "
        "   foreign key(gnumber) references groups(gnumber) "
        "   on delete cascade);";

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

gint db_get_all_users(sqlite3 *db, GPtrArray **result)
{
    if(db == NULL || result == NULL){
        return SQLITE_ERROR;
    }
    static const gchar *sql = "select qqnumber,last,passwd,status,rempw from qquser;";
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(db, sql, 500, &stmt, NULL) != SQLITE_OK){
        g_warning("prepare sql error. SQL(%s) (%s, %d)", sql
                        , __FILE__, __LINE__);
        return SQLITE_ERROR;
    }
    
    GPtrArray *re = g_ptr_array_new();
    GQQLoginUser *usr = NULL;
    gchar *decode_passwed;
    gsize out_len;
    gint retcode;
    while(TRUE){
        retcode = sqlite3_step(stmt);
        switch(retcode)
        {
        case SQLITE_ROW:
            usr = g_slice_new0(GQQLoginUser);
            g_strlcpy(usr -> qqnumber
                        , (const gchar *)sqlite3_column_text(stmt, 0), 100);
            usr -> last = sqlite3_column_int(stmt, 1);
			usr -> rempw = sqlite3_column_int(stmt, 4);
            decode_passwed = (gchar *)g_base64_decode(
                                (const gchar *)sqlite3_column_text(stmt, 2)
                                , &out_len);
            g_strlcpy(usr -> passwd, decode_passwed, 100);
            g_free(decode_passwed);
            g_strlcpy(usr -> status
                        , (const gchar *)sqlite3_column_text(stmt, 3), 100);
            g_ptr_array_add(re, usr);
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
    *result = re;
    sqlite3_finalize(stmt);
    return retcode;
}

gint db_update_all(sqlite3 *db, const gchar *table
                            , const gchar *col, const gchar *value)
{
    if(db == NULL || table == NULL){
        return SQLITE_ERROR;
    }

    if(col == NULL || value == NULL){
        return SQLITE_OK;
    }

    gchar sql[500];
    g_snprintf(sql, 500, "update %s set %s=%s;", table, col, value);

    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
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
					, const gchar *status, gint last, gint rempw)
{
    if(db == NULL || qqnum == NULL){
        return SQLITE_ERROR;
    }
    gchar *encoded_passwd = g_base64_encode((const guchar*)passwd
                                                , (gsize)strlen(passwd));
    gchar sql[500];
    g_snprintf(sql, 500, "insert or replace into qquser (qqnumber, last, "
			   "passwd, status, rempw)values('%s', %d, '%s', '%s', '%d');"
			   , qqnum, last, encoded_passwd, status, rempw);
    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

gint db_exec_sql(sqlite3 *db, const gchar *sql)
{
    if(db == NULL || sql == NULL){
        return SQLITE_ERROR;
    }

    gchar *err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if(err != NULL){
        g_warning("SQL:(%s) error. %s (%s, %d)", sql, err, __FILE__, __LINE__);
        sqlite3_free(err);
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

void db_config_save_sql_append(GString *sql, const gchar *owner
                                    , const gchar *key, const gchar *value)
{
    if(sql == NULL || owner == NULL || key == NULL){
        return;
    }

    if(value == NULL){
        value = "";
    }
    gchar buf[500];
    g_snprintf(buf, 500, "insert or replace into config (owner, key, value) "
                    "values ('%s', '%s', '%s');", owner, key, value);
    g_string_append(sql, buf);
}
gint db_config_save(sqlite3 *db, const gchar *owner
                            , const gchar *key, const gchar *value)
{
    if(db == NULL || owner == NULL || key == NULL){
        return SQLITE_ERROR;
    }
    GString *sql = g_string_new(NULL); 
    db_config_save_sql_append(sql, owner, key, value);
    gint retcode = db_exec_sql(db, sql -> str);
    g_string_free(sql, TRUE);
    return retcode;
}

void db_buddy_save_sql_append(GString *sql, const gchar *owner, QQBuddy *bdy)
{
    if(sql == NULL || owner == NULL || bdy == NULL){
        return;
    }
    g_string_append(sql, "insert or replace into buddies ("
                        "owner, qqnumber, vip_info, nick, markname,"
                        "country, province, city, gender,"
                        "face, flag, birthday_y, birthday_m, birthday_d,"
                        "blood, shengxiao, constel, phone, mobile, email,"
                        "occupation, college, homepage, personal, lnick"
                        ") values (");
    g_string_append_printf(sql, "'%s',", owner);
    g_string_append_printf(sql, "'%s',", bdy -> qqnumber -> str);
    g_string_append_printf(sql, "%d,", bdy -> vip_info);
    g_string_append_printf(sql, "'%s',", bdy -> nick -> str);
    g_string_append_printf(sql, "'%s',", bdy -> markname -> str);
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

}
gint db_buddy_save(sqlite3 *db, const gchar *owner, QQBuddy *bdy)
{
    if(db == NULL || owner == NULL || bdy == NULL){
        return SQLITE_ERROR;
    }

    GString *sql = g_string_sized_new(5000);
    db_buddy_save_sql_append(sql, owner, bdy);
    gint retcode = db_exec_sql(db, sql -> str);
    g_string_free(sql, TRUE);
    return retcode;
}

void db_group_save_sql_append(GString *sql, const gchar *owner, QQGroup *grp)
{
    if(sql == NULL || owner == NULL || grp == NULL){
        return;
    }

    g_string_append(sql, "insert or replace into groups ("
                    "gnumber, owner, name, flag, creator, mark, mask, "
                    "opt, createtime, gclass, glevel, face, memo, fingermemo"
                    ") values (");

    g_string_append_printf(sql, "'%s', ", grp -> gnumber -> str);
    g_string_append_printf(sql, "'%s', ", owner);
    g_string_append_printf(sql, "'%s', ", grp -> name -> str);
    g_string_append_printf(sql, "'%s', ", grp -> flag -> str);
    g_string_append_printf(sql, "'%s', ", grp -> owner -> str);
    g_string_append_printf(sql, "'%s', ", grp -> mark -> str);
    g_string_append_printf(sql, "'%s', ", grp -> mask -> str);
    g_string_append_printf(sql, "'%s', ", grp -> option -> str);
    g_string_append_printf(sql, "'%s', ", grp -> createtime -> str);
    g_string_append_printf(sql, "'%s', ", grp -> gclass -> str);
    g_string_append_printf(sql, "'%s', ", grp -> level -> str);
    g_string_append_printf(sql, "'%s', ", grp -> face -> str);
    g_string_append_printf(sql, "'%s', ", grp -> memo -> str);
    g_string_append_printf(sql, "'%s');", grp -> fingermemo -> str);
    
    gint i;
    QQGMember *gmem;
    for(i = 0; i < grp -> members -> len ; ++i){
        gmem = (QQGMember*)g_ptr_array_index(grp -> members, i);
        if(gmem == NULL){
            continue;
        }
        g_string_append(sql, "insert or replace into gmembers("
                    "id, gnumber, qqnumber, nick, flag, card"
                    ") values (");
        // Use the group number and qq number as the key
        g_string_append_printf(sql, "'%s%s',", grp -> gnumber -> str
                                                , gmem -> qqnumber -> str);

        g_string_append_printf(sql, "'%s',", grp -> gnumber -> str);
        g_string_append_printf(sql, "'%s',", gmem -> qqnumber -> str);
        g_string_append_printf(sql, "'%s',", gmem -> nick -> str);
        g_string_append_printf(sql, "'%s',", gmem -> flag -> str);
        g_string_append_printf(sql, "'%s');", gmem -> card -> str);
    }
}
gint db_group_save(sqlite3 *db, const gchar *owner, QQGroup *grp)
{
    if(db == NULL || owner == NULL || grp == NULL){
        return SQLITE_ERROR;
    }

    GString *sql = g_string_sized_new(5000);
    db_group_save_sql_append(sql, owner, grp); 
    gint retcode = db_exec_sql(db, sql -> str);
    g_string_free(sql, TRUE);
    return retcode;
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

gint db_get_buddy(sqlite3 *db, const gchar *owner, QQBuddy *bdy, gint *cnt)
{
    if(db == NULL || owner == NULL || bdy == NULL){
        return SQLITE_ERROR;
    }
    GString *sql = g_string_sized_new(500);
    g_string_append(sql, "select "
                        "vip_info, nick, markname,"
                        "country, province, city, gender,"
                        "face, flag,"
                        "blood, shengxiao, constel, phone, mobile, email,"
                        "occupation, college, homepage, personal, lnick, "
                        "birthday_y, birthday_m, birthday_d "
                        "from buddies where owner=");
    g_string_append_printf(sql, "'%s' and qqnumber='%s';"
                                    , owner, bdy -> qqnumber -> str);

    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(db, sql -> str, 500, &stmt, NULL) != SQLITE_OK){
        g_warning("prepare sql error. SQL(%s) (%s, %d)", sql -> str
                                                    , __FILE__, __LINE__);
        g_string_free(sql, TRUE);
        return SQLITE_ERROR;
    }
    g_string_free(sql, TRUE);

    gint retcode = SQLITE_OK;
    gint idx, y, m, d;
    gint num = 0;
    while(TRUE){
        retcode = sqlite3_step(stmt);
        switch(retcode)
        {
        case SQLITE_ROW:
            bdy = qq_buddy_new();
            idx = -1;
            qq_buddy_set(bdy, "vip_info", sqlite3_column_int(stmt, ++idx));
            qq_buddy_set(bdy, "nick", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "markname", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "country", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "province", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "city", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "gender", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "face", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "flag", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "blood", sqlite3_column_int(stmt, ++idx));
            qq_buddy_set(bdy, "shengxiao", sqlite3_column_int(stmt, ++idx));
            qq_buddy_set(bdy, "constel", sqlite3_column_int(stmt, ++idx));
            qq_buddy_set(bdy, "phone", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "mobile", (const gchar *)sqlite3_column_text(stmt,++idx));
            qq_buddy_set(bdy, "email", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "occupation", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "college", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "homepage", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "personal", (const gchar *)sqlite3_column_text(stmt, ++idx));
            qq_buddy_set(bdy, "lnick", (const gchar *)sqlite3_column_text(stmt, ++idx));
            y = sqlite3_column_int(stmt, ++idx);
            m = sqlite3_column_int(stmt, ++idx);
            d = sqlite3_column_int(stmt, ++idx);
            qq_buddy_set(bdy, "birthday", y, m, d);
            ++num;
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
    if(cnt != NULL){
        *cnt = num;
    }
    sqlite3_finalize(stmt);
    return retcode;
}

//
// Get all the group members
//
static gint db_get_group_members(sqlite3 *db, QQGroup *grp)
{
    if(db == NULL || grp == NULL){
        return SQLITE_ERROR;
    }

    gchar sql[500];
    g_snprintf(sql, 500, "select qqnumber, nick, flag, card"
                        " from gmembers where gnumber='%s';"
                        , grp -> gnumber -> str);
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(db, sql, 500, &stmt, NULL) != SQLITE_OK){
        g_warning("prepare sql error. SQL(%s) (%s, %d)", sql
                        , __FILE__, __LINE__);
        return SQLITE_ERROR;
    }

    gint retcode = SQLITE_OK;
    QQGMember *gmem;
    while(TRUE){
        retcode = sqlite3_step(stmt);
        switch(retcode)
        {
        case SQLITE_ROW:
            gmem = qq_gmember_new();
            qq_gmember_set(gmem, "qqnumber", (const gchar *)sqlite3_column_text(stmt, 0));
            qq_gmember_set(gmem, "nick", (const gchar *)sqlite3_column_text(stmt, 1));
            qq_gmember_set(gmem, "flag", (const gchar *)sqlite3_column_text(stmt, 2));
            qq_gmember_set(gmem, "card", (const gchar *)sqlite3_column_text(stmt, 3));
            g_ptr_array_add(grp -> members, gmem);
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
    return SQLITE_OK;
}


gint db_get_group(sqlite3 *db, const gchar *owner, QQGroup *grp, gint *cnt)
{
    if(db == NULL || owner == NULL || grp == NULL){
        return SQLITE_ERROR;
    }

    gchar sql[500];
    g_snprintf(sql, 500, "select name, flag, creator, mark, mask, "
                    "opt, createtime, gclass, glevel, face, memo, fingermemo"
                    " from groups where owner='%s' and gnumber = '%s';"
                                    , owner, grp -> gnumber -> str);

    g_debug("Run sql : %s (%s, %d)", sql, __FILE__, __LINE__);
    sqlite3_stmt *stmt = NULL;
    if(sqlite3_prepare_v2(db, sql, 500, &stmt, NULL) != SQLITE_OK){
        g_warning("prepare sql error. SQL(%s) (%s, %d)", sql
                        , __FILE__, __LINE__);
        return SQLITE_ERROR;
    }

    gint retcode = SQLITE_OK;
    gint num = 0;
    while(TRUE){
        retcode = sqlite3_step(stmt);
        switch(retcode)
        {
        case SQLITE_ROW:
            qq_group_set(grp, "name", (const gchar *)sqlite3_column_text(stmt, 0));
            qq_group_set(grp, "flag", (const gchar *)sqlite3_column_text(stmt, 1));
            qq_group_set(grp, "owner", (const gchar *)sqlite3_column_text(stmt, 2));
            qq_group_set(grp, "mark", (const gchar *)sqlite3_column_text(stmt, 3));
            qq_group_set(grp, "mask", (const gchar *)sqlite3_column_text(stmt, 4));
            qq_group_set(grp, "option", (const gchar *)sqlite3_column_text(stmt, 5));
            qq_group_set(grp, "createtime", (const gchar *)sqlite3_column_text(stmt, 6));
            qq_group_set(grp, "gclass", (const gchar *)sqlite3_column_text(stmt, 7));
            qq_group_set(grp, "level", (const gchar *)sqlite3_column_text(stmt, 8));
            qq_group_set(grp, "face", (const gchar *)sqlite3_column_text(stmt, 9));
            qq_group_set(grp, "memo", (const gchar *)sqlite3_column_text(stmt, 10));
            qq_group_set(grp, "fingermemo", (const gchar *)sqlite3_column_text(stmt, 11));
            //get group members
            db_get_group_members(db, grp);
            ++num;
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
    if(cnt != NULL){
        *cnt = num;
    }
    sqlite3_finalize(stmt);
    return retcode;
}
