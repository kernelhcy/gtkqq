/**
 * @file   proxypanel.c
 * @author Xiang Wang <xiang_wang@trendmicro.com.cn>
 * @date   Wed Jan 25 10:54:07 2012
 * 
 * @brief  handle proxy affairs
 * 
 * 
 */

#include <proxypanel.h>
#include <gqqconfig.h>
#include <string.h>
#include <qqproxy.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char proxy_xml[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<interface>\n"
  "<requires lib=\"gtk+\" version=\"2.24\"/>\n"
  "<!-- interface-naming-policy project-wide -->\n"
  "<object class=\"GtkDialog\" id=\"ProxyPanel\">\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"border_width\">5</property>\n"
    "<property name=\"type_hint\">normal</property>\n"
    "<child internal-child=\"vbox\">\n"
      "<object class=\"GtkVBox\" id=\"dialog-vbox\">\n"
        "<property name=\"width_request\">0</property>\n"
        "<property name=\"height_request\">0</property>\n"
        "<property name=\"visible\">True</property>\n"
        "<property name=\"app_paintable\">True</property>\n"
        "<property name=\"can_focus\">False</property>\n"
        "<property name=\"spacing\">2</property>\n"
        "<child internal-child=\"action_area\">\n"
          "<object class=\"GtkHButtonBox\" id=\"dialog-action_area1\">\n"
            "<property name=\"visible\">True</property>\n"
            "<property name=\"can_focus\">False</property>\n"
            "<property name=\"layout_style\">end</property>\n"
            "<child>\n"
              "<object class=\"GtkButton\" id=\"detect_connection_btn\">\n"
                "<property name=\"label\" translatable=\"yes\">Detect</property>\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"receives_default\">True</property>\n"
                "<property name=\"use_action_appearance\">False</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"expand\">False</property>\n"
                "<property name=\"fill\">False</property>\n"
                "<property name=\"position\">0</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkButton\" id=\"save_btn\">\n"
                "<property name=\"label\" translatable=\"yes\">Save</property>\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"receives_default\">True</property>\n"
                "<property name=\"use_action_appearance\">False</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"expand\">False</property>\n"
                "<property name=\"fill\">False</property>\n"
                "<property name=\"position\">1</property>\n"
              "</packing>\n"
            "</child>\n"
          "</object>\n"
          "<packing>\n"
            "<property name=\"expand\">False</property>\n"
            "<property name=\"fill\">True</property>\n"
            "<property name=\"pack_type\">end</property>\n"
            "<property name=\"position\">0</property>\n"
          "</packing>\n"
        "</child>\n"
        "<child>\n"
          "<object class=\"GtkTable\" id=\"table1\">\n"
            "<property name=\"visible\">True</property>\n"
            "<property name=\"can_focus\">False</property>\n"
            "<property name=\"n_rows\">3</property>\n"
            "<property name=\"n_columns\">4</property>\n"
            "<property name=\"row_spacing\">9</property>\n"
            "<child>\n"
              "<object class=\"GtkEntry\" id=\"proxy_ip_entry\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"invisible_char\">●</property>\n"
                "<property name=\"primary_icon_activatable\">False</property>\n"
                "<property name=\"secondary_icon_activatable\">False</property>\n"
                "<property name=\"primary_icon_sensitive\">True</property>\n"
                "<property name=\"secondary_icon_sensitive\">True</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">1</property>\n"
                "<property name=\"right_attach\">2</property>\n"
                "<property name=\"top_attach\">1</property>\n"
                "<property name=\"bottom_attach\">2</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkLabel\" id=\"proxy_ip_label\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">False</property>\n"
                "<property name=\"label\" translatable=\"yes\">IP:</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"top_attach\">1</property>\n"
                "<property name=\"bottom_attach\">2</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkLabel\" id=\"proxy_setting_label\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">False</property>\n"
                "<property name=\"label\" translatable=\"yes\">Proxy Type:</property>\n"
              "</object>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkLabel\" id=\"proxy_port_label\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">False</property>\n"
                "<property name=\"label\" translatable=\"yes\">Port:</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">2</property>\n"
                "<property name=\"right_attach\">3</property>\n"
                "<property name=\"top_attach\">1</property>\n"
                "<property name=\"bottom_attach\">2</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkLabel\" id=\"user_label\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">False</property>\n"
                "<property name=\"label\" translatable=\"yes\">User:</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"top_attach\">2</property>\n"
                "<property name=\"bottom_attach\">3</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkLabel\" id=\"password_label\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">False</property>\n"
                "<property name=\"label\" translatable=\"yes\">Password:</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">2</property>\n"
                "<property name=\"right_attach\">3</property>\n"
                "<property name=\"top_attach\">2</property>\n"
                "<property name=\"bottom_attach\">3</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkEntry\" id=\"user_entry\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"invisible_char\">●</property>\n"
                "<property name=\"invisible_char_set\">True</property>\n"
                "<property name=\"primary_icon_activatable\">False</property>\n"
                "<property name=\"secondary_icon_activatable\">False</property>\n"
                "<property name=\"primary_icon_sensitive\">True</property>\n"
                "<property name=\"secondary_icon_sensitive\">True</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">1</property>\n"
                "<property name=\"right_attach\">2</property>\n"
                "<property name=\"top_attach\">2</property>\n"
                "<property name=\"bottom_attach\">3</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkEntry\" id=\"password_entry\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"invisible_char\">●</property>\n"
                "<property name=\"invisible_char_set\">True</property>\n"
                "<property name=\"primary_icon_activatable\">False</property>\n"
                "<property name=\"secondary_icon_activatable\">False</property>\n"
                "<property name=\"primary_icon_sensitive\">True</property>\n"
                "<property name=\"secondary_icon_sensitive\">True</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">3</property>\n"
                "<property name=\"right_attach\">4</property>\n"
                "<property name=\"top_attach\">2</property>\n"
                "<property name=\"bottom_attach\">3</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkEntry\" id=\"proxy_port_entry\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"invisible_char\">●</property>\n"
                "<property name=\"invisible_char_set\">True</property>\n"
                "<property name=\"primary_icon_activatable\">False</property>\n"
                "<property name=\"secondary_icon_activatable\">False</property>\n"
                "<property name=\"primary_icon_sensitive\">True</property>\n"
                "<property name=\"secondary_icon_sensitive\">True</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">3</property>\n"
                "<property name=\"right_attach\">4</property>\n"
                "<property name=\"top_attach\">1</property>\n"
                "<property name=\"bottom_attach\">2</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<object class=\"GtkComboBox\" id=\"proxy_type_combobox\">\n"
                "<property name=\"visible\">True</property>\n"
                "<property name=\"can_focus\">True</property>\n"
                "<property name=\"has_frame\">True</property>\n"
                "<property name=\"button_sensitivity\">on</property>\n"
                "<property name=\"entry_text_column\">3</property>\n"
              "</object>\n"
              "<packing>\n"
                "<property name=\"left_attach\">2</property>\n"
                "<property name=\"right_attach\">3</property>\n"
                "<property name=\"x_options\">GTK_SHRINK | GTK_FILL</property>\n"
              "</packing>\n"
            "</child>\n"
            "<child>\n"
              "<placeholder/>\n"
            "</child>\n"
            "<child>\n"
              "<placeholder/>\n"
            "</child>\n"
          "</object>\n"
          "<packing>\n"
            "<property name=\"expand\">True</property>\n"
            "<property name=\"fill\">True</property>\n"
            "<property name=\"position\">1</property>\n"
          "</packing>\n"
        "</child>\n"
        "<child>\n"
          "<placeholder/>\n"
        "</child>\n"
      "</object>\n"
    "</child>\n"
    "<action-widgets>\n"
      "<action-widget response=\"0\">detect_connection_btn</action-widget>\n"
      "<action-widget response=\"0\">save_btn</action-widget>\n"
    "</action-widgets>\n"
  "</object>\n"
    "</interface>\n";




