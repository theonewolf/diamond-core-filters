#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/time.h>
#include <math.h>
#include <libgen.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>
#include "queue.h"
#include "rgb.h"
#include "img_search.h"

img_search::img_search(const char *name, char *descr)
{
	display_name = strdup(name);
	assert(display_name != NULL);

	descript = strdup(descr);
	assert(descript != NULL);

	search_selected = 0;
	hl_selected = 0;
	name_entry = NULL;
	search_label = NULL;
	adjust_label = NULL;
}


img_search::~img_search()
{
	free(display_name);
	free(descript);
	return;
}

char *
img_search::get_name()
{
	return(display_name);
}

int
img_search::add_patch(RGBImage *img, bbox_t bbox)
{
	return(0);
}

int
img_search::is_example()
{
	return(0);
}

static int
cleanup_name(char *name)
{
	int	modified = 0;
	unsigned int	i;
	for (i=0; i < strlen(name); i++) {
		if (name[i] == ' ') {
			name[i] = '_';
			modified = 1;
		}
	}
	return(modified);
}


int
img_search::set_name(const char *new_name)
{
	char *	newp;
	int	modified = 0;
	if (strcmp(new_name, display_name) != 0) {	
		newp = strdup(new_name);
		modified = cleanup_name(newp);
		if (modified) {
			printf("old <%s> new <%s> \n", new_name, newp);
		}
		free(display_name);
		display_name = newp;

		/* update the name in the display list */
		if (search_label != NULL) {
			gtk_label_set_text(GTK_LABEL(search_label), new_name);
		}
		if (adjust_label != NULL) {
			gtk_label_set_text(GTK_LABEL(adjust_label), new_name);
		}
	}
	return(modified);
}


int
img_search::is_selected()
{
	return(search_selected);
}

int
img_search::is_hl_selected()
{
	return(hl_selected);
}


void
img_search::set_active_val(int val)
{
	search_selected = val;
}

void
img_search::set_active_hlval(int val)
{
	hl_selected = val;
}

int
img_search::handle_config(config_types_t conf_type, char *data)
{
	/* XXX example search destruct */
	int		err;

	switch (conf_type) {
		default:
			err = 1;
			break;
	}
	return(err);
}


static void
edit_search_cb(GtkButton *item, gpointer data)
{
	img_search *obj = (img_search *)data;
	obj->edit_search();
}

static void
toggle_callback(GtkButton *item, gpointer data)
{
	img_search *obj = (img_search *)data;
	int		val;

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item));

	obj->set_active_val(val);

}


GtkWidget *
img_search::get_search_widget()
{
	GtkWidget * cb;
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 10);
	/* create the check box */
	cb = gtk_check_button_new();
	g_signal_connect(G_OBJECT(cb), "toggled",
                     G_CALLBACK(toggle_callback), this);
	gtk_widget_show(cb);
    	gtk_box_pack_start(GTK_BOX(hbox), cb, FALSE, FALSE, 0);
	gtk_widget_show(cb);

	search_label = gtk_label_new(display_name);
    	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);
	gtk_widget_show(search_label);

	gtk_widget_show(hbox);
	return(hbox);
}

static void
hl_toggle_callback(GtkButton *item, gpointer data)
{
	img_search *obj = (img_search *)data;
	int		val;

	val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item));

	obj->set_active_hlval(val);

}

GtkWidget *
img_search::get_highlight_widget()
{
	GtkWidget * cb;
	GtkWidget * hbox;

	hbox = gtk_hbox_new(FALSE, 10);
	/* create the check box */
	cb = gtk_check_button_new();
	g_signal_connect(G_OBJECT(cb), "toggled",
                     G_CALLBACK(hl_toggle_callback), this);
	gtk_widget_show(cb);
	gtk_box_pack_start(GTK_BOX(hbox), cb, FALSE, FALSE, 0);
	gtk_widget_show(cb);

	search_label = gtk_label_new(display_name);
	gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);
	gtk_widget_show(search_label);

	gtk_widget_show(hbox);
	return(hbox);
}


GtkWidget *
img_search::get_config_widget()
{
	GtkWidget * label;
	
	/* create label */
	label = gtk_label_new(descript);
	gtk_widget_show(label);

	return(label);	
}

static void
name_changed_cb(GtkButton *item, gpointer data)
{
	static int	pending_change = 0;
	if (pending_change != 0) {
		return;
	}

	pending_change = 1;
	img_search *obj = (img_search *)data;
	obj->save_edits();
	pending_change = 0;
}


GtkWidget *
img_search::img_search_display()
{
	GtkWidget *	widget;
	GtkWidget *	frame;
	GtkWidget *	container;
	GtkWidget *	box;

	//box = gtk_vbox_new(FALSE, 10);
	frame = gtk_frame_new("Search");
	//gtk_box_pack_start(GTK_BOX(box), frame, FALSE, FALSE, 10);
	
	container = gtk_vbox_new(FALSE, 10);
	gtk_container_add(GTK_CONTAINER(frame), container);

	box = gtk_hbox_new(FALSE, 10);
	gtk_box_pack_start(GTK_BOX(container), box, FALSE, FALSE, 0);
	gtk_widget_show(box);

	/* add a edit label */
	widget = gtk_label_new("Filter Name:");
	gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);
	gtk_widget_show(widget);

	/* add the text entry box */
	name_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(name_entry), get_name()); 
	gtk_box_pack_start(GTK_BOX(box), name_entry, FALSE, FALSE, 0);

	/* put callback to update changes to the name box */
	g_signal_connect(G_OBJECT(name_entry), "changed",
                   G_CALLBACK(name_changed_cb), this);

	gtk_widget_show_all(frame);

	/* XXX done button */	

	return (frame);
}


void
img_search::close_edit_win()
{
	save_edits();
	name_entry = NULL;
}



GtkWidget *
img_search::get_edit_widget()
{
	GtkWidget * ebutton;

	/* create the display label */
	ebutton = gtk_button_new_with_label("Edit");
	g_signal_connect(G_OBJECT(ebutton), "clicked",
                     G_CALLBACK(edit_search_cb), this);
   	GTK_WIDGET_SET_FLAGS(ebutton, GTK_CAN_DEFAULT);
   	gtk_widget_show (ebutton);
	return(ebutton);
}

void
img_search::save_edits()
{
	const char * 	name;
	int		ret;

	/* there are not active edits to save here */
	if (name_entry == NULL) {
		return;
	}

	name = gtk_entry_get_text(GTK_ENTRY(name_entry));
	ret = set_name(name);
	if (ret) {
		gtk_entry_set_text(GTK_ENTRY(name_entry), get_name());
	}	
	return;
}
