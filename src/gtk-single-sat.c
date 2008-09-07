/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*
  Gpredict: Real-time satellite tracking and orbit prediction program

  Copyright (C)  2001-2007  Alexandru Csete, OZ9AEC.

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
/** \brief Satellite List Widget.
 *
 * More info...
 */
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include "sgpsdp/sgp4sdp4.h"
#include "gtk-single-sat.h"
#include "sat-log.h"
#include "config-keys.h"
#include "sat-cfg.h"
#include "mod-cfg-get-param.h"
#include "gtk-sat-data.h"
#include "gpredict-utils.h"
#include "sat-popup-menu.h"
#include "locator.h"
#include "sat-vis.h"
#ifdef HAVE_CONFIG_H
#  include <build-config.h>
#endif



/** \brief Column titles indexed with column symb. refs. */
const gchar *SINGLE_SAT_FIELD_TITLE[SINGLE_SAT_FIELD_NUMBER] = {
	N_("Azimuth :"),
	N_("Elevation :"),
	N_("Direction :"),
	N_("Right Asc. :"),
	N_("Declination :"),
	N_("Slant Range :"),
	N_("Range Rate :"),
	N_("Next Event :"),
	N_("Next AOS :"),
	N_("Next LOS :"),
	N_("SSP Lat. :"),
	N_("SSP Lon. :"),
	N_("SSP Loc. :"),
	N_("Footprint :"),
	N_("Altitude :"),
	N_("Velocity :"),
	N_("Doppler :"),
	N_("Sig. Loss :"),
	N_("Sig. Delay :"),
	N_("Mean Anom. :"),
	N_("Orbit Phase :"),
	N_("Orbit Num. :"),
	N_("Visibility :")
};


/** \brief Column title hints indexed with column symb. refs. */
const gchar *SINGLE_SAT_FIELD_HINT[SINGLE_SAT_FIELD_NUMBER] = {
	N_("Azimuth"),
	N_("Elevation"),
	N_("Direction"),
	N_("Right Ascension"),
	N_("Declination"),
	N_("Slant Range"),
	N_("Range Rate"),
	N_("Next Event"),
	N_("Next AOS"),
	N_("Next LOS"),
	N_("Latitude"),
	N_("Longitude"),
	N_("Sub-Satellite Point"),
	N_("Footprint"),
	N_("Altitude"),
	N_("Velocity"),
	N_("Doppler Shift @ 100MHz"),
	N_("Signal Loss @ 100MHz"),
	N_("Signal Delay"),
	N_("Mean Anomaly"),
	N_("Orbit Phase"),
	N_("Orbit Number"),
	N_("Visibility")
};


static void gtk_single_sat_class_init (GtkSingleSatClass *class);
static void gtk_single_sat_init       (GtkSingleSat      *list);
static void gtk_single_sat_destroy    (GtkObject       *object);

static void store_sats              (gpointer key, gpointer value, gpointer user_data);
static void update_field            (GtkSingleSat *ssat, guint i);
static void Calculate_RADec         (sat_t *sat, qth_t *qth, obs_astro_t *obs_set);
static void gtk_single_sat_popup_cb (GtkWidget *button, gpointer data);
static void select_satellite        (GtkWidget *menuitem, gpointer data);



static GtkVBoxClass *parent_class = NULL;


GType
gtk_single_sat_get_type ()
{
	static GType gtk_single_sat_type = 0;

	if (!gtk_single_sat_type) {

		static const GTypeInfo gtk_single_sat_info = {
			sizeof (GtkSingleSatClass),
			NULL,  /* base_init */
			NULL,  /* base_finalize */
			(GClassInitFunc) gtk_single_sat_class_init,
			NULL,  /* class_finalize */
			NULL,  /* class_data */
			sizeof (GtkSingleSat),
			5,     /* n_preallocs */
			(GInstanceInitFunc) gtk_single_sat_init,
		};

		gtk_single_sat_type = g_type_register_static (GTK_TYPE_VBOX,
													  "GtkSingleSat",
													  &gtk_single_sat_info,
													  0);
	}

	return gtk_single_sat_type;
}


