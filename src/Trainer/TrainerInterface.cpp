#include "TrainerInterface.h"
#include "Hooks/hooks_trainer.h"
#include <Core/interfaces.h>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

TrainerItem* TrainerInterface::GetItem(int idx)
{
	return &trainerItems[idx];
}

TrainerItem* TrainerInterface::GetItem(std::string tag)
{
	for (int i = 0; i < trainerItems.size(); i++)
	{
		if (trainerItems[i].GetTag() == tag)
			return &trainerItems[i];
	}
	return NULL;
}

const char* specialMode[] = { "None", "Gold", "Shadow" };
const char* guardFrames[] = { "None", "Instant Block", "Faultless Defence", "Slashback" };
const char* attackLevels[] = { "None", "Lv 1", "Lv 2", "Lv 3" };
const char* faustItems[] = { "Random", "Small Faust", "Chocolate", "Doughnut", "Hammer", "Meteor", "Bomb", "Poison", "Small Potemkin", "Small Robo-Ky", "Coin", "Tarai", "Chikuwa", "Dumbell" };
const char* testCrowAttacks[] = { "None", "Wall Bounce", "Normal", "Overhead", "Projectile" };
const char* testCrowPatterns[] = { "None", "Pattern 1", "Pattern 2", "Pattern 3", "Pattern 4" };
const char* inoMegalomaniaPatterns[] = { "Random", "Diagonal", "Full Screen", "Zigzag" };
const char* zappaSummons[] = { "Random", "Sword", "Dog", "Triplet", "Raoh" };
const char* extraMenuTensionOptions[] = { "Normal", "None", "Increase", "Infinity" };
const char* extraMenuLifeOptions[] = { "Normal", "Poison", "Heal" };
const char* extraMenuBurstOptions[] = { "Normal", "None", "Infinity" };
const char* extraMenuGuardOptions[] = { "Normal", "Chokuzen", "None" };
const char* extraMenuGuardBalanceOptions[] = { "0", "-128", "-64", "+64", "+128" };
const char* extraMenuAirOptions[] = { "Normal", "+1", "+2", "None" };
const char* extraMenuEasyFRCOptions[] = { "Off", "5", "10", "15", "20" };
const char* extraMenuBoundOptions[] = { "Normal", "All B", "All S" };
const char* survivalBosses[] = { "None", "Shadow Anji", "Shadow Venom", "Shadow Faust", "Shadow Baiken", "Shadow Bridget", \
"Shadow Potemkin", "Shadow Ky", "Shadow Testament", "Shadow Millia", "Shadow Zappa", \
"Shadow Chipp", "Shadow Eddie", "Shadow Axl", "Shadow Slayer", "Shadow Jam", "Shadow May", "Shadow Johnny", \
"Shadow Dizzy", "Shadow Robo-Ky", "Shadow I-No", "Shadow Order-Sol", "Shadow Sol", "Shadow A.B.A", "Shadow Kliff", "Shadow Justice", \
"Gold Anji", "Gold Venom", "Gold Faust", "Gold Baiken", "Gold Bridget", "Gold Potemkin", "Gold Ky", "Gold Testament", \
"Gold Millia", "Gold Zappa", "Gold Chipp", "Gold Eddie", "Gold Axl", "Gold Slayer", "Gold Jam", "Gold May", "Gold Johnny", \
"Gold Dizzy", "Gold Robo-Ky", "Gold I-No", "Gold Sol", "Gold A.B.A", "Gold Kliff", "Gold Justice", "Gold Order-Sol" };
void TrainerInterface::initializeTrainer()
{
	//General
	trainerItems.emplace_back("Infinite health", 2, "ge:InfHealth", TrainerItemType_Bool,
		"Sets maximal health and disables any damage.", false, [] (uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->currentHP = 460;
		});

	trainerItems.emplace_back("Infinite meter:", 2, "ge:InfMeter", TrainerItemType_Bool,
		"Gives infinite tension.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->tension = 10000;
		});

	trainerItems.emplace_back("Infinite burst:", 2, "ge:InfBurst", TrainerItemType_Bool,
		"Gives infinite burst gauge.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->burstGauge = 15000;
		});

	trainerItems.emplace_back("Special mode:", 2, "ge:SpMode", TrainerItemType_List,
		"Enables effect of either gold or shadow mode.\nNote: turning the mode off does not completely\nremove some effects and a round reset is\nrequired.",
		specialMode, 3, 0, [](uint8_t flags, int index, int playerNr) {
			if ((flags & 0x09) > 0)
				if (index || !(flags & 0x08)) //Make sure None value doesn't overwrite existing gold mode value on round start
					*g_gameVals.pSpecialMode[playerNr] = index;
		});
	trainerItems.emplace_back("Disable hurtboxes:", 2, "ge:DisHurtbox", TrainerItemType_Bool,
		"Disables hurtboxes (throws still work).", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)
				g_interfaces.GetPlayer(playerNr)->GetData()->status |= 0x0080;
			else if ((flags & 0x02) > 0)
				g_interfaces.GetPlayer(playerNr)->GetData()->status &= 0xFFFFFF7F; //Flag doesn't turn off immediately so turn it off manually
		});
	trainerItems.emplace_back("Throw invul:", 2, "ge:ThInvul", TrainerItemType_Bool,
		"Character cannot be thrown.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)
				g_interfaces.GetPlayer(playerNr)->GetData()->status |= 0x00800000;
			else if ((flags & 0x04) > 0)
				g_interfaces.GetPlayer(playerNr)->GetData()->status &= 0xFF7FFFFF; //Flag doesn't turn off immediately so turn it off manually
		});
	trainerItems.emplace_back("Infinite jumps:", 2, "ge:InfJump", TrainerItemType_Bool,
		"Gives unlimited jumps.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->jumpCounter = 0x01;
		});
	trainerItems.emplace_back("Infinite airdash:", 2, "ge:InfAD", TrainerItemType_Bool,
		"Gives unlimited air dashes.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->airdashCounter = 0x01;
		});
	trainerItems.emplace_back("Unlimited gatlings:", 2, "ge:UnGatling", TrainerItemType_Bool,
		"Enables gatling for all normal moves to\neach other in any order.", false);
	trainerItems.emplace_back("Enable gatling on whiff:", 2, "ge:WhiffGatling", TrainerItemType_Bool,
		"Enables gatling normal moves even on whiff.\nDoesn't allow gatlings which do not exist.\nUse the unlimited gatling flag to allow that.", false);
	trainerItems.emplace_back("Jump cancel all moves:", 2, "ge:UnJumpCancel", TrainerItemType_Bool,
		"Makes all moves jump cancelable.", false);
	trainerItems.emplace_back("Infinite projectiles:", 2, "ge:InfProjectile", TrainerItemType_Bool,
		"Allows characters who use projectiles to summon\nadditional projectiles while an existing one is\non screen. Currently disabled for Dizzy as\nit crashes the game.",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (active && strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "di"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->projectileFlag = 0x00;
		});
	trainerItems.emplace_back("Always FRC:", 2, "ge:AlwaysFRC", TrainerItemType_Bool,
		"Allows performing FRC at any point. Note: Performing too many FRC's in a short time span can crash the game.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->FRCTimer = 0x05;
		});
	trainerItems.emplace_back("Poison state:", 2, "ge:Poison", TrainerItemType_Bool,
		"Sets poison state on.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->poisonTimer = 0x01;
		});
	trainerItems.emplace_back("Seal dashes:", 2, "ge:SealDash", TrainerItemType_Bool,
		"Disables all dashes (forward, back and air).", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->dashSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal jumps:", 2, "ge:SealJump", TrainerItemType_Bool,
		"Disables jumps.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->jumpSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal guard:", 2, "ge:SealGuard", TrainerItemType_Bool,
		"Disables blocking.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->guardSealTimer = 0x01;
		});
	trainerItems.emplace_back("Always counter:", 2, "ge:AlwaysCH", TrainerItemType_Bool,
		"Every move the player is hit by is a counter hit.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->allCounterTimer = 0x01;
		});
	trainerItems.emplace_back("Seal punch:", 2, "ge:SealP", TrainerItemType_Bool,
		"Disables the punch button.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->pSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal kick:", 2, "ge:SealK", TrainerItemType_Bool,
		"Disables the kick button.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->kSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal slash:", 2, "ge:SealS", TrainerItemType_Bool,
		"Disables the slash button.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->sSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal heavy:", 2, "ge:SealH", TrainerItemType_Bool,
		"Disables the heavy slash button.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->hSealTimer = 0x01;
		});
	trainerItems.emplace_back("Seal dust:", 2, "ge:SealD", TrainerItemType_Bool,
		"Disables the dust button.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active)	g_interfaces.GetPlayer(playerNr)->GetData()->extraData->dSealTimer = 0x01;
		});
	trainerItems.emplace_back("Disable recovery:", 2, "ge:DisRecovery", TrainerItemType_Bool,
		"Disables air recoveries.", false);
	trainerItems.emplace_back("Min combo damage:", 2, "ge:MinComboDmg", TrainerItemType_Int,
		"Sets the minimum number of hits in a combo\nbelow which no damage is done to the player.", 0, 30);
	trainerItems.emplace_back("Max combo damage:", 2, "ge:MaxComboDmg", TrainerItemType_Int,
		"Sets the maximum number of hits in a combo\nabove which no damage is done to the player.", 0, 30);
	trainerItems.emplace_back("Freeze match timer:", 1, "ge:FreezeTimer", TrainerItemType_Bool,
		"Set match timer to infinity.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active) *g_gameVals.pMatchTimer = -1; });

	//Defence
	trainerItems.emplace_back("Auto block:", 2, "de:AutoBlock", TrainerItemType_Bool,
		"Automatically block incoming attack if not in hitstun or recovery.\nCan be used in cojuction with CBR AI to control it's difficulty\nif it gets hit too often.",
		false);
	trainerItems.emplace_back("Normal block probability:", 2, "de:NormBlockProb", TrainerItemType_Float,
		"Sets the probability to block regular attacks when auto block is on.", 0.0f, 1.0f, 1.0f);
	trainerItems.emplace_back("Low block probability:", 2, "de:LowBlockProb", TrainerItemType_Float,
		"Sets the probability to block low attacks when auto block is on.", 0.0f, 1.0f, 1.0f);
	trainerItems.emplace_back("Overhead block probability:", 2, "de:OHBlockProb", TrainerItemType_Float,
		"Sets the probability to block overhead attacks when auto block is on.", 0.0f, 1.0f, 1.0f);
	trainerItems.emplace_back("Anti-air block probability:", 2, "de:AABlockProb", TrainerItemType_Float,
		"Sets the probability to block anti-air attacks when auto block is on.", 0.0f, 1.0f, 1.0f);
	trainerItems.emplace_back("Guard frame:", 2, "de:GuardFrame", TrainerItemType_List,
		"Sets all blocks to either instant block,\nfaultless defence or slashback.",
		guardFrames, 4, 0, [](uint8_t flags, int index, int playerNr) {
			switch (index)
			{
			case GuardFrame_IB:
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->ibTimer = 0x01; //When this is < 8 instant block occurs.
				break;
			case GuardFrame_FD:
				g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x08; //Force FD flag on.
				break;
			case GuardFrame_SB:
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->sbTimer = 0x05; //Set to a high enough value so that it doesn't tick baack to 0.
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->sbCooldownTimer = 0x00; //This needs to be set to SB consecutive attacks.
				break;
			}
		});
	trainerItems.emplace_back("Auto throw tech:", 2, "de:AutoThrowTech", TrainerItemType_Bool,
		"Automatically techs regular throws.", false);
	trainerItems.emplace_back("Auto late throw tech:", 2, "de:AutoLateThrowTech", TrainerItemType_Bool,
		"Automatically techs throws using the late throw animation.", false);
	trainerItems.emplace_back("Throw tech probability:", 2, "de:TTProb", TrainerItemType_Float,
		"Sets the probability to tech throws when auto throw tech is on.", 0.0f, 1.0f, 1.0f);
	trainerItems.emplace_back("Super armor:", 2, "de:SArmor", TrainerItemType_Bool,
		"Sets super armor effect to always on.", false);
	trainerItems.emplace_back("Hyper armor:", 2, "de:HArmor", TrainerItemType_Bool,
		"Sets hyper armor effect to always on. This is the effect\nused in EX Dizzy's Necro Install.", false);

	//Stylish mode
	//trainerItems.emplace_back("Stylish mode:", 2, "sm:StylishMode", TrainerItemType_Bool,
	//	"Enables stylish mode.", false);

	//Character specific
	trainerItems.emplace_back("Infinite DI:", 2, "cs:so:InfDI", TrainerItemType_Bool,
		"Gives infinite dragon install.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "so"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0x05;
		}, "so");
	trainerItems.emplace_back("Enable hyper gunflame:", 2, "cs:so:HyperGF", TrainerItemType_Bool,
		"Changes gunflame to the special version only used\nby the CPU in survival and mission modes.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "so");

	trainerItems.emplace_back("Enable dash:", 2, "cs:po:Dash", TrainerItemType_Bool,
		"Allows Potemkin to dash forward.", false, [](uint8_t flags, bool active, int playerNr) {}, "po");
	trainerItems.emplace_back("Enable air dash:", 2, "cs:po:AirDash", TrainerItemType_Bool,
		"Allows Potemkin to air dash.", false, [](uint8_t flags, bool active, int playerNr) {}, "po");
	trainerItems.emplace_back("Always flick:", 2, "cs:po:FDB", TrainerItemType_Bool,
		"Shoots projectile every time F.D.B is used.", false, [](uint8_t flags, bool active, int playerNr) {}, "po");

	trainerItems.emplace_back("Fast shuriken:", 2, "cs:ch:FShuriken", TrainerItemType_Bool,
		"Sets Chipp's shuriken to always be fast.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ch"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT4 = 0x01;
		}, "ch");
	trainerItems.emplace_back("Always invisible:", 2, "cs:ch:Inv", TrainerItemType_Bool,
		"Makes Chipp invisible all the time.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ch"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 0x05;
		}, "ch");

	trainerItems.emplace_back("Infinite Eddie gauge:", 2, "cs:ed:Gauge", TrainerItemType_Bool,
		"Gives infinite Eddie gauge.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ed"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 8000;
		}, "ed");

	trainerItems.emplace_back("Item:", 2, "cs:fa:Item", TrainerItemType_List,
		"Allows controlling the items thrown.", faustItems, 14, 0,
		[](uint8_t flags, int index, int playerNr) {}, "fa");

	trainerItems.emplace_back("Infinite dolls:", 2, "cs:te:Dolls", TrainerItemType_Bool,
		"Gives infinite puppets.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "te"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 0x01;
		}, "te");
	trainerItems.emplace_back("Crow always attacks:", 2, "cs:te:CrowActive", TrainerItemType_Bool,
		"Sets crow to always attack.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "te"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0x05;
		}, "te");
	trainerItems.emplace_back("Crow attack type:", 2, "cs:te:CrowAttack", TrainerItemType_List,
		"Sets crow attack to a specific attack.", testCrowAttacks, 5, 0,
		[](uint8_t flags, int index, int playerNr) {}, "te");
	trainerItems.emplace_back("Item:", 2, "cs:te:CrowPattern", TrainerItemType_List,
		"Sets the crow's attack pattern.", testCrowPatterns, 5, 0,
		[](uint8_t flags, int index, int playerNr) {
			if (index && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "te"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT6 = index - 1;
		}, "te");

	trainerItems.emplace_back("Infinite cards:", 2, "cs:ja:Cards", TrainerItemType_Bool,
		"Gives infinite cards (not necessarily shown).", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ja"))
			{
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT2 = 0x01;
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 0x0101;
			}
		}, "ja");
	trainerItems.emplace_back("Auto parry:", 2, "cs:ja:Parry", TrainerItemType_Bool,
		"Sets Jam's standing block to parry.", false, [](uint8_t flags, bool active, int playerNr) {}, "ja");
	
	trainerItems.emplace_back("Infinite coins:", 2, "cs:jo:Coins", TrainerItemType_Bool,
		"Gives infinite coins.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "jo"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 0;
		}, "jo");
	trainerItems.emplace_back("Mist finer level:", 2, "cs:jo:MF", TrainerItemType_List,
		"Sets mist finer level.", attackLevels, 4, 0,
		[](uint8_t flags, int index, int playerNr) {
			if (index && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "jo"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = (uint16_t)(index - 1);
		}, "jo");

	trainerItems.emplace_back("Stinger charge:", 2, "cs:ve:Stinger", TrainerItemType_List,
		"Sets level of venoms charge attacks.", attackLevels, 4, 0,
		[](uint8_t flags, int index, int playerNr) {
			if (!strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ve"))
			{
				switch (index)
				{
				case AttackLevel_Lv1:
					g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0;
					break;
				case AttackLevel_Lv2:
					g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0x1700;
					break;
				case AttackLevel_Lv3:
					g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0x6400;
					break;
				}
			}
		}, "ve");

	trainerItems.emplace_back("Enable WOL:", 2, "cs:di:WOL", TrainerItemType_Bool,
		"Enables performing Dizzy's boss move \"Wings of Light\" by pressing P+K+S+H.\nConsumes burst gauge.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "di");

	trainerItems.emplace_back("Enable Megalomania:", 2, "cs:in:Megalomania", TrainerItemType_Bool,
		"Enables performing I-No's boss move \"Megalomania\" by performing\n63214(x2)+P/K/S.\nConsumes burst gauge.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "in");
	trainerItems.emplace_back("Megalomania type:", 2, "cs:in:MegType", TrainerItemType_List,
		"Enables setting megalomania pattern to a specific one regardless of input.", inoMegalomaniaPatterns, 4, 0,
		[](uint8_t flags, int index, int playerNr) {
			if (index && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "in"))
				*g_gameVals.pMegalomaniaType[playerNr] = (byte)index - 1;
		}, "in");

	trainerItems.emplace_back("Summon type:", 2, "cs:za:Summon", TrainerItemType_List,
		"Sets summon to specific type.", zappaSummons, 5, 0,
		[](uint8_t flags, int index, int playerNr) {}, "za");
	trainerItems.emplace_back("Infinite Raoh:", 2, "cs:za:InfRaoh", TrainerItemType_Bool,
		"Gives infinite Raoh gauge.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "za"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 1000;
		}, "za");
	trainerItems.emplace_back("Total nightmare:", 2, "cs:za:Nightmare", TrainerItemType_Bool,
		"Trust me, it's terrifying.", false, [](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "za"))
			{
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 1000;
				if (flags & 0x08)
				{
					g_interfaces.trainerInterface.settingUpNightmare[playerNr] = true;
					g_interfaces.trainerInterface.nightmareFrameCount[playerNr] = 0;
				}
				if (g_interfaces.trainerInterface.settingUpNightmare[playerNr])
				{
					if (g_interfaces.trainerInterface.nightmareFrameCount[playerNr] < 4)
						g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 4 - g_interfaces.trainerInterface.nightmareFrameCount[playerNr];
					else
						g_interfaces.trainerInterface.settingUpNightmare[playerNr] = false;
					g_interfaces.trainerInterface.nightmareFrameCount[playerNr]++;
				}
			}
		}, "za");

	trainerItems.emplace_back("High heat:", 2, "cs:ro:Heat", TrainerItemType_Bool,
		"Sets heat gauge to 80%, giving powdered up normal moves.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ro"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT3 = 8001;
		}, "ro");

	trainerItems.emplace_back("Infinite packs:", 2, "cs:ab:InfPacks", TrainerItemType_Bool,
		"Gives infinite blood packs.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ab"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT2 = 0x01;
		}, "ab");
	trainerItems.emplace_back("Infinite berseker gauge:", 2, "cs:ab:InfGauge", TrainerItemType_Bool,
		"Gives infinite berserker gauge.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ab"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT5 = 10000;
		}, "ab");

	trainerItems.emplace_back("Attack level:", 2, "cs:os:Level", TrainerItemType_List,
		"Sets Order-Sol's attack to specified level.", attackLevels, 4, 0,
		[](uint8_t flags, int index, int playerNr) {
			if (index && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "os"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT7 = (uint16_t)(2000 * index - 1);
		}, "os");
	trainerItems.emplace_back("Enable flame distortion:", 2, "cs:os:FlameDistortion", TrainerItemType_Bool,
		"Allows performing Order-Sol's flame distortion move using 214(x2)+S.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "os");
	trainerItems.emplace_back("Infinite flame distortion:", 2, "cs:os:InfFD", TrainerItemType_Bool,
		"Gives infinite flame distortion time.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "os"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT6 = 0x05;
		}, "os");

	trainerItems.emplace_back("Infinite install:", 2, "cs:ju:InfInstall", TrainerItemType_Bool,
		"Gives infinite Omega Shift.", false,
		[](uint8_t flags, bool active, int playerNr) {
			if (active && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ju"))
				g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 = 0x05;
		}, "ju");
	trainerItems.emplace_back("Enable dash:", 2, "cs:ju:Dash", TrainerItemType_Bool,
		"Allows forward dashing.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "ju");
	trainerItems.emplace_back("Enable air dash:", 2, "cs:ju:AirDash", TrainerItemType_Bool,
		"Allows air dashing.", false,
		[](uint8_t flags, bool active, int playerNr) {}, "ju");

	//Extra Menu
	trainerItems.emplace_back("Attack:", 2, "em:Attack", TrainerItemType_Int, "",
		0, 20, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->attack = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Defense:", 2, "em:Defense", TrainerItemType_Int, "",
		0, 20, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->defense = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Speed:", 2, "em:Speed", TrainerItemType_Int, "",
		0, 20, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->speed = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Tension:", 2, "em:Tension", TrainerItemType_List, "",
		extraMenuTensionOptions, 4, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->tension = index;
		}, "", 0x77F);

	trainerItems.emplace_back("Tension rate:", 2, "em:TensionRate", TrainerItemType_Int, "",
		0, 20, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->tensionrate = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Start tension:", 2, "em:StartTension", TrainerItemType_Int, "",
		0, 8, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->starttension = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Life:", 2, "em:Life", TrainerItemType_List, "",
		extraMenuLifeOptions, 3, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->life = index;
		}, "", 0x77F);

	trainerItems.emplace_back("Burst:", 2, "em:Burst", TrainerItemType_List, "",
		extraMenuBurstOptions, 3, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->burst = index;
		}, "", 0x77F);

	trainerItems.emplace_back("Burst rate:", 2, "em:BurstRate", TrainerItemType_Int, "",
		0, 20, 0, [](uint8_t flags, int value, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->burstrate = value;
		}, "", 0x7FF);

	trainerItems.emplace_back("Guard:", 2, "em:Guard", TrainerItemType_List, "",
		extraMenuGuardOptions, 3, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guard = index;
		}, "", 0x77F);

	trainerItems.emplace_back("Guard balance:", 2, "em:GuardBal", TrainerItemType_List, "",
		extraMenuGuardBalanceOptions, 5, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guardbalance = index;
		}, "", 0x77F);

	trainerItems.emplace_back("Kizetsu:", 2, "em:Kizetsu", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->kizetsu = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Air dash:", 2, "em:AirDash", TrainerItemType_List, "",
		extraMenuAirOptions, 4, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->airdash = index;
		}, "", 0x7FF);

	trainerItems.emplace_back("Air jump:", 2, "em:AirJump", TrainerItemType_List, "",
		extraMenuAirOptions, 4, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->airjump = index;
		}, "", 0x7FF);

	trainerItems.emplace_back("Input easy:", 2, "em:InputEasy", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->inputeasy = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Easy FRC:", 2, "em:EasyFRC", TrainerItemType_List, "",
		extraMenuEasyFRCOptions, 5, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->easyfrc = index;
		}, "", 0x7FF);

	trainerItems.emplace_back("Thunder attack:", 2, "em:ThunderAtk", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->thunderattack = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Flame attack:", 2, "em:FlameAtk", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->flameattack = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Vampire attack:", 2, "em:VampireAtk", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->vampireattack = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Guard thunder:", 2, "em:GrdThunder", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guardthunder = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Guard flame:", 2, "em:GrdFlame", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guardflame = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Guard mist finer:", 2, "em:GrdMF", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guardmistfiner = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Guard beat:", 2, "em:GrdBeat", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->guardbeat = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Chaos RC:", 2, "em:CRC", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->chaosrc= active;
		}, "", 0x7FF);

	trainerItems.emplace_back("Yellow RC:", 2, "em:YRC", TrainerItemType_Bool, "",
		false, [](uint8_t flags, bool active, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->yellowrc = active;
		}, "", 0x7FF);

	trainerItems.emplace_back("All W bound:", 2, "em:AWB", TrainerItemType_List, "",
		extraMenuBoundOptions, 3, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->allwbound = index;
		}, "", 0x7FF);

	trainerItems.emplace_back("All G bound:", 2, "em:AGB", TrainerItemType_List, "",
		extraMenuBoundOptions, 2, 0, [](uint8_t flags, int index, int playerNr) {
			if (flags & 0x10)
				g_gameVals.pExtraMenu[playerNr]->allgbound = index;
		}, "", 0x7FF);

	//Game modes
	trainerItems.emplace_back("Arcade level:", 2, "gm:ArcadeLevel", TrainerItemType_Int,
		"Sets arcade level. Change value before starting\narcade mode to get the desired level.",
		1, 11, 1, [](uint8_t, int, int) {}, "", 0x01);

	trainerItems.emplace_back("Survival level:", 1, "gm:SurvivalLevel", TrainerItemType_List,
		"Set survival mode to desired boss.", survivalBosses, 51, 0,
		[](uint8_t flags, int index, int playerNr) {}, "", 0x100);

}

