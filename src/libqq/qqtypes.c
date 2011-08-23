#include <qqtypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/*
 * QQInfo
 */
QQInfo* qq_info_new()
{
    QQInfo *info = g_slice_new0(QQInfo);
    info -> need_vcimage = FALSE;
    
    info -> me = qq_buddy_new();
    info -> buddies = g_ptr_array_new();

    info -> groups = g_ptr_array_new();
    info -> recentcons = g_ptr_array_new();
    info -> categories = g_ptr_array_new();

    info -> buddies_ht = g_hash_table_new(g_str_hash, g_str_equal);
    info -> buddies_number_ht = g_hash_table_new(g_str_hash, g_str_equal);
    info -> groups_ht = g_hash_table_new(g_str_hash, g_str_equal);
    info -> groups_number_ht = g_hash_table_new(g_str_hash, g_str_equal);

    info -> lock = g_mutex_new();
    info -> clientid = g_string_new("");
    info -> psessionid = g_string_new("");

    GTimeVal now;
    g_get_current_time(&now);
    glong v = now.tv_usec;
    v = (v - v % 1000) / 1000;
    v = v % 10000 * 10000;
    info -> msg_id = v;
    return info;
}

void qq_info_free(QQInfo *info)
{
    if(info == NULL){
        return;
    }
    
    qq_buddy_free(info -> me);
#define FREE_STR(x) if(info -> x != NULL){g_string_free(info -> x, TRUE);}
    FREE_STR(vc_type);
    FREE_STR(vc_image_data);
    FREE_STR(vc_image_type);
    FREE_STR(verify_code);
    FREE_STR(ptvfsession);
    FREE_STR(version);
    FREE_STR(ptuserinfo);
    FREE_STR(ptwebqq);
    FREE_STR(ptcz);
    FREE_STR(skey);
    FREE_STR(cookie);
    FREE_STR(clientid);
    FREE_STR(seskey);
    FREE_STR(cip);
    FREE_STR(index);
    FREE_STR(port);
    FREE_STR(psessionid);
    FREE_STR(vfwebqq);
#undef FREE_STR

    gint i;
    for(i = 0; i < info -> buddies -> len; ++i){
        qq_buddy_free(info -> buddies -> pdata[i]);
    }
    g_ptr_array_free(info -> buddies, TRUE);

    for(i = 0; i < info -> groups -> len; ++i){
        qq_group_free(info -> groups -> pdata[i]);
    }
    g_ptr_array_free(info -> groups, TRUE);

    for(i = 0; i < info -> recentcons -> len; ++i){
        qq_recentcon_free(info -> recentcons -> pdata[i]);
    }
    g_ptr_array_free(info -> recentcons, TRUE);

    for(i = 0; i < info -> categories -> len; ++i){
        qq_category_free(info -> categories -> pdata[i]);
    }
    g_ptr_array_free(info -> categories, TRUE);


    g_hash_table_unref(info -> buddies_ht);
    g_hash_table_unref(info -> groups_ht);
    g_hash_table_unref(info -> buddies_number_ht);
    g_hash_table_unref(info -> groups_number_ht);

    g_mutex_free(info -> lock);
    g_slice_free(QQInfo, info);
}

QQBuddy* qq_info_lookup_buddy_by_uin(QQInfo *info, const gchar *uin)
{
    QQBuddy *bdy = (QQBuddy*)g_hash_table_lookup(info -> buddies_ht, uin);
    if(bdy == NULL){
        gint i;
        for(i = 0; i < info -> buddies -> len; ++i){
            bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
            if(g_strcmp0(uin, bdy -> uin -> str) == 0){
                g_hash_table_insert(info -> buddies_ht, (gpointer)uin, bdy);
                return bdy;
            }
            bdy = NULL;
        }
    }

    return bdy;
}
QQBuddy* qq_info_lookup_buddy_by_number(QQInfo *info, const gchar *number)
{
    QQBuddy *bdy = (QQBuddy*)g_hash_table_lookup(info -> buddies_number_ht
                                                            , number);
    if(bdy == NULL){
        gint i;
        for(i = 0; i < info -> buddies -> len; ++i){
            bdy = (QQBuddy*)g_ptr_array_index(info -> buddies, i);
            if(g_strcmp0(number, bdy -> qqnumber -> str) == 0){
                g_hash_table_insert(info -> buddies_number_ht
                                    , (gpointer)number, bdy);
                return bdy;
            }
            bdy = NULL;
        }
    }

    return bdy;
}