const char * success_xml=
    "<?xml version=\"1.0\"?>"
    "<interface>"
    "<!-- interface-requires gtk+ 2.24 -->"
    "<!-- interface-naming-policy project-wide -->"
    "<object class=\"GtkMessageDialog\" id=\"successdialog\">"
    "<property name=\"can_focus\">False</property>"
    "<property name=\"border_width\">5</property>"
    "<property name=\"type_hint\">dialog</property>"
    "<property name=\"skip_taskbar_hint\">True</property>"
    "<child internal-child=\"vbox\">"
    "<object class=\"GtkVBox\" id=\"dialog-vbox1\">"
    "<property name=\"visible\">True</property>"
    "<property name=\"can_focus\">False</property>"
    "<property name=\"spacing\">2</property>"
    "<child internal-child=\"action_area\">"
    "<object class=\"GtkHButtonBox\" id=\"dialog-action_area1\">"
    "<property name=\"visible\">True</property>"
    "<property name=\"can_focus\">False</property>"
    "<property name=\"layout_style\">end</property>"
    "<child>"
    "<placeholder/>"
    "</child>"
    "<child>"
    "<object class=\"GtkButton\" id=\"close_btn\">"
    "<property name=\"label\" translatable=\"yes\">Close</property>"
    "<property name=\"visible\">True</property>"
    "<property name=\"can_focus\">True</property>"
    "<property name=\"receives_default\">True</property>"
    "<property name=\"use_action_appearance\">False</property>"
    "</object>"
    "<packing>"
    "<property name=\"expand\">False</property>"
    "<property name=\"fill\">False</property>"
    "<property name=\"position\">1</property>"
    "</packing>"
    "</child>"
    "</object>"
    "<packing>"
    "<property name=\"expand\">False</property>"
    "<property name=\"fill\">True</property>"
    "<property name=\"pack_type\">end</property>"
    "<property name=\"position\">0</property>"
    "</packing>"
    "</child>"
    "<child>"
    "<object class=\"GtkEntry\" id=\"entry1\">"
    "<property name=\"visible\">True</property>"
    "<property name=\"can_focus\">False</property>"
    "<property name=\"invisible_char\">&#x25CF;</property>"
    "<property name=\"text\" translatable=\"yes\">Connection Success!!!</property>"
    "<property name=\"primary_icon_activatable\">False</property>"
    "<property name=\"secondary_icon_activatable\">False</property>"
    "<property name=\"primary_icon_sensitive\">True</property>"
    "<property name=\"secondary_icon_sensitive\">True</property>"
    "</object>"
    "<packing>"
    "<property name=\"expand\">True</property>"
    "<property name=\"fill\">True</property>"
    "<property name=\"position\">2</property>"
    "</packing>"
    "</child>"
    "</object>"
    "</child>"
    "</object>"
    "</interface>";

