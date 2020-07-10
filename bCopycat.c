//===== Hercules Plugin ========================================
//= bCopycat (from Epoque Expansion Pack)
//===== By: ====================================================
//= Yan Pitangui
//===== Current Version: =======================================
//= 0.1b
//===== Description: ===========================================
//= bonus bCopycat, n;
//= n% chance to auto-cast the last skill used on you back to 
// the caster. 
// Skills that doesn't work: TETRA VORTEX, CROSS RIPPER SLASHER, 
// ROLLING CUTTER and more due to their nature. Most of the 
// skills should work.
//= Example:
//= 	bonus bCopycat,10;
//===== Topic ==================================================
//= https://herc.ws/board/topic/18499-bcopycat/
//==============================================================

#include "common/hercules.h"
#include "map/pc.h"
#include "map/battle.h"
#include "common/memmgr.h"
#include "common/random.h"
#include "common/utils.h"
#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h"
HPExport struct hplugin_info pinfo = {
	"bCopycat",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	int copy_cat_chance;
};

int COPY_CAT = 0;

int pc_bonus_pre(struct map_session_data** sd, int *type, int *val) {
	if (*sd == NULL )
		return 0;
	if ( *type == COPY_CAT) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			ssd->copy_cat_chance = cap_value(*val, 0, 100);
			addToMSD(*sd, ssd, 0, true);
		}
		else {
			ssd->copy_cat_chance = cap_value(*val + ssd->copy_cat_chance, 0, 100);
		}
		hookStop();
	}
	return 0;
}
// flush bCopycat back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


int additional_effect_post(int retVal___, struct block_list* src, struct block_list* bl, uint16 skill_id, uint16 skill_lv, int attack_type, int dmg_lv, int64 tick) {
	if (!bl || bl->type != BL_PC) {
		return 0;
	}
	struct map_session_data * dstsd;
	dstsd = BL_CAST(BL_PC, bl);
	struct player_data* ssd = getFromMSD(dstsd, 0);
	if (ssd && skill_id > 0 && dstsd && src && !status->isdead(src) &&
		ssd->copy_cat_chance > rand() % 100 &&
		!skill->not_ok(skill_id, dstsd) && battle->check_range(bl, src, skill->get_range2(bl, skill_id, skill_lv))
			)
		{
			switch (skill->get_casttype(skill_id))
			{
				case CAST_GROUND:
					skill->castend_pos2(bl, src->x, src->y, skill_id, skill_lv, tick, 0);
					break;
				case CAST_NODAMAGE:
					skill->castend_nodamage_id(bl, src, skill_id, skill_lv, tick, 0);
					break;
				case CAST_DAMAGE:
					if (skill_id == WL_CHAINLIGHTNING_ATK) skill_id = WL_CHAINLIGHTNING;
					if (skill_id == WL_TETRAVORTEX_FIRE || skill_id == WL_TETRAVORTEX_GROUND || skill_id == WL_TETRAVORTEX_WATER || skill_id == WL_TETRAVORTEX_WIND) {
						break;
					}
					skill->castend_damage_id(bl, src, skill_id, skill_lv, tick, 0);
					break;
			}
	}
	return 1;
}

HPExport void plugin_init (void) {
	COPY_CAT = map->get_new_bonus_id();
	script->set_constant("bCopycat", COPY_CAT, false, false );
	addHookPre( pc, bonus, pc_bonus_pre);
	addHookPost(skill, additional_effect, additional_effect_post);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