QQGroup* qq_info_lookup_group_by_code(QQInfo *info, const gchar *code)
{
    QQGroup *grp = (QQGroup*)g_hash_table_lookup(info -> groups_ht, code);
    if(grp == NULL){
        gint i;
        for(i = 0; i < info -> groups -> len; ++i){
            grp = (QQGroup*)g_ptr_array_index(info -> groups, i);
            if(g_strcmp0(code, grp -> code -> str) == 0){
                g_hash_table_insert(info -> groups_ht, (gpointer)code, grp);
                return grp;
            }
            grp = NULL;
        }
    }
    return grp;
}
QQGroup* qq_info_lookup_group_by_number(QQInfo *info, const gchar *number)
{
    QQGroup *grp = (QQGroup*)g_hash_table_lookup(info -> groups_ht, number);
    if(grp == NULL){
        gint i;
        for(i = 0; i < info -> groups -> len; ++i){
            grp = (QQGroup*)g_ptr_array_index(info -> groups, i);
            if(g_strcmp0(number, grp -> gnumber -> str) == 0){
                g_hash_table_insert(info -> groups_number_ht
                                            , (gpointer)number, grp);
                return grp;
            }
            grp = NULL;
        }
    }
    return grp;
}

//
// QQMsgFont
//
QQMsgFont* qq_msgfont_new(const gchar *name, gint size, const gchar *color
                            , gint sa, gint sb, gint sc)
{
    QQMsgFont *font = g_slice_new0(QQMsgFont);
    if(font == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__);
        return NULL;
    }

    if(name == NULL){
        font -> name = g_string_new("Arial");
    }else{
        font -> name = g_string_new(name);
    }

    if(color == NULL){
        font -> color = g_string_new("000000");
    }else{
        font -> color = g_string_new(color);
    }

    font -> size = size;
    font -> style.a = sa;
    font -> style.b = sb;
    font -> style.c = sc;

    return font;
}
void qq_msgfont_free(QQMsgFont *font)
{
    if(font == NULL){
        return;
    }

    g_string_free(font -> name, TRUE);
    g_string_free(font -> color, TRUE);

    g_slice_free(QQMsgFont, font);
}

gboolean qq_msgfont_equal(QQMsgFont *a, QQMsgFont *b)
{
    if(a == b){
        return TRUE;
    }
    if(a == NULL || b == NULL){
        return FALSE;
    }

    if(!g_string_equal(a -> name, b -> name)){
        return FALSE;
    }
    if(!g_string_equal(a -> color, b -> color)){
        return FALSE;
    }
    if(a -> size != b -> size){
        return FALSE;
    }
    if(a -> style.a != b -> style.a){
        return FALSE;
    }
    if(a -> style.b != b -> style.b){
        return FALSE;
    }
    if(a -> style.c != b -> style.c){
        return FALSE;
    }
    return TRUE;
}
//
// QQMsgContent
//
QQMsgContent *qq_msgcontent_new(gint type, ...)
{
    QQMsgContent *cnt = g_slice_new0(QQMsgContent);
    if(cnt == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__);
        return NULL;
    }
    const gchar *name, *color;
    gint size, sa, sb, sc;
    cnt -> type = type;
    va_list ap;
    va_start(ap, type);
    switch(type)
    {
    case QQ_MSG_CONTENT_FACE_T:             //face
        cnt -> value.face = va_arg(ap, gint);
        break;
    case QQ_MSG_CONTENT_STRING_T:           //string
        cnt -> value.str = g_string_new(va_arg(ap, const gchar *));
        break;
    case QQ_MSG_CONTENT_FONT_T:             //font
        name = va_arg(ap, const gchar *);
        size = va_arg(ap, gint);
        color = va_arg(ap, const gchar *);
        sa= va_arg(ap, gint);
        sb= va_arg(ap, gint);
        sc = va_arg(ap, gint);
        cnt -> value.font = qq_msgfont_new(name, size, color, sa, sb, sc);
        break;
    default:
        g_warning("Unknown QQMsgContent type: %d! (%s, %d)"
                            , type, __FILE__, __LINE__);
        va_end(ap);
        g_slice_free(QQMsgContent, cnt);
        return NULL;
    }
    va_end(ap);
    return cnt;
}
void qq_msgcontent_free(QQMsgContent *cnt)
{
    if(cnt == NULL){
        return;
    }

    switch(cnt -> type)
    {
    case QQ_MSG_CONTENT_FACE_T:         //face
        // nothing to do...
        break;
    case QQ_MSG_CONTENT_STRING_T:       //string
        g_string_free(cnt -> value.str, TRUE);
        break;
    case QQ_MSG_CONTENT_FONT_T:         //font
        qq_msgfont_free(cnt -> value.font);
        break;
    default:
        g_warning("Unknown QQMsgContent type: %d! (%s, %d)"
                            , cnt -> type, __FILE__, __LINE__);
        break;
    }

    g_slice_free(QQMsgContent, cnt);
}

