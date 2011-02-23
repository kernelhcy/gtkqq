#include <qqtypes.h>
#include <qq.h>
#include <glib.h>

/*
 * send message to friends or groups.
 */

static gboolean do_sendmsg_to_friend(gpointer data)
{
	return FALSE;
}

static gboolean do_sendmsg_to_group(gpointer data)
{
	return FALSE;
}

void qq_sendmsg_to_friend(QQInfo *info, QQBuddy *to
			, const gchar *msg, QQCallBack cb)
{

}
void qq_sendmsg_to_group(QQInfo *info, QQGroup *to
			, const gchar *msg, QQCallBack cb)
{

}
