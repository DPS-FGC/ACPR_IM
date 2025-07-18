#include "hooks_trainer.h"

#include "Core/logger.h"
#include "HookManager.h"
#include "Game/gamestates.h"
#include <random>

bool activeP1, activeP2, activate;
uint16_t reduce;
byte playerNr;

bool checkDisableHpReduction(byte playerNr)
{
	return g_interfaces.trainerInterface.GetItem("ge:InfHealth")->GetBool(playerNr) ||
		(g_interfaces.trainerInterface.GetItem("ge:MinComboDmg")->GetInt(playerNr) > 0 &&
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->hitCount <=
			g_interfaces.trainerInterface.GetItem("ge:MinComboDmg")->GetInt(playerNr)) ||
		(g_interfaces.trainerInterface.GetItem("ge:MaxComboDmg")->GetInt(playerNr) > 0 &&
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->hitCount >=
			g_interfaces.trainerInterface.GetItem("ge:MaxComboDmg")->GetInt(playerNr));
}

bool checkUnlockAllGatlings(byte playerNr)
{
	return g_interfaces.trainerInterface.GetItem("ge:UnGatling")->GetBool(playerNr);
}

bool checkWhiffGatlings(byte playerNr)
{
	return g_interfaces.trainerInterface.GetItem("ge:WhiffGatling")->GetBool(playerNr);
}

bool checkJumpCancelAllMoves(byte playerNr)
{
	return g_interfaces.trainerInterface.GetItem("ge:UnJumpCancel")->GetBool(playerNr) ||
		g_interfaces.GetPlayer(playerNr)->GetData()->attackFlags & 0x00040000;
}

bool checkDisableRecovery(byte playerNr)
{
	return g_interfaces.trainerInterface.GetItem("ge:DisRecovery")->GetBool(playerNr) ||
		!(g_interfaces.GetPlayer(playerNr)->GetData()->commandFlags & 0x0800);
}

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(0.0, 1.0);
double test;
bool disableCH = false, reroll = true;

DWORD JamParryFunctionAddr = 0;
DWORD SuperArmorFunctionAddr = 0;
DWORD HyperArmorFunctionAddr = 0;

bool checkAutoBlock(byte playerNr, AutoBlockType_ type) {
	switch (type)
	{
	case AutoBlockType_Normal:
		return g_interfaces.trainerInterface.GetItem("de:AutoBlock")->GetBool(playerNr);
	case AutoBlockType_AutoThrowTech: //Auto throw tech
		return g_interfaces.trainerInterface.GetItem("de:AutoThrowTech")->GetBool(playerNr);
		break;
	case AutoBlockType_AutoLateThrowTech: //Auto late throw tech
		return g_interfaces.trainerInterface.GetItem("de:AutoLateThrowTech")->GetBool(playerNr);
		break;
	case AutoBlockType_SuperArmor: //Auto throw tech
		return g_interfaces.trainerInterface.GetItem("de:SArmor")->GetBool(playerNr);
		break;
	case AutoBlockType_HyperArmor: //Auto throw tech
		return g_interfaces.trainerInterface.GetItem("de:HArmor")->GetBool(playerNr);
		break;
	case AutoBlockType_AutoParry:
		return g_interfaces.trainerInterface.GetItem("cs:ja:Parry")->GetBool(playerNr);
		break;
	}
}

float getBlockProbability(byte playerNr, AutoBlockType_ type) {
	switch (type)
	{
	case AutoBlockType_Normal: //Normal
		return g_interfaces.trainerInterface.GetItem("de:NormBlockProb")->GetFloat(playerNr);
		break;
	case AutoBlockType_Low: //Low
		return g_interfaces.trainerInterface.GetItem("de:LowBlockProb")->GetFloat(playerNr);
		break;
	case AutoBlockType_Overhead: //Overhead
		return g_interfaces.trainerInterface.GetItem("de:OHBlockProb")->GetFloat(playerNr);
		break;
	case AutoBlockType_AntiAir: //AntiAir
		return g_interfaces.trainerInterface.GetItem("de:AABlockProb")->GetFloat(playerNr);
		break;
	case AutoBlockType_AutoThrowTech:
		return g_interfaces.trainerInterface.GetItem("de:TTProb")->GetFloat(playerNr);
		break;
	}
}

