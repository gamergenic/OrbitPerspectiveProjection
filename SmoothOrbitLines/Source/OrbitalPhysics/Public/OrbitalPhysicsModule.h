// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"



class ORBITALPHYSICS_API FOrbitalPhysicsModule : public IModuleInterface
{
public:
	static inline FOrbitalPhysicsModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FOrbitalPhysicsModule>("OrbitalPhysics");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OrbitalPhysics");
	}
};
