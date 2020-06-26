//===== Hercules Plugin ======================================
//= @mobalive
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= Simplification of mobsearch, to make it more fair, it
//= does not show de mob location.
//===== Topic ================================================
//= https://herc.ws/board/topic/18501-mobalive/
//============================================================


#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/strlib.h"
#include "common/timer.h"
#include "map/clif.h"
#include "map/mob.h"
#include "map/pc.h"
#include "map/map.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <string.h>

HPExport struct hplugin_info pinfo = {
	"@mobalive",    // Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
};

ACMD(mobalive) {
	char mob_name[100];
	int mob_id;
	int number = 0;
	struct s_mapiterator* it;
	const struct mob_data* md = NULL;
	char output[99];
	
	if (!*message || sscanf(message, "%99[^\n]", mob_name) < 1) {
		safesnprintf(output, 99, "Por favor, escreva o nome de um monstro (uso: @mobalive <nome do monstro>).");
		clif->message(fd, output); // Please enter a monster name (usage: @mobalive <monster name>).
		return false;
	}

	if ((mob_id = atoi(mob_name)) == 0)
		mob_id = mob->db_searchname(mob_name);
	if (mob_id > 0 && mob->db_checkid(mob_id) == 0) {
		safesnprintf(output, 99, "Mob %d não encontrado.", mob_id);
		clif->message(fd, output);
		return false;
	}
	if (mob_id == atoi(mob_name)) {
		strcpy(mob_name, mob->db(mob_id)->jname); // DEFAULT_MOB_JNAME
		//strcpy(mob_name,mob->db(mob_id)->name); // DEFAULT_MOB_NAME
	}

	safesnprintf(output, 99, "Mobalive... %s %s", mob_name, mapindex_id2name(sd->mapindex));
	clif->message(fd, output);

	it = mapit_geteachmob();
	for (md = BL_UCCAST(BL_MOB, mapit->first(it)); mapit->exists(it); md = BL_UCCAST(BL_MOB, mapit->next(it))) {
		if (md->bl.m != sd->bl.m)
			continue;
		if (mob_id != -1 && md->class_ != mob_id)
			continue;

		++number;
		safesnprintf(output, 99, "%2d [%s] %s", number, md->spawn_timer == INVALID_TIMER ? "Vivo" : "Morto", md->name);
		clif->message(fd, output);

	}
	mapit->free(it);

	return true;
}


/* run when server starts */
HPExport void plugin_init(void) {
	// add atcommand for mobalive
	addAtcommand("mobalive", mobalive);

}
