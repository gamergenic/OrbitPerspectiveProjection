// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "OrbitalMechanics.h"
#include "OrbitSystemStateComponent.generated.h"

/**
 * 
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ORBITALPHYSICS_API UOrbitSystemStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOrbitSystemStateComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Current Ephemeris Time"))
    double et;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Ephemeris Time Multiplier"))
    double et_scale;
};