static void
gtk_single_sat_class_init (GtkSingleSatClass *class)
{
	GObjectClass      *gobject_class;
	GtkObjectClass    *object_class;
	GtkWidgetClass    *widget_class;
	GtkContainerClass *container_class;

	gobject_class   = G_OBJECT_CLASS (class);
	object_class    = (GtkObjectClass*) class;
	widget_class    = (GtkWidgetClass*) class;
	container_class = (GtkContainerClass*) class;

	parent_class = g_type_class_peek_parent (class);

	object_class->destroy = gtk_single_sat_destroy;
 
}



static void
gtk_single_sat_init (GtkSingleSat *list)
{
	/* 	GtkWidget *vbox,*hbox; */


	/* 	hbox = gtk_hbox_new (TRUE, 5); */
	/* 	gtk_box_pack_start_defaults (GTK_BOX (hbox), gtk_label_new ("POLAR")); */
	/* 	gtk_box_pack_start_defaults (GTK_BOX (hbox), gtk_label_new ("LIST")); */

	/* 	vbox = gtk_vbox_new (TRUE, 5); */
	/* 	gtk_box_pack_start_defaults (GTK_BOX (vbox), gtk_label_new ("MAP")); */
	/* 	gtk_box_pack_start_defaults (GTK_BOX (vbox), hbox); */

	/* 	gtk_container_add (GTK_CONTAINER (module), vbox); */
	/* 	gtk_widget_show_all (vbox); */

	/* initialise data structures */

}

static void
gtk_single_sat_destroy (GtkObject *object)
{
	(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}




GtkWidget *
gtk_single_sat_new (GKeyFile *cfgdata, GHashTable *sats, qth_t *qth, guint32 fields)
{
	GtkWidget *widget;
	GtkWidget *hbox;      /* horizontal box for header */
	GtkWidget *label1;
	GtkWidget *label2;
	sat_t     *sat;
	gchar     *title;
	guint      i;



	widget = g_object_new (GTK_TYPE_SINGLE_SAT, NULL);

	GTK_SINGLE_SAT (widget)->update = gtk_single_sat_update;

	/* Read configuration data. */
	/* ... */

	g_hash_table_foreach (sats, store_sats, widget);
	GTK_SINGLE_SAT (widget)->selected = 0;
	GTK_SINGLE_SAT (widget)->qth = qth;
	GTK_SINGLE_SAT (widget)->cfgdata = cfgdata;

	/* initialise column flags */
	if (fields > 0)
		GTK_SINGLE_SAT (widget)->flags = fields;
	else
		GTK_SINGLE_SAT (widget)->flags = mod_cfg_get_int (cfgdata,
														  MOD_CFG_SINGLE_SAT_SECTION,
														  MOD_CFG_SINGLE_SAT_FIELDS,
														  SAT_CFG_INT_SINGLE_SAT_FIELDS);
	

	/* get refresh rate and cycle counter */
	GTK_SINGLE_SAT (widget)->refresh = mod_cfg_get_int (cfgdata,
														MOD_CFG_SINGLE_SAT_SECTION,
														MOD_CFG_SINGLE_SAT_REFRESH,
														SAT_CFG_INT_SINGLE_SAT_REFRESH);
	GTK_SINGLE_SAT (widget)->counter = 1;

	/* popup button */
	GTK_SINGLE_SAT (widget)->popup_button =
		gpredict_mini_mod_button ("gpredict-mod-popup.png",
								  _("Satellite options / shortcuts"));
	g_signal_connect (GTK_SINGLE_SAT (widget)->popup_button, "clicked",
					  G_CALLBACK (gtk_single_sat_popup_cb), widget);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_SINGLE_SAT (widget)->popup_button, FALSE, FALSE, 0);


	/* create header */
	sat = SAT (g_slist_nth_data (GTK_SINGLE_SAT (widget)->sats, 0));
	title = g_strdup_printf ("<b>%s</b>", sat ? sat->tle.sat_name : "noname");

	GTK_SINGLE_SAT (widget)->header = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (GTK_SINGLE_SAT (widget)->header), title);
	g_free (title);
	gtk_misc_set_alignment (GTK_MISC (GTK_SINGLE_SAT (widget)->header), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_SINGLE_SAT (widget)->header, TRUE, TRUE, 10);


	gtk_box_pack_start (GTK_BOX (widget), hbox, FALSE, FALSE, 0);

	/* create and initialise table  */
	GTK_SINGLE_SAT (widget)->table = gtk_table_new (SINGLE_SAT_FIELD_NUMBER, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (GTK_SINGLE_SAT (widget)->table), 5);
	gtk_table_set_row_spacings (GTK_TABLE (GTK_SINGLE_SAT (widget)->table), 0);
	gtk_table_set_col_spacings (GTK_TABLE (GTK_SINGLE_SAT (widget)->table), 10);

	/* create and add label widgets */
	for (i = 0; i < SINGLE_SAT_FIELD_NUMBER; i++) {

		if (GTK_SINGLE_SAT (widget)->flags & (1 << i)) {

			label1 = gtk_label_new (SINGLE_SAT_FIELD_TITLE[i]);
			gtk_misc_set_alignment (GTK_MISC (label1), 1.0, 0.5);
			gtk_table_attach (GTK_TABLE (GTK_SINGLE_SAT (widget)->table), label1,
							  0, 1, i, i+1,
							  GTK_FILL,  GTK_SHRINK, 0, 0);
			/* FIXME: does not work
			   tips = gtk_tooltips_new ();
			   gtk_tooltips_set_tip (tips, label1, SINGLE_SAT_FIELD_HINT[i], NULL);
			*/

			label2 = gtk_label_new ("-");
			gtk_misc_set_alignment (GTK_MISC (label2), 0.0, 0.5);
			gtk_table_attach (GTK_TABLE (GTK_SINGLE_SAT (widget)->table), label2,
							  1, 2, i, i+1,
							  GTK_FILL,  GTK_SHRINK, 0, 0);

			/* store reference */
			GTK_SINGLE_SAT (widget)->labels[i] = label2;
		}
		else
			GTK_SINGLE_SAT (widget)->labels[i] = NULL;
	}

	/* create and initialise scrolled window */
	GTK_SINGLE_SAT (widget)->swin = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (GTK_SINGLE_SAT (widget)->swin),
									GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (GTK_SINGLE_SAT (widget)->swin),
										   GTK_SINGLE_SAT (widget)->table);


	gtk_box_pack_end (GTK_BOX (widget), GTK_SINGLE_SAT (widget)->swin, TRUE, TRUE, 0);

	gtk_widget_show_all (widget);

	return widget;
}







