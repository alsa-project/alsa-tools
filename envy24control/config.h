#ifndef CONFIG__H
#define CONFIG__H

void config_open();
void config_close();
void config_set_stereo(GtkWidget *but, gpointer data);
void config_restore_stereo();

#endif
