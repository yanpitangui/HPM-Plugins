//===== Hercules Plugin ======================================
//= bElementDamage (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus2 bElementDamage, n, x;
//= Increase damage when using element n by x%;
//= Example:
//= 	bonus bElementDamage, 1, 10;
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
	"bElementDamage",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	struct {
		int damage;
	} status[ELE_MAX];
};

int ELEMENT_DAMAGE = 0;

int pc_bonus2_pre(struct map_session_data** sd, int* type, int* type2, int* val) {
	if (*sd == NULL)
		return 0;
	if (*type == ELEMENT_DAMAGE) {
		struct player_data* ssd = getFromMSD(*sd, 0);
		if (ssd == NULL) {
			CREATE(ssd, struct player_data, 1);
			addToMSD(*sd, ssd, 0, true);
		}
		int i = *type2;
		if (i >= ELE_MAX) {
			ShowWarning("bElementDamage: Element type %d not found.", i);
			return 0;
		}
		ssd->status[i].damage += *val;
		hookStop();
	}
	return 0;
}

int64 battle_attr_fix_pre(struct block_list** src, struct block_list** target, int64* damage, int* atk_elem, int* def_type, int* def_lv) {
	if (!*src || (*src)->type != BL_PC) return 0;
	struct map_session_data* sd;
	sd = BL_CAST(BL_PC, *src);
	if (sd) {
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd && ssd->status[*atk_elem].damage) {
			*damage += (*damage * ssd->status[*atk_elem].damage / 100);
		}
	}
	return *damage;
}


// flush bElementDamage back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


HPExport void plugin_init(void) {
	ELEMENT_DAMAGE = map->get_new_bonus_id();
	script->set_constant("bElementDamage", ELEMENT_DAMAGE, false, false);
	addHookPre(battle, attr_fix, battle_attr_fix_pre);
	addHookPre(pc, bonus2, pc_bonus2_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
