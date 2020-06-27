//===== Hercules Plugin ======================================
//= bHealAttack (from Epoque Expansion Pack)
//===== By: ==================================================
//= Yan Pitangui
//===== Current Version: =====================================
//= 0.1
//===== Description: =========================================
//= bonus bHealAttack, n; (n is meaningless)
//= Causes Heal to become attack type regardless of Undead
//= element/race
//= Example:
//= 	bonus bHealAttack,1;
//===== Topic ================================================
//= https://herc.ws/board/topic/18511-bhealattack/
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
	"bHealAttack",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	char heal_attack;
};

int HEAL_ATTACK = 0;

int pc_bonus_pre(struct map_session_data** sd, int *type, int *val) {
	if (*sd == NULL )
		return 0;
	if ( *type == HEAL_ATTACK) {
		struct player_data *ssd = getFromMSD( *sd, 0 );
		if ( ssd == NULL ) {
			CREATE( ssd, struct player_data, 1 );
			addToMSD(*sd, ssd, 0, true);
		}
		ssd->heal_attack = 1;
		hookStop();
	}
	return 0;
}

int skill_castend_nodamage_id_pre(struct block_list** src, struct block_list** bl, uint16* skill_id, uint16* skill_lv, int64* tick, int* flag) {
	if (!(*src) || (*src)->type != BL_PC) {
		return 0;
	}

	if (*src != *bl && status->isdead(*bl)) { return 0; }
	struct map_session_data* sd = BL_CAST(BL_PC, (*src));
	if (sd)
	{
		struct status_data* tstatus;
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd)
		{
			tstatus = status->get_status_data(*bl);
			switch (*skill_id)
			{
			case AL_HEAL:
			case AB_RENOVATIO:
			case AB_HIGHNESSHEAL:
				// the original function already takes care of that.
				if (battle->check_undead(tstatus->race, tstatus->def_ele))
				{
					return 0;
				}
				if (battle->check_target(*src, *bl, BCT_ENEMY) > 0) {
					skill->castend_damage_id(*src, *bl, *skill_id, *skill_lv, *tick, *flag);
					hookStop();
				}
				break;
			default:
				return 0;
			}
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


HPExport void plugin_init (void) {
	HEAL_ATTACK = map->get_new_bonus_id();
	script->set_constant("bHealAttack", HEAL_ATTACK, false, false );
	addHookPre(skill, castend_nodamage_id, skill_castend_nodamage_id_pre);
	addHookPre( pc, bonus, pc_bonus_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