void TrainerInterface::OnMatchInit()
{
	updateGameValues(0x08);
	refreshSettingsFiles();
	if (disableInReplayAndOnline && (g_gameVals.GetGameMode() == GameMode_ReplayTheater || g_gameVals.GetGameMode() == GameMode_Online))
		disableTrainer();
	else
		enableTrainer();
}

void TrainerInterface::resetTrainer()
{
	for (int i = 0; i < trainerItems.size(); i++)
		GetItem(i)->ResetValue();
}

void TrainerInterface::updateGameValues(uint8_t flags)
{
	if (g_interfaces.Player1.IsCharDataNullPtr() || g_interfaces.Player2.IsCharDataNullPtr() ||
		g_interfaces.Player1.GetData()->extraData == NULL || g_interfaces.Player2.GetData()->extraData == NULL ||
		disabled)
		return;
	for (int i = 0; i < trainerItems.size(); i++)
		GetItem(i)->UpdateGameValue(flags);
}

namespace fs = boost::filesystem;
void TrainerInterface::refreshSettingsFiles()
{
	settingFiles.clear();
	fs::path dirPath(u8".\\ACPR_IM");
	if (!fs::exists(dirPath) || !fs::is_directory(dirPath))
		return;

	for (const auto& entry : fs::directory_iterator(dirPath)) {
		if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".trainer") {
			settingFiles.push_back(entry.path().stem().string());
		}
	}
}