bool canBlock(int playerNr)
{
	std::vector<Hitbox> hitboxSet = HitboxReader::getHitboxes(g_interfaces.GetPlayer(playerNr)->GetData());
	bool hasHitbox = false;
	for (const Hitbox& entry : hitboxSet)
	{
		if (entry.type == HitboxType_Hitbox)
			hasHitbox = true;
	}
	//Counter type move
	if (g_interfaces.GetPlayer(playerNr)->GetData()->mark == 1 &&
		MoveData::IsActiveByMark(g_interfaces.GetPlayer(playerNr)->GetData()->charIndex,
			g_interfaces.GetPlayer(playerNr)->GetData()->actId))
	{
		return false;
	}
	else if (((*g_gameVals.pGlobalThrowFlags & 0x1) > 0 && !playerNr &&          //During throws
		(g_interfaces.GetPlayer(!playerNr)->GetData()->status & 0x0020) > 0 ||
		(*g_gameVals.pGlobalThrowFlags & 0x2) > 0 && playerNr &&
		(g_interfaces.GetPlayer(playerNr)->GetData()->status & 0x0020) > 0) &&
		!((g_interfaces.GetPlayer(playerNr)->GetData()->commandFlags & 0x2000) > 0))
	{
		return false;
	}
	else if (hasHitbox && !(g_interfaces.GetPlayer(playerNr)->GetData()->status & 0x0040) > 0) //During active frames
	{
		return false;
	}
	else if ((g_interfaces.GetPlayer(playerNr)->GetData()->status & 0x0020) > 0)               //During hit stun
	{
		return false;
	}
	else if ((g_interfaces.GetPlayer(playerNr)->GetData()->attackFlags & 0x0800) > 0)          //During recovery
	{
		return false;
	}
	else if ((g_interfaces.GetPlayer(playerNr)->GetData()->commandFlags & 0xFFFF) == 0xC05F &&  //During move which is not recovery
		!(g_interfaces.GetPlayer(playerNr)->GetData()->status & 0x0200) > 0)
	{
		return false;
	}
	else if ((g_interfaces.GetPlayer(playerNr)->GetData()->commandFlags & 0x1000) > 0)  //During pre-jump
	{
		return false;
	}
	else
	{
		uint16_t actId = g_interfaces.GetPlayer(!playerNr)->GetData()->actId;
		switch (g_interfaces.GetPlayer(!playerNr)->GetData()->charIndex)
		{
		case 6: //Potemkin
			if (actId == 114) //Heat grab
				return false;
			break;
		case 22: //A.B.A
			if ((actId == 274 && //Ground key grab
				g_interfaces.GetPlayer(!playerNr)->GetData()->mark == 2) ||   // Blockable part of key-grab
				actId == 278)  //Air key grab
				g_interfaces.GetPlayer(!playerNr)->GetData()->hitcolliflag = -2; //Set to -2 so that A.B.A doesn't continue with the animation.
			break;
		}
	}
	return true;
}

bool checkEnableEnhancedGunFlame(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:so:HyperGF")->GetBool(playerNr);
}

bool checkPotemkinSpecific(byte playerNr, int type) {
	switch (type)
	{
	case 0: //Enable dash
		return g_interfaces.trainerInterface.GetItem("cs:po:Dash")->GetBool(playerNr);
		break;
	case 1: //Enable air dash
		return g_interfaces.trainerInterface.GetItem("cs:po:AirDash")->GetBool(playerNr);
		break;
	case 2: //Always flick
		return g_interfaces.trainerInterface.GetItem("cs:po:FDB")->GetBool(playerNr);
		break;
	}
}

bool checkEnableWOL(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:di:WOL")->GetBool(playerNr);
}

bool checkEnableMegalomania(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:in:Megalomania")->GetBool(playerNr);
}

bool checkEnableFlameDistortion(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:os:FlameDistortion")->GetBool(playerNr);
}

bool checkJusticeDash(byte playerNr, int type) {
	switch (type)
	{
	case 0: //Ground dash
		return g_interfaces.trainerInterface.GetItem("cs:ju:Dash")->GetBool(playerNr);
		break;
	case 1: //Air dash
		return g_interfaces.trainerInterface.GetItem("cs:ju:AirDash")->GetBool(playerNr);
		break;
	}
}

