using Gtk;
using System;
using GLib;

namespace Gtml4 {
	public class HelloWorld  {
		const string PN = "Mozilla Launcher";
		const string CN = "Gravastar Technologies";
		const string AN = "Kuliwa Emmanuel SImubali";

		static Gtk.FileChooserAction fcact = Gtk.FileChooserAction.SelectFolder ;
		static Gtk.DialogFlags dflags = Gtk.DialogFlags.DestroyWithParent | Gtk.DialogFlags.Modal ;
		static GLib.KeyFileFlags gkfflags = GLib.KeyFileFlags.KeepComments | GLib.KeyFileFlags.KeepTranslations ;
		static GLib.FileQueryInfoFlags feflags = GLib.FileQueryInfoFlags.None ;
		static GLib.FileCopyFlags fcflags = GLib.FileCopyFlags.Backup | GLib.FileCopyFlags.AllMetadata ;

		static Gtk.Notebook stack;
		static Gtk.Grid grid;
		static Gtk.Window window;
		static Gtk.Button import_button;
		static Gtk.IconFactory icons;
		static Gtk.Application app;

		public static int Main (string[] args) {
			app = new Gtk.Application ("gtml.testing", GLib.ApplicationFlags.None);
			Gtk.Application.Init ("gtml.testing",ref args);
			gtml_activate();
			Gtk.Application.Run ();
			return 0;
		}

		static void gtml_activate () {
			icons = new Gtk.IconFactory ();
			icons.AddDefault ();

			window = new Gtk.Window (PN);
			window.DeleteEvent += new DeleteEventHandler (Window_Delete);
			window.SetDefaultSize (800,120);
			window.Decorated = true;

			grid = new Gtk.Grid();
			grid.ColumnHomogeneous = true;

			Gtk.Image icon = new Gtk.Image ();
			icon.IconName = ("list-add-symbolic");
			import_button = new Gtk.Button (icon);
			import_button.Clicked += new EventHandler (gtml_import_full);
			Gtk.Box naw = new Gtk.Box (Gtk.Orientation.Horizontal,1);
			naw.PackStart (import_button,true,true,1);
			naw.ShowAll ();

			stack = new Gtk.Notebook ();
			stack.EnablePopup = true;
			stack.GroupName = "Mozilla Programs";
			stack.Scrollable = true;

			stack.SetActionWidget (naw,Gtk.PackType.End);

			stack.Hexpand = true;
			stack.Vexpand = true;

			grid.Attach (stack,1,1,1,1);
			gtml_reload ();

			window.Add (grid);

			window.ShowAll ();
		}

		static void zero_tog (Gtk.ToggleButton butn, Gtk.ToggleButton butn2) {
			if (butn.Active == true ) {
				butn.Active = false;
			} else {
				butn.Active = false;
			}
		}

		static void zero_act (Gtk.Entry entry, Gtk.ToggleButton butn) {
			if (butn.Active == true) {
				entry.Sensitive = true;
				entry.Show ();
			} else {
				entry.Sensitive = false;
				entry.Hide ();
			}
		}

		static void one_act (Gtk.Entry entry, Gtk.ToggleButton butn) {
			if (butn.Active == true) {
				entry.Sensitive = false;
				entry.Hide ();
			} else {
				entry.Sensitive = true;
				entry.Show ();
			}
		}

		static void create_stacks (string name, string location) {
			Gtk.Grid sgrid = new Gtk.Grid () ;
			Gtk.Entry[] password = {
				new Gtk.Entry ("Password"),
				new Gtk.Entry ("New Password"),
				null
			};
			Gtk.Label slabel = new Gtk.Label (name);
			Gtk.ToggleButton[] switchs = {
				new Gtk.ToggleButton ("Secure"),
				new Gtk.ToggleButton ("Launch"),
				null
			};

			sgrid.ColumnHomogeneous = true;
			switchs[1].Active = true;

			password[1].Sensitive = false;
			password[0].Visibility = false;
			password[1].Visibility = false;

			//password[0].Activated += new EventHandler ();
			//password[1].Activated += new EventHandler ();
			//switchs[0].Activated += new EventHandler (zero_tog);
			//switchs[1].Activated += new EventHandler (zero_tog);

			sgrid.Attach (switchs[0],1,2,1,1);
			sgrid.Attach (switchs[1],0,2,1,1);
			sgrid.Attach (password[0],0,3,1,1);
			sgrid.Attach (password[1],1,3,1,1);
			/*
			g_signal_connect (password[0],"activate",(GCallback)check_pass,password[1]);
			g_signal_connect_swapped (password[1],"activate",(GCallback)check_pass,password[0]);
			g_signal_connect (switchs[0],"toggled",(GCallback)zero_tog,switchs[1]);
			g_signal_connect (switchs[1],"toggled",(GCallback)zero_tog,switchs[0]);
			g_signal_connect_swapped (switchs[0],"toggled",(GCallback)zero_act,password[1]);
			g_signal_connect_swapped (switchs[1],"toggled",(GCallback)one_act,password[1]);
			*/
			sgrid.ShowAll ();
			one_act (password[1],switchs[1]);
			stack.AppendPage (sgrid,slabel);
			stack.SetMenuLabelText (sgrid,name);
		}

