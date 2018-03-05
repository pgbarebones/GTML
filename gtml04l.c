#include <stdlib.h>
#include <gtk/gtk.h>

#define PN "Mozilla Launcher"
#define CN "Gravastar Technologies"
#define AN "Kuliwa Emmanuel SImubali"

GtkDialogFlags dflags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT ;
GFileCopyFlags cflags = G_FILE_COPY_BACKUP | G_FILE_COPY_ALL_METADATA ;
GKeyFileFlags gkfflags= G_KEY_FILE_KEEP_COMMENTS |G_KEY_FILE_KEEP_TRANSLATIONS ;

static GNotification *app_notification = NULL;
static GList *iconsl = NULL;
static GKeyFile *pdata = NULL;

static GtkApplication *app = NULL;

static GtkSettings *settings = NULL;

static GtkApplicationWindow	*window = NULL;
static GtkWidget *header = NULL,
			*notebook = NULL
			;

static void gtml_activate (void);
static void gtml_reload (void);
static void gtml_import_full (void);
static void start_import (GtkFileChooserButton *widget);
static void create_notebooks (gchar *name, gchar *location);
static void focus_current_entry (GtkNotebook *notebook, GtkWidget *page, guint page_num);
static void gtml_auto_select (void);
static GtkWidget *gtk_notebook_get_child_by_name (GtkNotebook *notebook, gchar *name);
//main loop
int main (int argc, char **argv){
	pdata = g_key_file_new ();
	gboolean test = g_key_file_load_from_file (pdata, "./gtml/.data/pdata.gkf",gkfflags,NULL);
	if (!test) {
		g_file_make_directory_with_parents (g_file_new_for_path("./gtml/.data"),NULL,NULL);
		g_key_file_save_to_file (pdata,"./gtml/.data/pdata.gkf", NULL);
		test = g_key_file_load_from_file (pdata, "./gtml/.data/pdata.gkf",gkfflags,NULL);
	}

	for (int i = 1; i <= 5; ++i) {
		guint x = i*16;
		if(x > 64) x=128;
		GdkPixbuf *icon = gdk_pixbuf_new_from_file (g_strdup_printf ("./gtml/.data/icons/logo%d.png",x), NULL);
		iconsl = g_list_prepend (iconsl, icon);
	}
//apply icons to all windows
	gtk_window_set_default_icon_list (iconsl);

	app = gtk_application_new ("org.gt.ml.linux",G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app,"activate",(GCallback)gtml_activate,NULL);
	return g_application_run (app,argc,argv);
}

static void gtml_activate (void) {
	settings = gtk_settings_get_default ();

	g_object_set (settings , "gtk-application-prefer-dark-theme",TRUE,NULL);
	g_object_set (settings , "gtk-dialogs-use-header",TRUE,NULL);
	g_object_set (settings , "gtk-entry-select-on-focus", TRUE, NULL);
	g_object_set (settings , "gtk-enable-animations", TRUE, NULL);
	g_object_set (settings , "gtk-auto-mnemonics", TRUE, NULL);

	app_notification = g_notification_new ("Mozilla Launcher");
	g_notification_set_priority (app_notification,G_NOTIFICATION_PRIORITY_NORMAL);

	window = gtk_application_window_new (app);
	gtk_window_set_title (window,PN);
	gtk_window_set_role (window,"main_window");
	gtk_window_set_decorated (window,TRUE);
	gtk_window_set_default_size (window,800,110);

	header = gtk_header_bar_new ();
	gtk_header_bar_set_title (header, PN);
	gtk_header_bar_set_show_close_button (header,TRUE);
	gtk_header_bar_set_decoration_layout (header,"menu,stick,shade:minimize,maximize,close" );
	gtk_window_set_titlebar (window,header);

	GtkWidget *import_button = gtk_button_new_from_icon_name ("list-add-symbolic",GTK_ICON_SIZE_BUTTON);
	g_signal_connect (import_button,"clicked",(GCallback)gtml_import_full,NULL);

	notebook = gtk_notebook_new ();
	gtk_container_add (window,notebook);
	gtk_widget_set_hexpand (notebook,TRUE);
	gtk_widget_set_vexpand (notebook,TRUE);
	gtk_notebook_popup_enable (notebook);
	gtk_notebook_set_scrollable (notebook,TRUE);
	gtk_widget_show_all(import_button);
	gtk_notebook_set_action_widget (notebook,import_button,GTK_PACK_END);

	gtml_reload ();
	gtml_auto_select ();
	g_signal_connect (notebook,"switch-page",(GCallback)focus_current_entry,NULL);

	gtk_widget_set_name (window, "gtml");
	gtk_widget_set_name (header, "header");

	gtk_widget_show_all (notebook);
	gtk_widget_show_all (window);
}

