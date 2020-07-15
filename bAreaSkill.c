//===== Hercules Plugin ========================================
//= bAreaSkill (from Epoque Expansion Pack)
//===== By: ====================================================
//= Yan Pitangui
//===== Current Version: =======================================
//= 0.1
//===== Description: ===========================================
//= bonus3 bAreaSkill,n,x,y;
//= y% chance for skill (skill_id) n to be AoE in y*y cells 
//= Example:
//= 	bonus3 bAreaSkill,271,100,7;
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
	"bAreaSkill",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	struct {
		int skill_id;
		short rate, range;
	} area_skill[MAX_PC_BONUS];
};

int AREA_SKILL = 0;

int pc_bonus3_pre(struct map_session_data** sd, int* type, int* type2, int* type3, int* val) {
	if (*sd == NULL)
		return 0;
	int i;
	if (*type == AREA_SKILL) {
		hookStop();
		struct player_data* ssd = getFromMSD(*sd, 0);
		if (ssd == NULL) {
			CREATE(ssd, struct player_data, 1);
			addToMSD(*sd, ssd, 0, true);
		}
		ARR_FIND(0, MAX_PC_BONUS, i, ssd->area_skill[i].skill_id == 0);
		if (*val == 0) {
			ShowWarning("pc_bonus3: bAreaSkill-> Invalid range specified (%d)\n", *val);
			return 0;
		}

		if (i == MAX_PC_BONUS) {
			ShowWarning("pc_bonus3: bAreaSkill-> Reached maximum area skills (%d)\n", MAX_PC_BONUS);
			return 0;
		}

		if (skill->get_splash(*type2, 1) != 0) {
			ShowWarning("pc_bonus3: bAreaSkill-> Specified skill already has area effect\n");
			return 0;
		}
		ssd->area_skill[i].skill_id = *type2;
		ssd->area_skill[i].rate = cap_value(*type3, 0, 100);
		ssd->area_skill[i].range = cap_value(*val, 0, SHRT_MAX);
	}
	return 0;
}
// flush bAreaSkill back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


int castend_damage_id_pre(struct block_list** src, struct block_list** bl, uint16* skill_id, uint16* skill_lv, int64* tick, int* flag) {
	struct map_session_data* sd;
	if (!*src) return 0;

	if ((*src)->type == BL_PC && !(*flag & 1)) {
		sd = BL_CAST(BL_PC, *src);
		if (sd)
		{
			struct player_data* ssd = getFromMSD(sd, 0);
			if (ssd) {
				int i;
				ARR_FIND(0, MAX_PC_BONUS, i, ssd->area_skill[i].skill_id == *skill_id);
				if (i < MAX_PC_BONUS && ssd->area_skill[i].rate > rand() % 100)
				{
					int target = ((skill->get_inf(*skill_id) & INF_SELF_SKILL) || (skill->get_inf(*skill_id) & INF_SUPPORT_SKILL)) ? BCT_PARTY : BCT_ENEMY;

					if (target == BCT_PARTY && map_flag_gvg((*src)->m))
						target |= BCT_GUILD;
					map->foreachinrange(skill->area_sub, *bl, ssd->area_skill[i].range, skill->splash_target(*src),
						*src, *skill_id, *skill_lv, *tick, *flag | target | 1, skill->castend_damage_id);
					hookStop();
				}
			}
		}
	}
	return 1;
}

int castend_nodamage_id_pre(struct block_list** src, struct block_list** bl, uint16* skill_id, uint16* skill_lv, int64* tick, int* flag) {
	struct map_session_data* sd;
	if (!*src) return 0;
	if ((*src)->type == BL_PC && !(*flag & 1))
	{
		sd = BL_CAST(BL_PC, *src);
		if (sd)
		{
			struct player_data* ssd = getFromMSD(sd, 0);
			if (ssd) {
				int i;
				ARR_FIND(0, MAX_PC_BONUS, i, ssd->area_skill[i].skill_id == *skill_id);
				if (i < MAX_PC_BONUS && ssd->area_skill[i].rate > rand() % 100)
				{
					int target = ((skill->get_inf(*skill_id) & INF_SELF_SKILL) || (skill->get_inf(*skill_id) & INF_SUPPORT_SKILL)) ? BCT_PARTY : BCT_ENEMY;

					if (target == BCT_PARTY && map_flag_gvg((*src)->m))
						target |= BCT_GUILD;
					map->foreachinrange(skill->area_sub, *bl, ssd->area_skill[i].range, skill->splash_target(*src),
						*src, *skill_id, *skill_lv, *tick, *flag | target | 1, skill->castend_nodamage_id);
					hookStop();
				}
			}
		}
	}
	return 1;
}

HPExport void plugin_init(void) {
	AREA_SKILL = map->get_new_bonus_id();
	script->set_constant("bAreaSkill", AREA_SKILL, false, false);
	addHookPre(pc, bonus3, pc_bonus3_pre);
	addHookPre(skill, castend_damage_id, castend_damage_id_pre);
	addHookPre(skill, castend_nodamage_id, castend_nodamage_id_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
