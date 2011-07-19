#include <qqtypes.h>

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

    /*
     * Just set to 1000.
     * I don't know how to calculate the msg_id.
     * But this works.
     */
    info -> msg_id = 1000;
    return info;
}

void qq_info_free(QQInfo *info)
{
    if(info == NULL){
        return;
    }
    
    g_main_loop_unref(info -> mainloop);
    g_main_context_unref(info -> mainctx);

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
    g_snprintf(buf, 20, "%d", info -> msg_id ++);
#define NEW_STR(x, y) msg -> x = g_string_new(y)
    NEW_STR(to_uin, to_uin);
    NEW_STR(face, info -> me -> face -> str);
    NEW_STR(msg_id, buf);
    NEW_STR(clientid, info -> clientid -> str);
    NEW_STR(psessionid, info -> psessionid -> str);
#undef NEW_STR
    return msg;
}

void qq_sendmsg_set_font(QQSendMsg *msg, const gchar *name, gint size, const gchar *color
                                , gint sa, gint sb, gint sc)
{
       if(msg == NULL){
            return;
       }

       QQMsgFont *font = qq_msgfont_new(name, size, color, sa, sb, sc);
       if(font == NULL){
           return;
       }

       msg -> font = font;
}
void qq_sendmsg_add_context(QQSendMsg *msg, QQMsgContent *content)
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
    qq_msgfont_free(msg -> font);
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
        g_string_append(str, ",");
    }
    g_string_append(str, "\\\"\\\", ");
        
    //add font
    //font, ["font",{"size":"11","color":"000000"
    //      ,"style":[0,0,0],"name":"\u5FAE\u8F6F\u96C5\u9ED1"}]
    gchar buf[500];
    g_snprintf(buf, 500, "[\\\"font\\\", {\\\"name\\\": \\\"%s\\\", "
                        "\\\"size\\\": \\\"%d\\\", "
                        "\\\"style\\\": [%d,%d,%d], "
                        "\\\"color\\\": \\\"%s\\\"}]"
                        , msg -> font -> name -> str
                        , msg -> font -> size
                        , msg -> font -> style.a
                        , msg -> font -> style.b
                        , msg -> font -> style.c
                        , msg -> font -> color -> str);

    g_string_append(str, "]\"");
    g_debug("contents_tostring: %s (%s, %d)", str -> str, __FILE__, __LINE__);
    return str;
}

//
// QQBuddy
//
QQBuddy* qq_buddy_new()
{
    QQBuddy *bd = g_slice_new0(QQBuddy);
    bd -> vip_info = -1;
    return bd;
}
void qq_buddy_free(QQBuddy *bd)
{
    if(bd == NULL){
        return;
    }    
    g_string_free(bd -> uin, TRUE);
    g_string_free(bd -> status, TRUE);
    g_string_free(bd -> nick, TRUE);
    g_string_free(bd -> markname, TRUE);
    g_string_free(bd -> country, TRUE);
    g_string_free(bd -> city, TRUE);
    g_string_free(bd -> province, TRUE);
    g_string_free(bd -> gender, TRUE);
    g_string_free(bd -> face, TRUE);
    g_string_free(bd -> flag, TRUE);
    g_string_free(bd -> phone, TRUE);
    g_string_free(bd -> mobile, TRUE);
    g_string_free(bd -> email, TRUE);
    g_string_free(bd -> college, TRUE);
    g_string_free(bd -> occupation, TRUE);
    g_string_free(bd -> personal, TRUE);
    g_string_free(bd -> homepage, TRUE);
    g_string_free(bd -> lnick, TRUE);
    g_string_free(bd -> faceimgfile, TRUE);

    qq_faceimg_free(bd -> faceimg);

    g_slice_free(QQBuddy, bd);
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