const char * fail_xml=
    "<?xml version=\"1.0\"?>\n"
    "<interface>\n"
    "<!-- interface-requires gtk+ 2.24 -->\n"
    "<!-- interface-naming-policy project-wide -->\n"
    "<object class=\"GtkMessageDialog\" id=\"connection_fail\">\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"border_width\">5</property>\n"
    "<property name=\"type_hint\">dialog</property>\n"
    "<property name=\"skip_taskbar_hint\">True</property>\n"
    "<child internal-child=\"vbox\">\n"
    "<object class=\"GtkVBox\" id=\"dialog-vbox1\">\n"
    "<property name=\"visible\">True</property>\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"spacing\">2</property>\n"
    "<child internal-child=\"action_area\">\n"
    "<object class=\"GtkHButtonBox\" id=\"dialog-action_area1\">\n"
    "<property name=\"visible\">True</property>\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"layout_style\">end</property>\n"
    "<child>\n"
    "<placeholder/>\n"
    "</child>\n"
    "<child>\n"
    "<object class=\"GtkButton\" id=\"close_btn\">\n"
    "<property name=\"label\" translatable=\"yes\">Close</property>\n"
    "<property name=\"visible\">True</property>\n"
    "<property name=\"can_focus\">True</property>\n"
    "<property name=\"receives_default\">True</property>\n"
    "<property name=\"use_action_appearance\">False</property>\n"
    "<signal handler=\"gtk_widget_destroy\" name=\"clicked\" swapped=\"no\"/>\n"
    "</object>\n"
    "<packing>\n"
    "<property name=\"expand\">False</property>\n"
    "<property name=\"fill\">False</property>\n"
    "<property name=\"position\">1</property>\n"
    "</packing>\n"
    "</child>\n"
    "</object>\n"
    "<packing>\n"
    "<property name=\"expand\">False</property>\n"
    "<property name=\"fill\">True</property>\n"
    "<property name=\"pack_type\">end</property>\n"
    "<property name=\"position\">0</property>\n"
    "</packing>\n"
    "</child>\n"
    "<child>\n"
    "<object class=\"GtkEntry\" id=\"error_entry\">\n"
    "<property name=\"visible\">True</property>\n"
    "<property name=\"can_focus\">True</property>\n"
    "<property name=\"editable\">False</property>\n"
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
    "<property name=\"text\" translatable=\"yes\">Connection Fail!! </property>\n"
    "<property name=\"primary_icon_activatable\">False</property>\n"
    "<property name=\"secondary_icon_activatable\">False</property>\n"
    "<property name=\"primary_icon_sensitive\">True</property>\n"
    "<property name=\"secondary_icon_sensitive\">True</property>\n"
    "</object>\n"
    "<packing>\n"
    "<property name=\"expand\">True</property>\n"
    "<property name=\"fill\">True</property>\n"
    "<property name=\"position\">2</property>\n"
    "</packing>\n"
    "</child>\n"
    "</object>\n"
    "</child>\n"
    "</object>\n"
    "</interface>\n";