static void gtml_auto_select (void) {
	gchar *alocation = NULL;
	guint lead = g_key_file_get_integer (pdata,"Opened","Leader",NULL), x = 0;

	gchar **win = g_key_file_get_keys (pdata,"Opened",NULL,NULL);
	if (lead) {
		while (win[x]) {
			guint open = (guint)g_key_file_get_integer(pdata,"Opened",win[x],NULL);
			if (g_strcmp0 (win[x],"Leader")) {
				if (lead < open) {
					alocation = win[x];
					g_key_file_set_integer (pdata, "Opened", "Leader", x);
					g_key_file_save_to_file (pdata,"./gtml/.data/pdata.gkf", NULL);
				} else if (lead == open) {
					alocation = win[x];
				} else {
					;
				}
			}
			x++;
		}
		gtk_notebook_set_current_page (notebook, gtk_notebook_page_num (notebook, gtk_notebook_get_child_by_name (notebook, alocation)));
	}
}

static void update_launch_count () {
	GtkGrid *cgrid = gtk_notebook_get_tab_label(notebook,gtk_notebook_get_nth_page (notebook, gtk_notebook_get_current_page (notebook)));
	GtkLabel *c3 = gtk_grid_get_child_at (cgrid,2,0);
	gdouble d = g_strtod((gchar *) gtk_label_get_text (c3),NULL) + 1;
	gtk_label_set_text (c3,g_strdup_printf("%0.0f",d));
	gtk_widget_set_tooltip_text (c3,(d==0) ? g_strdup_printf("This program has never been run.") : (d==1) ? g_strdup_printf("This program has been run only once.") : g_strdup_printf("This program has been run %0.0f times.",d));
}

static void launch_prog (gchar *location, gint x) {
	gchar *basename = g_file_get_basename(g_file_new_for_path(location));

	GString *iname = g_string_truncate (g_string_new (basename), 5),
			*pl = g_string_new (location),
			*rname = g_string_new (g_filename_display_basename (location));

	pl = g_string_truncate (pl, (pl->len - rname->len));
	if (!g_strcmp0(iname->str, "firef")) {
		g_string_append_printf (pl, "firefox/profile");
		basename = "firefox";
	} else if (!g_strcmp0(iname->str, "thund")) {
		g_string_append_printf (pl, "thunderbird/profile");
		basename = "thunderbird";
	} else {
		pl = g_string_new (g_strdup_printf("%s/profile",location));
	}
	gchar *program_location = g_strdup_printf ("%s/%s",location,basename);
	gchar *profile_location = pl->str;

	g_print ("program_location %s\nprofile_location %s\n",program_location,profile_location);
	if (x == 0) {
		system (g_strdup_printf ("%s &",program_location));
	} else if (x == 1) {
		system (g_strdup_printf ("%s --profile %s &",program_location,profile_location));
	}

	update_launch_count ();
}

static gchar *gtk_notebook_get_visible_child_name (GtkNotebook *notebook) {
	return (gchar *)gtk_widget_get_name (gtk_notebook_get_nth_page(notebook,gtk_notebook_get_current_page(notebook)));
}