GString* qq_msgcontent_tostring(QQMsgContent *cnt)
{
    if(cnt == NULL){
        return g_string_new("");
    }

    GString * str = g_string_new("");
    gchar buf[500];
    gint i;
    switch(cnt -> type)
    {
    case QQ_MSG_CONTENT_FACE_T:             //face. [\"face\",110]
        g_snprintf(buf, 500, "[\\\"face\\\", %d]", cnt -> value.face);
        g_string_append(str, buf);
        break;
    case QQ_MSG_CONTENT_STRING_T:           //string, \"test\"
        g_string_append(str, "\\\"");
        for(i = 0; i < cnt -> value.str -> len; ++i){
            switch(cnt -> value.str -> str[i])
            {
            case '\\':
                g_string_append(str, "\\\\\\\\");
                break;
            case '"':\
                g_string_append(str, "\\\\\\\"");
                break;
            case '\n':
                g_string_append(str, "\\\\n");
                break;
            case '\r':
                g_string_append(str, "\\\\r");
                break;
            case '\t':
                g_string_append(str, "\\\\t");
                break;
            default:
                g_string_append_c(str, cnt -> value.str -> str[i]);
                break;
            }
        }
        g_string_append(str, "\\\"");
        break;
    case QQ_MSG_CONTENT_FONT_T:
        g_snprintf(buf, 500, "[\\\"font\\\", {\\\"name\\\": \\\"%s\\\", "
                        "\\\"size\\\": %d, "
                        "\\\"style\\\": [%d,%d,%d], "
                        "\\\"color\\\": \\\"%s\\\"}]"
                        , cnt -> value.font -> name -> str
                        , cnt -> value.font -> size
                        , cnt -> value.font -> style.a
                        , cnt -> value.font -> style.b
                        , cnt -> value.font -> style.c
                        , cnt -> value.font -> color -> str);
        g_string_append(str, buf);
        break;
    default:
        g_snprintf(buf, 500, "%s", "");
        g_string_append(str, buf);
        break;
    }
    return str;
}

//
// QQSendMsg
//
QQSendMsg* qq_sendmsg_new(QQInfo *info, QQMsgType type, const gchar *to_uin)
{
    QQSendMsg *msg = g_slice_new0(QQSendMsg);

    if(msg == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__);
        return NULL;
    }
    
    msg -> contents = g_ptr_array_new();
    if(msg -> contents == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__);
        g_slice_free(QQSendMsg, msg);
        return NULL;
    }
    gchar buf[20];
    g_snprintf(buf, 20, "%ld", ++(info -> msg_id));
#define NEW_STR(x, y) msg -> x = g_string_new(y)
    NEW_STR(to_uin, to_uin);
    NEW_STR(face, info -> me -> face -> str);
    NEW_STR(msg_id, buf);
    NEW_STR(clientid, info -> clientid -> str);
    NEW_STR(psessionid, info -> psessionid -> str);
#undef NEW_STR
    return msg;
}

void qq_sendmsg_add_content(QQSendMsg *msg, QQMsgContent *content)
{
    if(msg == NULL || msg -> contents == NULL){
        return;
    }

    if(content == NULL){
        return;
    }

    g_ptr_array_add(msg -> contents, content);
}
void qq_sendmsg_free(QQSendMsg *msg)
{
    if(msg == NULL){
        return;
    }
#define FREE_STR(x) g_string_free(msg -> x, TRUE)
    FREE_STR(to_uin);
    FREE_STR(face);
    FREE_STR(msg_id);
    FREE_STR(clientid);
    FREE_STR(psessionid);
#undef FREE_STR
    guint i;
    for(i = 0; i < msg -> contents -> len; ++i){
        qq_msgcontent_free(
                (QQMsgContent*)g_ptr_array_index(msg -> contents, i));
    }
    g_slice_free(QQSendMsg, msg);
}

