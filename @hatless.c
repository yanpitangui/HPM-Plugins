//===== Hercules Plugin ======================================
//= @hatless
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1.a
//===== Description: =========================================
//= Activate this command to, when you equip a head item or
// costume robe, it will hide it.
//===== Topic ================================================
//= https://herc.ws/board/topic/18488-hatless/
//============================================================


#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/strlib.h"
#include "login/login.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/map.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HPExport struct hplugin_info pinfo = {
	"Hatless",    // Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
};

ACMD(hatless) {//@hatless command - 5 params: const int fd, struct map_session_data* sd, const char* command, const char* message, struct AtCommandInfo *info
	toggle_hatless(fd, sd);
	return true;
}
struct hatless {
	int i;
};


/* toggle in memory switch to detect session hatless */
int toggle_hatless(int fd, struct map_session_data *sd) {
	struct hatless* data;
	if (!sd) return 0;

	/* socket didn't fully log-in? this packet shouldn't do anything then! */
	if (!(data = getFromMSD(sd, 0))) {
		CREATE(data, struct hatless, 1);
		addToMSD(sd, data, 0, false);
		data->i;
		char output[99];
		safesnprintf(output, 99, "Hatless está ligado, equipe seus itens para que eles nao apareçam.");
		clif->messagecolor_self(fd, COLOR_GREEN, output);
		return 1;
	}
	else {
		removeFromMSD(sd, 0);
		char output[99];
		safesnprintf(output, 99, "Hatless está desligado, equipe seus itens para que eles apareçam.");
		clif->messagecolor_self(fd, COLOR_GREEN, output);
		return 0;
	}
	return 0;
}


/* prehook for hatless */
void hatless_pre_clif_changeloook(struct block_list** bl, int *type, int *val) {
	struct hatless* data;
	struct map_session_data* sd;
	nullpo_retv(bl);
	sd = BL_CAST(BL_PC, *bl);
	if ((data = getFromMSD(sd, 0))) {
		switch (*type) {
		case LOOK_HEAD_BOTTOM:
			sd->status.look.head_bottom = *val = 0;
			break;
		case LOOK_HEAD_TOP:
			sd->status.look.head_top = *val = 0;
			break;
		case LOOK_HEAD_MID:
			sd->status.look.head_mid = *val = 0;
			break;
		case LOOK_ROBE:
			sd->status.look.robe = *val = 0;
			break;
		default:
			break;
		}
	}
}

/* run when server starts */
HPExport void plugin_init(void) {
	// add atcommand for hatless
	addAtcommand("hatless", hatless);

	/* Pre hook to changelook to make it think the value for top mid or bottom head is 0. */
	addHookPre(clif, changelook, hatless_pre_clif_changeloook);
}