static void check_pass (GtkEntry *entry1, GtkEntry *entry2) {
	gchar *text1 = (gchar *)gtk_entry_get_text(entry1);
	gchar *text2 = (gchar *)gtk_entry_get_text(entry2);
	GKeyFile *p = g_key_file_new();
	gchar 	*location = g_strdup_printf("%s",gtk_notebook_get_visible_child_name (notebook)),
		*name = g_filename_display_basename (location),
		*profile_location = NULL,
		*keys;
	GFile *prog = g_file_new_for_path(location);
	gboolean secure = gtk_widget_get_sensitive (GTK_WIDGET(entry2));

	GString 	*iname = g_string_truncate (g_string_new (name), 5),
			*pl = g_string_new (location),
			*rname = g_string_new (name);
	pl = g_string_truncate (pl, (pl->len - rname->len));
	if (!g_strcmp0(iname->str, "firef")) {
		g_string_append_printf (pl, "firefox/profile");
	} else if (!g_strcmp0(iname->str, "thund")) {
		g_string_append_printf (pl, "thunderbird/profile");
	} else {
		pl = g_string_new (g_strdup_printf("%s/profile",location));
	}
	profile_location = pl->str;

	if ( g_key_file_load_from_file(p,g_strdup_printf("%s/pref.gtdb",profile_location),gkfflags,NULL) == FALSE) { //key file doesn't exist and needs to be created
		if (!g_file_query_exists (prog,NULL)) { //folder doesn't exist and is not a valid program (most likely)
			g_notification_set_body (app_notification,g_strdup_printf("No such program: %s\nTry again",name));
			g_application_send_notification (app,"app-notification",app_notification);
		} else {
			if (secure) {
				if (g_strcmp0(text1,text2) == 0) {
					gtk_entry_set_text (entry1,"");
					gtk_entry_set_text (entry2,"");
					g_key_file_set_string (p,"key","first",text1);
					g_key_file_set_string (p,"key","second",text2);
					g_key_file_save_to_file (p,g_strdup_printf("%s/pref.gtdb",profile_location),NULL);
					g_notification_set_body (app_notification,g_strdup_printf("%s is secured.",name));
					g_application_send_notification (app,"app-notification",app_notification);
				} else {
					g_notification_set_body (app_notification,g_strdup_printf("%s is not secured.\nMake sure your passwords match",name));
					g_application_send_notification (app,"app-notification",app_notification);
				}
			} else {
				if ((g_strcmp0(text1,text2) == 0) && (g_strcmp0(text1,"\0") == 0)) {
					gtk_entry_set_text (entry1,"");
					gtk_entry_set_text (entry2,"");
					gtk_window_iconify(window);
					g_notification_set_body (app_notification,g_strdup_printf("%s is being launched without a password.\nPlease wait.",name));
					g_application_send_notification (app,"app-notification",app_notification);

					guint open = g_key_file_get_integer (pdata, "Opened", location, NULL);
					open++;
					guint leader = g_key_file_get_integer (pdata, "Opened", "Leader", NULL);
					g_key_file_set_integer (pdata, "Opened", location, open);
					if  (open > leader) {
						g_key_file_set_integer (pdata, "Opened", "Leader", open);
					}
					g_key_file_save_to_file (pdata,"./gtml/.data/pdata.gkf", NULL);

					launch_prog (location,0);
				} else {
					g_notification_set_body (app_notification,g_strdup_printf("%s is not being launched.\nCheck your password and try again.",name));
					g_application_send_notification (app,"app-notification",app_notification);
				}
			}
		}
	} else {
		if (secure) {
			if (!g_strcmp0(text1,text2)) {
				gtk_entry_set_text (entry1,"");
				gtk_entry_set_text (entry2,"");
				g_key_file_set_string (p,"key","first",text1);
				g_key_file_set_string (p,"key","second",text2);
				g_key_file_save_to_file (p,g_strdup_printf("%s/pref.gtdb",profile_location),NULL);
				g_notification_set_body (app_notification,g_strdup_printf("The password from %s has been changed.",name));
				g_application_send_notification (app,"app-notification",app_notification);
			} else {
				g_notification_set_body (app_notification,g_strdup_printf("The password for %s has not been changed.\nMake sure your passwords match",name));
				g_application_send_notification (app,"app-notification",app_notification);
			}
		} else {
			keys = g_key_file_get_string(p,"key","first",NULL);
			if (!g_strcmp0(text1,keys)) {
				gtk_entry_set_text (entry1,"");
				gtk_window_iconify(window);
				g_notification_set_body (app_notification,g_strdup_printf("%s is being launched.\nProfile Location: %s\nProgram Location: %s\nPlease wait.",name,profile_location,location));
				g_application_send_notification (app,"app-notification",app_notification);

				guint open = g_key_file_get_integer (pdata, "Opened", location, NULL);
				open++;
				guint leader = g_key_file_get_integer (pdata, "Opened", "Leader", NULL);

				if (leader == 0) {
					gboolean gft = g_file_test ("./gtml/.data/pdata.gkf", G_FILE_TEST_IS_REGULAR);
					if (gft) { g_file_make_directory_with_parents (g_file_new_for_path ("./gtml/.data") ,NULL,NULL); }
				}
				g_key_file_set_integer (pdata, "Opened", location, open);
				if  (open > leader) {
					g_key_file_set_integer (pdata, "Opened", "Leader", open);
				}
				g_key_file_save_to_file (pdata,"./gtml/.data/pdata.gkf", NULL);

				launch_prog (location,1);
			} else {
				g_notification_set_body (app_notification,g_strdup_printf("%s is not being launched.\nCheck your password and try again.",name));
				g_application_send_notification (app,"app-notification",app_notification);
			}
		}
	}
}