		static void gtml_reload () {
			GLib.IFile applications = GLib.FileFactory.NewForPath ("./gtml");
			GLib.KeyFile programs = new GLib.KeyFile ();
			bool load = false;
			if (GLib.FileFactory.NewForPath ("./data").Exists) {
				if (GLib.FileFactory.NewForPath ("./data/programs.gkf").Exists)
				load = programs.LoadFromFile ("./data/programs.gkf",gkfflags);
			} else {
				GLib.FileFactory.NewForPath ("./data").MakeDirectoryWithParents(null);
			}
			if (load == true) {
				if (programs.HasGroup ("Programs")) {
					string[] names = programs.GetKeys ("Programs");
					int x = 0;
					while (names[x].Equals("") != true || names[x] != null) {
						string location = programs.GetString ("Programs",names[x]);
						create_stacks (names[x],location);
						location = null;
						x++;
						break;
					}
				} else System.Console.WriteLine ("No linked programs in data.");
			} else System.Console.WriteLine ("No linked programs\n");

			GLib.FileEnumerator apps = applications.EnumerateChildren ("*",feflags,null);
			while (true) {
				GLib.FileInfo info = apps.NextFile ();
				if (info == null) {
					apps.Dispose ();
					break;
				} else {
					GLib.IFile outf = applications.GetChildForDisplayName (info.DisplayName);
					string name = info.DisplayName;
					if ((name.Equals (".") && name.Equals("..")) == false && (name.Length != 0 || name != null)) {
						string path = outf.Path;
						if (info.FileType == GLib.FileType.Directory) {
							create_stacks (name,path);
							/*
							if (!gtk_stack_get_child_by_name (GTK_STACK(stack),path)) {
								create_stacks (name,path);
							}




							*/
						}
					}
				}
			}

			if (stack.CurrentPageWidget == null) ;//gtml_import_full ();
		}

		static void start_import (Gtk.FileChooserButton widget) {
			string type = widget.Title;
			string path = widget.Filename;

			GLib.IFile old = GLib.FileFactory.NewForPath (path);
			GLib.IFile news = GLib.FileFactory.NewForPath ("./gtml" + old.Basename);
			GLib.KeyFile programs = new GLib.KeyFile ();
			if (GLib.FileFactory.NewForPath ("./data/programs.gkf").Exists) programs.LoadFromFile ("./data/programs.gkf",gkfflags);
			if (type.Equals ("Full")) {
				old.Move (news,fcflags,null,null);
				GLib.FileFactory.NewForPath (news.Path + "/profile").MakeDirectoryWithParents (null);
				create_stacks (news.Basename,news.Path);
			} else if (type.Equals ("Partial")) {
				/*
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

								if (!gtk_stack_get_child_by_name (GTK_STACK(stack),n->str)) {
									g_print("%s\n",n->str);
									//gtk_stack_add_titled (GTK_STACK(stack),create_tabs(),n->str,n->str);
								}
										//gtk_stack_set_visible_child_name (GTK_STACK(stack),n->str);
							}
						}
					}
					if (!info) break;
				}
				g_file_make_directory_with_parents (g_file_new_for_path(g_strdup_printf("%s/profile",g_file_get_path(new))),NULL,NULL);
				*/
			} else if (type.Equals ("Link")) {
				programs.SetString ("Programs",old.Basename,old.Path);
				GLib.FileFactory.NewForPath (news.Path + "/profile").MakeDirectoryWithParents (null);
				create_stacks (old.Basename,old.Path);
			}

			programs.Save ("./data/programs.gkf");
			/*
			GNotification *inotification = g_notification_new ("Import Complete");
			g_notification_set_priority (inotification,G_NOTIFICATION_PRIORITY_NORMAL);
				... Data was here ...
			g_notification_set_body (inotification,g_strdup_printf ("The %s import of the file located at %s is now complete.\nReturn to Mozilla Launcher to secure and launch it.", type,path));
			g_application_send_notification (G_APPLICATION(app),"import-complete",inotification);
			*/
		}

		static void gtml_import_full (object obj, EventArgs args) {
			Gtk.Dialog dialog = new Gtk.Dialog ("Full Import",window,dflags);
			Gtk.Grid dgrid = new Gtk.Grid ();
			Gtk.FileChooserButton[] import = {
				new Gtk.FileChooserButton ("Full",fcact),
				new Gtk.FileChooserButton ("Partial",fcact),
				new Gtk.FileChooserButton ("Link",fcact),
				null
			};
			Gtk.Button[] importb = {
				new Gtk.Button ("Full"),
				new Gtk.Button ("Partial"),
				new Gtk.Button ("Link"),
				null
			};

			//importb[0].TooltipText ("Moves all the application\'s information from its current location.");
			//importb[1].TooltipText ("Copies all the application\'s information from its current location.");
			//importb[2].TooltipText ("Links the application to the program.");

			dgrid.ColumnHomogeneous = true;

			dgrid.Attach (importb[0] ,0,0,1,1);
			dgrid.Attach (importb[1] ,1,0,1,1);
			dgrid.Attach (importb[2] ,2,0,1,1);
			dgrid.Attach (import[0] ,0,1,1,1);
			dgrid.Attach (import[1] ,1,1,1,1);
			dgrid.Attach (import[2] ,2,1,1,1);

			Gtk.Box dbox = dialog.ContentArea;
			dbox.PackStart (dgrid,true,true,0);

			dialog.ShowAll ();
			import[0].Hide ();
			import[1].Hide ();
			import[2].Hide ();

			/*
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

			*/
		}

		static void Window_Delete (object obj, DeleteEventArgs args) {
			SignalArgs sa = (SignalArgs) args;
			Gtk.Application.Quit ();
			sa.RetVal = true;
		}
	}
}