//
//"content":
//        "[\"test.\"
//          ,[\"font\",{\"name\":\"\\\"微软雅黑\\\"\",\"size\":\"11\"
//                    ,\"style\":[0,0,0],\"color\":\"000000\"}
//           ]
//          ,[\"face\", 20]
//        ]" 
//
GString * qq_sendmsg_contents_tostring(QQSendMsg *msg)
{
    if(msg == NULL || msg -> contents == NULL){
        return g_string_new("");
    }

    GString *str = g_string_new("\"content\":\"[");
    GString *tmp;
    guint i;
    for(i = 0; i < msg -> contents -> len; ++i){
        tmp = qq_msgcontent_tostring(
                    (QQMsgContent*)g_ptr_array_index(msg -> contents, i));
        g_string_append(str, tmp -> str);
        g_string_free(tmp, TRUE);
        if(i != msg -> contents -> len - 1){
            g_string_append(str, ",");
        }
    }
        
    g_string_append(str, "]\"");
    g_debug("contents_tostring: %s (%s, %d)", str -> str, __FILE__, __LINE__);
    return str;
}
//
// QQRecvMsg
//
QQRecvMsg* qq_recvmsg_new(QQInfo *info, QQMsgType type)
{
    QQRecvMsg *msg = g_slice_new0(QQRecvMsg);
    if(msg == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__); 
        return NULL;
    }

    msg -> msg_type = type;

#define NEW_STR(x, y) msg -> x = g_string_new(y)
    NEW_STR(msg_id, "");
    NEW_STR(msg_id2, "");
    NEW_STR(from_uin, "");
    NEW_STR(to_uin, "");
    NEW_STR(reply_ip, "");
    NEW_STR(group_code, "");
    NEW_STR(send_uin, "");
    NEW_STR(time, "");
    NEW_STR(raw_content, "");
    NEW_STR(uin, "");
    NEW_STR(status, "offline");
    NEW_STR(client_type, "");
#undef NEW_STR

    msg -> contents = g_ptr_array_new();
    return msg;
}
void qq_recvmsg_add_content(QQRecvMsg *msg, QQMsgContent *content)
{
    if(msg == NULL || msg -> contents == NULL){
        return;
    }

    if(content == NULL){
        return;
    }

    g_ptr_array_add(msg -> contents, content);
}
void qq_recvmsg_set(QQRecvMsg *msg, const gchar *name, const gchar *value)
{
    if(msg == NULL || name == NULL){
        return;
    }
#define SET_STR(x)  g_string_truncate(msg -> x, 0);\
                    g_string_append(msg -> x, value);
    if(g_strcmp0(name, "msg_id") == 0){
        SET_STR(msg_id);
    }else if(g_strcmp0(name, "msg_id2") == 0){
        SET_STR(msg_id2);
    }else if(g_strcmp0(name, "from_uin") == 0){
        SET_STR(from_uin);
    }else if(g_strcmp0(name, "to_uin") == 0){
        SET_STR(to_uin);
    }else if(g_strcmp0(name, "reply_ip") == 0){
        SET_STR(reply_ip);
    }else if(g_strcmp0(name, "group_code") == 0){
        SET_STR(group_code);
    }else if(g_strcmp0(name, "send_uin") == 0){
        SET_STR(send_uin);
    }else if(g_strcmp0(name, "time") == 0){
        SET_STR(time);
    }else if(g_strcmp0(name, "raw_content") == 0){
        SET_STR(raw_content);
    }else if(g_strcmp0(name, "uin") == 0){
        SET_STR(uin);
    }else if(g_strcmp0(name, "status") == 0){
        SET_STR(status);
    }else if(g_strcmp0(name, "client_type") == 0){
        SET_STR(client_type);
    }else{
        g_warning("Unknown QQRecvMsg memeber: %s=%s (%s, %d)", name, value
                                    , __FILE__, __LINE__);
    }
#undef SET_STR
}