static void create_notebooks (gchar *name, gchar *location) {
	void zero_tog (GtkToggleButton *butn, GtkToggleButton *butn2) {
		if (gtk_toggle_button_get_active (butn)) {
			gtk_toggle_button_set_active (butn2,FALSE);
		} else {
			gtk_toggle_button_set_active (butn2,TRUE);
		}
	}
	void zero_act (GtkWidget *widget, GtkToggleButton *butn) {
		if (gtk_toggle_button_get_active (butn)) {
			gtk_widget_set_sensitive(widget,TRUE);
			gtk_widget_show(widget);
		} else {
			gtk_widget_set_sensitive(widget,FALSE);
			gtk_widget_hide(widget);
		}
	}
	void one_act (GtkWidget *widget, GtkToggleButton *butn) {
		if (gtk_toggle_button_get_active (butn)) {
			gtk_widget_set_sensitive(widget,FALSE);
			gtk_widget_hide(widget);
		} else {
			gtk_widget_set_sensitive(widget,TRUE);
			gtk_widget_show(widget);
		}
	}

	GtkWidget 	*sgrid = gtk_grid_new (),
			*slgrid = gtk_grid_new (),
			*password[] = {
				gtk_entry_new (),
				gtk_entry_new (),
				NULL
			},
			*slabel = gtk_label_new (name),
			*slcount = NULL,
			*switchs[] = {
				gtk_toggle_button_new_with_mnemonic ("_Secure"),
				gtk_toggle_button_new_with_mnemonic ("_Launch"),
				NULL
			};

	GString *iname = g_string_truncate (g_string_new (name), 5);
	if (!g_strcmp0(iname->str, "firef")) {
		gtk_grid_attach (slgrid,gtk_image_new_from_icon_name ("firefox",GTK_ICON_SIZE_BUTTON),0,0,1,1);
		gtk_grid_attach (sgrid,gtk_image_new_from_icon_name ("firefox",GTK_ICON_SIZE_DIALOG),0,1,2,1);
	} else if (!g_strcmp0(iname->str, "thund")) {
		gtk_grid_attach (slgrid,gtk_image_new_from_icon_name ("thunderbird",GTK_ICON_SIZE_BUTTON),0,0,1,1);
		gtk_grid_attach (sgrid,gtk_image_new_from_icon_name ("thunderbird",GTK_ICON_SIZE_DIALOG),0,1,2,1);
	} else {
		;
	}

	gtk_grid_set_column_homogeneous (sgrid,TRUE);

	gtk_entry_set_placeholder_text (password[0],"Password");
	gtk_entry_set_placeholder_text (password[1],"New Password");

	gtk_toggle_button_set_active (switchs[1],TRUE);

	gtk_widget_set_sensitive(password[1],FALSE);

	gtk_entry_set_visibility (password[0],FALSE);
	gtk_entry_set_visibility (password[1],FALSE);

	g_signal_connect (password[0],"activate",(GCallback)check_pass,password[1]);
	g_signal_connect_swapped (password[1],"activate",(GCallback)check_pass,password[0]);
	g_signal_connect (switchs[0],"toggled",(GCallback)zero_tog,switchs[1]);
	g_signal_connect (switchs[1],"toggled",(GCallback)zero_tog,switchs[0]);
	g_signal_connect_swapped (switchs[0],"toggled",(GCallback)zero_act,password[1]);
	g_signal_connect_swapped (switchs[1],"toggled",(GCallback)one_act,password[1]);


	GFile *applications = g_file_new_for_path ("./gtml");
	guint counted = 0;

	GFileEnumerator *apps = g_file_enumerate_children (applications,"*",G_FILE_QUERY_INFO_NONE,NULL,NULL);
	while (TRUE) {
		GFile *out;
		GFileInfo *info;
		if (!g_file_enumerator_iterate(apps,&info,&out,NULL,NULL)) {
				g_object_unref(apps);
				break;
			} else {
				if (!info) break;
				gchar *tname = g_file_info_get_attribute_as_string (info,G_FILE_ATTRIBUTE_STANDARD_NAME);
				if ((!g_strcmp0(tname,name) && !g_strcmp0(tname,name)))	{
					gchar *path = g_file_get_path (out);
					counted = g_key_file_get_integer (pdata, "Opened", path,NULL);
				}
			}
		if (!info) break;
	}

	slabel = gtk_label_new (g_strdup_printf("%s : ",name));
	slcount = gtk_label_new(g_strdup_printf("%d",counted));

	gtk_widget_set_tooltip_text (slcount,(counted==0) ? g_strdup_printf("This program has never been run.") : (counted==1) ? g_strdup_printf("This program has been run only once.") : g_strdup_printf("This program has been run %d times.",counted));

	gtk_widget_set_tooltip_text (slabel,g_strdup_printf("This program is called %s.",name));

	gtk_grid_attach (slgrid,slabel,1,0,1,1);
	gtk_grid_attach (slgrid,slcount,2,0,1,1);
	gtk_grid_attach (sgrid,switchs[0],1,2,1,1);
	gtk_grid_attach (sgrid,switchs[1],0,2,1,1);
	gtk_grid_attach (sgrid,password[0],0,3,1,1);
	gtk_grid_attach (sgrid,password[1],1,3,1,1);

	gtk_notebook_append_page (notebook,sgrid,slgrid);
	gtk_notebook_set_menu_label_text (notebook,sgrid,name);
	gtk_widget_set_name (sgrid,location);

	gtk_widget_set_can_focus (password[0],TRUE);
	gtk_entry_grab_focus_without_selecting (password[0]);

	gtk_widget_set_name (password[0],g_strdup_printf("%s entry 0",name));

	gtk_widget_show_all (sgrid);
	gtk_widget_show_all (slgrid);
	one_act (password[1],switchs[1]);
}

