#include "Trainer/StylishModeManager.h"
#include <memory>

#define ACT_ID_PUNCH 24
#define ACT_ID_KICK 25
#define ACT_ID_SLASH 26
#define ACT_ID_CSLASH 27
#define ACT_ID_HEAVY 28
#define ACT_ID_CROUCH_PUNCH 29
#define ACT_ID_CROUCH_KICK 30
#define ACT_ID_CROUCH_SLASH 31
#define ACT_ID_CROUCH_HEAVY 32
#define ACT_ID_JUMP_PUNCH 33
#define ACT_ID_JUMP_KICK 34
#define ACT_ID_JUMP_SLASH 35
#define ACT_ID_JUMP_HEAVY 36
#define ACT_ID_6P 62
#define ACT_ID_DUST 63
#define ACT_ID_SWEEP 64
#define ACT_ID_6H 65
#define ACT_ID_6K 71
#define ACT_ID_JUMP_DUST 79

#define pause10F 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
#define waitHitstop 0xF000
#define chargeBack30F 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4

#define T 512
#define D 256
#define H 128
#define S 64
#define K 32
#define P 16

std::shared_ptr<StylishComboItem> StylishModeManager::GetStylishCombosSol()
{
	std::shared_ptr<StylishComboItem> sol_TyrantRaveBeta = std::make_shared<StylishComboItem>(
		StylishComboItem({ 256 }, { 6, 5, 5, 4, D }, 0, false));
	sol_TyrantRaveBeta->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 2500 && !(enemy->status & 0x0200); });

	std::shared_ptr<StylishComboItem> sol_Fafnir = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_HEAVY }, { 4, 1, 2, 3, 6, H }, 0, true));
	sol_Fafnir->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 2500; });
	sol_Fafnir->AddItem(P, sol_TyrantRaveBeta);
	sol_Fafnir->AddItem(K, sol_TyrantRaveBeta);
	sol_Fafnir->AddItem(S, sol_TyrantRaveBeta);
	sol_Fafnir->AddItem(H, sol_TyrantRaveBeta);
	sol_Fafnir->AddItem(D, sol_TyrantRaveBeta);

	std::shared_ptr<StylishComboItem> sol_BanditRevolverFinisher = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_SWEEP }, { 2, 3, 6, K }, 0, false));
	sol_BanditRevolverFinisher->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return enemy->extraData->hitCount <= 2; });

	std::shared_ptr<StylishComboItem> sol_BanditRevolverAA = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH, ACT_ID_KICK, ACT_ID_SLASH, ACT_ID_HEAVY }, { 2, 3, 6, K }, 0, false));
	sol_BanditRevolverAA->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return (enemy->status & 0x0010) > 0; });

	std::shared_ptr<StylishComboItem> sol_Gunflame = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_HEAVY }, { 2, 3, 6, P }, 0, false));
	sol_Gunflame->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });

	std::shared_ptr<StylishComboItem> sol_TyrantRave = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_SWEEP }, { 6, 3, 2, 1, 4, 6, H }, 0, false));
	sol_TyrantRave->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 5000 && !(enemy->status & 0x0200); });

	std::shared_ptr<StylishComboItem> sol_2D = std::make_shared<StylishComboItem>(
		StylishComboItem({ }, { 2 + D }, 0, true));
	sol_2D->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });
	sol_2D->AddItem(P, sol_BanditRevolverFinisher);
	sol_2D->AddItem(K, sol_BanditRevolverFinisher);
	sol_2D->AddItem(S, sol_TyrantRave);
	sol_2D->AddItem(S, sol_BanditRevolverFinisher);
	sol_2D->AddItem(H, sol_TyrantRave);
	sol_2D->AddItem(H, sol_BanditRevolverFinisher);
	sol_2D->AddItem(D, sol_TyrantRave);
	sol_2D->AddItem(D, sol_BanditRevolverFinisher);

	std::shared_ptr<StylishComboItem> sol_jump = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_HEAVY }, { 9, waitHitstop }, 0, false));
	sol_jump->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return (enemy->status & 0x0010) > 0; });

	std::shared_ptr<StylishComboItem> sol_2H_antiair = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH, ACT_ID_KICK, ACT_ID_SLASH, ACT_ID_HEAVY, ACT_ID_6P }, { 2 + H }, 0, true));
	sol_2H_antiair->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return (enemy->status & 0x0010) > 0; });
	sol_2H_antiair->AddItem(P, sol_jump);
	sol_2H_antiair->AddItem(K, sol_jump);
	sol_2H_antiair->AddItem(S, sol_jump);
	sol_2H_antiair->AddItem(H, sol_jump);
	sol_2H_antiair->AddItem(D, sol_jump);

	std::shared_ptr<StylishComboItem> sol_5H = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CSLASH, ACT_ID_SLASH, ACT_ID_CROUCH_SLASH }, { H }, 0, true));
	sol_5H->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return !(enemy->status & 0x0010); });
	sol_5H->AddItem(P, sol_Fafnir);
	sol_5H->AddItem(P, sol_Gunflame);
	sol_5H->AddItem(P, sol_2H_antiair);
	sol_5H->AddItem(K, sol_Fafnir);
	sol_5H->AddItem(K, sol_Gunflame);
	sol_5H->AddItem(K, sol_2H_antiair);
	sol_5H->AddItem(S, sol_Fafnir);
	sol_5H->AddItem(S, sol_Gunflame);
	sol_5H->AddItem(S, sol_2H_antiair);
	sol_5H->AddItem(H, sol_Fafnir);
	sol_5H->AddItem(H, sol_Gunflame);
	sol_5H->AddItem(H, sol_2H_antiair);
	sol_5H->AddItem(D, sol_Fafnir);
	sol_5H->AddItem(D, sol_Gunflame);
	sol_5H->AddItem(D, sol_2H_antiair);
	sol_5H->AddItem(2 + P, sol_2D);
	sol_5H->AddItem(2 + P, sol_2H_antiair);
	sol_5H->AddItem(2 + K, sol_2D);
	sol_5H->AddItem(2 + K, sol_2H_antiair);
	sol_5H->AddItem(2 + S, sol_2D);
	sol_5H->AddItem(2 + S, sol_2H_antiair);
	sol_5H->AddItem(2 + H, sol_2D);
	sol_5H->AddItem(2 + H, sol_2H_antiair);
	sol_5H->AddItem(2 + D, sol_2D);
	sol_5H->AddItem(2 + D, sol_2H_antiair);

	std::shared_ptr<StylishComboItem> sol_5S = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_KICK, ACT_ID_CROUCH_KICK }, { S }, 0, false));
	sol_5S->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return !(enemy->status & 0x0010); });
	sol_5S->AddItem(P, sol_5H);
	sol_5S->AddItem(P, sol_2H_antiair);
	sol_5S->AddItem(K, sol_5H);
	sol_5S->AddItem(K, sol_2H_antiair);
	sol_5S->AddItem(S, sol_5H);
	sol_5S->AddItem(S, sol_2H_antiair);
	sol_5S->AddItem(H, sol_5H);
	sol_5S->AddItem(H, sol_2H_antiair);
	sol_5S->AddItem(D, sol_5H);
	sol_5S->AddItem(D, sol_2H_antiair);
	sol_5S->AddItem(2 + P, sol_2D);
	sol_5S->AddItem(2 + P, sol_2H_antiair);
	sol_5S->AddItem(2 + K, sol_2D);
	sol_5S->AddItem(2 + K, sol_2H_antiair);
	sol_5S->AddItem(2 + S, sol_2D);
	sol_5S->AddItem(2 + S, sol_2H_antiair);
	sol_5S->AddItem(2 + H, sol_2D);
	sol_5S->AddItem(2 + H, sol_2H_antiair);
	sol_5S->AddItem(2 + D, sol_2D);
	sol_5S->AddItem(2 + D, sol_2H_antiair);

	std::shared_ptr<StylishComboItem> sol_5K = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH, ACT_ID_CROUCH_PUNCH }, { K }, 0, false));
	sol_5K->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return !(enemy->status & 0x0010); });
	sol_5K->AddItem(P, sol_5S);
	sol_5K->AddItem(P, sol_2H_antiair);
	sol_5K->AddItem(K, sol_5S);
	sol_5K->AddItem(K, sol_2H_antiair);
	sol_5K->AddItem(S, sol_5S);
	sol_5K->AddItem(S, sol_2H_antiair);
	sol_5K->AddItem(H, sol_5S);
	sol_5K->AddItem(H, sol_2H_antiair);
	sol_5K->AddItem(D, sol_5S);
	sol_5K->AddItem(D, sol_2H_antiair);
	sol_5K->AddItem(2 + P, sol_2H_antiair);
	sol_5K->AddItem(2 + K, sol_2H_antiair);
	sol_5K->AddItem(2 + S, sol_2H_antiair);
	sol_5K->AddItem(2 + H, sol_2H_antiair);
	sol_5K->AddItem(2 + D, sol_2D);
	sol_5K->AddItem(2 + D, sol_2H_antiair);

	std::shared_ptr<StylishComboItem> sol_2Dfrom2S = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_SLASH }, { 2 + D }, 0, true));
	sol_2Dfrom2S->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });
	sol_2Dfrom2S->AddItem(P, sol_BanditRevolverFinisher);
	sol_2Dfrom2S->AddItem(K, sol_BanditRevolverFinisher);
	sol_2Dfrom2S->AddItem(S, sol_TyrantRave);
	sol_2Dfrom2S->AddItem(S, sol_BanditRevolverFinisher);
	sol_2Dfrom2S->AddItem(H, sol_TyrantRave);
	sol_2Dfrom2S->AddItem(H, sol_BanditRevolverFinisher);
	sol_2Dfrom2S->AddItem(D, sol_TyrantRave);
	sol_2Dfrom2S->AddItem(D, sol_BanditRevolverFinisher);

	std::shared_ptr<StylishComboItem> sol_2S = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_KICK, ACT_ID_CROUCH_KICK }, { 2 + S }, 0, false));
	sol_2S->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });
	sol_2S->AddItem(P, sol_5H);
	sol_2S->AddItem(K, sol_5H);
	sol_2S->AddItem(S, sol_5H);
	sol_2S->AddItem(H, sol_5H);
	sol_2S->AddItem(D, sol_5H);
	sol_2S->AddItem(2 + P, sol_2Dfrom2S);
	sol_2S->AddItem(2 + K, sol_2Dfrom2S);
	sol_2S->AddItem(2 + S, sol_2Dfrom2S);
	sol_2S->AddItem(2 + H, sol_2Dfrom2S);
	sol_2S->AddItem(2 + D, sol_2Dfrom2S);

	std::shared_ptr<StylishComboItem> sol_2K = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH, ACT_ID_CROUCH_PUNCH }, { 2 + K }, 0, false));
	sol_2K->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });
	sol_2K->AddItem(P, sol_5S);
	sol_2K->AddItem(K, sol_5S);
	sol_2K->AddItem(S, sol_5S);
	sol_2K->AddItem(H, sol_5S);
	sol_2K->AddItem(D, sol_5S);
	sol_2K->AddItem(2 + P, sol_2S);
	sol_2K->AddItem(2 + K, sol_2S);
	sol_2K->AddItem(2 + S, sol_2S);
	sol_2K->AddItem(2 + H, sol_2S);
	sol_2K->AddItem(2 + D, sol_2D);

	std::shared_ptr<StylishComboItem> sol_jH_air2ground = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_SLASH }, { H }, 0, false));
	sol_jH_air2ground->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return !(enemy->status & 0x0010); });

	std::shared_ptr<StylishComboItem> sol_jS_air2ground = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_PUNCH, ACT_ID_JUMP_KICK }, { S }, 0, true));
	sol_jS_air2ground->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return !(enemy->status & 0x0010); });
	sol_jS_air2ground->AddItem(P, sol_jH_air2ground);
	sol_jS_air2ground->AddItem(K, sol_jH_air2ground);
	sol_jS_air2ground->AddItem(S, sol_jH_air2ground);
	sol_jS_air2ground->AddItem(H, sol_jH_air2ground);
	sol_jS_air2ground->AddItem(D, sol_jH_air2ground);

	std::shared_ptr<StylishComboItem> sol_Knockdown = std::make_shared<StylishComboItem>(
		StylishComboItem({ 100, 101, 104, 105, 110, 111, 112, 113 }, { 2, 1, 4, K }, 0, true));

	std::shared_ptr<StylishComboItem> sol_airHVV = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_HEAVY }, { 6, 2, 3, H }, 0, true));
	sol_airHVV->AddItem(P, sol_Knockdown);
	sol_airHVV->AddItem(K, sol_Knockdown);
	sol_airHVV->AddItem(S, sol_Knockdown);
	sol_airHVV->AddItem(H, sol_Knockdown);
	sol_airHVV->AddItem(D, sol_Knockdown);

	std::shared_ptr<StylishComboItem> sol_jH = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_SLASH }, { 2 + H }, 0, true));
	sol_jH->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return (enemy->status & 0x0010) > 0; });
	sol_jH->AddItem(P, sol_airHVV);
	sol_jH->AddItem(K, sol_airHVV);
	sol_jH->AddItem(S, sol_airHVV);
	sol_jH->AddItem(H, sol_airHVV);
	sol_jH->AddItem(D, sol_airHVV);

	std::shared_ptr<StylishComboItem> sol_j9S = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_SLASH }, { 9, waitHitstop, 5, 5, 5, S }, 0, true));
	sol_j9S->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return (player->extraData->jumpCounter > 0) && (enemy->status & 0x0010) > 0; });
	sol_j9S->AddItem(P, sol_jH);
	sol_j9S->AddItem(K, sol_jH);
	sol_j9S->AddItem(S, sol_jH);
	sol_j9S->AddItem(H, sol_jH);
	sol_j9S->AddItem(D, sol_jH);

	std::shared_ptr<StylishComboItem> sol_jS = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_PUNCH, ACT_ID_JUMP_KICK }, { S }, 0, true));
	sol_jS->SetRequirement([](CharData* player, CharData* enemy) -> bool {
		return (enemy->status & 0x0010) > 0; });
	sol_jS->AddItem(P, sol_j9S);
	sol_jS->AddItem(P, sol_jH);
	sol_jS->AddItem(K, sol_j9S);
	sol_jS->AddItem(K, sol_jH);
	sol_jS->AddItem(S, sol_j9S);
	sol_jS->AddItem(S, sol_jH);
	sol_jS->AddItem(H, sol_j9S);
	sol_jS->AddItem(H, sol_jH);
	sol_jS->AddItem(D, sol_j9S);
	sol_jS->AddItem(D, sol_jH);

	std::shared_ptr<StylishComboItem> sol_default = std::make_shared<StylishComboItem>(
		StylishComboItem({}, {}, 0, false));
	sol_default->AddItem(P, sol_5K);
	sol_default->AddItem(P, sol_5S);
	sol_default->AddItem(P, sol_5H);
	sol_default->AddItem(P, sol_Gunflame);
	sol_default->AddItem(P, sol_Fafnir);
	sol_default->AddItem(P, sol_BanditRevolverAA);
	sol_default->AddItem(P, sol_jS_air2ground);
	sol_default->AddItem(P, sol_jH_air2ground);
	sol_default->AddItem(P, sol_jump);
	sol_default->AddItem(P, sol_jS);
	sol_default->AddItem(P, sol_jH);
	sol_default->AddItem(P, sol_airHVV);
	sol_default->AddItem(P, sol_Knockdown);
	sol_default->AddItem(K, sol_5K);
	sol_default->AddItem(K, sol_5S);
	sol_default->AddItem(K, sol_5H);
	sol_default->AddItem(K, sol_Gunflame);
	sol_default->AddItem(K, sol_Fafnir);
	sol_default->AddItem(K, sol_BanditRevolverAA);
	sol_default->AddItem(K, sol_jS_air2ground);
	sol_default->AddItem(K, sol_jH_air2ground);
	sol_default->AddItem(K, sol_jump);
	sol_default->AddItem(K, sol_jS);
	sol_default->AddItem(K, sol_jH);
	sol_default->AddItem(K, sol_airHVV);
	sol_default->AddItem(K, sol_Knockdown);
	sol_default->AddItem(S, sol_5K);
	sol_default->AddItem(S, sol_5S);
	sol_default->AddItem(S, sol_5H);
	sol_default->AddItem(S, sol_Gunflame);
	sol_default->AddItem(S, sol_Fafnir);
	sol_default->AddItem(S, sol_BanditRevolverAA);
	sol_default->AddItem(S, sol_jS_air2ground);
	sol_default->AddItem(S, sol_jH_air2ground);
	sol_default->AddItem(S, sol_jump);
	sol_default->AddItem(S, sol_jS);
	sol_default->AddItem(S, sol_jH);
	sol_default->AddItem(S, sol_airHVV);
	sol_default->AddItem(S, sol_Knockdown);
	sol_default->AddItem(H, sol_5K);
	sol_default->AddItem(H, sol_5S);
	sol_default->AddItem(H, sol_5H);
	sol_default->AddItem(H, sol_Gunflame);
	sol_default->AddItem(H, sol_Fafnir);
	sol_default->AddItem(H, sol_BanditRevolverAA);
	sol_default->AddItem(H, sol_jS_air2ground);
	sol_default->AddItem(H, sol_jH_air2ground);
	sol_default->AddItem(H, sol_jump);
	sol_default->AddItem(H, sol_jS);
	sol_default->AddItem(H, sol_jH);
	sol_default->AddItem(H, sol_airHVV);
	sol_default->AddItem(H, sol_Knockdown);
	sol_default->AddItem(D, sol_5K);
	sol_default->AddItem(D, sol_5S);
	sol_default->AddItem(D, sol_5H);
	sol_default->AddItem(D, sol_Gunflame);
	sol_default->AddItem(D, sol_Fafnir);
	sol_default->AddItem(D, sol_BanditRevolverAA);
	sol_default->AddItem(D, sol_jS_air2ground);
	sol_default->AddItem(D, sol_jH_air2ground);
	sol_default->AddItem(D, sol_jump);
	sol_default->AddItem(D, sol_jS);
	sol_default->AddItem(D, sol_jH);
	sol_default->AddItem(D, sol_airHVV);
	sol_default->AddItem(D, sol_Knockdown);
	sol_default->AddItem(2 + P, sol_2K);
	sol_default->AddItem(2 + P, sol_2S);
	sol_default->AddItem(2 + P, sol_2D);
	sol_default->AddItem(2 + P, sol_2H_antiair);
	sol_default->AddItem(2 + K, sol_2K);
	sol_default->AddItem(2 + K, sol_2S);
	sol_default->AddItem(2 + K, sol_2D);
	sol_default->AddItem(2 + K, sol_2H_antiair);
	sol_default->AddItem(2 + S, sol_2K);
	sol_default->AddItem(2 + S, sol_2S);
	sol_default->AddItem(2 + S, sol_2D);
	sol_default->AddItem(2 + S, sol_2H_antiair);
	sol_default->AddItem(2 + H, sol_2K);
	sol_default->AddItem(2 + H, sol_2S);
	sol_default->AddItem(2 + H, sol_2D);
	sol_default->AddItem(2 + H, sol_2H_antiair);
	sol_default->AddItem(2 + D, sol_2K);
	sol_default->AddItem(2 + D, sol_2S);
	sol_default->AddItem(2 + D, sol_2D);
	sol_default->AddItem(2 + D, sol_2H_antiair);


	return sol_default;

}