/** \brief Update satellites */
void
gtk_single_sat_update          (GtkWidget *widget)
{
	GtkSingleSat *ssat = GTK_SINGLE_SAT (widget);
	guint         i;

	/* first, do some sanity checks */
	if ((ssat == NULL) || !IS_GTK_SINGLE_SAT (ssat)) {

		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s: Invalid GtkSingleSat!"),
					 __FUNCTION__);
	}


	/* check refresh rate */
	if (ssat->counter < ssat->refresh) {
		ssat->counter++;
	}
	else {

		/* we calculate here to avoid double calc */
		if ((ssat->flags & SINGLE_SAT_FLAG_RA) ||
		    (ssat->flags & SINGLE_SAT_FLAG_DEC)) {

			sat_t      *sat = SAT (g_slist_nth_data (ssat->sats, ssat->selected));
			obs_astro_t astro;


			Calculate_RADec (sat, ssat->qth, &astro);
			sat->ra = Degrees(astro.ra);
			sat->dec = Degrees(astro.dec);
		}

		/* update visible fields one by one */
		for (i = 0; i < SINGLE_SAT_FIELD_NUMBER; i++) {

			if (ssat->flags & (1 << i))
				update_field (ssat, i);

		}
		ssat->counter = 1;
	}

}


/** \brief Update a field in the GtkSingleSat view.
 */
