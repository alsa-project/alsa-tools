#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <sys/asoundlib.h>

#define ICE1712_SUBDEVICE_DELTA1010	0x121430d6
#define ICE1712_SUBDEVICE_DELTADIO2496	0x121431d6
#define ICE1712_SUBDEVICE_DELTA66	0x121432d6
#define ICE1712_SUBDEVICE_DELTA44	0x121433d6

typedef struct {
	unsigned int subvendor;	/* PCI[2c-2f] */
	unsigned char size;	/* size of EEPROM image in bytes */
	unsigned char version;	/* must be 1 */
	unsigned char codec;	/* codec configuration PCI[60] */
	unsigned char aclink;	/* ACLink configuration PCI[61] */
	unsigned char i2sID;	/* PCI[62] */
	unsigned char spdif;	/* S/PDIF configuration PCI[63] */
	unsigned char gpiomask;	/* GPIO initial mask, 0 = write, 1 = don't */
	unsigned char gpiostate; /* GPIO initial state */
	unsigned char gpiodir;	/* GPIO direction state */
	unsigned short ac97main;
	unsigned short ac97pcm;
	unsigned short ac97rec;
	unsigned char ac97recsrc;
	unsigned char dacID[4];	/* I2S IDs for DACs */
	unsigned char adcID[4];	/* I2S IDs for ADCs */
	unsigned char extra[4];
} ice1712_eeprom_t;

extern snd_ctl_t *ctl;
extern ice1712_eeprom_t card_eeprom;

extern GtkWidget *mixer_mix_drawing;
extern GtkWidget *mixer_clear_peaks_button;
extern GtkWidget *mixer_drawing[20];
extern GtkObject *mixer_adj[20][2];
extern GtkWidget *mixer_vscale[20][2];
extern GtkWidget *mixer_solo_toggle[20][2];
extern GtkWidget *mixer_mute_toggle[20][2];
extern GtkWidget *mixer_stereo_toggle[20];

extern GtkWidget *router_radio[10][12];

extern GtkWidget *hw_master_clock_xtal_radio;
extern GtkWidget *hw_master_clock_spdif_radio;
extern GtkWidget *hw_master_clock_word_radio;
extern GtkWidget *hw_master_clock_status_label;

extern GtkObject *hw_volume_change_adj;
extern GtkWidget *hw_volume_change_spin;

extern GtkWidget *hw_spdif_profi_nonaudio_radio;
extern GtkWidget *hw_spdif_profi_audio_radio;

extern GtkWidget *hw_profi_stream_stereo_radio;
extern GtkWidget *hw_profi_stream_notid_radio;

extern GtkWidget *hw_profi_emphasis_none_radio;
extern GtkWidget *hw_profi_emphasis_5015_radio;
extern GtkWidget *hw_profi_emphasis_ccitt_radio;
extern GtkWidget *hw_profi_emphasis_notid_radio;

extern GtkWidget *hw_consumer_copyright_on_radio;
extern GtkWidget *hw_consumer_copyright_off_radio;

extern GtkWidget *hw_consumer_copy_1st_radio;
extern GtkWidget *hw_consumer_copy_original_radio;

extern GtkWidget *hw_consumer_emphasis_none_radio;
extern GtkWidget *hw_consumer_emphasis_5015_radio;

extern GtkWidget *hw_consumer_category_dat_radio;
extern GtkWidget *hw_consumer_category_pcm_radio;
extern GtkWidget *hw_consumer_category_cd_radio;
extern GtkWidget *hw_consumer_category_general_radio;

extern GtkWidget *hw_spdif_professional_radio;
extern GtkWidget *hw_spdif_consumer_radio;
extern GtkWidget *hw_spdif_output_notebook;

extern GtkWidget *hw_spdif_input_coaxial_radio;
extern GtkWidget *hw_spdif_input_optical_radio;


gint level_meters_configure_event(GtkWidget *widget, GdkEventConfigure *event);
gint level_meters_expose_event(GtkWidget *widget, GdkEventExpose *event);
gint level_meters_timeout_callback(gpointer data);
void level_meters_reset_peaks(GtkButton *button, gpointer data);
void level_meters_init(void);
void level_meters_postinit(void);

void mixer_update_stream(int stream, int vol_flag, int sw_flag);
void mixer_toggled_solo(GtkWidget *togglebutton, gpointer data);
void mixer_toggled_mute(GtkWidget *togglebutton, gpointer data);
void mixer_adjust(GtkAdjustment *adj, gpointer data);
void mixer_postinit(void);

int patchbay_stream_is_active(int stream);
void patchbay_update(void);
void patchbay_toggled(GtkWidget *togglebutton, gpointer data);
void patchbay_init(void);
void patchbay_postinit(void);

void master_clock_update(void);
void master_clock_toggled(GtkWidget *togglebutton, gpointer data);
gint master_clock_status_timeout_callback(gpointer data);
void volume_change_rate_update(void);
void volume_change_rate_adj(GtkAdjustment *adj, gpointer data);
void profi_data_toggled(GtkWidget *togglebutton, gpointer data);
void profi_stream_toggled(GtkWidget *togglebutton, gpointer data);
void profi_emphasis_toggled(GtkWidget *togglebutton, gpointer data);
void consumer_copyright_toggled(GtkWidget *togglebutton, gpointer data);
void consumer_copy_toggled(GtkWidget *togglebutton, gpointer data);
void consumer_emphasis_toggled(GtkWidget *togglebutton, gpointer data);
void consumer_category_toggled(GtkWidget *togglebutton, gpointer data);
void spdif_output_update(void);
void spdif_output_toggled(GtkWidget *togglebutton, gpointer data);
void spdif_input_update(void);
void spdif_input_toggled(GtkWidget *togglebutton, gpointer data);
void hardware_init(void);
void hardware_postinit(void);

void control_input_callback(gpointer data, gint source, GdkInputCondition condition);
void mixer_input_callback(gpointer data, gint source, GdkInputCondition condition);
