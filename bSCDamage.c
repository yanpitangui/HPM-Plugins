//===== Hercules Plugin ======================================
//= bSCDamage (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus2 bSCDamage, n, x;
//= Increase damage on enemy inflicted with status n by x%.
//= Example:
//= 	bonus bSCDamage, SC_FREEZE, 10;
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
	"bSCDamage",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	struct {
		int damage;
		int effect;
	} status[MAX_PC_BONUS];
};

int STATUS_DAMAGE = 0;

int pc_bonus2_pre(struct map_session_data** sd, int* type, int* type2, int* val) {
	if (*sd == NULL )
		return 0;
	if ( *type == STATUS_DAMAGE) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			addToMSD(*sd, ssd, 0, true);
		}
		int i;
		ARR_FIND(0, MAX_PC_BONUS, i, ssd->status[i].effect == *type2 || ssd->status[i].damage == 0);
		ssd->status[i].effect = *type2;
		ssd->status[i].damage += *val;
		hookStop();
	}
	return 0;
}


int64 battle_calc_damage_pre(struct block_list** src, struct block_list** bl, struct Damage** d, int64* damage, uint16* skill_id, uint16* skill_lv)
{
	if (!*src || (*src)->type != BL_PC || !*bl) return 0;
	if (!*damage)
		return 0;
	struct map_session_data* sd;
	struct status_change* sc;
	sd = BL_CAST(BL_PC, *src);
	sc = status->get_sc(*bl);
	if (sd) {
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd) {
			int i;
			for (i = 0; i < ARRAYLENGTH(ssd->status); i++) {
				if (ssd->status[i].damage && SC_MAX > ssd->status[i].effect && sc->data[ssd->status[i].effect]) {
					*damage += (*damage * ssd->status[i].damage / 100);
				}
			}
		}
	}
	return 0;
}


// flush bSCDamage back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


HPExport void plugin_init (void) {
	STATUS_DAMAGE = map->get_new_bonus_id();
	script->set_constant("bSCDamage", STATUS_DAMAGE, false, false );
	addHookPre(battle, calc_damage, battle_calc_damage_pre);
	addHookPre( pc, bonus2, pc_bonus2_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