static void
update_field            (GtkSingleSat *ssat, guint i)
{
	sat_t     *sat;
	gchar     *buff = NULL;
	gchar      tbuf[TIME_FORMAT_MAX_LENGTH];
	gchar      hmf = ' ';
	gdouble    number;
	gint       retcode;
	gchar     *fmtstr;
	gchar     *alstr;
	time_t     t;
	guint      size;
	sat_vis_t  vis;


	/* make some sanity checks */
	if (ssat->labels[i] == NULL) {

		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s:%d: Can not update invisible field (I:%d F:%d)"),
					 __FILE__, __LINE__, i, ssat->flags);

		return;
	}

	/* get selected satellite */
	sat = SAT (g_slist_nth_data (ssat->sats, ssat->selected));
    
    if G_UNLIKELY(!sat) {
        sat_log_log (SAT_LOG_LEVEL_BUG,
                     _("%s:%d: Can not update non-existing sat"),
                     __FILE__, __LINE__);
        return;
    }

	/* update requested field */
	switch (i) {


	case SINGLE_SAT_FIELD_AZ:
		buff = g_strdup_printf ("%6.2f\302\260", sat->az);
		break;


	case SINGLE_SAT_FIELD_EL:
		buff = g_strdup_printf ("%6.2f\302\260", sat->el);
		break;


	case SINGLE_SAT_FIELD_DIR:
		if (sat->otype == ORBIT_TYPE_GEO) {
			buff = g_strdup ("Geostationary");
		}
		else if (sat->otype == ORBIT_TYPE_DECAYED) {
			buff = g_strdup ("Decayed");
		}
		else if (sat->range_rate > 0.0) {
			/* Receeding */
			buff = g_strdup ("Receeding");
		}
		else if (sat->range_rate < 0.0) {
			/* Approaching */
			buff = g_strdup ("Approaching");
		}
		else {
			buff = g_strdup ("N/A");
		}
		break;


	case SINGLE_SAT_FIELD_RA:
		buff = g_strdup_printf ("%6.2f\302\260", sat->ra);
		break;


	case SINGLE_SAT_FIELD_DEC:
		buff = g_strdup_printf ("%6.2f\302\260", sat->dec);
		break;


	case SINGLE_SAT_FIELD_RANGE:
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_IMPERIAL)) {
			buff = g_strdup_printf ("%.0f mi", KM_TO_MI (sat->range));
		}
		else {
			buff = g_strdup_printf ("%.0f km", sat->range);
		}
		break;


	case SINGLE_SAT_FIELD_RANGE_RATE:
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_IMPERIAL)) {
			buff = g_strdup_printf ("%.3f mi/sec", KM_TO_MI (sat->range_rate));
		}
		else {
			buff = g_strdup_printf ("%.3f km/sec", sat->range_rate);
		}
		break;


	case SINGLE_SAT_FIELD_NEXT_EVENT:
		if (sat->aos > sat->los) {
			/* next event is LOS */
			number = sat->los;
			alstr = g_strdup ("LOS: ");
		}
		else {
			/* next event is AOS */
			number = sat->aos;
			alstr = g_strdup ("AOS: ");
		}
		if (number > 0.0) {

			/* convert julian date to struct tm */
			t = (number - 2440587.5)*86400.;

			/* format the number */
			fmtstr = sat_cfg_get_str (SAT_CFG_STR_TIME_FORMAT);

			/* format either local time or UTC depending on check box */
			if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_LOCAL_TIME))
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, localtime (&t));
			else
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, gmtime (&t));
		
			if (size < TIME_FORMAT_MAX_LENGTH)
				tbuf[size]='\0';
			else
				tbuf[TIME_FORMAT_MAX_LENGTH]='\0';

			g_free (fmtstr);

			buff = g_strconcat (alstr, tbuf, NULL);

		}
		else {
			buff = g_strdup (_("N/A"));
		}

		g_free (alstr);

		break;


	case SINGLE_SAT_FIELD_AOS:
		if (sat->aos > 0.0) {
			/* convert julian date to struct tm */
			t = (sat->aos - 2440587.5)*86400.;

			/* format the number */
			fmtstr = sat_cfg_get_str (SAT_CFG_STR_TIME_FORMAT);

			/* format either local time or UTC depending on check box */
			if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_LOCAL_TIME))
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, localtime (&t));
			else
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, gmtime (&t));
		
			if (size < TIME_FORMAT_MAX_LENGTH)
				tbuf[size]='\0';
			else
				tbuf[TIME_FORMAT_MAX_LENGTH]='\0';

			g_free (fmtstr);

			buff = g_strdup (tbuf);
		}
		else {
			buff = g_strdup (_("N/A"));
		}
		break;


	case SINGLE_SAT_FIELD_LOS:
		if (sat->los > 0.0) {
			/* convert julian date to struct tm */
			t = (sat->los - 2440587.5)*86400.;

			/* format the number */
			fmtstr = sat_cfg_get_str (SAT_CFG_STR_TIME_FORMAT);

			/* format either local time or UTC depending on check box */
			if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_LOCAL_TIME))
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, localtime (&t));
			else
				size = strftime (tbuf, TIME_FORMAT_MAX_LENGTH, fmtstr, gmtime (&t));
		
			if (size < TIME_FORMAT_MAX_LENGTH)
				tbuf[size]='\0';
			else
				tbuf[TIME_FORMAT_MAX_LENGTH]='\0';

			g_free (fmtstr);

			buff = g_strdup (tbuf);
		}
		else {
			buff = g_strdup (_("N/A"));
		}
		break;


	case SINGLE_SAT_FIELD_LAT:
		number = sat->ssplat;
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_NSEW)) {
			if (number < 0.00) {
				number = -number;
				hmf = 'S';
			}
			else {
				hmf = 'N';
			}
		}
		buff = g_strdup_printf ("%.2f\302\260%c", number, hmf);
		break;


	case SINGLE_SAT_FIELD_LON:
		number = sat->ssplon;
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_NSEW)) {
			if (number < 0.00) {
				number = -number;
				hmf = 'W';
			}
			else {
				hmf = 'E';
			}
		}
		buff = g_strdup_printf ("%.2f\302\260%c", number, hmf);
		break;


	case SINGLE_SAT_FIELD_SSP:
		/* SSP locator */
		buff = g_try_malloc (7);

		retcode = longlat2locator (sat->ssplon, sat->ssplat, buff, 3);
		if (retcode == RIG_OK) {
			buff[6] = '\0';
		}
		else {
			g_free (buff);
			buff = NULL;
		}

		break;


	case SINGLE_SAT_FIELD_FOOTPRINT:
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_IMPERIAL)) {
			buff = g_strdup_printf ("%.0f mi", KM_TO_MI (sat->footprint));
		}
		else {
			buff = g_strdup_printf ("%.0f km", sat->footprint);
		}
		break;


	case SINGLE_SAT_FIELD_ALT:
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_IMPERIAL)) {
			buff = g_strdup_printf ("%.0f mi", KM_TO_MI (sat->alt));
		}
		else {
			buff = g_strdup_printf ("%.0f km", sat->alt);
		}
		break;


	case SINGLE_SAT_FIELD_VEL:
		if (sat_cfg_get_bool (SAT_CFG_BOOL_USE_IMPERIAL)) {
			buff = g_strdup_printf ("%.3f mi/sec", KM_TO_MI (sat->velo));
		}
		else {
			buff = g_strdup_printf ("%.3f km/sec", sat->velo);
		}
		break;


	case SINGLE_SAT_FIELD_DOPPLER:
		number = -100.0e06 * (sat->range_rate / 299792.4580); // Hz
		buff = g_strdup_printf ("%.0f Hz", number);
		break;


	case SINGLE_SAT_FIELD_LOSS:
		number = 72.4 + 20.0*log10(sat->range);               // dB
		buff = g_strdup_printf ("%.2f dB", number);
		break;


	case SINGLE_SAT_FIELD_DELAY:
		number = sat->range / 299.7924580;         // msec 
		buff = g_strdup_printf ("%.2f msec", number);
		break;


	case SINGLE_SAT_FIELD_MA:
		buff = g_strdup_printf ("%.2f\302\260", sat->ma);
		break;


	case SINGLE_SAT_FIELD_PHASE:
		buff = g_strdup_printf ("%.2f\302\260", sat->phase);
		break;


	case SINGLE_SAT_FIELD_ORBIT:
		buff = g_strdup_printf ("%lu", sat->orbit);
		break;


	case SINGLE_SAT_FIELD_VISIBILITY:
		vis = get_sat_vis (sat, ssat->qth, sat->jul_utc);
		buff = vis_to_str (vis);
		break;


	default:
		sat_log_log (SAT_LOG_LEVEL_BUG,
					 _("%s:%d: Invalid field number (%d)"),
					 __FILE__, __LINE__, i);

		break;


	}

	if (buff != NULL) {
		gtk_label_set_text (GTK_LABEL (ssat->labels[i]), buff);
		g_free (buff);
	}

}




