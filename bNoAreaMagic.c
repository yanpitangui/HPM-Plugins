//===== Hercules Plugin ======================================
//= bNoAreaMagic (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus bNoAreaMagic, n;
//= Reduce area target magic damage in n%.
//= Example:
//= 	bonus bNoAreaMagic,10;
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
	"bNoAreaMagic",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	int area_magic_resist;
};

int AREA_MAGIC_RESIST = 0;

int pc_bonus_pre(struct map_session_data** sd, int *type, int *val) {
	if (*sd == NULL )
		return 0;
	if ( *type == AREA_MAGIC_RESIST) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			ssd->area_magic_resist = cap_value(*val, 0, 100);
			addToMSD(*sd, ssd, 0, true);
		}
		else {
			ssd->area_magic_resist = cap_value(*val + ssd->area_magic_resist, 0, 100);
		}
		hookStop();
	}
	return 0;
}
// flush bNoAreaMagic back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}

int64 battle_calc_damage_post(int64 retVal___, struct block_list* src, struct block_list* bl, struct Damage* d, int64 damage, uint16 skill_id, uint16 skill_lv)
{
	if (!damage || !bl || bl->type != BL_PC) return retVal___;
	int flag = d->flag;
	if (flag & BF_MAGIC) {
		struct map_session_data* sd = BL_CAST(BL_PC, bl);
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd && skill->get_casttype(skill_id) == CAST_GROUND && ssd->area_magic_resist != 0) {
			retVal___ -= retVal___ * ssd->area_magic_resist / 100;
		}
	}
	return retVal___;
}

HPExport void plugin_init (void) {
	AREA_MAGIC_RESIST = map->get_new_bonus_id();
	script->set_constant("bNoAreaMagic", AREA_MAGIC_RESIST, false, false );
	addHookPre( pc, bonus, pc_bonus_pre);
	addHookPost( battle, calc_damage, battle_calc_damage_post);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