bool __stdcall SafeHookChecks(byte playerNr, int id)
{
	switch (id)
	{
	case 0:
		return checkDisableHpReduction(playerNr);
		break;
	case 1:
		if (checkWhiffGatlings(playerNr))
			g_interfaces.GetPlayer(playerNr)->GetData()->attackFlags |= 0x1000; //Set attack has connected flag to on.
		if (checkUnlockAllGatlings(playerNr))
		{
			g_interfaces.GetPlayer(playerNr)->GetData()->attackFlags |= 0x10; //Set has gatlings flag to on.
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->gatlingFlags = 0xFFFFFFFF; //Set all gatling flags to on.
		}
		break;
	case 2:
		return checkJumpCancelAllMoves(playerNr);
		break;
	case 3:
		return checkDisableRecovery(playerNr);
		break;
	case 4:
		if (reroll) //Do not generate another number if test was already performed to disable counter hit effect
			test = dis(gen);
		if (disableCH) //Test for block was already performed, so set test = 0 to always succeed.
			test = 0.0f;
		disableCH = false;
		reroll = true;
		activate = checkAutoBlock(playerNr, AutoBlockType_Normal);

		if (activate)
		{
			if (canBlock(playerNr))
			{
				if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0c00) == 0xc00)  //Check if attack can be blocked both ways
				{
					if (test < getBlockProbability(playerNr, AutoBlockType_Normal))
						g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0001;               //Set normal guard flag
				}
				else if (g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0400)    //Check if enemy attack needs to be blocked high
				{
					if (test < getBlockProbability(playerNr, AutoBlockType_Overhead))
						g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0001;               //Set normal guard flag
				}
				else if (g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0800)  //Check if enemy attack needs to be blocked low.
					if (test < getBlockProbability(playerNr, AutoBlockType_Low))
						g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0002;                   //Set low guard flag
			}
		}

		if (checkAutoBlock(playerNr, AutoBlockType_AutoParry) && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ja"))
		{
			if (canBlock(playerNr))
			{
				if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0400) > 0)
				{
					g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x1001;   //Turn on parry flag + standing block
					g_interfaces.GetPlayer(playerNr)->GetData()->extraData->specialFunction = JamParryFunctionAddr; //Set special function to Jam parry
				}
			}
		}

		if (checkAutoBlock(playerNr, AutoBlockType_SuperArmor))
		{
			g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0400;  //Turn on armor flag
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->specialFunction = SuperArmorFunctionAddr;  //Set special function to super armor
		}
		if (checkAutoBlock(playerNr, AutoBlockType_HyperArmor))
		{
			g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0400;  //Turn on armor flag
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->specialFunction = HyperArmorFunctionAddr;  //Set special function to hyper armor
		}
		break;
	case 5:
		if (reroll)  //Do not generate another number if test was already performed to disable counter hit effect
			test = dis(gen);
		if (disableCH) //Test for block was already performed, so set test = 0 to always succeed.
			test = 0.0f;
		disableCH = false;
		reroll = true;

		if (checkAutoBlock(playerNr, AutoBlockType_Normal))
		{
			if (canBlock(playerNr))
			{
				if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x1000))  //Check if attack can be blocked in the air
				{
					if (test < getBlockProbability(playerNr, AutoBlockType_Normal))
						g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0004;               //Set air guard flag
				}
				else
				{
					if (test < getBlockProbability(playerNr, AutoBlockType_AntiAir))
						g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x000C;               //Set air guard + FD flags
				}
			}
		}
		break;
	case 6:
		if (checkAutoBlock(0, AutoBlockType_AutoThrowTech))
		{
			if (*g_gameVals.pGlobalThrowFlags & 0x02) //Check if P2 is throwing
				if (test < getBlockProbability(0, AutoBlockType_AutoThrowTech))
					return true;
		}

		if (checkAutoBlock(1, AutoBlockType_AutoThrowTech))
		{
			if (*g_gameVals.pGlobalThrowFlags & 0x01) //Check if P1 is throwing
				if (test < getBlockProbability(1, AutoBlockType_AutoThrowTech))
					return true;
		}
		return false;
		break;
	case 7:
		test = dis(gen);

		if (checkAutoBlock(playerNr, AutoBlockType_AutoLateThrowTech))
		{
			if (g_interfaces.GetPlayer(playerNr)->GetData()->extraData->lateThrowFlag & 0x01)     //Check if being thrown
				if (test < getBlockProbability(playerNr, AutoBlockType_AutoThrowTech))
					g_interfaces.GetPlayer(playerNr)->GetData()->extraData->lateThrowFlag = 0x0F;
		}
		break;
	case 8:
		if (checkAutoBlock(playerNr, AutoBlockType_SuperArmor))
		{
			g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0400;  //Turn on armor flag
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->specialFunction = SuperArmorFunctionAddr;  //Set special function to super armor
		}
		if (checkAutoBlock(playerNr, AutoBlockType_HyperArmor))
		{
			g_interfaces.GetPlayer(playerNr)->GetData()->blockType |= 0x0400;  //Turn on armor flag
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->specialFunction = HyperArmorFunctionAddr;  //Set special function to hyper armor
		}
		break;
	case 9:
		return checkEnableEnhancedGunFlame(playerNr);
		break;
	case 10:
		return (checkPotemkinSpecific(playerNr, 0) || *g_gameVals.pSpecialMode[playerNr]);
		break;
	case 11:
		return (checkPotemkinSpecific(playerNr, 1) || *g_gameVals.pSpecialMode[playerNr]);
		break;
	case 12:
		return checkPotemkinSpecific(playerNr, 2);
		break;
	case 15:
		activate = checkEnableWOL(playerNr) && !strcmp(g_interfaces.Player1.GetCharAbbr(), "di")
			&& ((g_interfaces.cbrInterface.inputMemory[playerNr] & 0xF0) == 0xF0) && //Use inputs stored in cbrInterface to check for P+K+S+H
			(g_interfaces.GetPlayer(playerNr)->GetData()->extraData->burstGauge == 15000);
		if (activate)
			g_interfaces.GetPlayer(playerNr)->GetData()->extraData->burstGauge = 0; //Consume burst gauge to mimic how the attack is used by the CPU.
		return activate;
		break;
	case 16:
		return (*g_gameVals.pExMode[playerNr] ||
			(checkEnableMegalomania(playerNr) && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "in")));
		break;
	case 18:
		return (*g_gameVals.pSpecialMode[playerNr] || checkEnableFlameDistortion(playerNr));
		break;
	case 19:
		return g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 > 0 || checkJusticeDash(playerNr, 0);
		break;
	case 20:
		return g_interfaces.GetPlayer(playerNr)->GetData()->extraData->characterSLOT1 > 0 || checkJusticeDash(playerNr, 1);
		break;
	case 21:
		test = dis(gen);
		activate = checkAutoBlock(playerNr, AutoBlockType_Normal);

		if (activate)
		{
			if (canBlock(playerNr))
			{
				reroll = false;
				if ((g_interfaces.GetPlayer(playerNr)->GetData()->status & 0x0010) > 0)  //If airborne
				{
					if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x1000))  //Check if attack can be blocked in the air
					{
						if (test < getBlockProbability(playerNr, AutoBlockType_Normal))
							disableCH = true;
					}
					else
					{
						if (test < getBlockProbability(playerNr, AutoBlockType_AntiAir))
							disableCH = true;
					}
				}
				else
				{
					if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0c00) == 0xc00)  //Check if attack can be blocked both ways
					{
						if (test < getBlockProbability(playerNr, AutoBlockType_Normal))
							disableCH = true;
					}
					else if (g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0400)    //Check if enemy attack needs to be blocked high
					{
						if (test < getBlockProbability(playerNr, AutoBlockType_Overhead))
							disableCH = true;
					}
					else if (g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0800)  //Check if enemy attack needs to be blocked low.
						if (test < getBlockProbability(playerNr, AutoBlockType_Low))
							disableCH = true;
				}
			}
		}

		if (checkAutoBlock(playerNr, AutoBlockType_AutoParry) && !strcmp(g_interfaces.GetPlayer(playerNr)->GetCharAbbr(), "ja"))
		{
			if (canBlock(playerNr))
			{
				reroll = false;
				if ((g_interfaces.GetPlayer(playerNr)->GetData()->enemyAttackFlags & 0x0400) > 0)
					disableCH = true;
			}
		}
		return disableCH;
		break;
	default:
		return false;
		break;
	}
	return true;
}

