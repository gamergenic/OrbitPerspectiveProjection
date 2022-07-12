// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "OrbitalMechanics.h"
#include "OrbitGameState.generated.h"

/**
 * 
 */
UCLASS()
class AOrbitGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AOrbitGameState();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Current Ephemeris Time"))
    class UOrbitSystemStateComponent* OrbitSystemState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe", meta = (ToolTip = "Ephemeris Time Multiplier"))
    FString et_string;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Universe")
    float TimeScalePowerValue;
};
