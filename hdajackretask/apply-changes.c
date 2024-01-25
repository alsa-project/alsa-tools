/* Copyright 2011 David Henningsson, Canonical Ltd.
   License: GPLv2+ 
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <glib/gstdio.h>

#include "apply-changes.h"

static gchar* tempdir = NULL;
static gchar* scriptfile = NULL;
static gchar* errorfile = NULL;

struct soundserver {
    enum {
        PULSEAUDIO,
        PIPEWIRE
    } type;
    gboolean was_killed;
    gchar *user;
};

static GQuark quark()
{
    return g_quark_from_static_string("hda-jack-retask-error");
}

static gboolean ensure_tempdir(GError** err)
{
    if (!tempdir) {
        tempdir = g_dir_make_tmp("hda-jack-retask-XXXXXX", err);
        if (!tempdir)
            return FALSE;
        scriptfile = g_strdup_printf("%s/script.sh", tempdir);
        errorfile = g_strdup_printf("%s/errors.log", tempdir);
    }
    g_unlink(errorfile); /* Ignore file does not exist error */
    return TRUE;
}

static gboolean create_reconfig_script(pin_configs_t* pins, int entries, int card, int device, 
    const char* model, const char* hints, GError** err)
{
    gchar* hwdir = g_strdup_printf("/sys/class/sound/hwC%dD%d", card, device);
    gchar destbuf[150*40] = "#!/bin/sh\n";
    int bufleft = sizeof(destbuf) - strlen(destbuf);
    gboolean ok = FALSE;
    gchar* s = destbuf + strlen(destbuf);

    if (!ensure_tempdir(err))
        goto cleanup;

    if (model) {
        int l = g_snprintf(s, bufleft, "echo \"%s\" | tee %s/modelname 2>>%s\n",
            model, hwdir, errorfile);
        bufleft-=l;
        s+=l;
    }

    if (hints) {
        int l = g_snprintf(s, bufleft, "echo \"%s\" | tee %s/hints 2>>%s\n",
            hints, hwdir, errorfile);
        bufleft-=l;
        s+=l;
    }

    while (entries) {
        int l = g_snprintf(s, bufleft, "echo \"0x%02x 0x%08x\" | tee %s/user_pin_configs 2>>%s\n",
            pins->nid, (unsigned int) actual_pin_config(pins), hwdir, errorfile);
        bufleft-=l;
        s+=l;
        pins++;
        entries--;
    }    

    if (bufleft < g_snprintf(s, bufleft, "echo 1 | tee %s/reconfig 2>>%s", hwdir, errorfile)) {
        g_set_error(err, quark(), 0, "Bug in %s:%d!", __FILE__, __LINE__);
        goto cleanup;
    }

    if (!g_file_set_contents(scriptfile, destbuf, -1, err))
        goto cleanup;
    
    ok = TRUE;
cleanup:
    g_free(hwdir);
    return ok;
}


//#define SUDO_COMMAND "gksudo --description \"Jack retasking\""
#define SUDO_COMMAND "pkexec"

gboolean run_sudo_script(const gchar* script_name, GError** err)
{
    gchar* errfilecontents = NULL;
    gchar* cmdline = g_strdup_printf("%s %s", SUDO_COMMAND, script_name);
    int exit_status;
    gsize errlen;
    gboolean ok;

    g_chmod(script_name, 0755);
    g_spawn_command_line_sync(cmdline, NULL, NULL, &exit_status, NULL);
    if (errorfile && g_file_get_contents(errorfile, &errfilecontents, &errlen, NULL) && errlen) {
        g_set_error(err, quark(), 0, "%s", errfilecontents);
        ok = FALSE;
    }
    else ok = TRUE;

    g_free(errfilecontents);
    g_free(cmdline);
    return ok;
}

static gchar* get_pulseaudio_client_conf()
{
    /* Reference: See src/pulsecore/core-util.c in pulseaudio */
    gchar* fname;
    gchar* dir = g_strdup_printf("%s/.pulse", g_get_home_dir());
    if (access(dir, F_OK) < 0) {
	const gchar* xch = g_getenv("XDG_CONFIG_HOME");
	g_free(dir);
	if (xch)
	    dir = g_strdup_printf("%s/pulse", xch);
	else
	    dir = g_strdup_printf("%s/.config/pulse", g_get_home_dir());
    }
    fname = g_strdup_printf("%s/client.conf", dir);
    g_free(dir);
    return fname;
}