DWORD DisableHPReductionJmpBackAddr = 0;
void __declspec(naked)DisableHPReduction()
{
	__asm
	{
		pushad
		mov al, byte ptr [ebx + 27h]
		mov playerNr, al
		mov reduce, si
	}

	if (SafeHookChecks(playerNr, 0))
		reduce = 0;

	__asm
	{
		popad
		mov si, reduce
		sub[ebx + 1Eh], si
		mov ecx, [ebp + 10h]
		jmp[DisableHPReductionJmpBackAddr]
	}
}

DWORD UnlockAllGatlingsJmpBackAddr = 0;
void __declspec(naked)UnlockAllGatlings()
{
	__asm
	{
		pushad
		mov al, [ebx + 27h]
		mov playerNr, al
	}

	SafeHookChecks(playerNr, 1);

	__asm
	{
		popad
		mov eax, [ebx + 34h]
		test eax, 00001000h
		//mov eax, [edx + 74h]
		//mov ecx, ebx
		jmp[UnlockAllGatlingsJmpBackAddr]
	}

}

DWORD JumpCancelAllMovesJmpBackAddr = 0;
void __declspec(naked)JumpCancelAllMoves()
{
	__asm
	{
		pushad
		mov al, byte ptr [ebx + 27h]
		mov playerNr, al
	}

	activate = SafeHookChecks(playerNr, 2);

	__asm
	{
		popad
		//test eax, 00040000h
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[JumpCancelAllMovesJmpBackAddr]
	}

}

DWORD DisableRecoveryJmpBackAddr = 0;
void __declspec(naked)DisableRecovery()
{
	__asm
	{
		pushad
		mov al, [ebx + 27h]
		mov playerNr, al
	}

	activate = !SafeHookChecks(playerNr, 3);

	__asm
	{
		popad
		//test[ebx + 38h], 00000800h
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[DisableRecoveryJmpBackAddr]
	}

}

DWORD DisableCounterHitJmpBackAddr = 0;
DWORD CheckCounterHitAddr = 0;
void __declspec(naked)DisableCounterHit()
{
	__asm
	{
		call[CheckCounterHitAddr]
		mov activate, al
		pushad
		mov al, byte ptr[ebx + 27h]
		mov playerNr, al
	}

	if (activate)
		activate &= !SafeHookChecks(playerNr, 21);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[DisableCounterHitJmpBackAddr]
	}
}

DWORD DisableMistFinerBlockJmpBackAddr = 0;
void __declspec(naked)DisableMistFinerBlock()
{
	reroll = false;
	test = 1.0;
	__asm
	{
		and ebx, 0FFFFBFF0h
		jmp[DisableMistFinerBlockJmpBackAddr]
	}
}

DWORD AutoGroundGuardJmpBackAddr = 0;
DWORD FailedGroundGuardJmpAddr = 0;
void __declspec(naked)AutoGroundGuard()
{
	__asm
	{
		pushad
		mov al, byte ptr [ebx + 27h]
		mov playerNr, al
	}

	SafeHookChecks(playerNr, 4);

	__asm
	{
		popad
		test byte ptr[ebx + 2Ah], 03h
		je NOBLOCK
		jmp[AutoGroundGuardJmpBackAddr]
	NOBLOCK:
		jmp[FailedGroundGuardJmpAddr]
	}

}

DWORD AutoAirGuardJmpBackAddr = 0;
DWORD FailedAirGuardJmpAddr = 0;
void __declspec(naked)AutoAirGuard()
{
	__asm
	{
		pushad
		mov al, byte ptr[ebx + 27h]
		mov playerNr, al
	}

	SafeHookChecks(playerNr, 5);

	__asm
	{
		popad
		test byte ptr[ebx + 2Ah], 04h
		je NOBLOCK
		jmp[AutoAirGuardJmpBackAddr]
	NOBLOCK:
		jmp[FailedAirGuardJmpAddr]
	}

}

DWORD AutoThrowTechJmpBackAddr = 0;
bool throwTeched = false;
void __declspec(naked)AutoThrowTech()
{
	_asm pushad

	throwTeched = SafeHookChecks(-1, 6);

	__asm
	{
		popad
		mov ecx, [g_gameVals.pGlobalThrowFlags]
		push eax
		movzx eax, throwTeched
		test eax, eax
		pop eax
		je NOTECH
		or cl, 03   //ECX holds the global throw flags. Mark both players as throwing to get a throw tech.
	NOTECH:
		jmp[AutoThrowTechJmpBackAddr]
	}

}

