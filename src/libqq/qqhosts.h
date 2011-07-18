#ifndef __GTKQQ_QQ_HOSTS_H
#define __GTKQQ_QQ_HOSTS_H

/*
 * The hosts and paths used by webqq 
 */

#define WEBQQHOST 	    "web2.qq.com"

#define LOGINPAGEHOST	"ui.ptlogin2.qq.com"
#define LOGINPAGEPATH	"/cgi-bin/login"
#define LOGINPROXY	    "web2.qq.com/loginproxy.html"
#define LOGIN_S_URL	    "http%3A%2F%2Fweb.qq.com%2Floginproxy.html%3F"\
                        "login2qq%3D1%26webqq_type%3D10"

#define VERPATH		    "/cgi-bin/ver"

#define LOGINHOST	    "ptlogin2.qq.com"
#define VCCHECKPATH	    "/check"
#define LOGINPATH	    "/login"

#define APPID		    "1003903"

#define IMAGEHOST	    "captcha.qq.com"
#define IMAGEPATH	    "/getimage"

#define PSIDHOST	    "d.web2.qq.com"
#define PSIDPATH	    "/channel/login2"

#define LOGOUTPATH	    "/channel/logout2"
#define LOGOUTHOST	    PSIDHOST

#define REFERER		    "http://"LOGOUTHOST"/proxy.html?v=2010102002"

#define SWQQHOST	    "s.web2.qq.com"
#define GETMYINFO	    "/api/get_friend_info2"
#define LNICKPATH	    "/api/get_single_long_nick2"
#define FRIENDSPATH	    "/api/get_user_friends2"
#define GNAMEPATH	    "/api/get_group_name_list_mask2"

#define ONLINEHOST  	PSIDHOST
#define ONLINEPATH	    "/channel/get_online_buddies2"

#define RECENTHOST	    SWQQHOST
#define RECENTPATH	    "/api/get_recent_contact2"

#define MSGHOST		    PSIDHOST
#define MSGFRIPATH	    "/channel/send_msg2"
#define MSGGRPPATH	    "/channel/send_group_msg2"

#define POLLHOST	    PSIDHOST
#define POLLPATH	    "/channel/poll2"

#define FIMGHOST	    "face2.qun.qq.com"
#define FIMGPATH	    "/cgi/svr/face/getface"
#endif
