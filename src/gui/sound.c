/**
 * @file   sound.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Thu Dec  1 14:31:03 2011
 * 
 * @brief  Gtkqq sound implementation
 * 
 * 
 */

#include <config.h>

#ifdef USE_GSTREAMER

#include <gst/gst.h>
#include <glib.h>
#include <unistd.h>

#define MAX_SOUND_LEN 2			/**< Max audio length.*/

#if 0
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
	GstElement *play = (GstElement *)data;
	gboolean res = gst_pipeline_get_auto_flush_bus((GstPipeline *)play);
	GError *err = NULL;

	switch (GST_MESSAGE_TYPE (msg)) {
	case GST_MESSAGE_ERROR:
		gst_message_parse_error(msg, &err, NULL);
		g_warning("Play sound error!! (%s, %d)", __FILE__, __LINE__);
		g_error_free(err);
		
	case GST_MESSAGE_EOS:
		gst_element_set_state(play, GST_STATE_NULL);
		gst_object_unref(GST_OBJECT(play));
		break;
		
	case GST_MESSAGE_WARNING:
		gst_message_parse_warning(msg, &err, NULL);
		g_warning("Play sound error!! (%s, %d)", __FILE__, __LINE__);
		g_error_free(err);
		break;
		
	default:
		break;
	}
	
	return TRUE;
}
#endif

/** 
 * Play a audio of wav format when qq client get message. e.g get new chat message.
 * 
 * @param filename The wavfile name.
 */
void qq_play_wavfile(const gchar *filename)
{
	char *uri;
	GstElement *sink = NULL, *play = NULL;
#if 0
	GstBus *bus = NULL;
#endif 

	if (!filename) {
		g_warning("Null point access!! (%s, %d)", __FILE__, __LINE__);
		goto error;
	}
	
	if (!g_file_test(filename, G_FILE_TEST_EXISTS)) {
		g_warning("Sound file is not exist!! (%s, %d)", __FILE__, __LINE__);
		goto error;
	}

	/* Create elements for wav file */
	sink = gst_element_factory_make("alsasink", "sink");
	play = gst_element_factory_make("playbin2", "play");
	uri = g_strdup_printf("file://%s", filename);

	/* Connect elements. */
	g_object_set(G_OBJECT(play), "uri", uri, "audio-sink", sink, NULL);

	if (!sink || !play || !uri) {
		g_warning("One element could not be created!! (%s, %d)", __FILE__, __LINE__);
		goto error;
    }

	if (!sink || !play || !uri) {
		g_warning("One element could not be created!! (%s, %d)", __FILE__, __LINE__);
		goto error;
    }

#if 0
    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, bus_call, play);
    gst_object_unref(bus);
#endif 
	
	/* Start play. The code is some ugly now. */
    gst_element_set_state(play,GST_STATE_PLAYING);
	sleep(MAX_SOUND_LEN);
	gst_element_set_state(play,GST_STATE_NULL);
	
error:
	if (play) {
		g_free(uri);
		gst_object_unref(GST_OBJECT(play));
	}
}

#endif // USE_GSTREAMER