DWORD AutoLateThrowTechJmpBackAddr = 0;
void __declspec(naked)AutoLateThrowTech()
{
	__asm
	{
		pushad
		movzx eax, byte ptr [esi + 27h]
		mov playerNr, al
	}

	SafeHookChecks(playerNr, 7);

	__asm
	{
		popad
		mov cl, [edi + 00000112h]
		test cl, 01
		jmp[AutoLateThrowTechJmpBackAddr]
	}

}

DWORD AirArmorCheckJmpBackAddr = 0;
DWORD AirArmorJumpAddr = 0;
void __declspec(naked)AirArmorCheck()
{
	__asm
	{
		push eax
		movzx al, byte ptr [ebx + 27h]
		mov playerNr, al
		pop eax
		jne CONTINUE
		jmp[AirArmorJumpAddr]
	CONTINUE:
		pushad
	}

	SafeHookChecks(playerNr, 8);

	__asm
	{
		popad
		mov edx, 00000400h
		jmp[AirArmorCheckJmpBackAddr]
	}
}

//Character specific hooks

DWORD EnableEnhancedGunflameJmpBackAddr = 0;
DWORD NormalGunflameAddr = 0;
DWORD CheckSolBossModeAddr = 0;
DWORD DIGunFlameAddr = 0;
void __declspec(naked)EnableEnhancedGunflame()
{
	__asm
	{
		pushad
		movzx eax, byte ptr[edi + 27h]
		mov playerNr, al
	}

	activate = SafeHookChecks(playerNr, 9);

	__asm
	{
		popad
		cmp word ptr[esi + 00000088h], 00   //Check if Sol is in DI mode
		je NORMAL                           //If not, perform normal gun flame
	CONT1:
		mov eax, edi
		call[CheckSolBossModeAddr]          //Call a function to check if conditions for Enhanced gunflame are active
		test eax, eax
		je DRAGONINST                       //If not perform DI gun flame
	CONT2:
		mov esi, 02080132h                  //Set animation for Enhanced gunflame
		jmp[EnableEnhancedGunflameJmpBackAddr]
	NORMAL:
		push eax                            //If cheat is active continue the original code
		movzx eax, activate
		test eax, eax
		pop eax
		jne CONT1
		jmp[NormalGunflameAddr]
	DRAGONINST:
		push eax                            //If cheat is active continue the original code
		movzx eax, activate
		test eax, eax
		pop eax
		jne CONT2
		jmp[DIGunFlameAddr]
	}
}

DWORD EnablePotemkinGroundDashJmpBackAddr = 0;
void __declspec(naked)EnablePotemkinGroundDash()
{
	__asm
	{
		pushad
		mov playerNr, cl
	}

	activate = !SafeHookChecks(playerNr, 10);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax                    //This should set ZF to 1 when we want the cheat active
		pop eax
		jmp[EnablePotemkinGroundDashJmpBackAddr]
	}
}

DWORD EnablePotemkinAirDashJmpBackAddr = 0;
void __declspec(naked)EnablePotemkinAirDash()
{
	__asm
	{
		pushad
		mov playerNr, al
	}

	
	activate = !SafeHookChecks(playerNr, 11);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax                    //This should set ZF to 1 when we want the cheat active
		pop eax
		jmp[EnablePotemkinAirDashJmpBackAddr]
	}
}

DWORD AlwaysFDBProjectileJmpBackAddr = 0;
void __declspec(naked)AlwaysFDBProjectile()
{
	__asm
	{
		pushad
		movzx eax, byte ptr [edi + 27h]
		mov playerNr, al
	}

	activate = SafeHookChecks(playerNr, 12);

	__asm
	{
		popad
		mov esi, 020800AFh
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		je EXIT
		mov esi, 020800DFh    //Force flick projectile animation
	EXIT:
		jmp[AlwaysFDBProjectileJmpBackAddr]
	}
}

int getFaustItem(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:fa:Item")->GetInt(playerNr);
}

byte faustItemCodes[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x1E, 0x21, 0x22, 0x23, 0x27, 0x34, 0x36 };
byte itemCode = 0;

DWORD SetFaustItemJmpBackAddr = 0;
void __declspec(naked)SetFaustItem()
{
	__asm
	{
		pushad
		movzx eax, byte ptr[esi + 27h]
		mov playerNr, al
	}

	itemCode = getFaustItem(playerNr);
	if (itemCode > 0)
		itemCode = faustItemCodes[itemCode - 1];
	activate = itemCode > 0;

	__asm
	{
		popad
		mov cl, [ebp + ecx - 0000008Ch]
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		je EXIT
		mov cl, itemCode
	EXIT:
		jmp[SetFaustItemJmpBackAddr]
	}
}


int getCrowAttack(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:te:CrowAttack")->GetInt(playerNr);
}

DWORD CrowAttackArrayAddr = 0;
bool __stdcall HandleSetCrowAttack(byte playerNr, int offset, byte& outAttack, byte& outItemCode)
{
	DWORD address = CrowAttackArrayAddr + offset;
	outAttack = *(byte*)address;

	outItemCode = getCrowAttack(playerNr);
	return outItemCode > 0;
}

