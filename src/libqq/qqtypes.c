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

    info -> buddies_image_ht = g_hash_table_new_full(g_str_hash, g_str_equal
                            , NULL, g_free);

    info -> buddies_ht = g_hash_table_new(g_str_hash, g_str_equal);
    info -> groups_ht = g_hash_table_new(g_str_hash, g_str_equal);

    info -> lock = g_mutex_new();

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

#define FREE_STR(x) g_string_free(info -> x, TRUE)
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
    g_hash_table_unref(info -> buddies_image_ht);

    g_mutex_free(info -> lock);
    g_slice_free(QQInfo, info);
}

QQBuddy* qq_info_lookup_buddy(QQInfo *info, const gchar *uin)
{
    return (QQBuddy*)g_hash_table_lookup(info -> buddies_ht, uin);
}
QQGroup* qq_info_lookup_group(QQInfo *info, const gchar *gid)
{
    return (QQGroup*)g_hash_table_lookup(info -> buddies_ht, gid);
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
    case 1:         //face
        cnt -> value.face = va_arg(ap, gint);
        break;
    case 2:         //string
        cnt -> value.str = g_string_new(va_arg(ap, const gchar *));
        break;
    case 3:         //font
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
    case 1:     //face
        // nothing to do...
        break;
    case 2:     //string
        g_string_free(cnt -> value.str, TRUE);
        break;
    case 3:     //font
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

    gchar buf[500];
    switch(cnt -> type)
    {
    case 1:         //face. [\"face\",110]
        g_snprintf(buf, 500, "[\\\"face\\\", %d]", cnt -> value.face);
        break;
    case 2:         //string, \"test\"
        g_snprintf(buf, 500, "\\\"%s\\\"", cnt -> value.str -> str);
        break;
    case 3:
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

        break;
    default:
        g_snprintf(buf, 500, "%s", "");
        break;
    }
    return g_string_new(buf);
}

//
// QQSendMsg
//
QQSendMsg* qq_sendmsg_new(QQInfo *info, gint type, const gchar *to_uin)
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
QQRecvMsg* qq_recvmsg_new(QQInfo *info, const gchar *poll_type)
{
    QQRecvMsg *msg = g_slice_new0(QQRecvMsg);
    if(msg == NULL){
        g_warning("OOM...(%s, %d)", __FILE__, __LINE__); 
        return NULL;
    }

#define NEW_STR(x, y) msg -> x = g_string_new(y)
    NEW_STR(poll_type, poll_type);
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
    NEW_STR(status, "");
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
    if(g_strcmp0(name, "poll_type") == 0){
        SET_STR(poll_type);
    }else if(g_strcmp0(name, "msg_id") == 0){
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
    FREE_STR(poll_type);
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
#define NEW_STR(x) bd -> x = g_string_new("")
    NEW_STR(uin);
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
    NEW_STR(faceimgfile);
#undef NEW_STR

    return bd;
}
void qq_buddy_free(QQBuddy *bd)
{
    if(bd == NULL){
        return;
    }

#define FREE_STR(x) g_string_free(bd -> x, TRUE)
    FREE_STR(uin);
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
    FREE_STR(faceimgfile);
#undef FREE_STR

    qq_faceimg_free(bd -> faceimg);

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
    }else if(g_strcmp0(name, "faceimgfile") == 0){
        SET_STR(lnick);
    }
#undef SET_STR

    if(g_strcmp0(name, "faceimg") == 0){
        bdy -> faceimg = va_arg(ap, QQFaceImg *);
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
    }else if(g_strcmp0(name, "category") == 0){
    }else if(g_strcmp0(name, "birthday") == 0){
        bdy -> birthday.year = va_arg(ap, gint);
        bdy -> birthday.month = va_arg(ap, gint);
        bdy -> birthday.day = va_arg(ap, gint);
    }else if(g_strcmp0(name, "cate") == 0){
        bdy -> cate = va_arg(ap, QQCategory*);
    }
    va_end(ap);
}

