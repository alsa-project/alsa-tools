#include <glib.h>
#include <gtk/gtk.h>
#include <alsa/asoundlib.h>

typedef enum card_type
{
	NO_CARD,
	DIGI32,
	DIGI96,
	DIGI96_8,
	DIGI96_8_OTHER
}card_type_t;

typedef struct ctl_elem_info_val
{
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *val;
}ctl_elem_info_val_t;

extern snd_ctl_t *ctl;

GtkWidget *create_enum_elem_radio(char *elem_name,ctl_elem_info_val_t *iv);

GtkWidget *create_loopback_toggle();
GtkWidget *create_level_box();

static inline GtkWidget *__gtk_hbox_new(gboolean homogeneous, gint spacing)
{
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
  return box;
}
/* override for deprecation */
#define gtk_hbox_new	__gtk_hbox_new

static inline GtkWidget*__gtk_vbox_new(gboolean homogeneous, gint spacing)
{
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
  gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
  return box;
}
/* override for deprecation */
#define gtk_vbox_new	__gtk_vbox_new
