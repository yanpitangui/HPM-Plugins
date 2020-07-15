//===== Hercules Plugin ======================================
//= bDeathKeepBuffs (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus bDeathKeepBuffs, n; (n is meaningless)
//= Allow for status to remain when player is killed.
//= Example:
//= 	bonus bDeathKeepBuffs,1;
//============================================================

#include "common/hercules.h"
#include "map/pc.h"
#include "map/battle.h"
#include "common/memmgr.h"
#include "common/utils.h"
#include "common/nullpo.h"
#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h"
HPExport struct hplugin_info pinfo = {
	"bDeathKeepBuffs",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	char keep_buffs;
};

int KEEP_BUFFS = 0;

int pc_bonus_pre(struct map_session_data** sd, int* type, int* val) {
	if (*sd == NULL)
		return 0;
	if (*type == KEEP_BUFFS) {
		struct player_data* ssd = getFromMSD(*sd, 0);
		if (ssd == NULL) {
			CREATE(ssd, struct player_data, 1);
			addToMSD(*sd, ssd, 0, true);
		}
		ssd->keep_buffs = 1;
		hookStop();
	}
	return 0;
}

int status_change_clear_pre(struct block_list** bl, int* type)
{
	if (*type != 0 || !*bl || !(*bl)->type == BL_PC)
		return 0;
	struct map_session_data* sd = BL_CAST(BL_PC, *bl);
	if (sd) 
	{
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd)
		{
			hookStop();
			return 1;
		}
	}
	return 0;
}

// flush bHealAttack back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


HPExport void plugin_init(void) {
	KEEP_BUFFS = map->get_new_bonus_id();
	script->set_constant("bDeathKeepBuffs", KEEP_BUFFS, false, false);
	addHookPre(status, change_clear, status_change_clear_pre);
	addHookPre(pc, bonus, pc_bonus_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