/** \brief Copy satellite from hash table to singly linked list.
 */
static void
store_sats (gpointer key, gpointer value, gpointer user_data)
{
	GtkSingleSat *single_sat = GTK_SINGLE_SAT (user_data);
	sat_t        *sat = SAT (value);

	single_sat->sats = g_slist_append (single_sat->sats, sat);
}




static void
Calculate_RADec (sat_t *sat, qth_t *qth, obs_astro_t *obs_set)
{
	/* Reference:  Methods of Orbit Determination by  */
	/*                Pedro Ramon Escobal, pp. 401-402 */

	double phi,theta,sin_theta,cos_theta,sin_phi,cos_phi,
		az,el,Lxh,Lyh,Lzh,Sx,Ex,Zx,Sy,Ey,Zy,Sz,Ez,Zz,
		Lx,Ly,Lz,cos_delta,sin_alpha,cos_alpha;
	geodetic_t geodetic;


	geodetic.lon = qth->lon * de2ra;
	geodetic.lat = qth->lat * de2ra;
	geodetic.alt = qth->alt / 1000.0;
	geodetic.theta = 0;



	az = sat->az * de2ra;
	el = sat->el * de2ra;
	phi   = geodetic.lat;
	theta = FMod2p(ThetaG_JD(sat->jul_utc) + geodetic.lon);
	sin_theta = sin(theta);
	cos_theta = cos(theta);
	sin_phi = sin(phi);
	cos_phi = cos(phi);
	Lxh = -cos(az) * cos(el);
	Lyh =  sin(az) * cos(el);
	Lzh =  sin(el);
	Sx = sin_phi * cos_theta;
	Ex = -sin_theta;
	Zx = cos_theta * cos_phi;
	Sy = sin_phi * sin_theta;
	Ey = cos_theta;
	Zy = sin_theta*cos_phi;
	Sz = -cos_phi;
	Ez = 0;
	Zz = sin_phi;
	Lx = Sx*Lxh + Ex * Lyh + Zx*Lzh;
	Ly = Sy*Lxh + Ey * Lyh + Zy*Lzh;
	Lz = Sz*Lxh + Ez * Lyh + Zz*Lzh;
	obs_set->dec = ArcSin(Lz);  /* Declination (radians)*/
	cos_delta = sqrt(1 - Sqr(Lz));
	sin_alpha = Ly / cos_delta;
	cos_alpha = Lx / cos_delta;
	obs_set->ra = AcTan(sin_alpha,cos_alpha); /* Right Ascension (radians)*/
	obs_set->ra = FMod2p(obs_set->ra);

}