std::shared_ptr<StylishComboItem> StylishModeManager::GetStylishCombosPotemkin()
{ 
	std::shared_ptr<StylishComboItem> pot_APB = std::make_shared<StylishComboItem>(
		StylishComboItem({ }, { 6, 3, 2, 1, 4, 7, waitHitstop, 6, D }, 0, false));
	pot_APB->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 2500 && (enemy->status & 0x0020) > 0; });

	std::shared_ptr<StylishComboItem> pot_jD = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_SLASH }, { D }, 0, false));

	std::shared_ptr<StylishComboItem> pot_jS = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_KICK }, { S }, 0, true));
	pot_jS->AddItem(P, pot_jD);
	pot_jS->AddItem(K, pot_jD);
	pot_jS->AddItem(S, pot_jD);
	pot_jS->AddItem(H, pot_jD);
	pot_jS->AddItem(D, pot_jD);

	std::shared_ptr<StylishComboItem> pot_j9K = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_KICK }, { 9, waitHitstop, 5, K }, 0, true));
	pot_j9K->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->jumpCounter > 0 && (enemy->status & 0x0030) == 0x0030; });
	pot_j9K->AddItem(P, pot_jS);
	pot_j9K->AddItem(K, pot_jS);
	pot_j9K->AddItem(S, pot_jS);
	pot_j9K->AddItem(H, pot_jS);
	pot_j9K->AddItem(D, pot_jS);

	std::shared_ptr<StylishComboItem> pot_jK = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_JUMP_PUNCH }, { K }, 0, true));
	pot_jK->AddItem(P, pot_j9K);
	pot_jK->AddItem(P, pot_jS);
	pot_jK->AddItem(K, pot_j9K);
	pot_jK->AddItem(K, pot_jS);
	pot_jK->AddItem(S, pot_APB);
	pot_jK->AddItem(S, pot_j9K);
	pot_jK->AddItem(S, pot_jS);
	pot_jK->AddItem(H, pot_APB);
	pot_jK->AddItem(H, pot_j9K);
	pot_jK->AddItem(H, pot_jS);
	pot_jK->AddItem(D, pot_APB);
	pot_jK->AddItem(D, pot_j9K);
	pot_jK->AddItem(D, pot_jS);

	std::shared_ptr<StylishComboItem> pot_HeatExtend = std::make_shared<StylishComboItem>(
		StylishComboItem({ 116 }, { 6, 3, 2, 1, 4, H }, 0, false));

	std::shared_ptr<StylishComboItem> pot_HeatGrab = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_6P, ACT_ID_SLASH, ACT_ID_HEAVY, ACT_ID_CROUCH_SLASH, ACT_ID_CROUCH_HEAVY }, { 6, 2, 3, H }, 0, true));
	pot_HeatGrab->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return ((enemy->status & 0x0020) > 0) && 
			(player->actId == ACT_ID_6P || player->actId == ACT_ID_CROUCH_SLASH ||
				player->actId == ACT_ID_SLASH || player->actId == ACT_ID_HEAVY && (enemy->status & 0x0010) > 0) ||
			(player->actId == ACT_ID_CROUCH_HEAVY); });
	pot_HeatGrab->AddItem(P, pot_HeatExtend);
	pot_HeatGrab->AddItem(K, pot_HeatExtend);
	pot_HeatGrab->AddItem(S, pot_HeatExtend);
	pot_HeatGrab->AddItem(H, pot_HeatExtend);
	pot_HeatGrab->AddItem(D, pot_HeatExtend);

	std::shared_ptr<StylishComboItem> pot_GiganticBullet = std::make_shared<StylishComboItem>(
		StylishComboItem({ 107, 108 }, { 4, 1, 2, 3, 6, 4, 1, 2, 3, 6, P }, 0, false));

	std::shared_ptr<StylishComboItem> pot_Giganter = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_6K, ACT_ID_HEAVY, ACT_ID_CROUCH_SLASH }, { 6, 3, 2, 1, 4, 6, H }, 0, true));
	pot_Giganter->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 5000 && (enemy->status & 0x0030) == 0x0020 &&
			(player->actId == ACT_ID_HEAVY && abs(player->posX - enemy->posX) <= 10000) ||
			(player->actId == ACT_ID_CROUCH_SLASH && abs(player->posX - enemy->posX) <= 25500) ||
			player->actId == ACT_ID_6K; });
	pot_Giganter->AddItem(P, pot_GiganticBullet);
	pot_Giganter->AddItem(K, pot_GiganticBullet);
	pot_Giganter->AddItem(S, pot_GiganticBullet);
	pot_Giganter->AddItem(H, pot_GiganticBullet);
	pot_Giganter->AddItem(D, pot_GiganticBullet);

	std::shared_ptr<StylishComboItem> pot_HPB = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_HEAVY }, { 2, 3, 6, 2, 3, 6, S }, 0, false));
	pot_HPB->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return player->extraData->tension >= 5000 && (enemy->status & 0x0020) > 0; });

	std::shared_ptr<StylishComboItem> pot_2H = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_SLASH }, { 2 + H }, 0, true));
	pot_2H->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return abs(player->posX - enemy->posX) <= 25500 && (enemy->status & 0x0020) > 0; });
	pot_2H->AddItem(P, pot_HeatGrab);
	pot_2H->AddItem(K, pot_HeatGrab);
	pot_2H->AddItem(S, pot_HPB);
	pot_2H->AddItem(S, pot_HeatGrab);
	pot_2H->AddItem(H, pot_HPB);
	pot_2H->AddItem(H, pot_HeatGrab);
	pot_2H->AddItem(D, pot_HPB);
	pot_2H->AddItem(D, pot_HeatGrab);

	std::shared_ptr<StylishComboItem> pot_2D = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_6K, ACT_ID_HEAVY, ACT_ID_CROUCH_SLASH, ACT_ID_SLASH, ACT_ID_CSLASH }, { 2 + D }, 0, false));
	pot_2D->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });

	std::shared_ptr<StylishComboItem> pot_2SGround = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_KICK, ACT_ID_CSLASH, ACT_ID_SLASH }, { 2 + S, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }, 0, true));
	pot_2SGround->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return !(enemy->status & 0x0010); });
	pot_2SGround->AddItem(P, pot_2H);
	pot_2SGround->AddItem(P, pot_2D);
	pot_2SGround->AddItem(K, pot_2H);
	pot_2SGround->AddItem(K, pot_2D);
	pot_2SGround->AddItem(S, pot_Giganter);
	pot_2SGround->AddItem(S, pot_2H);
	pot_2SGround->AddItem(S, pot_2D);
	pot_2SGround->AddItem(H, pot_Giganter);
	pot_2SGround->AddItem(H, pot_2H);
	pot_2SGround->AddItem(H, pot_2D);
	pot_2SGround->AddItem(D, pot_Giganter);
	pot_2SGround->AddItem(D, pot_2H);
	pot_2SGround->AddItem(D, pot_2D);

	std::shared_ptr<StylishComboItem> pot_5K = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH }, { K }, 0, false));
	pot_5K->AddItem(P, pot_2SGround);
	pot_5K->AddItem(K, pot_2SGround);
	pot_5K->AddItem(S, pot_2SGround);
	pot_5K->AddItem(H, pot_2SGround);
	pot_5K->AddItem(D, pot_2SGround);
	pot_5K->AddItem(2 + P, pot_2D);
	pot_5K->AddItem(2 + K, pot_2D);
	pot_5K->AddItem(2 + S, pot_2D);
	pot_5K->AddItem(2 + H, pot_2D);
	pot_5K->AddItem(2 + D, pot_2D);

	std::shared_ptr<StylishComboItem> pot_5S = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_KICK }, { S, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }, 0, true));
	pot_5S->AddItem(P, pot_2D);
	pot_5S->AddItem(K, pot_2D);
	pot_5S->AddItem(S, pot_2D);
	pot_5S->AddItem(H, pot_2D);
	pot_5S->AddItem(D, pot_2D);

	std::shared_ptr<StylishComboItem> pot_2K = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_PUNCH }, { 2 + K }, 0, true));
	pot_2K->AddItem(P, pot_5S);
	pot_2K->AddItem(K, pot_5S);
	pot_2K->AddItem(S, pot_5S);
	pot_2K->AddItem(H, pot_5S);
	pot_2K->AddItem(D, pot_5S);

	std::shared_ptr<StylishComboItem> pot_2HAir = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_CROUCH_SLASH }, { 2 + H }, 0, true));
	pot_2HAir->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return (enemy->status & 0x0030) == 0x0030 && player->extraData->tension >= 5000; });
	pot_2HAir->AddItem(P, pot_HPB);
	pot_2HAir->AddItem(K, pot_HPB);
	pot_2HAir->AddItem(S, pot_HPB);
	pot_2HAir->AddItem(H, pot_HPB);
	pot_2HAir->AddItem(D, pot_HPB);

	std::shared_ptr<StylishComboItem> pot_2SAir = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_PUNCH, ACT_ID_KICK, ACT_ID_CSLASH }, { 2 + S, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 }, 0, true));
	pot_2SAir->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return (enemy->status & 0x0010) > 0; });
	pot_2SAir->AddItem(P, pot_HeatGrab);
	pot_2SAir->AddItem(K, pot_HeatGrab);
	pot_2SAir->AddItem(S, pot_2HAir);
	pot_2SAir->AddItem(S, pot_HeatGrab);
	pot_2SAir->AddItem(H, pot_2HAir);
	pot_2SAir->AddItem(H, pot_HeatGrab);
	pot_2SAir->AddItem(D, pot_2HAir);
	pot_2SAir->AddItem(D, pot_HeatGrab);

	std::shared_ptr<StylishComboItem> pot_Megafist = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_6H }, { 2, 3, 6, P }, 0, false));
	pot_Megafist->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return (enemy->status & 0x0020) > 0; });

	std::shared_ptr<StylishComboItem> pot_Slidehead = std::make_shared<StylishComboItem>(
		StylishComboItem({ ACT_ID_6H }, { 2, 3, 6, S }, 0, false));
	pot_Slidehead->SetRequirement([](CharData* player, CharData* enemy)->bool {
		return (enemy->status & 0x0200) > 0; });


	std::shared_ptr<StylishComboItem> pot_default = std::make_shared<StylishComboItem>(
		StylishComboItem({}, {}, 0, false));
	pot_default->AddItem(P, pot_2SAir);
	pot_default->AddItem(P, pot_2HAir);
	pot_default->AddItem(P, pot_5K);
	pot_default->AddItem(P, pot_2SGround);
	pot_default->AddItem(P, pot_2H);
	pot_default->AddItem(P, pot_2D);
	pot_default->AddItem(P, pot_HeatGrab);
	pot_default->AddItem(P, pot_2K);
	pot_default->AddItem(P, pot_5S);
	pot_default->AddItem(P, pot_jK);
	pot_default->AddItem(P, pot_j9K);
	pot_default->AddItem(P, pot_jS);
	pot_default->AddItem(P, pot_jD);
	pot_default->AddItem(P, pot_Megafist);
	pot_default->AddItem(P, pot_Slidehead);
	pot_default->AddItem(K, pot_2SAir);
	pot_default->AddItem(K, pot_2HAir);
	pot_default->AddItem(K, pot_5K);
	pot_default->AddItem(K, pot_2SGround);
	pot_default->AddItem(K, pot_2H);
	pot_default->AddItem(K, pot_2D);
	pot_default->AddItem(K, pot_HeatGrab);
	pot_default->AddItem(K, pot_2K);
	pot_default->AddItem(K, pot_5S);
	pot_default->AddItem(K, pot_jK);
	pot_default->AddItem(K, pot_j9K);
	pot_default->AddItem(K, pot_jS);
	pot_default->AddItem(K, pot_jD);
	pot_default->AddItem(K, pot_Megafist);
	pot_default->AddItem(K, pot_Slidehead);
	pot_default->AddItem(S, pot_2SAir);
	pot_default->AddItem(S, pot_2HAir);
	pot_default->AddItem(S, pot_5K);
	pot_default->AddItem(S, pot_2SGround);
	pot_default->AddItem(S, pot_Giganter);
	pot_default->AddItem(S, pot_2H);
	pot_default->AddItem(S, pot_2D);
	pot_default->AddItem(S, pot_HPB);
	pot_default->AddItem(S, pot_HeatGrab);
	pot_default->AddItem(S, pot_2K);
	pot_default->AddItem(S, pot_5S);
	pot_default->AddItem(S, pot_jK);
	pot_default->AddItem(S, pot_APB);
	pot_default->AddItem(S, pot_j9K);
	pot_default->AddItem(S, pot_jS);
	pot_default->AddItem(S, pot_jD);
	pot_default->AddItem(S, pot_Megafist);
	pot_default->AddItem(S, pot_Slidehead);
	pot_default->AddItem(H, pot_2SAir);
	pot_default->AddItem(H, pot_2HAir);
	pot_default->AddItem(H, pot_5K);
	pot_default->AddItem(H, pot_2SGround);
	pot_default->AddItem(H, pot_Giganter);
	pot_default->AddItem(H, pot_2H);
	pot_default->AddItem(H, pot_2D);
	pot_default->AddItem(H, pot_HPB);
	pot_default->AddItem(H, pot_HeatGrab);
	pot_default->AddItem(H, pot_2K);
	pot_default->AddItem(H, pot_5S);
	pot_default->AddItem(H, pot_jK);
	pot_default->AddItem(H, pot_APB);
	pot_default->AddItem(H, pot_j9K);
	pot_default->AddItem(H, pot_jS);
	pot_default->AddItem(H, pot_jD);
	pot_default->AddItem(H, pot_Megafist);
	pot_default->AddItem(H, pot_Slidehead);
	pot_default->AddItem(D, pot_2SAir);
	pot_default->AddItem(D, pot_2HAir);
	pot_default->AddItem(D, pot_5K);
	pot_default->AddItem(D, pot_2SGround);
	pot_default->AddItem(D, pot_Giganter);
	pot_default->AddItem(D, pot_2H);
	pot_default->AddItem(D, pot_2D);
	pot_default->AddItem(D, pot_HPB);
	pot_default->AddItem(D, pot_HeatGrab);
	pot_default->AddItem(D, pot_2K);
	pot_default->AddItem(D, pot_5S);
	pot_default->AddItem(D, pot_jK);
	pot_default->AddItem(D, pot_APB);
	pot_default->AddItem(D, pot_j9K);
	pot_default->AddItem(D, pot_jS);
	pot_default->AddItem(D, pot_jD);
	pot_default->AddItem(D, pot_Megafist);
	pot_default->AddItem(D, pot_Slidehead);
	pot_default->AddItem(2 + P, pot_2SAir);
	pot_default->AddItem(2 + P, pot_2HAir);
	pot_default->AddItem(2 + P, pot_2K);
	pot_default->AddItem(2 + P, pot_5S);
	pot_default->AddItem(2 + P, pot_2SGround);
	pot_default->AddItem(2 + P, pot_2H);
	pot_default->AddItem(2 + P, pot_HeatGrab);
	pot_default->AddItem(2 + P, pot_2D);
	pot_default->AddItem(2 + P, pot_jK);
	pot_default->AddItem(2 + P, pot_APB);
	pot_default->AddItem(2 + P, pot_j9K);
	pot_default->AddItem(2 + P, pot_jS);
	pot_default->AddItem(2 + P, pot_jD);
	pot_default->AddItem(2 + P, pot_Megafist);
	pot_default->AddItem(2 + P, pot_Slidehead);
	pot_default->AddItem(2 + K, pot_2SAir);
	pot_default->AddItem(2 + K, pot_2HAir);
	pot_default->AddItem(2 + K, pot_2K);
	pot_default->AddItem(2 + K, pot_5S);
	pot_default->AddItem(2 + K, pot_2SGround);
	pot_default->AddItem(2 + K, pot_2H);
	pot_default->AddItem(2 + K, pot_HeatGrab);
	pot_default->AddItem(2 + K, pot_2D);
	pot_default->AddItem(2 + K, pot_jK);
	pot_default->AddItem(2 + K, pot_APB);
	pot_default->AddItem(2 + K, pot_j9K);
	pot_default->AddItem(2 + K, pot_jS);
	pot_default->AddItem(2 + K, pot_jD);
	pot_default->AddItem(2 + K, pot_Megafist);
	pot_default->AddItem(2 + K, pot_Slidehead);
	pot_default->AddItem(2 + S, pot_2SAir);
	pot_default->AddItem(2 + S, pot_2HAir);
	pot_default->AddItem(2 + S, pot_2K);
	pot_default->AddItem(2 + S, pot_5S);
	pot_default->AddItem(2 + S, pot_2SGround);
	pot_default->AddItem(2 + S, pot_2H);
	pot_default->AddItem(2 + S, pot_HPB);
	pot_default->AddItem(2 + S, pot_HeatGrab);
	pot_default->AddItem(2 + S, pot_2D);
	pot_default->AddItem(2 + S, pot_jK);
	pot_default->AddItem(2 + S, pot_APB);
	pot_default->AddItem(2 + S, pot_j9K);
	pot_default->AddItem(2 + S, pot_jS);
	pot_default->AddItem(2 + S, pot_jD);
	pot_default->AddItem(2 + S, pot_Megafist);
	pot_default->AddItem(2 + S, pot_Slidehead);
	pot_default->AddItem(2 + H, pot_2SAir);
	pot_default->AddItem(2 + H, pot_2HAir);
	pot_default->AddItem(2 + H, pot_2K);
	pot_default->AddItem(2 + H, pot_5S);
	pot_default->AddItem(2 + H, pot_2SGround);
	pot_default->AddItem(2 + H, pot_2H);
	pot_default->AddItem(2 + H, pot_HPB);
	pot_default->AddItem(2 + H, pot_HeatGrab);
	pot_default->AddItem(2 + H, pot_2D);
	pot_default->AddItem(2 + H, pot_jK);
	pot_default->AddItem(2 + H, pot_APB);
	pot_default->AddItem(2 + H, pot_j9K);
	pot_default->AddItem(2 + H, pot_jS);
	pot_default->AddItem(2 + H, pot_jD);
	pot_default->AddItem(2 + H, pot_Megafist);
	pot_default->AddItem(2 + H, pot_Slidehead);
	pot_default->AddItem(2 + D, pot_2SAir);
	pot_default->AddItem(2 + D, pot_2HAir);
	pot_default->AddItem(2 + D, pot_2K);
	pot_default->AddItem(2 + D, pot_5S);
	pot_default->AddItem(2 + D, pot_2SGround);
	pot_default->AddItem(2 + D, pot_2H);
	pot_default->AddItem(2 + D, pot_HPB);
	pot_default->AddItem(2 + D, pot_HeatGrab);
	pot_default->AddItem(2 + D, pot_2D);
	pot_default->AddItem(2 + D, pot_jK);
	pot_default->AddItem(2 + D, pot_APB);
	pot_default->AddItem(2 + D, pot_j9K);
	pot_default->AddItem(2 + D, pot_jS);
	pot_default->AddItem(2 + D, pot_jD);
	pot_default->AddItem(2 + D, pot_Megafist);
	pot_default->AddItem(2 + D, pot_Slidehead);
	pot_default->AddItem(6 + P, pot_HeatGrab);
	pot_default->AddItem(6 + P, pot_2D);
	pot_default->AddItem(6 + P, pot_jK);
	pot_default->AddItem(6 + P, pot_APB);
	pot_default->AddItem(6 + P, pot_j9K);
	pot_default->AddItem(6 + P, pot_jS);
	pot_default->AddItem(6 + P, pot_jD);
	pot_default->AddItem(6 + P, pot_Megafist);
	pot_default->AddItem(6 + P, pot_Slidehead);
	pot_default->AddItem(6 + K, pot_HeatGrab);
	pot_default->AddItem(6 + K, pot_2D);
	pot_default->AddItem(6 + K, pot_jK);
	pot_default->AddItem(6 + K, pot_APB);
	pot_default->AddItem(6 + K, pot_j9K);
	pot_default->AddItem(6 + K, pot_jS);
	pot_default->AddItem(6 + K, pot_jD);
	pot_default->AddItem(6 + K, pot_Megafist);
	pot_default->AddItem(6 + K, pot_Slidehead);
	pot_default->AddItem(6 + S, pot_HeatGrab);
	pot_default->AddItem(6 + S, pot_2D);
	pot_default->AddItem(6 + S, pot_jK);
	pot_default->AddItem(6 + S, pot_APB);
	pot_default->AddItem(6 + S, pot_j9K);
	pot_default->AddItem(6 + S, pot_jS);
	pot_default->AddItem(6 + S, pot_jD);
	pot_default->AddItem(6 + S, pot_Megafist);
	pot_default->AddItem(6 + S, pot_Slidehead);
	pot_default->AddItem(6 + H, pot_HeatGrab);
	pot_default->AddItem(6 + H, pot_2D);
	pot_default->AddItem(6 + H, pot_jK);
	pot_default->AddItem(6 + H, pot_APB);
	pot_default->AddItem(6 + H, pot_j9K);
	pot_default->AddItem(6 + H, pot_jS);
	pot_default->AddItem(6 + H, pot_jD);
	pot_default->AddItem(6 + H, pot_Megafist);
	pot_default->AddItem(6 + H, pot_Slidehead);
	pot_default->AddItem(6 + D, pot_HeatGrab);
	pot_default->AddItem(6 + D, pot_2D);
	pot_default->AddItem(6 + D, pot_jK);
	pot_default->AddItem(6 + D, pot_APB);
	pot_default->AddItem(6 + D, pot_j9K);
	pot_default->AddItem(6 + D, pot_jS);
	pot_default->AddItem(6 + D, pot_jD);
	pot_default->AddItem(6 + D, pot_Megafist);
	pot_default->AddItem(6 + D, pot_Slidehead);

	return pot_default;
}