DWORD SetCrowAttackJmpBackAddr = 0;
void __declspec(naked)SetCrowAttack()
{
	int arrayoffset;
	DWORD address, backup;
	byte nextAttack;
	__asm
	{
		push eax
		push ebx
		movzx eax, byte ptr[edi + 27h]
		mov playerNr, al
		mov ebx, esi
		add ebx, ecx
		mov arrayoffset, ebx
		pop ebx
		pop eax

		mov backup, ebx

		pushad
	}

	activate = HandleSetCrowAttack(playerNr, arrayoffset, nextAttack, itemCode);

	__asm
	{
		popad
		mov ebx, backup
		movzx eax, nextAttack
		mov ecx, [ebp + 08h]
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		je EXIT
		movzx eax, itemCode              //Overwrite eax with custom crow attack
	EXIT:
		jmp[SetCrowAttackJmpBackAddr]
	}
}

DWORD EnableWOLJmpBackAddr = 0;
void __declspec(naked)EnableWOL()
{
	__asm
	{
		push eax
		movzx eax, byte ptr [edi + 27h]
		mov playerNr, al
		pop eax

		pushad
	}

	activate = SafeHookChecks(playerNr, 15);

	__asm
	{
		popad

		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		je NORMAL
		or esi, 04080107h //wings of light animation code
		jmp EXIT
	NORMAL:
		or esi, 00080018h
	EXIT:
		jmp[EnableWOLJmpBackAddr]
	}

}



DWORD EnableMegalomaniaJmpBackAddr = 0;
void __declspec(naked)EnableMegalomania()
{
	__asm
	{
		pushad
		mov playerNr, cl
	}

	activate = !SafeHookChecks(playerNr, 16);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax,eax
		pop eax
		jmp[EnableMegalomaniaJmpBackAddr]
	}

}

int getZappaSummonType(byte playerNr) {
	return g_interfaces.trainerInterface.GetItem("cs:za:Summon")->GetInt(playerNr);
}

DWORD SetZappaSummonJmpBackAddr = 0;
void __declspec(naked)SetZappaSummon()
{
	__asm
	{
		pushad
		movzx eax, byte ptr[ecx + 27h]
		mov playerNr, al
		mov itemCode, bl
	}

	activate = getZappaSummonType(playerNr) > 0;
	if (activate)
		itemCode = (byte)getZappaSummonType(playerNr);;

	__asm
	{
		popad
		mov bl, itemCode
		mov edx, 0xE0C00000
		jmp[SetZappaSummonJmpBackAddr]
	}
}

DWORD EnableFlameDistortionJmpBackAddr = 0;
void __declspec(naked)EnableFlameDistortion()
{
	__asm
	{
		pushad
		mov playerNr, cl
	}

	activate = !SafeHookChecks(playerNr, 18);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[EnableFlameDistortionJmpBackAddr]
	}
}

DWORD EnableJusticeGroundDashJmpBackAddr = 0;
void __declspec(naked)EnableJusticeGroundDash()
{
	__asm
	{
		pushad
		mov al, byte ptr [esi + 27h]
		mov playerNr, al
	}

	activate = SafeHookChecks(playerNr, 19);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[EnableJusticeGroundDashJmpBackAddr]
	}
}

DWORD EnableJusticeAirDashJmpBackAddr = 0;
void __declspec(naked)EnableJusticeAirDash()
{
	__asm
	{
		pushad
		mov bl, byte ptr [eax + 27h]
		mov playerNr, bl
	}

	activate = SafeHookChecks(playerNr, 20);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[EnableJusticeAirDashJmpBackAddr]
	}
}

DWORD EnableJusticeTripleJumpJmpBackAddr = 0;
void __declspec(naked)EnableJusticeTripleJump()
{
	__asm
	{
		pushad
		mov al, byte ptr[ecx + 27h]
		mov playerNr, al
	}

	activate = SafeHookChecks(playerNr, 20);

	__asm
	{
		popad
		push eax
		movzx eax, activate
		test eax, eax
		pop eax
		jmp[EnableJusticeTripleJumpJmpBackAddr]
	}
}

