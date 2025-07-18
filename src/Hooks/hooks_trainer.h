#pragma once
#include "Core/interfaces.h"

enum AutoBlockType_
{
	AutoBlockType_Normal,
	AutoBlockType_Low,
	AutoBlockType_Overhead,
	AutoBlockType_AntiAir,
	AutoBlockType_AutoThrowTech,
	AutoBlockType_AutoLateThrowTech,
	AutoBlockType_SuperArmor,
	AutoBlockType_HyperArmor,
	AutoBlockType_AutoParry
};

bool placeHooks_trainer();