void TrainerInterface::saveSettings(std::string filename)
{
	auto fullfile = u8".\\ACPR_IM\\" + filename + ".trainer";
	boost::filesystem::path dir("ACPR_IM");
	if (!(boost::filesystem::exists(dir))) {
		boost::filesystem::create_directory(dir);
	}
	std::ofstream outfile(fullfile, std::ios_base::binary);
	{
		boost::iostreams::filtering_stream<boost::iostreams::output> f;
		f.push(boost::iostreams::gzip_compressor(boost::iostreams::gzip_params(boost::iostreams::gzip::best_compression)));
		f.push(outfile);
		boost::archive::binary_oarchive archive(f);
		archive << trainerItems;
	}
}

void TrainerInterface::loadSettings(std::string filename)
{
	auto fullfile = u8".\\ACPR_IM\\" + filename + ".trainer";
	std::ifstream infile(fullfile, std::ios_base::binary);
	if (infile.fail()) {
		//File does not exist code here
	}
	else {
		//std::string b = "";
		//infile >> b;

		boost::iostreams::filtering_stream<boost::iostreams::input> f;
		f.push(boost::iostreams::gzip_decompressor());
		f.push(infile);
		boost::archive::binary_iarchive archive(f);
		std::vector<TrainerItem> storedItems;
		archive >> storedItems;

		
		for (TrainerItem& item : storedItems)
		{
			for (int playerNr = 0; playerNr < 2; playerNr++)
			{
				if (playerNr >= item.GetNplayers())
					continue;
				switch (item.GetType())
				{
				case TrainerItemType_Bool:
					GetItem(item.GetTag())->SetBool(item.GetBool(playerNr), playerNr);
					break;
				case TrainerItemType_Int:
					GetItem(item.GetTag())->SetInt(item.GetInt(playerNr), playerNr);
					break;
				case TrainerItemType_Float:
					GetItem(item.GetTag())->SetFloat(item.GetFloat(playerNr), playerNr);
					break;
				}
			}
		}
	}
}

void TrainerInterface::disableTrainer()
{
	disabled = true;
	for (int i = 0; i < trainerItems.size(); i++)
		trainerItems[i].Disable();
	disableTrainerHooks();
}

void TrainerInterface::enableTrainer()
{
	disabled = false;
	for (int i = 0; i < trainerItems.size(); i++)
		trainerItems[i].Enable();
	enableTrainerHooks();
}