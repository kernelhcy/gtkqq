#include <log.h>
#include <cprint.h>
#include <unistd.h>
/* 
 * the log handler
 */
static void log_handler(const gchar *log_domain,
			GLogLevelFlags log_level,
			const gchar *message,
			gpointer user_data)
{
	switch(log_level)
	{
	case G_LOG_LEVEL_MESSAGE:
		fcprintf(STDOUT_FILENO, GREEN_F, UNSET_B, HIGHLIGHT
				, "MESSAGE : ");
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, message);
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, "\n");
		break;
	case G_LOG_LEVEL_DEBUG:
#ifdef GTKQQ_DEBUG
		fcprintf(STDOUT_FILENO, BLUE_F, UNSET_B, HIGHLIGHT
				, "DEBUG : ");
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, message);
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, "\n");
#endif
		break;
	case G_LOG_LEVEL_WARNING:
		fcprintf(STDERR_FILENO, PURPLE_F, UNSET_B, HIGHLIGHT
				, "** WARNING ** : ");
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, message);
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, "\n");
		break;
	case G_LOG_LEVEL_ERROR:
	case G_LOG_FLAG_FATAL:
	case G_LOG_FLAG_RECURSION:
		fcprintf(STDOUT_FILENO, RED_F, UNSET_B, HIGHLIGHT
				, "!!ERROR!! : ");
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, message);
		fcprintf(STDOUT_FILENO, UNSET_F, UNSET_B, UNSET_T, "\n");
		break;
	default:
		break;
	}	
}


void log_init()
{
	g_log_set_handler(NULL, G_LOG_LEVEL_MESSAGE, log_handler, NULL);
	g_log_set_handler(NULL, G_LOG_LEVEL_DEBUG, log_handler, NULL);
	g_log_set_handler(NULL, G_LOG_LEVEL_WARNING, log_handler, NULL);
}