static gboolean call_systemctl(gchar* user, gchar* operation, gchar *unit, GError **err)
{
    gchar* s;
    gboolean ok;

    if (getuid() == 0) {
        // special case for root
        // XDG_RUNTIME_DIR setup seems to be mandatory for Fedora, may differ for other distros
        s = g_strdup_printf("runuser -l %s -c 'XDG_RUNTIME_DIR=/var/run/user/$(id -u) systemctl --user %s %s'", user, operation, unit);
    } else {
        s = g_strdup_printf("systemctl --user %s %s", operation, unit);
    }
    ok = g_spawn_command_line_sync(s, NULL, NULL, NULL, err);
    g_free(s);
    return ok;
}

static gboolean kill_soundserver(struct soundserver* state, int card, GError** err)
{
    gchar* fuser = NULL, *fuser2 = NULL;
    gchar* s = NULL;
    gchar* clientconf = NULL;
    gboolean ok;
    char *p;
    state->type = PULSEAUDIO;
    state->was_killed = FALSE;
    state->user = NULL;
    /* Is PA having a lock on the sound card? */
    s = g_strdup_printf("fuser -v /dev/snd/controlC%d", card);
    /* Due to some bug in fuser, stdout and stderr output is unclear. Better check both. */
    if (!(ok = g_spawn_command_line_sync(s, &fuser, &fuser2, NULL, err))) 
        goto cleanup;
    if (strstr(fuser, "pulseaudio") != NULL || strstr(fuser2, "pulseaudio") != NULL) {
        clientconf = get_pulseaudio_client_conf();
        if (!(ok = !g_file_test(clientconf, G_FILE_TEST_EXISTS))) {
            g_set_error(err, quark(), 0, "Cannot block PulseAudio from respawning:\n"
                "Please either remove '%s' or kill PulseAudio manually.", clientconf);
            goto cleanup;
        }
        
        if (!(ok = g_file_set_contents(clientconf, "autospawn=no\n", -1, err)))
            goto cleanup;
        state->was_killed = TRUE;
        ok = g_spawn_command_line_sync("pulseaudio -k", NULL, NULL, NULL, err);
    } else if ((p = strstr(fuser, "wireplumber")) != NULL || (p = strstr(fuser2, "wireplumber")) != NULL) {
        *p = '\0';
        while (p != fuser && p != fuser2 && *p != '\n')
            p--;
        if (*p == '\n')
            p++;
        
        GRegex *regex;
        GMatchInfo *match_info;

        regex = g_regex_new (" ([a-zA-Z0-9_-]+) ", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, NULL);
        g_regex_match (regex, p, 0, &match_info);
        if (g_match_info_matches (match_info))
            state->user = g_match_info_fetch (match_info, 1);
        g_match_info_free (match_info);
        g_regex_unref (regex);
        
        state->type = PIPEWIRE;
        ok = call_systemctl(state->user, "stop", "wireplumber.service", err);
        state->was_killed = ok;
    } else {
        // Sound server not locking the sound card
    }

cleanup:
    g_free(clientconf);
    g_free(fuser);
    g_free(fuser2);
    g_free(s);
    return ok;
}

static gboolean restore_soundserver(struct soundserver* state, GError** err)
{
    gboolean ok = FALSE;
    switch (state->type) {
        case PULSEAUDIO:
            gchar* clientconf = get_pulseaudio_client_conf();
            if (state->was_killed && g_unlink(clientconf) != 0) {
                g_set_error(err, quark(), 0, "%s", g_strerror(errno));
                g_free(clientconf);
                goto cleanup;
            }
            g_free(clientconf);
            ok = TRUE;
            break;
        case PIPEWIRE:
            if (state->was_killed)
                ok = call_systemctl(state->user, "start", "wireplumber.service", err);
            else
                ok = TRUE;
            break;
    }

cleanup:
    g_free(state->user);
    state->user = NULL;
    return ok;
}

gboolean apply_changes_reconfig(pin_configs_t* pins, int entries, int card, int device, 
    const char* model, const char* hints, GError** err)
{
    gboolean result = FALSE;
//    gchar* script_name = NULL;
    struct soundserver state = { 0 };
    /* Check for users of the sound card */
    /* Kill pulseaudio if necessary (and possible) */
    if (!kill_soundserver(&state, card, err))
        goto cleanup;
    /* Create script */
    if (!create_reconfig_script(pins, entries, card, device, model, hints, err))
        goto cleanup;
    /* Run script as root */
    if (!run_sudo_script(scriptfile, err))
        goto cleanup;
    result = TRUE;
cleanup:
    if (!restore_soundserver(&state, result ? err : NULL)) {
        result = FALSE;
    }
//    g_free(script_name);
    return result;
}