/** \brief Songle sat options
 *
 * Invoke single sat popup menu
 */
static void
gtk_single_sat_popup_cb       (GtkWidget *button, gpointer data)
{
	GtkSingleSat *single_sat = GTK_SINGLE_SAT (data);
	GtkWidget    *menu;
	GtkWidget    *select_menu;
	GtkWidget    *menuitem;
	GtkWidget    *image;
	GtkWidget    *label;
	GSList       *group = NULL;
	gchar        *buff;
	sat_t        *sat;
	sat_t        *sati;    /* used to create list of satellites */
	guint         i,n;


	sat = SAT (g_slist_nth_data (single_sat->sats, single_sat->selected));
	n = g_slist_length (single_sat->sats);
	
	menu = gtk_menu_new ();


	/* satellite name/info */
	menuitem = gtk_image_menu_item_new ();
	label = gtk_label_new (NULL);
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	buff = g_strdup_printf ("<b>%s</b>", sat->tle.sat_name);
	gtk_label_set_markup (GTK_LABEL (label), buff);
	g_free (buff);
	gtk_container_add (GTK_CONTAINER (menuitem), label);
	image = gtk_image_new_from_stock (GTK_STOCK_INFO,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);

	/* attach data to menuitem and connect callback */
	g_object_set_data (G_OBJECT (menuitem), "sat", sat);
	g_object_set_data (G_OBJECT (menuitem), "qth", single_sat->qth);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (show_sat_info),
					  gtk_widget_get_toplevel (button));

	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* next pass and predict passes */
	menuitem = gtk_image_menu_item_new_with_label (_("Show next pass"));
	image = gtk_image_new_from_stock (GTK_STOCK_JUSTIFY_FILL,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	g_object_set_data (G_OBJECT (menuitem), "sat", sat);
	g_object_set_data (G_OBJECT (menuitem), "qth", single_sat->qth);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (show_next_pass),
					  gtk_widget_get_toplevel (button));
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);
		
	menuitem = gtk_image_menu_item_new_with_label (_("Future passes"));
	image = gtk_image_new_from_stock (GTK_STOCK_INDEX,
									  GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), image);
	g_object_set_data (G_OBJECT (menuitem), "sat", sat);
	g_object_set_data (G_OBJECT (menuitem), "qth", single_sat->qth);
	g_signal_connect (menuitem, "activate",
					  G_CALLBACK (show_future_passes),
					  gtk_widget_get_toplevel (button));
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* Alarm */
	menuitem = gtk_check_menu_item_new_with_label (_("Alarm"));
	gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* Announce */
	menuitem = gtk_check_menu_item_new_with_label (_("Announce"));
	gtk_widget_set_sensitive (menuitem, FALSE);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	/* separator */
	menuitem = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	select_menu = gtk_menu_new ();

	for (i = 0; i < n; i++) {

		sati = SAT (g_slist_nth_data (single_sat->sats, i));

		menuitem = gtk_radio_menu_item_new_with_label (group, sati->tle.sat_name);
		group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menuitem));

		if (i == single_sat->selected) {
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem), TRUE);
		}

		/* store item index so that it is available in the callback */
		g_object_set_data (G_OBJECT (menuitem), "index", GUINT_TO_POINTER (i));

		/* connect signal */
		g_signal_connect_after (menuitem, "activate",
								G_CALLBACK (select_satellite), single_sat);

		gtk_menu_shell_append (GTK_MENU_SHELL (select_menu), menuitem);
		//gtk_widget_show (menuitem);
	}


	/* seelct sat */
	menuitem = gtk_menu_item_new_with_label (_("Select satellite"));
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), select_menu);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menuitem);

	gtk_widget_show_all (menu);

	/* Note: event can be NULL here when called from view_onPopupMenu;
	 *  gdk_event_get_time() accepts a NULL argument */
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL,
					0, gdk_event_get_time ((GdkEvent*) NULL));
		

}