void qq_recvmsg_free(QQRecvMsg *msg)
{
    if(msg == NULL){
        return;
    }

#define FREE_STR(x) g_string_free(msg -> x, TRUE)
    FREE_STR(msg_id);
    FREE_STR(msg_id2);
    FREE_STR(from_uin);
    FREE_STR(to_uin);
    FREE_STR(reply_ip);
    FREE_STR(group_code);
    FREE_STR(send_uin);
    FREE_STR(time);
    FREE_STR(raw_content);
#undef FREE_STR

    guint i;
    for(i = 0; i < msg -> contents -> len; ++i){
        qq_msgcontent_free(
                (QQMsgContent*)g_ptr_array_index(msg -> contents, i));
    }
    g_slice_free(QQRecvMsg, msg);
}

//
// QQBuddy
//
QQBuddy* qq_buddy_new()
{
    QQBuddy *bd = g_slice_new0(QQBuddy);
    bd -> vip_info = -1;
    bd -> client_type = -1;
#define NEW_STR(x) bd -> x = g_string_new("")
    NEW_STR(uin);
    NEW_STR(qqnumber);
    NEW_STR(status);
    NEW_STR(nick);
    NEW_STR(markname);
    NEW_STR(country);
    NEW_STR(city);
    NEW_STR(province);
    NEW_STR(gender);
    NEW_STR(face);
    NEW_STR(flag);
    NEW_STR(phone);
    NEW_STR(mobile);
    NEW_STR(email);
    NEW_STR(college);
    NEW_STR(occupation);
    NEW_STR(personal);
    NEW_STR(homepage);
    NEW_STR(lnick);
#undef NEW_STR
    
    qq_buddy_set(bd, "status", "offline");
    return bd;
}
void qq_buddy_free(QQBuddy *bd)
{
    if(bd == NULL){
        return;
    }

#define FREE_STR(x) g_string_free(bd -> x, TRUE)
    FREE_STR(uin);
    FREE_STR(qqnumber);
    FREE_STR(status);
    FREE_STR(nick);
    FREE_STR(markname);
    FREE_STR(country);
    FREE_STR(city);
    FREE_STR(province);
    FREE_STR(gender);
    FREE_STR(face);
    FREE_STR(flag);
    FREE_STR(phone);
    FREE_STR(mobile);
    FREE_STR(email);
    FREE_STR(college);
    FREE_STR(occupation);
    FREE_STR(personal);
    FREE_STR(homepage);
    FREE_STR(lnick);
#undef FREE_STR

    g_slice_free(QQBuddy, bd);
}