extern GQQConfig *cfg;

static GtkWidget * ip_entry;
static GtkWidget * port_entry;
static GtkWidget * user_entry;
static GtkWidget * password_entry;
static GtkWidget * proxy_type_combobox;

const gchar *ip_text;
const gchar *port_text;
const gchar * user_text;
const gchar * password_text;

void set_proxy_btn_cb(GtkButton *btn, gpointer data)
{
    /*
*/
    GtkBuilder *builder;
    GtkWidget *window;
    GError *err = NULL;
    builder = gtk_builder_new ();
    gint ret = gtk_builder_add_from_string (builder, proxy_xml, strlen(proxy_xml), &err);
    if (err || ! ret)
    {
        g_debug("builder load fail...(%s,%d)", __FILE__ , __LINE__);
        g_warning ("Couldn't load builder file: %s", err->message);
        g_error_free(err);
    }
    window = GTK_WIDGET (gtk_builder_get_object (builder, "ProxyPanel"));
    //gtk_builder_connect_signals (builder, NULL);
    if (! window)
    {
        g_debug("builder generate fail...(%s,%d)", __FILE__ , __LINE__);
        return;
    }
    
    /* GtkWidget * ip_entry = GTK_WIDGET (gtk_builder_get_object(builder,"proxy_ip_entry")); */
    /* GtkWidget * port_entry = GTK_WIDGET( gtk_builder_get_object(builder,"proxy_port_entry")); */
    /* GtkWidget * user_entry = GTK_WIDGET (gtk_builder_get_object(builder, "user_entry")); */
    /* GtkWidget * password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "password_entry")); */

    proxy_type_combobox =(GtkWidget *) GTK_WIDGET(gtk_builder_get_object(builder,"proxy_type_combobox"));

    if (! proxy_type_combobox )
    {
        g_debug("Fail to get combobox...(%s,%d)",__FILE__, __LINE__);
    }
    GtkTreeIter iter;
    GtkListStore *store = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "HTTP Proxy", -1);
    gtk_combo_box_set_model (GTK_COMBO_BOX (proxy_type_combobox), GTK_TREE_MODEL(store));

    GtkCellRenderer * cell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( proxy_type_combobox ), cell, TRUE );
    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( proxy_type_combobox), cell, "text", 0, NULL );
    gtk_combo_box_append_text(GTK_COMBO_BOX (proxy_type_combobox),"Socks5 Proxy");

    int index = 0;
    gqq_config_get_int(cfg, "proxy_type",&index);
    g_debug("the proxy type index is %d... (%s,%d)", index, __FILE__, __LINE__);
    gtk_combo_box_set_active (GTK_COMBO_BOX(proxy_type_combobox), index);
    
    GtkWidget * detect_btn = GTK_WIDGET(gtk_builder_get_object(builder, "detect_connection_btn"));
    GtkWidget * save_btn = GTK_WIDGET(gtk_builder_get_object(builder, "save_btn"));
    
    ip_entry = GTK_WIDGET (gtk_builder_get_object(builder,"proxy_ip_entry"));
    port_entry = GTK_WIDGET( gtk_builder_get_object(builder,"proxy_port_entry"));
    user_entry = GTK_WIDGET (gtk_builder_get_object(builder, "user_entry"));
    password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "password_entry"));

    
    if( ! ip_entry || ! port_entry || !user_entry|| !password_entry)
    {
        g_debug("Get Widget Object Error...(%s,%d)",__FILE__, __LINE__);
    }


    if(gqq_config_get_str(cfg, "proxy_ip", &ip_text )== -1 ||
       gqq_config_get_str(cfg, "proxy_port",&port_text ) == -1 )
    {
        g_debug( "Did not have proxy settings...(%s,%d)", __FILE__, __LINE__);
    }
    else
    {
        if (ip_text && port_text)
        {
            gtk_entry_set_text( (GtkEntry*)ip_entry, ip_text);
            gtk_entry_set_text( (GtkEntry *)port_entry, port_text);
            g_debug("using proxy %s:%s...(%s,%d)", ip_text, port_text, __FILE__, __LINE__);
        }
    }

    if(gqq_config_get_str(cfg, "proxy_user", &user_text )== -1 ||
       gqq_config_get_str(cfg, "proxy_password",&password_text ) == -1 )
    {
        g_debug( "Did not have user settings...(%s,%d)", __FILE__, __LINE__);
    }
    else
    {
        if (user_text && password_text)
        {
            gtk_entry_set_text( (GtkEntry*)user_entry, user_text);
            gtk_entry_set_text( (GtkEntry *)password_entry, password_text);
            
            g_debug("using proxy %s:%s...(%s,%d)", ip_text, port_text, __FILE__, __LINE__);
        }
    }

    g_signal_connect(G_OBJECT(detect_btn), "clicked", G_CALLBACK(on_click_detect_cb),NULL);
    g_signal_connect(G_OBJECT(save_btn), "clicked", G_CALLBACK(on_click_save_cb),(gpointer) window);
    g_object_unref (G_OBJECT (builder));
     
    gtk_widget_show_all(window);
}

