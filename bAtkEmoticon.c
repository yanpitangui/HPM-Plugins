//===== Hercules Plugin ========================================
//= bAtkEmoticon (from Epoque Expansion Pack)
//===== By: ====================================================
//= Yan Pitangui
//===== Current Version: =======================================
//= 0.1a
//===== Description: ===========================================
//= bonus3 bAtkEmoticon,n,x,y;
//= x% chance to use emoticon n when :
//=						y:	1 = Attacking
//=							2 = Being attacked
//=							3 = Both
//= Max of 5 emoticons
//= Example:
//= 	bonus3 bAtkEmoticon,100,2,1;
//===== Topic ==================================================
//= https://herc.ws/board/topic/18500-batkemoticon/
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
	"bAtkEmoticon",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	struct {
		char emotion;
		short rate, flag;
	} show_emoticon[5];
};

int ATK_EMOTICON = 0;

int pc_bonus3_pre(struct map_session_data** sd, int* type, int* type2, int* type3, int* val) {
	if (*sd == NULL)
		return 0;
	int i;
	if (*type == ATK_EMOTICON) {
		struct player_data* ssd = getFromMSD(*sd, 0);
		if (ssd == NULL) {
			CREATE(ssd, struct player_data, 1);
			addToMSD(*sd, ssd, 0, true);
		}
		ARR_FIND(0, 5, i, ssd->show_emoticon[i].rate == 0 || ssd->show_emoticon[i].emotion == *type3);
		ssd->show_emoticon[i].rate += *type2;
		ssd->show_emoticon[i].emotion = *type3;
		ssd->show_emoticon[i].flag |= cap_value(*val, 0, 3);
		hookStop();
	}
	return 0;
}
// flush bAtkEmoticon back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}


int additional_effect_post(int retVal___, struct block_list* src, struct block_list* bl, uint16 skill_id, uint16 skill_lv, int attack_type, int dmg_lv, int64 tick) {
	struct map_session_data* sd, * dstsd;
	if (!src || !bl || !(src->type == BL_PC || bl->type == BL_PC)) return 0;

	// Show emoticon for attacker
	if (src && src->type == BL_PC) {
		sd = BL_CAST(BL_PC, src);
		if (sd)
		{
			struct player_data* ssd = getFromMSD(sd, 0);
			if (ssd) {
				int i;
				for (i = 0; ARRAYLENGTH(ssd->show_emoticon) > i; i++)
				{
					if (ssd->show_emoticon[i].flag & 1 &&
						ssd->show_emoticon[i].rate > rand() % 100)
					{
						clif->emotion(&sd->bl, ssd->show_emoticon[i].emotion);
						break;
					}
				}
			}

		}
	}


	if (bl && bl->type == BL_PC) {
		dstsd = BL_CAST(BL_PC, bl);
		// show emoticon for target
		if (dstsd)
		{
			struct player_data* stsd = getFromMSD(dstsd, 0);
			if (stsd) {
				int i;
				for (i = 0; ARRAYLENGTH(stsd->show_emoticon) > i; i++)
				{
					if (stsd->show_emoticon[i].flag & 2 &&
						stsd->show_emoticon[i].rate > rand() % 100)
					{
						clif->emotion(&dstsd->bl, stsd->show_emoticon[i].emotion);
						break;
					}
				}
			}
		}
	}



	return 1;
}

HPExport void plugin_init(void) {
	ATK_EMOTICON = map->get_new_bonus_id();
	script->set_constant("bAtkEmoticon", ATK_EMOTICON, false, false);
	addHookPre(pc, bonus3, pc_bonus3_pre);
	addHookPost(skill, additional_effect, additional_effect_post);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
