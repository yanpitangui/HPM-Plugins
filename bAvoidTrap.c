//===== Hercules Plugin ======================================
//= bAvoidTrap (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus bAvoidTrap, n;
//= N% chance to avoid activating a trap when stepping on it.
//= Example:
//= 	bonus bAvoidTrap,10;
//===== Topic ================================================
//= https://herc.ws/board/topic/18498-blaststand/
//============================================================

#include "common/hercules.h"
#include "map/pc.h"
#include "map/battle.h"
#include "common/memmgr.h"
#include "common/random.h"
#include "common/nullpo.h"
#include "common/utils.h"
#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h"
HPExport struct hplugin_info pinfo = {
	"bAvoidTrap",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	int avoid_trap_chance;
};

int AVOID_TRAP = 0;

int pc_bonus_pre(struct map_session_data** sd, int *type, int *val) {
	if (*sd == NULL )
		return 0;
	if ( *type == AVOID_TRAP) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			ssd->avoid_trap_chance = cap_value(*val, 0, 100);
			addToMSD(*sd, ssd, 0, true);
		}
		else {
			ssd->avoid_trap_chance += cap_value(*val + ssd->avoid_trap_chance, 0, 100);
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


int skill_strap_splash_pre(struct block_list** bl, va_list ap){
	if (*bl == NULL || (*bl)->type != BL_PC)
		return 0;
	struct map_session_data* sd = BL_CAST(BL_PC, *bl);
	struct player_data* ssd = getFromMSD(sd, 0);
	if (ssd && ssd->avoid_trap_chance > rand() % 100) {
		hookStop();
		return 0;
	}
	return 1;
}

int skill_unit_onplace_timer(struct skill_unit** src, struct block_list** bl, int64* tick) {
	struct skill_unit_group* sg;
	struct block_list* ss;
	nullpo_ret(*src);
	nullpo_ret(*bl);

	if ((*bl)->prev == NULL || !(*src)->alive || status->isdead(*bl))
		return 0;

	nullpo_ret(sg = (*src)->group);
	nullpo_ret(ss = map->id2bl(sg->src_id));
	if (!*bl || (*bl)->type != BL_PC) return 0;
	struct map_session_data* sd = BL_CAST(BL_PC, *bl);
	nullpo_ret(sg = (*src)->group);
	if (sg->interval == -1) return 0;
	struct player_data* ssd = getFromMSD(sd, 0);
	switch (sg->unit_id) {
		case UNT_SKIDTRAP:
		case UNT_ANKLESNARE:
		case UNT_MANHOLE:
		case UNT_ELECTRICSHOCKER:
		case UNT_MAGENTATRAP:
		case UNT_COBALTTRAP:
		case UNT_MAIZETRAP:
		case UNT_VERDURETRAP:
		case UNT_FIRINGTRAP:
		case UNT_ICEBOUNDTRAP:
		case UNT_CLUSTERBOMB:
		case UNT_LANDMINE:
		case UNT_BLASTMINE:
		case UNT_SHOCKWAVE:
		case UNT_SANDMAN:
		case UNT_FLASHER:
		case UNT_FREEZINGTRAP:
		case UNT_CLAYMORETRAP:
		case UNT_TALKIEBOX:
		case UNT_DIMENSIONDOOR:
		case UNT_NETHERWORLD:
		case UNT_THORNS_TRAP:
		case UNT_VACUUM_EXTREME:
		case UNT_CHAOSPANIC:
		case UNT_BLOODYLUST:
			if (ssd && ssd->avoid_trap_chance > rand() % 100) {
				hookStop();
			}
			break;
		default:
			break;
	}
	return 0;

}

HPExport void plugin_init (void) {
	AVOID_TRAP = map->get_new_bonus_id();
	script->set_constant("bAvoidTrap", AVOID_TRAP, false, false );
	addHookPre( pc, bonus, pc_bonus_pre);
	addHookPre(skill, unit_onplace_timer, skill_unit_onplace_timer);
	addHookPre(skill, trap_splash, skill_strap_splash_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
