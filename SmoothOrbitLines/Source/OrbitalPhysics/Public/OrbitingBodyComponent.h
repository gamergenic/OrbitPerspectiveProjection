// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "OrbitalMechanics.h"
#include "OrbitingBodyComponent.generated.h"

UCLASS()
class ORBITALPHYSICS_API UOrbitingBodyComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:	
    // Sets default values for this actor's properties
    UOrbitingBodyComponent();

    /*
    *   Assigned by OrbitingBody.cpp
    */
    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Orbiting Body")
    class UStaticMeshComponent* Mesh;

    UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Orbiting Body")
    class UOrbitSystemStateComponent* GameState;

    /*
    *   Editable Values
    */
    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Body",
        meta = (
            ToolTip = "NAIF Body ID"
            ))
    FString BodyId;

    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body",
        meta = (
            ToolTip = "True for intertial objects (The solar system barycenter is considered to be inertial.  Also, the sun's position is considered to be at the same.)"
            ))
    bool IsInertial;

    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Orbit",
        meta = (
            ToolTip = "Conic Elements"
            ))
    FConicElements ConicElements;

    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Body",
        meta = (
            ToolTip = "Orbit Line color"
            ))
    FColor LineColor;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Orbiting Body|Debug|Draw Debug Orbit",
        meta = (
            ToolTip = "Draw debug orbit in Scene Space"
            ))
    bool DrawDebug;


    /*
    * Derived Quantity Shadow Values (for display)
    * The primary values are stored as double-precision Vectors (kilometers)
    */
    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Orbit",
        meta = (ToolTip = "Computed orbit geometry"))
    FOscullatingOrbitGeometry OrbitGeometry;

    UPROPERTY(EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Orbiting Body|Orbit",
        meta = (ToolTip = "Current body state"))
    FState OrbitState;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