void qq_buddy_set(QQBuddy *bdy, const gchar *name, ...)
{
    if(bdy == NULL || name == NULL){
        return;
    }

    va_list ap;
    va_start(ap, name);
    const gchar *strvalue;
#define SET_STR(x)  g_string_truncate(bdy -> x, 0);\
                    strvalue = va_arg(ap, const gchar *);\
                    g_string_append(bdy -> x, strvalue);
    if(g_strcmp0(name, "uin") == 0){
        SET_STR(uin);
    }else if(g_strcmp0(name, "status") == 0){
        SET_STR(status);
    }else if(g_strcmp0(name, "qqnumber") == 0){
        SET_STR(qqnumber);
    }else if(g_strcmp0(name, "nick") == 0){
        SET_STR(nick);
    }else if(g_strcmp0(name, "markname") == 0){
        SET_STR(markname);
    }else if(g_strcmp0(name, "country") == 0){
        SET_STR(country);
    }else if(g_strcmp0(name, "province") == 0){
        SET_STR(province);
    }else if(g_strcmp0(name, "city") == 0){
        SET_STR(city);
    }else if(g_strcmp0(name, "gender") == 0){
        SET_STR(gender);
    }else if(g_strcmp0(name, "face") == 0){
        SET_STR(face);
    }else if(g_strcmp0(name, "flag") == 0){
        SET_STR(flag);
    }else if(g_strcmp0(name, "phone") == 0){
        SET_STR(phone);
    }else if(g_strcmp0(name, "mobile") == 0){
        SET_STR(mobile);
    }else if(g_strcmp0(name, "email") == 0){
        SET_STR(email);
    }else if(g_strcmp0(name, "occupation") == 0){
        SET_STR(occupation);
    }else if(g_strcmp0(name, "college") == 0){
        SET_STR(college);
    }else if(g_strcmp0(name, "homepage") == 0){
        SET_STR(homepage);
    }else if(g_strcmp0(name, "personal") == 0){
        SET_STR(personal);
    }else if(g_strcmp0(name, "lnick") == 0){
        SET_STR(lnick);
    }else if(g_strcmp0(name, "vip_info") == 0){
        bdy -> vip_info = va_arg(ap, gint);
    }else if(g_strcmp0(name, "blood") == 0){
        bdy -> blood = va_arg(ap, gint);
    }else if(g_strcmp0(name, "shengxiao") == 0){
        bdy -> shengxiao = va_arg(ap, gint);
    }else if(g_strcmp0(name, "constel") == 0){
        bdy -> constel = va_arg(ap, gint);
    }else if(g_strcmp0(name, "allow") == 0){
        bdy -> allow = va_arg(ap, gint);
    }else if(g_strcmp0(name, "client_type") == 0){
        bdy -> client_type = va_arg(ap, gint);
    }else if(g_strcmp0(name, "birthday") == 0){
        bdy -> birthday.year = va_arg(ap, gint);
        bdy -> birthday.month = va_arg(ap, gint);
        bdy -> birthday.day = va_arg(ap, gint);
    }else if(g_strcmp0(name, "cate_index") == 0){
        bdy -> cate_index = va_arg(ap, gint); 
    }else{
        g_warning("Unknown member %s in QQBuddy. (%s, %d)", name
                            , __FILE__, __LINE__);
    }
#undef SET_STR
    va_end(ap);
}
void qq_buddy_copy(QQBuddy *from, QQBuddy *to)
{
    if(from == NULL || to == NULL){
        return;
    }
#define COPY_STR(x) qq_buddy_set(to, #x, from -> x -> str)
    COPY_STR(uin);
    COPY_STR(qqnumber);
    COPY_STR(status);
    COPY_STR(nick);
    COPY_STR(markname);
    COPY_STR(country);
    COPY_STR(city);
    COPY_STR(province);
    COPY_STR(gender);
    COPY_STR(face);
    COPY_STR(flag);
    COPY_STR(phone);
    COPY_STR(mobile);
    COPY_STR(email);
    COPY_STR(college);
    COPY_STR(occupation);
    COPY_STR(personal);
    COPY_STR(homepage);
    COPY_STR(lnick);
#undef COPY_STR
#define COPY_INT(x) to -> x = from -> x
    COPY_INT(vip_info);
    COPY_INT(blood);
    COPY_INT(shengxiao);
    COPY_INT(constel);
    COPY_INT(allow);
    COPY_INT(client_type);
    COPY_INT(birthday.year);
    COPY_INT(birthday.month);
    COPY_INT(birthday.day);
    COPY_INT(cate_index);
#undef COPY_INT 
}
//
// QQGMeber
//
QQGMember* qq_gmember_new()
{
    QQGMember *m = g_slice_new0(QQGMember);
    
    if(m == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__);
        return NULL;
    }

    m -> uin = g_string_new("");
    m -> qqnumber = g_string_new("");
    m -> nick = g_string_new("");
    m -> flag = g_string_new("");
    m -> status = g_string_new("0");
    m -> card = g_string_new("");
    m -> client_type = g_string_new("0");
    return m;
}
void qq_gmember_free(QQGMember *m)
{
    if(m == NULL){
        return;
    }

    g_string_free(m -> uin, TRUE);
    g_string_free(m -> qqnumber, TRUE);
    g_string_free(m -> nick, TRUE);
    g_string_free(m -> flag, TRUE);
    g_string_free(m -> status, TRUE);
    g_string_free(m -> card, TRUE);
    g_string_free(m -> client_type, TRUE);

    g_slice_free(QQGMember, m);
}