static GtkWidget *gtk_notebook_get_child_by_name (GtkNotebook *notebook, gchar *name) {
	GtkWidget *child = NULL;
	GList *childs = gtk_container_get_children (notebook);
	for ( GList *childss = childs; childss != NULL; childss = childss->next) {
		if (!g_strcmp0(gtk_widget_get_name(childss->data),name)) child = childss->data;
	}
	return child;
}

static GtkWidget *gtk_notebook_get_visible_child (GtkNotebook *notebook) {
	return gtk_notebook_get_nth_page (notebook, gtk_notebook_get_current_page(notebook));
}

static void focus_current_entry (GtkNotebook *notebook, GtkWidget *page, guint page_num) {
	GtkWidget *child = gtk_grid_get_child_at (page,0,3);
	gtk_widget_grab_focus (child);
}

static void gtml_reload (void) {
	GKeyFile *programs = g_key_file_new ();
	GFile *applications = g_file_new_for_path ("./gtml");
	gboolean load = g_key_file_load_from_file (programs,"./gtml/.data/programs.gkf",gkfflags,NULL);
	guint leader = g_key_file_get_integer (pdata, "Opened", "Leader",NULL);
	guint counted = 0;
	if (load) {
		if (g_key_file_has_group (programs,"Programs")){
			gchar **names = g_key_file_get_keys (programs,"Programs",NULL,NULL);
			int x = 0;
			while (names[x]) {
				gchar *location = g_key_file_get_string (programs,"Programs",names[x],NULL);
				guint open = g_key_file_get_integer (pdata, "Opened", names[x],NULL);
				create_notebooks (names [x],location);
				location = NULL;
				if (open > leader) {
					gtk_notebook_set_current_page(notebook, x);
				}
				counted++;
				x++;
			}
		} else g_print ("No linked programs in data.\n");
	} else g_print ("No linked programs\n");

	GFileEnumerator *apps = g_file_enumerate_children (applications,"*",G_FILE_QUERY_INFO_NONE,NULL,NULL);
	while (TRUE) {
		GFile *out;
		GFileInfo *info;
		if (!g_file_enumerator_iterate(apps,&info,&out,NULL,NULL)) {
				g_object_unref(apps);
				break;
			} else {
				if (!info) break;
				gchar *name = g_file_info_get_attribute_as_string (info,G_FILE_ATTRIBUTE_STANDARD_NAME);
				if ((g_strcmp0(name,".") && g_strcmp0(name,"..")) && name[0] != '.')	{
					gchar *path = g_file_get_path (out);
					GString *n = g_string_new (name);
					if (g_file_test(path,G_FILE_TEST_IS_DIR)) {
						if (!gtk_notebook_get_child_by_name (notebook,path)) {
							create_notebooks (n->str,path);
							guint open = g_key_file_get_integer (pdata, "Opened", path,NULL);
							if (open > leader) {
								gtk_notebook_set_current_page(notebook, counted);
							}
							counted++;
						}
					}
				}
			}
		if (!info) break;
	}

	if (gtk_notebook_get_visible_child (notebook) == NULL) gtml_import_full ();
}

