// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"



class ORBITRENDERING_API FOrbitRenderingModule : public IModuleInterface
{
public:
	static inline FOrbitRenderingModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FOrbitRenderingModule>("OrbitRendering");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("OrbitRendering");
	}
};