QQBuddy* qq_buddy_new_from_string(gchar *str)
{
    gssize len = strlen(str);
    if(str == NULL || len <=0){
        return qq_buddy_new();
    }
    QQBuddy *bdy = g_slice_new0(QQBuddy);
    gchar *colon, *nl;
    gchar *name, *value;
#define SET_VALUE_STR(x) \
    name = g_strstr_len(str, len, #x);\
    if(name != NULL){\
        colon = name;\
        while(*colon != '\0' && *colon != ':') ++colon;\
        value = colon + 1;\
        nl = value;\
        while(*nl != '\n' && *nl != '\n') ++nl;\
        *nl = '\0';\
        bdy -> x = g_string_new(value);\
        *nl = '\n';\
    }

    SET_VALUE_STR(uin);
    SET_VALUE_STR(status);
    SET_VALUE_STR(nick);
    SET_VALUE_STR(markname);
    SET_VALUE_STR(country);
    SET_VALUE_STR(province);
    SET_VALUE_STR(city);
    SET_VALUE_STR(gender);
    SET_VALUE_STR(face);
    SET_VALUE_STR(flag);
    SET_VALUE_STR(phone);
    SET_VALUE_STR(mobile);
    SET_VALUE_STR(email);
    SET_VALUE_STR(occupation);
    SET_VALUE_STR(college);
    SET_VALUE_STR(homepage);
    SET_VALUE_STR(personal);
    SET_VALUE_STR(lnick);
#undef SET_VALUE_STR
    
    gint tmpi;
#define SET_VALUE_INT(x) \
    name = g_strstr_len(str, len, #x);\
    if(name != NULL){\
        colon = name;\
        while(*colon != '\0' && *colon != ':') ++colon;\
        value = colon + 1;\
        nl = value;\
        while(*nl != '\n' && *nl != '\n') ++nl;\
        *nl = '\0';\
        tmpi = (gint)strtol(value, NULL, 10);\
        bdy -> x = tmpi;\
        *nl = '\n';\
    }
    
    SET_VALUE_INT(vip_info);
    SET_VALUE_INT(blood);
    SET_VALUE_INT(shengxiao);
    SET_VALUE_INT(constel);
    SET_VALUE_INT(allow);
    SET_VALUE_INT(client_type);
#undef SET_VALUE_INT

    //birthday
    gint y, m, d;
    gchar *ys, *ms, *ds;
    name = g_strstr_len(str, len, "birthday");
    if(name != NULL){
        colon = name;
        while(*colon != '\0' && *colon != ':') ++colon;
        value = colon + 1;
        nl = value;
        //year
        while(*nl > '9' || *nl < '0') ++nl;
        ys = nl; 
        while(*nl <= '9' && *nl >= '0') ++nl;
        *nl = '\0';
        //month
        while(*nl > '9' || *nl < '0') ++nl;
        ms = nl;
        while(*nl <= '9' && *nl >= '0') ++nl;
        *nl = '\0';
        //day
        while(*nl > '9' || *nl < '0') ++nl;
        ds = nl;
        while(*nl <= '9' && *nl >= '0') ++nl;
        *nl = '\0';

        y = (gint)strtol(ys, NULL, 10);
        m = (gint)strtol(ms, NULL, 10);
        d = (gint)strtol(ds, NULL, 10);
        bdy -> birthday.year = y;
        bdy -> birthday.month = m;
        bdy -> birthday.day = d;
    }
    return bdy;
}

GString* qq_buddy_tostring(QQBuddy *bdy)
{
    if(bdy == NULL){
        g_string_new("");
    }
    GString *str = g_string_new("");
#define APP_STR(x) \
    g_string_append(str, #x":");\
    g_string_append(str, bdy -> x -> str);\
    g_string_append(str, "\n")\

    APP_STR(uin);
    APP_STR(status);
    APP_STR(nick);
    APP_STR(markname);
    APP_STR(country);
    APP_STR(province);
    APP_STR(city);
    APP_STR(gender);
    APP_STR(face);
    APP_STR(flag);
    APP_STR(phone);
    APP_STR(mobile);
    APP_STR(email);
    APP_STR(occupation);
    APP_STR(college);
    APP_STR(homepage);
    APP_STR(personal);
    APP_STR(lnick);
#undef APP_STR

#define APP_STR_INT(x) \
    g_string_append(str, #x":");\
    g_string_append_printf(str, "%d", bdy -> x);\
    g_string_append(str, "\n")

    APP_STR_INT(vip_info);
    APP_STR_INT(blood);
    APP_STR_INT(shengxiao);
    APP_STR_INT(constel);
    APP_STR_INT(allow);
    APP_STR_INT(client_type);
#undef APP_STR_INT

    g_string_append(str, "birthday:");
    g_string_append_printf(str, "%d ", bdy -> birthday.year);
    g_string_append_printf(str, "%d ", bdy -> birthday.month);
    g_string_append_printf(str, "%d", bdy -> birthday.day);
    g_string_append(str, "\n\r");
    return str;
}

QQGMember* qq_gmember_new()
{
    QQGMember *m = g_slice_new0(QQGMember);

    return m;
}
void qq_gmember_free(QQGMember *m)
{
    if(m == NULL){
        return;
    }

    g_string_free(m -> uin, TRUE);
    g_string_free(m -> nick, TRUE);
    g_string_free(m -> flag, TRUE);
    g_string_free(m -> status, TRUE);
    g_string_free(m -> card, TRUE);

    qq_faceimg_free(m -> faceimg);
    g_slice_free(QQGMember, m);
}
QQGroup* qq_group_new()
{
    QQGroup *grp = g_slice_new0(QQGroup);
    grp -> members = g_ptr_array_new();
    return grp;
}
void qq_group_free(QQGroup *grp)
{
    if(grp == NULL){
        return;
    }

    g_string_free(grp -> name, TRUE);
    g_string_free(grp -> gid, TRUE);
    g_string_free(grp -> code, TRUE);
    g_string_free(grp -> flag, TRUE);
    g_string_free(grp -> owner, TRUE);
    g_string_free(grp -> mark, TRUE);
    g_string_free(grp -> mask, TRUE);
    g_string_free(grp -> memo, TRUE);
    g_string_free(grp -> fingermemo, TRUE);

    gint i;
    for(i = 0; i < grp -> members -> len; ++i){
        qq_gmember_free((QQGMember*)(grp -> members -> pdata[i]));
    }
    g_ptr_array_free(grp -> members, TRUE);

    g_slice_free(QQGroup, grp);
}

QQCategory* qq_category_new()
{
    QQCategory *c = g_slice_new0(QQCategory);
    c -> members = g_ptr_array_new();
    return c;
}
void qq_category_free(QQCategory *cty)
{
    if(cty == NULL){
        return;
    }
    g_ptr_array_free(cty -> members, TRUE);
    g_slice_free(QQCategory, cty);
}

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

QQFaceImg* qq_faceimg_new()
{
    QQFaceImg *img = g_slice_new0(QQFaceImg);
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

    g_slice_free(QQFaceImg, img);
}