void qq_gmember_set(QQGMember *m, const gchar *name, ...)
{
    if(m == NULL || name == NULL){
        return ;
    }

    va_list ap;
    va_start(ap, name);
    const gchar *value = NULL;
#define SET_VALUE_STR(x)    \
    value = va_arg(ap, const gchar *);\
    g_string_truncate(m -> x, 0);\
    g_string_append(m -> x, value)
    
    if(g_strcmp0("uin", name) == 0){
        SET_VALUE_STR(uin);
    }else if(g_strcmp0("qqnumber", name) == 0){
        SET_VALUE_STR(qqnumber);
    }else if(g_strcmp0("nick", name) == 0){
        SET_VALUE_STR(nick);
    }else if(g_strcmp0("flag", name) == 0){
        SET_VALUE_STR(flag);
    }else if(g_strcmp0("status", name) == 0){
        SET_VALUE_STR(status);
    }else if(g_strcmp0("card", name) == 0){
        SET_VALUE_STR(card);
    }else if(g_strcmp0("client_type", name) == 0){
        SET_VALUE_STR(client_type);
    }else{
        g_warning("Unknown member %s in QQGMember. (%s, %d)", name
                        , __FILE__, __LINE__);
    }
#undef SET_VALUE_STR
    va_end(ap);
    return ;
}

//
// QQGroup
//
QQGroup* qq_group_new()
{
    QQGroup *grp = g_slice_new0(QQGroup);
    grp -> members = g_ptr_array_new();

#define NEW_STR(x) grp -> x = g_string_new("")
    NEW_STR(name);
    NEW_STR(gid);
    NEW_STR(gnumber);
    NEW_STR(createtime);
    NEW_STR(code);
    NEW_STR(flag);
    NEW_STR(owner);
    NEW_STR(mark);
    NEW_STR(mask);
    NEW_STR(memo);
    NEW_STR(fingermemo);
    NEW_STR(option);
    NEW_STR(gclass);
    NEW_STR(face);
    NEW_STR(level);
#undef NEW_STR
    return grp;
}
void qq_group_free(QQGroup *grp)
{
    if(grp == NULL){
        return;
    }

#define FREE_STR(x)    g_string_free(grp -> x, TRUE)
    FREE_STR(name);
    FREE_STR(gid);
    FREE_STR(gnumber);
    FREE_STR(createtime);
    FREE_STR(code);
    FREE_STR(flag);
    FREE_STR(owner);
    FREE_STR(mark);
    FREE_STR(mask);
    FREE_STR(memo);
    FREE_STR(fingermemo);
    FREE_STR(option);
    FREE_STR(face);
    FREE_STR(gclass);
    FREE_STR(level);
#undef FREE_STR

    gint i;
    for(i = 0; i < grp -> members -> len; ++i){
        qq_gmember_free((QQGMember*)(grp -> members -> pdata[i]));
    }
    g_ptr_array_free(grp -> members, TRUE);

    g_slice_free(QQGroup, grp);
}

void qq_group_set(QQGroup *grp, const gchar *name, ...)
{
    if(grp == NULL || name == NULL){
        return;
    }
    
    va_list ap;
    va_start(ap, name);
    const gchar *strvalue;
#define SET_STR(x)  g_string_truncate(grp -> x, 0);\
                    strvalue = va_arg(ap, const gchar *);\
                    g_string_append(grp -> x, strvalue);\
                    g_debug("Group set %s=%s (%s, %d)", #x, grp -> x -> str\
                                        , __FILE__, __LINE__)
    if(g_strcmp0("name", name) == 0){
        SET_STR(name);
    }else if(g_strcmp0("gid", name) == 0){
        SET_STR(gid);
    }else if(g_strcmp0("gnumber", name) == 0){
        SET_STR(gnumber);
    }else if(g_strcmp0("code", name) == 0){
        SET_STR(code);
    }else if(g_strcmp0("flag", name) == 0){
        SET_STR(flag);
    }else if(g_strcmp0("owner", name) == 0){
        SET_STR(owner);
    }else if(g_strcmp0("mark", name) == 0){
        SET_STR(mark);
    }else if(g_strcmp0("mask", name) == 0){
        SET_STR(mask);
    }else if(g_strcmp0("memo", name) == 0){
        SET_STR(memo);
    }else if(g_strcmp0("createtime", name) == 0){
        SET_STR(createtime);
    }else if(g_strcmp0("fingermemo", name) == 0){
        SET_STR(fingermemo);
    }else if(g_strcmp0("option", name) == 0){
        SET_STR(option);
    }else if(g_strcmp0("gclass", name) == 0){
        SET_STR(gclass);
    }else if(g_strcmp0("level", name) == 0){
        SET_STR(level);
    }else if(g_strcmp0("face", name) == 0){
        SET_STR(face);
    }
#undef SET_STR
    va_end(ap);
    return;
}

