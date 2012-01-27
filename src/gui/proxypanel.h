/**
 * @file   proxypanel.h
 * @author Xiang Wang <xiang_wang@trendmicro.com.cn>
 * @date   Wed Jan 25 10:56:16 2012
 *
 * @brief
 *
 *
 */

#ifndef __PROXY_SETTING_H__
#define __PROXY_SETTING_H__

#include <gtk/gtk.h>

void set_proxy_btn_cb(GtkButton *btn, gpointer data);

void on_click_detect_cb(GtkButton *btn, gpointer data);

void on_click_save_cb(GtkButton * btn , gpointer data);

#endif
