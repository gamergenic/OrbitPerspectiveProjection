// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitGameState.h"
#include "OrbitSystemStateComponent.h"

AOrbitGameState::AOrbitGameState()
{
    TimeScalePowerValue = 20.;

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    OrbitSystemState = CreateDefaultSubobject<UOrbitSystemStateComponent>(TEXT("OrbitSystemState"));
}

// Called every frame
void AOrbitGameState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeScalePowerValue = FMath::Clamp(TimeScalePowerValue, 1.f, 110.f);

    OrbitSystemState->et_scale = TimeScalePowerValue * TimeScalePowerValue * TimeScalePowerValue;

    FDateTime dateTime = FDateTime::FromUnixTimestamp(315964800);
    dateTime += FTimespan::FromSeconds(OrbitSystemState->et);

    et_string = dateTime.ToString(TEXT("%y %Dd %H:%M:%S.%s"));;
}