static void on_close_cb(GtkButton * btn, gpointer data)
{
    g_debug("close widget ... (%s,%d)", __FILE__ ,__LINE__);
    GtkWidget * window = GTK_WIDGET(data);
    gtk_widget_destroy (window);
}


/** 
 * Call back function for detecting proxy settings
 * 
 * @param btn is the button in widget
 * @param data 
 */
void on_click_detect_cb(GtkButton *btn, gpointer data)
{
    g_debug("detect connection...(%s,%d)",__FILE__, __LINE__);
    ip_text = gtk_entry_get_text((GtkEntry *)ip_entry);
    port_text = gtk_entry_get_text((GtkEntry *)port_entry);
    if ( ! ip_text || ! port_text)
    {
        g_debug("invalid ip and port for proxy....(%s,%d)",__FILE__, __LINE__);
        return ;
    }
    else
    {
        g_debug("Detect proxy server %s with port %s ...(%s,%d)", ip_text, port_text, __FILE__ , __LINE__);
        int sock = -1;
        if ( (sock = open_connection(ip_text, atoi(port_text))) != -1)
        {
            close(sock);
            
            GtkBuilder *builder;
            GtkWidget *window;
            GError *err = NULL;
            builder = gtk_builder_new ();
            gint ret = gtk_builder_add_from_string (builder, success_xml, strlen(success_xml), &err);
            if (err || ! ret)
            {
                g_debug("builder load fail...(%s,%d)", __FILE__ , __LINE__);
                g_error_free(err);
            }
            window = GTK_WIDGET (gtk_builder_get_object (builder, "successdialog"));
            if (! window)
            {
                g_debug("can not get widget...(%s,%d)",__FILE__, __LINE__);
            }
            GtkWidget * close_btn = GTK_WIDGET(gtk_builder_get_object(builder, "close_btn"));
            g_signal_connect(G_OBJECT(close_btn), "clicked", G_CALLBACK(on_close_cb),window);
           
            g_object_unref (G_OBJECT (builder));
            gtk_widget_show_all(window);
        }
        else{
            GtkBuilder *builder;
            GtkWidget *window;
            GError *err = NULL;
            builder = gtk_builder_new ();
            gint ret = gtk_builder_add_from_string (builder,fail_xml, strlen(fail_xml), &err);
            if (err || ! ret)
            {
                g_debug("builder load fail...(%s,%d)", __FILE__ , __LINE__);
                g_error_free(err);
            }
            window = GTK_WIDGET (gtk_builder_get_object (builder, "connection_fail"));

            GtkWidget * close_btn = GTK_WIDGET(gtk_builder_get_object(builder, "close_btn"));
            g_signal_connect(G_OBJECT(close_btn), "clicked", G_CALLBACK(on_close_cb),window);
            
            g_object_unref (G_OBJECT (builder));
            gtk_widget_show_all(window);
            gtk_entry_set_text( (GtkEntry*)ip_entry, "");
            gtk_entry_set_text( (GtkEntry *)port_entry, "");
        }
    }
}

