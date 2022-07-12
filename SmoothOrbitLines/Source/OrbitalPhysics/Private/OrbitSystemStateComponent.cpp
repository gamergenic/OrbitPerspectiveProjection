// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitSystemStateComponent.h"

UOrbitSystemStateComponent::UOrbitSystemStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    et = 0.;
    et_scale = 10000;
}

// Called every frame
void UOrbitSystemStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    et += et_scale * (double)DeltaTime;
}
