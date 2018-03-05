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
static GtkHeaderBar *header = NULL;
static GtkStack *stack = NULL;
static GtkStackSidebar *stack_sidebar = NULL;

static void gtml_activate (void);

//main loop
int main (int argc, char **argv){
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
	//gtk_window_set_role (window,"main_window");
	//gtk_window_set_decorated (window,TRUE);
	//gtk_window_set_default_size (window,800,110);

	header = gtk_header_bar_new ();
	gtk_header_bar_set_title (header, PN);
	gtk_header_bar_set_show_close_button (header,TRUE);
	gtk_header_bar_set_decoration_layout (header,"menu,stick,shade:minimize,maximize,close" );
	gtk_window_set_titlebar (window,header);

	stack = gtk_stack_new ();
	stack_sidebar = gtk_stack_sidebar_new ();
	gtk_stack_sidebar_set_stack (stack_sidebar,stack);

	gtk_widget_show_all (window);
}

static void create_stack_children (void) {
	GFile *applications = g_file_new_for_path ("./gtml");

	GFileEnumerator *apps = g_file_enumerate_children (applications,"*",G_FILE_QUERY_INFO_NONE,NULL,NULL);
	while (TRUE) {
		GFile *out;
		GFileInfo *info;

		GtkWidget *child = NULL;
		gchar *dir_name = NULL;
		gchar *dir_path = NULL;

		if (!g_file_enumerator_iterate(apps,&info,&out,NULL,NULL)) {
				g_object_unref(apps);
				break;
			} else {
				if (!info) break;
				dir_name = g_file_info_get_attribute_as_string (info,G_FILE_ATTRIBUTE_STANDARD_NAME);
				if ((g_strcmp0(dir_name,".") && g_strcmp0(dir_name,"..")) && dir_name[0] != '.')	{
					dir_path = g_file_get_path (out);
					GtkEntry *pass[] = {gtk_entry_new (),gtk_entry_new (),gtk_entry_new (), NULL};
					child = gtk_grid_new ();
				}

			}
		if (!info) break;
	}
}