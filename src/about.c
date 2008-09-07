/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
    Gpredict: Real-time satellite tracking and orbit prediction program

    Copyright (C)  2001-2008  Alexandru Csete, OZ9AEC.

    Authors: Alexandru Csete <oz9aec@gmail.com>

    Comments, questions and bugreports should be submitted via
    http://sourceforge.net/projects/gpredict/
    More details can be found at the project home page:

            http://gpredict.oz9aec.net/
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
  
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, visit http://www.fsf.org/
*/
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#ifdef HAVE_CONFIG_H
#  include <build-config.h>
#endif
#include "gpredict-url-hook.h"
#include "compat.h"
#include "about.h"


const gchar *authors[] = {
	"Alexandru Csete, OZ9AEC (design and development)",
	"",
	"Contributors:",
	"Damon Chaplin (GooCanvas)",
	"Dr. T.S. Kelso (SGP4/SDP4 algorithms)",
	"John A. Magliacane, KD2BD (prediction code)",
	"Neoklis Kyriazis, 5B4AZ (SGP4/SDP4 in C)",
	"William J Beksi, KC2EXL (GtkSatMap)",
	"Stephane Fillod (locator.c)",
	"Nate Bargmann (locator.c)",
	"Dave Hines (locator.c)",
	"Mirko Caserta (locator.c)",
	"S. R. Sampson (locator.c)",
	NULL
};


const gchar license[] = N_("Copyright (C) 2001-2007 Alexandru Csete OZ9AEC\n"\
						   "Contact: oz9aec at googlemail.com\n\n"\
						   "Gpredict is free software; you can redistribute it and "\
						   "mofdify it under the terms of the GNU General Public License "\
						   "as published by the Free Software Foundation; either version 2 "\
						   "of the License, or (at your option) any later version.\n\n"\
						   "This program is distributed free of charge in the hope that it will "\
						   "be useful, but WITHOUT ANY WARRANTY; without even the implied "\
						   "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "\
						   "See the GNU Library General Public License for more details.\n\n"\
						   "You should have received a copy of the GNU General Public License "\
						   "along with this program (see Help->License). Otherwise you can find "\
                           "a copy on the FSF "\
						   "website http://www.fsf.org/licensing/licenses/gpl.html or you can "\
						   "write to the\n\n"
						   "Free Software Foundation, Inc.\n"\
						   "59 Temple Place - Suite 330\n"
						   "Boston\n"\
						   "MA 02111-1307\n"
						   "USA.\n");



/** \brief Create and run the gpredict about dialog. */
void about_dialog_create ()
{
	GtkWidget *dialog;
	GdkPixbuf *icon;
	gchar     *iconfile;


	dialog = gtk_about_dialog_new ();
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (dialog), _("GPREDICT"));
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), VERSION);
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog),
									_("Copyright (C) 2001-2008 Alexandru Csete OZ9AEC\n\n"\
									  "Gpredict is available free of charge from:"));
	gtk_about_dialog_set_url_hook (gpredict_url_hook_cb, NULL, NULL);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog),
								  "http://gpredict.sourceforge.net/");
/* 	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), */
/* 										_("Gpredict Website")); */
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (dialog), license);
	gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (dialog), TRUE);
	iconfile = icon_file_name ("gpredict-icon.png");
	icon = gdk_pixbuf_new_from_file (iconfile, NULL);
	gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialog), icon);
	g_free (iconfile);
	g_object_unref (icon);

	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialog), authors);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialog),
											 _("translator-credits"));

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}
