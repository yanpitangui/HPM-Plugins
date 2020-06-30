//===== Hercules Plugin ========================================
//= bAtkEmoticon (from Epoque Expansion Pack)
//===== By: ====================================================
//= Yan Pitangui
//===== Current Version: =======================================
//= 0.1a
//===== Description: ===========================================
//= bonus3 bSCExchange,n,x,y;
//= y% chance for n to become y when cast on you
// SC_BLIND, SC_DECREASEAGI etc (see /db/const.txt for full list)
//
//= Example:
//= bonus3 bSCExchange,SC_FREEZE,SC_TRUESIGHT,100;
//===== Topic ==================================================
//= https://herc.ws/board/topic/18527-bscexchange/
//==============================================================

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
	"bSCExchange",
	SERVER_TYPE_MAP,
	"0.1",
	HPM_VERSION,
};

struct player_data {
	struct {
		short rate, old_status, new_status;
	} status_exchange[MAX_PC_BONUS];
};

int STATUS_EXCHANGE = 0;

int pc_bonus3_pre(struct map_session_data** sd, int* type, int* type2, int* type3, int* val) {
	if (*sd == NULL)
		return 0;
	int i;
	if (*type == STATUS_EXCHANGE) {
		struct player_data* ssd = getFromMSD(*sd, 0);
		if (ssd == NULL) {
			CREATE(ssd, struct player_data, 1);
			addToMSD(*sd, ssd, 0, true);
		}
		ARR_FIND(0, 5, i, ssd->status_exchange[i].rate == 0 || ssd->status_exchange[i].old_status == *type2);
		if (i >= MAX_PC_BONUS) {
			ShowWarning("Maximum of exchanges reached: %d", MAX_PC_BONUS);
		}
		ssd->status_exchange[i].old_status = *type2;
		ssd->status_exchange[i].new_status = *type3;
		ssd->status_exchange[i].rate = cap_value(*val + ssd->status_exchange[i].rate, 0, 100);
		hookStop();
	}
	return 0;
}
// flush bSCExchange back to NULL when recalculating bonus
int status_calc_pc_pre(struct map_session_data** sd, enum e_status_calc_opt* opt) {
	struct player_data* ssd = getFromMSD(*sd, 0);
	if (ssd)
		removeFromMSD(*sd, 0);
	return 0;
}

int status_change_start_sub_pre(struct block_list** src, struct block_list** bl, enum sc_type* type, int* rate, int* val1, int* val2, int* val3, int* val4, int* tick, int* total_tick, int* flag)
{
	struct map_session_data* sd = NULL;
	nullpo_ret(*bl);
	if (!*bl || !(*bl)->type == BL_PC) return 0;
	sd = BL_CAST(BL_PC, *bl);
	if (sd) {
		struct player_data* ssd = getFromMSD(sd, 0);
		if (ssd)
		{
			int i;
			for (i = 0; i < MAX_PC_BONUS; i++) {
				if (ssd->status_exchange[i].old_status == *type && ssd->status_exchange[i].rate > rand() % 100) {
					*type = (enum sc_type)(ssd->status_exchange[i].new_status);
					break;
				}
			}
		}
	}
	return 0;
	
}



HPExport void plugin_init(void) {
	STATUS_EXCHANGE = map->get_new_bonus_id();
	script->set_constant("bSCExchange", STATUS_EXCHANGE, false, false);
	addHookPre(pc, bonus3, pc_bonus3_pre);
	addHookPre(status, change_start_sub, status_change_start_sub_pre);
	addHookPre(status, calc_pc_, status_calc_pc_pre);
}
