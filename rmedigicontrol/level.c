/*****************************************************************************
   level.c
   Copyright (C) 2003 by Robert Vetter <postmaster@robertvetter.com>
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*****************************************************************************/

#include "rmedigicontrol.h"

static snd_ctl_elem_value_t *val;
static snd_ctl_elem_info_t *info;
static char vlab[8];

static char *val2char(gdouble val)
{
	sprintf(vlab,"%2.0f",100-val);
	return(vlab);
}
static void changed(GtkAdjustment *a, gpointer p)
{
	double value = gtk_adjustment_get_value(a);
	int adjusted_value = ((100 - value) * snd_ctl_elem_info_get_max(info)) / 100;
	snd_ctl_elem_value_set_integer(val, 0, adjusted_value);
	snd_ctl_elem_value_set_integer(val, 1, adjusted_value);
	snd_ctl_elem_write(ctl, val);
	gtk_label_set_text(GTK_LABEL(p), val2char(value));
}

GtkWidget *create_level_box()
{
	GtkAdjustment *adjust;
	GtkWidget *box,*slider1,*label1,*vlabel;
	char *elem_name="DAC Playback Volume";
	box=gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

	snd_ctl_elem_info_malloc(&info);
	snd_ctl_elem_value_malloc(&val);

	snd_ctl_elem_info_set_interface(info,SND_CTL_ELEM_TYPE_INTEGER);
	snd_ctl_elem_info_set_name(info,elem_name);
	snd_ctl_elem_info_set_numid(info,0);
	snd_ctl_elem_info(ctl,info);

	snd_ctl_elem_value_set_interface(val,SND_CTL_ELEM_TYPE_INTEGER);
	snd_ctl_elem_value_set_name(val,elem_name);
	snd_ctl_elem_read(ctl,val);

	adjust = gtk_adjustment_new(
		100 - (snd_ctl_elem_value_get_integer(val, 0) * 100) / snd_ctl_elem_info_get_max(info),
		0,   // lower
		100, // upper
		1,   // step increment
		5,   // page increment
		0    // page size
	);
	vlabel = gtk_label_new(val2char(gtk_adjustment_get_value(adjust)));
	g_signal_connect(adjust,"value_changed",G_CALLBACK(changed),vlabel);
	slider1=gtk_scale_new(GTK_ORIENTATION_VERTICAL, adjust);
	gtk_scale_set_draw_value(GTK_SCALE(slider1),FALSE);
	gtk_scale_set_digits(GTK_SCALE(slider1),0);

	label1=gtk_label_new("Level");
	gtk_box_pack_start(GTK_BOX(box),label1,FALSE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(box),slider1,TRUE,TRUE,5);
	gtk_box_pack_start(GTK_BOX(box),vlabel,FALSE,TRUE,5);
	return box;
}