bool placeHooks_trainer()
{
	LOG(2, "placeHooks_trainer\n");

	DisableHPReductionJmpBackAddr = HookManager::SetHook("DisableHPReduction", "\x66\x29\x73\x1E\x8B\x4D\x10",
		"xxxxxxx", 7, DisableHPReduction, true);

	UnlockAllGatlingsJmpBackAddr = HookManager::SetHook("UnlockAllGatlings", "\x8B\x43\x34\xA9\x00\x10\x00\x00\x74\x28",
		"xxxxxxxxxx", 8, UnlockAllGatlings, true);

	JumpCancelAllMovesJmpBackAddr = HookManager::SetHook("JumpCancelAllMoves", "\xA9\x00\x00\x04\x00\x74\x11\xF6\x43\x0C\x10",
		"xxxxxxxxxxx", 5, JumpCancelAllMoves, true);

	DisableRecoveryJmpBackAddr = HookManager::SetHook("DisableRecovery", "\xF7\x43\x38\x00\x08\x00\x00\x74\x11",
		"xxxxxxxxx", 7, DisableRecovery, true);

	DisableCounterHitJmpBackAddr = HookManager::SetHook("DisableCounterHit", "\xE8\x00\x00\x00\x00\x85\xC0\x74\x0F\x8B\x43\x2C\x83\x88\xAC\x00\x00\x00\x04",
		"x????xxxxxxxxxxxxxx", 7, DisableCounterHit, false);
	CheckCounterHitAddr = DisableCounterHitJmpBackAddr + (int)HookManager::GetBytesFromAddr("DisableCounterHit", 1, 4) - 2;
	HookManager::ActivateHook("DisableCounterHit");

	DisableMistFinerBlockJmpBackAddr = HookManager::SetHook("DisableMistFinerBlock", "\x81\xE3\xF0\xBF\xFF\xFF\x66\x89\x5E\x2A",
		"xxxxxxxxxx", 6, DisableMistFinerBlock, true);

	AutoGroundGuardJmpBackAddr = HookManager::SetHook("AutoGroundGuard", "\xF6\x43\x2A\x03\x74\x22",
		"xxxxxx", 6, AutoGroundGuard, true);
	FailedGroundGuardJmpAddr = AutoGroundGuardJmpBackAddr + 0x22;

	AutoAirGuardJmpBackAddr = HookManager::SetHook("AutoAirGuard", "\xF6\x43\x2A\x04\x0F\x84\x1E\x01\x00\x00",
		"xxxxxxxxxx", 10, AutoAirGuard, true);
	FailedAirGuardJmpAddr = AutoAirGuardJmpBackAddr + 0x11E;

	AirArmorCheckJmpBackAddr = HookManager::SetHook("AirArmorCheck", "\x0F\x84\x3F\x03\x00\x00\xBA\x00\x04\x00\x00",
		"xxxxxxxxxxx", 11, AirArmorCheck, true);
	AirArmorJumpAddr = AirArmorCheckJmpBackAddr - 5 + 0x33F;

	AutoThrowTechJmpBackAddr = HookManager::SetHook("AutoThrowTech", "\x8B\x0D\x00\x00\x00\x00\x8B\xC1\x23\xC2\x74\x05",
		"xx????xxxxxx", 6, AutoThrowTech, false);
	g_gameVals.pGlobalThrowFlags  = (byte*)HookManager::GetBytesFromAddr("AutoThrowTech", 2, 4);
	HookManager::ActivateHook("AutoThrowTech");

	AutoLateThrowTechJmpBackAddr = HookManager::SetHook("AutoLateThrowTech", "\x8A\x8F\x12\x01\x00\x00\xF6\xC1\x01",
		"xxxxxxxxx", 9, AutoLateThrowTech, true);

	HookManager::RegisterHook("GetSpecialModeP1", "\x89\x3C\xB5\x00\x00\x00\x00\x83\xFB\x13",
		"xxx????xxx", 7);
	g_gameVals.pSpecialMode[0] = (byte*)HookManager::GetBytesFromAddr("GetSpecialModeP1", 3, 4);
	g_gameVals.pSpecialMode[1] = (byte*)((char*)g_gameVals.pSpecialMode[0] + 4);

	EnableEnhancedGunflameJmpBackAddr = HookManager::SetHook("EnableEnhancedGunflame",
		"\x66\x83\xBE\x88\x00\x00\x00\x00\x74\x31\x8B\xC7\xE8\x00\x00\x00\x00\x85\xC0\x74\x13\xBE\x32\x01\x08\x02",
		"xxxxxxxxxxxxx????xxxxxxxxx", 26, EnableEnhancedGunflame, false);
	NormalGunflameAddr = EnableEnhancedGunflameJmpBackAddr + 0x21;
	DWORD offset = (DWORD)HookManager::GetBytesFromAddr("EnableEnhancedGunflame", 13, 4);
	CheckSolBossModeAddr = EnableEnhancedGunflameJmpBackAddr - 9 + offset;
	DIGunFlameAddr = EnableEnhancedGunflameJmpBackAddr + 0x0E;
	HookManager::ActivateHook("EnableEnhancedGunflame");

	EnablePotemkinGroundDashJmpBackAddr = HookManager::SetHook("EnablePotemkinGroundDash", "\x83\x3C\x8D\x00\x00\x00\x00\x01\x75\x4D",
		"xxx????xxx", 8, EnablePotemkinGroundDash, true);

	EnablePotemkinAirDashJmpBackAddr = HookManager::SetHook("EnablePotemkinAirDash", "\x83\x3C\x85\x00\x00\x00\x00\x01\x75\x32",
		"xxx????xxx", 8, EnablePotemkinAirDash, true);

	AlwaysFDBProjectileJmpBackAddr = HookManager::SetHook("AlwaysFDBProjectile", "\xBE\xAF\x00\x08\x02\xE8\x4E\x21\x05\x00",
		"xxxxxxxxxx", 5, AlwaysFDBProjectile, true);

	SetFaustItemJmpBackAddr = HookManager::SetHook("SetFaustItem", "\x8A\x8C\x0D\x74\xFF\xFF\xFF",
		"xxxxxxx", 7, SetFaustItem, true);

	SetCrowAttackJmpBackAddr = HookManager::SetHook("SetCrowAttack", "\x0f\xB6\x84\x0E\x00\x00\x00\x00\x8B\x4D\x08",
		"xxxx????xxx", 11, SetCrowAttack, false);
	CrowAttackArrayAddr = (DWORD)HookManager::GetBytesFromAddr("SetCrowAttack", 4, 4);
	HookManager::ActivateHook("SetCrowAttack");

	HookManager::RegisterHook("EnableJamAutoParry", "\xBA\x01\x10\x00\x00\xC7\x81\xC8\x00\x00\x00\x00\x00\x00\x00\x66\x89\x56\x2A",
		"xxxxxxxxxxx????xxxx", 5);
	JamParryFunctionAddr = (DWORD)HookManager::GetBytesFromAddr("EnableJamAutoParry", 11, 4);

	HookManager::RegisterHook("EnableSuperArmor", "\xC7\x46\x38\x5F\xC0\x00\x00\xC7\x80\xC8\x00\x00\x00\x00\x00\x00\x00\x8B\x4E\x2C",
		"xxxxxxxxxxxxx????xxx", 7);
	SuperArmorFunctionAddr = (DWORD)HookManager::GetBytesFromAddr("EnableSuperArmor", 13, 4);

	HookManager::RegisterHook("EnableHyperArmor", "\xC7\x82\xC8\x00\x00\x00\x00\x00\x00\x00\x81\x8E\xAC\x00\x00\x00\x00\x40\x00\x00",
		"xxxxxx????xxxxxxxxxx", 10);
	HyperArmorFunctionAddr = (DWORD)HookManager::GetBytesFromAddr("EnableHyperArmor", 6, 4);

	EnableWOLJmpBackAddr = HookManager::SetHook("EnableWOL", "\x81\xCE\x18\x00\x08\x00", "xxxxxx", 6, EnableWOL, true);

	EnableMegalomaniaJmpBackAddr = HookManager::SetHook("EnableMegalomania", "\x83\x3C\x8D\x00\x00\x00\x00\x01\x0F\x85\xDE\x00\x00\x00",
		"xxx????xxxxxxx", 8, EnableMegalomania, false);
	g_gameVals.pExMode[0] = (byte*)HookManager::GetBytesFromAddr("EnableMegalomania", 3, 4);
	g_gameVals.pExMode[1] = (byte*)((char*)g_gameVals.pExMode[0] + 4);

	HookManager::ActivateHook("EnableMegalomania");

	HookManager::RegisterHook("GetMegalomaniaType", "\xC6\x81\x00\x00\x00\x00\x01\xEB\x0B\x0F\xB6\x57\x27",
		"xx????xxxxxxx", 7);
	g_gameVals.pMegalomaniaType[0] = (byte*)HookManager::GetBytesFromAddr("GetMegalomaniaType", 2, 4);
	g_gameVals.pMegalomaniaType[1] = (byte*)((char*)g_gameVals.pMegalomaniaType[0] + 1);

	SetZappaSummonJmpBackAddr = HookManager::SetHook("SetZappaSummon", "\xBA\x00\x00\xC0\xE0\x57", "xxxxxx", 5, SetZappaSummon, true);

	EnableFlameDistortionJmpBackAddr = HookManager::SetHook("EnableFlameDistortion", "\x83\x3C\x8D\x00\x00\x00\x00\x01\x75\x05\xB8\x01\x00\x00\x00",
		"xxx????xxxxxxxx", 8, EnableFlameDistortion, true);

	EnableJusticeGroundDashJmpBackAddr = HookManager::SetHook("EnableJusticeGroundDash", "\x66\x83\xB8\x88\x00\x00\x00\x00\x74\x24",
		"xxxxxxxxxx", 8, EnableJusticeGroundDash, true);

	EnableJusticeAirDashJmpBackAddr = HookManager::SetHook("EnableJusticeAirDash", "\x66\x83\xB9\x88\x00\x00\x00\x00\x7E\x06",
		"xxxxxxxxxx", 8, EnableJusticeAirDash, true);

	EnableJusticeTripleJumpJmpBackAddr = HookManager::SetHook("EnableJusticeTripleJump", "\x66\x83\xB8\x88\x00\x00\x00\x00\x7E\x06\xC6\x40\x70\x02",
		"xxxxxxxxxxxxxx", 8, EnableJusticeTripleJump, true);

	HookManager::RegisterHook("GetExtraMenuPointer", "\x8D\xB3\x00\x00\x00\x00\x51\xE8\x00\x00\x00\x00\x83\xC4\x04",
		"xx????xx????xxx", 6);
	g_gameVals.pExtraMenu[0] = (ExtraMenu*)HookManager::GetBytesFromAddr("GetExtraMenuPointer", 2, 4);
	g_gameVals.pExtraMenu[1] = (ExtraMenu*)((char*)g_gameVals.pExtraMenu[0] + 0x98);


	HookManager::RegisterHook("GetArcadeLevel", "\x66\x01\x14\x45\x00\x00\x00\x00\x8B\xC2", "xxxx????xx", 8);
	g_gameVals.pArcadeLevel_minus_1[0] = (uint16_t*)HookManager::GetBytesFromAddr("GetArcadeLevel", 4, 4);
	g_gameVals.pArcadeLevel_minus_1[1] = (uint16_t*)((char*)g_gameVals.pArcadeLevel_minus_1[0] + 2);

	HookManager::RegisterHook("GetSurvivalLevel", "\x66\xA3\x00\x00\x00\x00\x33\xD2\x89\x0D\x00\x00\x00\x00",
		"xx????xxxx????", 6);
	g_gameVals.pSurvivalLevel = (uint16_t*)HookManager::GetBytesFromAddr("GetSurvivalLevel", 2, 4);

	HookManager::RegisterHook("GetDaredevilCounter", "\x66\x89\x15\x00\x00\x00\x00\x33\xD2\x33\xC0", "xxx????xxxx", 7);
	g_gameVals.pDaredevilCounter = (uint16_t*)HookManager::GetBytesFromAddr("GetDaredevilCounter", 3, 4);

	return true;
}