static gboolean create_firmware_file(pin_configs_t* pins, int entries, int card, int device, 
    const char* model, const char* hints, GError** err)
{
    gboolean ok;
    gchar destbuf[40*40+40*24] = "";
    gchar* s = destbuf;
    gchar* filename = g_strdup_printf("%s/hda-jack-retask.fw", tempdir);
    unsigned int address, codec_vendorid, codec_ssid;
    int bufleft = sizeof(destbuf);
    int l;

    get_codec_header(card, device, &address, &codec_vendorid, &codec_ssid);
    l = g_snprintf(s, bufleft, "[codec]\n0x%08x 0x%08x %u\n\n[pincfg]\n", codec_vendorid, codec_ssid, address);
    bufleft -= l;
    s += l;

    while (entries) {
        l = g_snprintf(s, bufleft, "0x%02x 0x%08x\n", pins->nid, (unsigned int) actual_pin_config(pins));
        bufleft -= l;
        s += l;
        pins++;
        entries--;
    }

    if (model) {
        int l = g_snprintf(s, bufleft, "\n[model]\n%s\n", model);
        bufleft-=l;
        s+=l;
    }

    if (hints) {
        int l = g_snprintf(s, bufleft, "\n[hints]\n%s\n", hints);
        bufleft-=l;
        s+=l;
    }

    ok = g_file_set_contents(filename, destbuf, -1, err);
    g_free(filename);
    return ok;
}


static const gchar* remove_script =
"#!/bin/sh\n"
"rm /etc/modprobe.d/hda-jack-retask.conf 2>>%s\n"
"rm /lib/firmware/hda-jack-retask.fw 2>>%s\n";

static const gchar* retask_conf = 
"# This file was added by the program 'hda-jack-retask'.\n"
"# If you want to revert the changes made by this program, you can simply erase this file and reboot your computer.\n"
"options snd-hda-intel patch=hda-jack-retask.fw,hda-jack-retask.fw,hda-jack-retask.fw,hda-jack-retask.fw\n";

static const gchar* install_script =
"#!/bin/sh\n"
"mv %s/hda-jack-retask.fw /lib/firmware/hda-jack-retask.fw\n 2>>%s\n"
"mv %s/hda-jack-retask.conf /etc/modprobe.d/hda-jack-retask.conf 2>>%s\n";

gboolean apply_changes_boot(pin_configs_t* pins, int entries, int card, int device, 
    const char* model, const char* hints, GError** err)
{
    gchar *s;

    if (!ensure_tempdir(err))
        return FALSE;

    if (!create_firmware_file(pins, entries, card, device, model, hints, err))
        return FALSE;

    /* Create hda-jack-retask.conf */
    s = g_strdup_printf("%s/hda-jack-retask.conf", tempdir);
    if (!g_file_set_contents(s, retask_conf, -1, err)) {
        g_free(s);
        return FALSE;
    }
    g_free(s);

    /* Create install script */    
    s = g_strdup_printf(install_script, tempdir, errorfile, tempdir, errorfile);
    if (!g_file_set_contents(scriptfile, s, -1, err)) {
        g_free(s);
        return FALSE;
    }
    g_free(s);

    /* Run script as root */
    if (!run_sudo_script(scriptfile, err))
        return FALSE;
    return TRUE;
}

gboolean reset_changes_boot(GError** err)
{
    gchar *s;

    if ((g_file_test("/etc/modprobe.d/hda-jack-retask.conf", G_FILE_TEST_EXISTS) == 0) &&
        (g_file_test("/lib/firmware/hda-jack-retask.fw", G_FILE_TEST_EXISTS) == 0))
    {
        g_set_error(err, quark(), 0, "No boot override is currently installed, nothing to remove.");
        return FALSE;
    }

    if (!ensure_tempdir(err))
        return FALSE;
    s = g_strdup_printf(remove_script, errorfile, errorfile);
    if (!g_file_set_contents(scriptfile, s, -1, err)) {
        g_free(s);
        return FALSE;
    }
    g_free(s);

    /* Run script as root */
    if (!run_sudo_script(scriptfile, err))
        return FALSE;
    return TRUE;
}