static void
select_satellite        (GtkWidget *menuitem, gpointer data)
{
	GtkSingleSat *ssat = GTK_SINGLE_SAT (data);
	guint i = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (menuitem), "index"));
	gchar *title;
	sat_t *sat;


	/* there are many "ghost"-trigging of this signal, but we only need to make
	   a new selection when the received menuitem is selected
	*/
	if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menuitem))) {
		ssat->selected = i;

		sat = SAT (g_slist_nth_data (ssat->sats, i));

		title = g_strdup_printf ("<b>%s</b>", sat->tle.sat_name);
		gtk_label_set_markup (GTK_LABEL (ssat->header), title);
		g_free (title);

	}
}



/** \brief Refresh internal references to the satellites.
 */
void
gtk_single_sat_reload_sats (GtkWidget *single_sat, GHashTable *sats)
{

	/* free GSlists */
	g_slist_free (GTK_SINGLE_SAT (single_sat)->sats);
	GTK_SINGLE_SAT (single_sat)->sats = NULL;

	/* reload satellites */
	g_hash_table_foreach (sats, store_sats, single_sat);

}


/** \brief Reload configuration.
 *  \param widget The GtkSingleSat widget.
 *  \param newcfg The new configuration data for the module
 *  \param sats   The satellites.
 *  \param local  Flag indicating whether reconfiguration is requested from 
 *                local configuration dialog.
 */
void
gtk_single_sat_reconf          (GtkWidget    *widget,
								GKeyFile     *newcfg,
								GHashTable   *sats,
								qth_t        *qth,
								gboolean      local)
{
	guint32 fields;


	/* store pointer to new cfg data */
	GTK_SINGLE_SAT (widget)->cfgdata = newcfg;

	/* get visible fields from new configuration */
	fields = mod_cfg_get_int (newcfg,
							  MOD_CFG_SINGLE_SAT_SECTION,
							  MOD_CFG_SINGLE_SAT_FIELDS,
							  SAT_CFG_INT_SINGLE_SAT_FIELDS);

	if (fields != GTK_SINGLE_SAT (widget)->flags) {
		GTK_SINGLE_SAT (widget)->flags = fields;

		/* */
	}

	/* if this is a local reconfiguration sats may have changed */
	if (local) {
		gtk_single_sat_reload_sats (widget, sats);
	}

	/* QTH may have changed too since we have a default QTH */
	GTK_SINGLE_SAT (widget)->qth = qth;


	/* get refresh rate and cycle counter */
	GTK_SINGLE_SAT (widget)->refresh = mod_cfg_get_int (newcfg,
														MOD_CFG_SINGLE_SAT_SECTION,
														MOD_CFG_SINGLE_SAT_REFRESH,
														SAT_CFG_INT_SINGLE_SAT_REFRESH);
	GTK_SINGLE_SAT (widget)->counter = 1;
}

