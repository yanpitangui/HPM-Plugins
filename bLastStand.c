//===== Hercules Plugin ======================================
//= bLastStand (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1a
//===== Description: =========================================
//= bonus bLastStand, n;
//= N% chance to survive a killing blow.
//= Example:
//= 	bonus bLastStand,10;
//===== Topic ================================================
//= https://herc.ws/board/topic/18498-blaststand/
//============================================================

#include "common/hercules.h"
#include "map/pc.h"
#include "map/battle.h"
#include "common/memmgr.h"
#include "common/random.h"
#include "common/utils.h"
#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h"
HPExport struct hplugin_info pinfo = {
	"bLastStand",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	int last_stand_chance;
};

int LAST_STAND = 0;

int pc_bonus_pre(struct map_session_data** sd, int *type, int *val) {
	if (*sd == NULL )
		return 0;
	if ( *type == LAST_STAND ) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			ssd->last_stand_chance = cap_value(*val, 0, 100);
			addToMSD(*sd, ssd, 0, true);
		}
		else {
			ssd->last_stand_chance = cap_value(*val + ssd->last_stand_chance, 0, 100);
		}
		hookStop();
	}
	return 0;
}
// flush bLastStand back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


int pc_dead_pre(struct map_session_data** sd, struct block_list** src) {
	if (*sd == NULL)
		return 0;
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd && ssd->last_stand_chance > rand() % 100) {
		hookStop();
		return 0;
	}
	return 1;
}

HPExport void plugin_init (void) {
	LAST_STAND = map->get_new_bonus_id();
	script->set_constant("bLastStand", LAST_STAND, false, false );
	addHookPre( pc, bonus, pc_bonus_pre);
	addHookPre(pc, dead, pc_dead_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