/** 
 * Call back function for saving proxy settings
 * 
 * @param btn the button item which will act the save behavior.
 * @param data the widget pointer
 */

void on_click_save_cb(GtkButton *btn, gpointer data)
{
    g_debug("save proxy settings...(%s,%d)", __FILE__, __LINE__);
    GtkWidget * window = GTK_WIDGET(data);
    
    ip_text = gtk_entry_get_text((GtkEntry *)ip_entry);
    port_text = gtk_entry_get_text((GtkEntry *)port_entry);
    user_text = gtk_entry_get_text((GtkEntry *)user_entry);
    password_text = gtk_entry_get_text((GtkEntry *)password_entry);
    gqq_config_set_str(cfg, "proxy_ip", ip_text );
    gqq_config_set_str(cfg, "proxy_port",port_text );
    if (user_text && password_text)
    {
        gqq_config_set_str(cfg, "proxy_user", user_text );
        gqq_config_set_str(cfg, "proxy_password", password_text );
        g_debug("using proxy %s:%s...(%s,%d)", ip_text, port_text, __FILE__, __LINE__);
    }
    int index = gtk_combo_box_get_active (GTK_COMBO_BOX(proxy_type_combobox));
    gqq_config_set_int(cfg,"proxy_type",index);
    g_debug("Current combobox index is %d...(%s,%d)",index, __FILE__, __LINE__);
    switch (index)
    {
        case 0:
            set_relay(METHOD_HTTP, ip_text, atoi(port_text),user_text,password_text);
            break;
        case 1:
            set_relay(METHOD_SOCKS, ip_text, atoi(port_text),user_text,password_text);
            break;
        default:
            break;
    }
    gtk_widget_destroy (window);
}


