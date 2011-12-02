/**
 * @file   sound.c
 * @author mathslinux <riegamaths@gmail.com>
 * @date   Thu Dec  1 14:31:03 2011
 * 
 * @brief  Gtkqq sound implementation
 * 
 * 
 */

#ifndef SOUND_H
#define SOUND_H

#ifdef USE_GSTREAMER

/** 
 * Play a audio of wav format when qq client get message. e.g get new chat message.
 * 
 * @param filename The wavfile name.
 */
void qq_play_wavfile(const gchar *filename);
#endif // USE_GSTREAMER
#endif
