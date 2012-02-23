#include <stdlib.h>
#include <qq.h>
#include <string.h>
#include <log.h>
#include <stdio.h>
#include <assert.h>

extern gint save_img_to_file(const gchar *data, gint len, const gchar *ext, 
                const gchar *path, const gchar *fname);

int main(int argc, char **argv)
{
    const gchar *qquin = "1421032531";
    const gchar *qqpwd = "1234567890";
    gchar vc[20];

    log_init(TRUE);
    g_message("Log initial done.");
    QQInfo *info = qq_init(NULL);
    qq_check_verifycode(info, qquin ,NULL);
    if(info -> need_vcimage){
        save_img_to_file(info -> vc_image_data -> str, info -> vc_image_size
                        , info -> vc_image_type -> str
                        , "/home/hcy/tmp/gtkqq/"
                        , "vc");    
        printf("Input verify code : ");
        int ret = scanf("%s", vc);
        assert( ret > 0 );
        info -> verify_code = g_string_new(vc);
    }

    qq_login(info, qquin, qqpwd, "online", NULL);
    
    g_debug("poll...");
    qq_start_poll(info, NULL, NULL, NULL);

    g_message("get my information...");
    qq_get_buddy_info(info, info -> me,  NULL);
    g_message("get my friends...");
    qq_get_buddies_and_categories(info, NULL);
    g_message("get group name list mask...");
    qq_get_groups(info, NULL);
    g_message("get group info...");
    qq_get_group_info(info, (QQGroup*)g_ptr_array_index(info -> groups, 0), NULL);
    g_message("get online buddies...");
    qq_get_online_buddies(info, NULL);
    g_message("get recent contact...");
    qq_get_recent_contact(info, NULL);

    gchar buf[500];
    while(1){
        printf("MSG:");
        int ret = scanf("%s", buf);
	assert( ret > 0 );
        if(g_strcmp0("quit", buf) == 0){
            break;
        }
        QQSendMsg *msg = qq_sendmsg_new(info, 0 //buddy message
                    , ((QQBuddy*)(g_ptr_array_index(info -> buddies, 0))) -> uin -> str);
        QQMsgContent *ctent = qq_msgcontent_new(1, 62);
        qq_sendmsg_add_content(msg, ctent);
        ctent = qq_msgcontent_new(1, 51);
        qq_sendmsg_add_content(msg, ctent);
        ctent = qq_msgcontent_new(2, buf);
        qq_sendmsg_add_content(msg, ctent);
        ctent = qq_msgcontent_new(3, "宋体", 20, "0000FF", 1, 0, 1);
        qq_sendmsg_add_content(msg, ctent);
        qq_send_message(info, msg, NULL);
        qq_sendmsg_free(msg);
    }

    g_message("Will logout...");
    qq_logout(info, NULL);
    return 0;
}