static void start_import (GtkFileChooserButton *widget) {
	gchar *type = (gchar *) gtk_file_chooser_button_get_title (widget);
	gchar *path = gtk_file_chooser_get_filename (widget);

	GNotification *inotification = g_notification_new ("Import Complete");
	g_notification_set_priority (inotification,G_NOTIFICATION_PRIORITY_NORMAL);

	GFile 	*old = g_file_new_for_path (path),
		*new = g_file_new_for_path (g_strdup_printf("./gtml/%s/",g_file_get_basename (old)));
	GKeyFile *programs = g_key_file_new ();
	g_key_file_load_from_file (programs,"./gtml/.data/programs.gkf",gkfflags,NULL);

	if (!g_strcmp0(type,"Full")) {
		g_file_move (old, new,cflags,NULL,NULL,NULL,NULL);
		g_file_make_directory_with_parents (g_file_new_for_path(g_strdup_printf("%s/profile",g_file_get_path(new))),NULL,NULL);
		create_notebooks (g_file_get_basename(new),g_file_get_path(new));
	} else if (!g_strcmp0(type,"Partial")) {/*
		g_file_copy (old, new,cflags,NULL,NULL,NULL,NULL);
		GFileEnumerator *ennew = g_file_enumerate_children (old,"*",G_FILE_QUERY_INFO_NONE,NULL,NULL);
		while (TRUE) {
			GFile *out;
			GFileInfo *info;
			if (!g_file_enumerator_iterate(ennew,&info,&out,NULL,NULL)) {
				g_object_unref(ennew);
				break;
			} else {
				if (!info) break;
				gchar *name = g_file_info_get_attribute_as_string (info,G_FILE_ATTRIBUTE_STANDARD_NAME);
				if ((g_strcmp0(name,".") && g_strcmp0(name,"..")) && name[0] != '.')	{
					gchar *path = g_file_get_path (out);
					GString *n = g_string_new (name);
					if (g_file_test(path,G_FILE_TEST_IS_DIR)){

						if (!gtk_notebook_get_child_by_name (notebook,n->str)) {
							g_print("%s\n",n->str);
							//gtk_notebook_add_titled (notebook,create_tabs(),n->str,n->str);
						}
								//gtk_notebook_set_current_page (notebook,n->str);
					}
				}
			}
			if (!info) break;
		}
		g_file_make_directory_with_parents (g_file_new_for_path(g_strdup_printf("%s/profile",g_file_get_path(new))),NULL,NULL);
		*/
	} else if (!g_strcmp0(type,"Link")) {
		g_key_file_set_string (programs,"Programs",g_strdup_printf("%s",g_file_get_basename(old)),g_strdup_printf("%s",g_file_get_path(old)));
		g_file_make_directory_with_parents (g_file_new_for_path(g_strdup_printf("%s/profile",g_file_get_path(new))),NULL,NULL);
		create_notebooks (g_file_get_basename(old),g_file_get_path(old));
	}

	g_key_file_save_to_file (programs,"./gtml/.data/programs.gkf",NULL);

	g_notification_set_body (inotification,g_strdup_printf ("The %s import of the file located at %s is now complete.\nReturn to Mozilla Launcher to secure and launch it.", type,path));
	g_application_send_notification (app,"import-complete",inotification);
}

