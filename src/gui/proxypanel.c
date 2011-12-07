#include <proxypanel.h>
#include <gqqconfig.h>
#include <string.h>
#include <qqproxy.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const char proxy_xml[] =
    "<?xml version=\"1.0\"?>\n"
    "<interface>\n"
    "<!-- interface-requires gtk+ 2.24 -->\n"
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
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
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
    "<property name=\"label\" translatable=\"yes\">HTTP Proxy:</property>\n"
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
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
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
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
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
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
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
    "<placeholder/>\n"
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
    "</object>\n"
    "</interface>\n";


const char * success_xml=
    "<?xml version=\"1.0\"?>\n"
    "<interface>\n"
    "<!-- interface-requires gtk+ 2.24 -->\n"
    "<!-- interface-naming-policy project-wide -->\n"
    "<object class=\"GtkAboutDialog\" id=\"successdialog\">\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"border_width\">5</property>\n"
    "<property name=\"type_hint\">dialog</property>\n"
    "<property name=\"program_name\">Success</property>\n"
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
    "<signal handler=\"gtk_widget_destroy\" name=\"button_press_event\" swapped=\"no\"/>\n"
    "</object>\n"
    "<packing>\n"
    "<property name=\"expand\">False</property>\n"
    "<property name=\"fill\">True</property>\n"
    "<property name=\"pack_type\">end</property>\n"
    "<property name=\"position\">0</property>\n"
    "</packing>\n"
    "</child>\n"
    "<child>\n"
    "<object class=\"GtkEntry\" id=\"entry1\">\n"
    "<property name=\"visible\">True</property>\n"
    "<property name=\"sensitive\">False</property>\n"
    "<property name=\"can_focus\">False</property>\n"
    "<property name=\"editable\">False</property>\n"
    "<property name=\"invisible_char\">&#x25CF;</property>\n"
    "<property name=\"text\" translatable=\"yes\">Connection success</property>\n"
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


const gchar *ip_text;
const gchar *port_text;
const gchar * user_text;
const gchar * password_text;

void set_proxy_btn_cb(GtkButton *btn, gpointer data)
{
    /*  
    */
    GtkBuilder      *builder;
    GtkWidget       *window;
    GError *err = NULL;
    builder = gtk_builder_new ();
    gint ret = gtk_builder_add_from_string (builder, proxy_xml, strlen(proxy_xml), &err);
    if (err || ! ret)
    {
        g_debug("builder load fail...(%s,%d)",  __FILE__ , __LINE__);
        g_warning ("Couldn't load builder file: %s", err->message);
        g_error_free(err); 
    }
    window = GTK_WIDGET (gtk_builder_get_object (builder, "ProxyPanel"));
    //gtk_builder_connect_signals (builder, NULL);
    if (! window)
    {
        g_debug("builder generate fail...(%s,%d)",  __FILE__ , __LINE__);
        return;
    }
    
    /* GtkWidget * ip_entry = GTK_WIDGET (gtk_builder_get_object(builder,"proxy_ip_entry")); */
    /* GtkWidget * port_entry = GTK_WIDGET( gtk_builder_get_object(builder,"proxy_port_entry")); */
    /* GtkWidget * user_entry = GTK_WIDGET (gtk_builder_get_object(builder, "user_entry")); */
    /* GtkWidget * password_entry = GTK_WIDGET(gtk_builder_get_object(builder, "password_entry")); */

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
            
            GtkBuilder      *builder;
            GtkWidget       *window;
            GError *err = NULL;
            builder = gtk_builder_new ();
            gint ret = gtk_builder_add_from_string (builder, success_xml, strlen(success_xml), &err);
            if (err || ! ret)
            {
                g_debug("builder load fail...(%s,%d)",  __FILE__ , __LINE__);
                g_error_free(err); 
            }
            window = GTK_WIDGET (gtk_builder_get_object (builder, "successdialog"));
            if (! window)
            {
                g_debug("can not get widget...(%s,%d)",__FILE__, __LINE__);
            }
            g_object_unref (G_OBJECT (builder));            
            gtk_widget_show_all(window);     
        }
        else{
            GtkBuilder      *builder;
            GtkWidget       *window;
            GError *err = NULL;
            builder = gtk_builder_new ();
            gint ret = gtk_builder_add_from_string (builder,fail_xml, strlen(fail_xml), &err);
            if (err || ! ret)
            {
                g_debug("builder load fail...(%s,%d)",  __FILE__ , __LINE__);
                g_error_free(err); 
            }
            window = GTK_WIDGET (gtk_builder_get_object (builder, "connection_fail"));    
            g_object_unref (G_OBJECT (builder));            
            gtk_widget_show_all(window);     
            gtk_entry_set_text( (GtkEntry*)ip_entry, "");
            gtk_entry_set_text( (GtkEntry *)port_entry, "");
        }
    }
}

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

    set_relay(METHOD_HTTP, ip_text, atoi(port_text),user_text,password_text);
    gtk_widget_destroy (window);
}
