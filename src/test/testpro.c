#include <stdlib.h>
#include <qq.h>
#include <string.h>
#include <log.h>
#include <stdio.h>

extern gint save_img_to_file(const gchar *data, gint len, const gchar *ext, 
                const gchar *path, const gchar *fname);

int main(int argc, char **argv)
{
    const gchar *qquin = "1421032531";
    const gchar *qqpwd = "1234567890";
    gchar vc[20];

    log_init();
    g_message("Log initial done.");
    QQInfo *info = qq_init(NULL);
    qq_check_verifycode(info, qquin ,NULL);
    if(info -> need_vcimage){
        save_img_to_file(info -> vc_image_data -> str, info -> vc_image_size
                        , info -> vc_image_type -> str
                        , "/home/hcy/tmp/gtkqq/"
                        , "vc");    
        printf("Input verify code : ");
        scanf("%s", vc);
        info -> verify_code = g_string_new(vc);
    }

    qq_login(info, qquin, qqpwd, "online", NULL);

    g_message("get my information...");
    qq_get_my_info(info, NULL);
    g_message("get my friends...");
    qq_get_my_friends(info, NULL);
    g_message("get group name list mask...");
    qq_get_group_name_list_mask(info, NULL);
    g_message("get online buddies...");
    qq_get_online_buddies(info, NULL);
    g_message("get recent contact...");
    qq_get_recent_contact(info, NULL);

    QQSendMsg *msg = qq_sendmsg_new(info, 0
                    , (QQBuddy*)(g_ptr_array_index(info -> buddies, 0)) -> uin -> str);
    msg -> info = info;
    msg -> type = 0;
    msg -> bdy = info -> buddies -> pdata[0];
    msg -> content = g_string_new("test from gtkqq.来自gtkqq的测试."
            "我只是在测试程序，各位可以无视我。打扰了。^v^");
    msg -> font.name = g_string_new("微软雅黑");
    msg -> font.color = g_string_new("00000");
    msg -> font.size = 15;
    msg -> font.style.a = 0;
    msg -> font.style.b = 0;
    msg -> font.style.c = 0;
    //msg -> grp = info -> groups -> pdata[0];
    //qq_sendmsg_to_group(info, msg, NULL);

    g_message("start poll ...");
    qq_start_poll(info, NULL);
    qq_sendmsg_to_friend(info, msg, NULL);
    qq_get_face_img(info, "65359140", NULL);
    qq_get_face_img(info, "1421032531", NULL);
    g_message("Will logout...");
    qq_logout(info, NULL);
    return 0;
}