static void gtml_import_full (void) {
	GtkWidget 	*dialog = gtk_dialog_new_with_buttons ("Full Import",window,dflags,"Accept",GTK_RESPONSE_ACCEPT,"Cancel",GTK_RESPONSE_REJECT,NULL),
			*dgrid = gtk_grid_new(),
			*timport = gtk_file_chooser_button_new ("Full",GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER),
			*pimport = gtk_file_chooser_button_new ("Partial",GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER),
			*limport = gtk_file_chooser_button_new ("Link",GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER),
			*tbutton = gtk_button_new_with_mnemonic ("Total Import"),
			*pbutton = gtk_button_new_with_mnemonic ("Partial Import"),
			*lbutton = gtk_button_new_with_mnemonic ("Link Import")
			;

	gtk_widget_set_tooltip_markup (timport,"Moves all the application\'s information from its current location.");
	gtk_widget_set_tooltip_markup (pimport,"Copies all the application\'s information from its current location.");
	gtk_widget_set_tooltip_markup (limport,"Links the application to the program.");

	gtk_grid_set_column_homogeneous(dgrid,TRUE);

	gtk_grid_attach (dgrid,tbutton ,0,0,1,1);
	gtk_grid_attach (dgrid,pbutton ,1,0,1,1);
	gtk_grid_attach (dgrid,lbutton ,2,0,1,1);

	gtk_grid_attach (dgrid,timport ,0,1,1,1);
	gtk_grid_attach (dgrid,pimport ,1,1,1,1);
	gtk_grid_attach (dgrid,limport ,2,1,1,1);

	gtk_box_pack_start (gtk_dialog_get_content_area(dialog),dgrid,TRUE,TRUE,0);

	g_signal_connect_swapped (tbutton,"clicked",(GCallback)gtk_widget_show,timport);
	g_signal_connect_swapped (tbutton,"clicked",(GCallback)gtk_widget_hide,pimport);
	g_signal_connect_swapped (tbutton,"clicked",(GCallback)gtk_widget_hide,limport);

	g_signal_connect_swapped (pbutton,"clicked",(GCallback)gtk_widget_hide,timport);
	g_signal_connect_swapped (pbutton,"clicked",(GCallback)gtk_widget_show,pimport);
	g_signal_connect_swapped (pbutton,"clicked",(GCallback)gtk_widget_hide,limport);

	g_signal_connect_swapped (lbutton,"clicked",(GCallback)gtk_widget_hide,timport);
	g_signal_connect_swapped (lbutton,"clicked",(GCallback)gtk_widget_hide,pimport);
	g_signal_connect_swapped (lbutton,"clicked",(GCallback)gtk_widget_show,limport);

	g_signal_connect (timport,"file-set",(GCallback)start_import,dialog);
	g_signal_connect (pimport,"file-set",(GCallback)start_import,dialog);
	g_signal_connect (limport,"file-set",(GCallback)start_import,dialog);

	gtk_widget_show_all (dialog);
	gtk_widget_hide (timport);
	gtk_widget_hide (pimport);
	gtk_widget_hide (limport);
}