gint qq_group_add(QQGroup *grp, QQGMember *m)
{
    if(grp == NULL || m == NULL){
        return -1;
    }
    g_ptr_array_add(grp -> members, m);
    return 0;
}

QQGMember* qq_group_lookup_member_by_uin(QQGroup *grp, const gchar *uin)
{
    if(grp == NULL || uin == NULL){
        return NULL;
    }
    gint i;
    QQGMember *gmem;
    for(i = 0; i < grp -> members -> len; ++i){
        gmem = g_ptr_array_index(grp -> members, i);
        if(gmem){
            continue;
        }
        if(g_strcmp0(uin, gmem -> uin -> str) == 0){
            return gmem;
        }
    }
    return NULL;
}

//
// QQCategory
//
QQCategory* qq_category_new()
{
    QQCategory *c = g_slice_new0(QQCategory);
    c -> members = g_ptr_array_new();
    c -> name = g_string_new(NULL);
    return c;
}
void qq_category_free(QQCategory *cate)
{
    if(cate == NULL){
        return;
    }
    g_ptr_array_free(cate -> members, TRUE);
    g_string_free(cate -> name, TRUE);
    g_slice_free(QQCategory, cate);
}

void qq_category_set(QQCategory *cate, const gchar *key, ...)
{
    if(cate == NULL || key == NULL){
        return;
    }

    va_list ap;
    va_start(ap, key);

    if(g_strcmp0(key, "name") == 0){
        g_string_truncate(cate -> name, 0);
        const gchar *v = va_arg(ap, const gchar *);
        g_string_append(cate -> name, v);
    }else if(g_strcmp0(key, "index") == 0){
        cate -> index = va_arg(ap, gint);
    }else{
        g_warning("No member named %s in QQCategory. (%s, %d)", key
                                , __FILE__, __LINE__);
    }
    va_end(ap);
}
//
// QQRecentCon
//
QQRecentCon* qq_recentcon_new()
{
    QQRecentCon *rc = g_slice_new0(QQRecentCon);
    rc -> type = -1;
    return rc;
}
void qq_recentcon_free(QQRecentCon *rc)
{
    if(rc == NULL){
        return;
    }
    g_string_free(rc -> uin, TRUE);
    g_slice_free(QQRecentCon, rc);
}

//
// QQFaceImg
//
QQFaceImg* qq_faceimg_new()
{
    QQFaceImg *img = g_slice_new0(QQFaceImg);
    img -> data = g_string_new(NULL);
    img -> type = g_string_new(NULL);
    img -> uin = g_string_new(NULL);
    img -> num = g_string_new(NULL);
    return img;
}
void qq_faceimg_free(QQFaceImg *img)
{
    if(img == NULL){
        return;
    }

    g_string_free(img -> data, TRUE);
    g_string_free(img -> type, TRUE);
    g_string_free(img -> uin, TRUE);
    g_string_free(img -> num, TRUE);

    g_slice_free(QQFaceImg, img);
}

void qq_faceimg_set(QQFaceImg *img, const gchar *key, GString *val)
{
    if(img == NULL || key == NULL || val == NULL){
        return;
    }

    if(g_strcmp0("data", key) == 0){
        g_string_truncate(img -> data, 0);
        g_string_append_len(img -> data, val -> str, val -> len);
    }else if(g_strcmp0("type", key) == 0){
        g_string_truncate(img -> type, 0);
        g_string_append_len(img -> type, val -> str, val -> len);
    }else if(g_strcmp0("uin", key) == 0){
        g_string_truncate(img -> uin, 0);
        g_string_append_len(img -> uin, val -> str, val -> len);
    }else if(g_strcmp0("num", key) == 0){
        g_string_truncate(img -> num, 0);
        g_string_append_len(img -> num, val -> str, val -> len);
    }else{
        g_warning("QQFaceImg has no member named %s. (%s, %d)"
                                , key, __FILE__, __LINE__);
    }
}

void qq_faceimg_copy(QQFaceImg *from, QQFaceImg *to)
{
    if(from == NULL || to == NULL){
        return;
    }

    qq_faceimg_set(to, "data", from -> data);
    qq_faceimg_set(to, "type", from -> type);
    qq_faceimg_set(to, "uin", from -> uin);
}
