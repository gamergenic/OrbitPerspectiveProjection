// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "OrbitProjectorComponent.h"
#include "ConicRendererComponent.generated.h"


/**
 * 
 */
UCLASS()
class ORBITRENDERING_API UConicRendererComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conic Renderer")
    class UCameraComponent* OrbitCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conic Renderer")
    class UOrbitProjectorComponent* Projector;

    UPROPERTY(EditAnywhere, Category = "Conic Renderer")
    float LineThickness = 1.f;

    // Tesselation
    UPROPERTY(EditAnywhere, Category = "Conic Renderer")
    uint32 LinesPerConicSegment = 255;

    // The vertex buffer is not resized, so just ensure it can hold all the vertices and
    // be done with it.
    UPROPERTY(EditAnywhere, Category = "Conic Renderer")
    int MaxOrbits = 4;

public:
	UConicRendererComponent();

    virtual void BeginPlay();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

    // notes:
    // The Tesselate methods are threadsafe are called from the render thread.
    // It's safe to call them from any thread.   To make that a bit easier they're
    // static and only access local variables.  Since they don't access any state
    // type data, they have a lot of params - they need to be explicitly told
    // what values to use, which they keep locally.
    // Anyways, don't change these to be non-static and start accessing member variables
    // unless without a good understanding of this.
    // It would probably be better architecturally speaking to make these methods of the
    // render proxy thread, but that would be conceptually less clear, because the
    // logic and business of interpreting the conic data wouldn't be kept in close
    // proximity and all together.
    static void Tessellate(
        const FMatrix& LocalToWorld,
        const FVector& CameraPosition,
        const FVector& CameraDirection,
        const TArray <FConicSection>& Conics,
        float LineThickness,
        uint32 LinesPerSegment,
        TArray<FDynamicMeshVertex>& LineVertices,
        TArray<uint32>& LineIndices,
        int MaxOrbits
    );

    // See notes above for Tesselate
    static void Tessellate(
        const FMatrix& LocalToWorld,
        const FVector& CameraPosition,
        const FVector& CameraDirection,
        const FConicSection& Conic,
        float LineThickness,
        uint32 LinesPerSegment,
        TArray<FDynamicMeshVertex>& LineVertices,
        TArray<uint32>& LineIndices
    );

    // See notes above for Tesselate
    static void Tessellate(
        const float TrueAnomalyState,
        const FMatrix& LocalToWorld,
        const FVector& CameraPosition,
        const FVector& CameraDirection,
        ES_ConicType ConicType,
        const FColor& Color,
        const FVector& Center,
        const FVector& Axis1,
        const FVector& Axis2,
        const FVector& OrbitPlaneCenter,
        const FVector& OrbitPlaneNormal,
        float LineThickness,
        uint32 LinesPerSegment,
        float SegmentStart,
        float SegmentEnd,
        float TrueAnomalyStart,
        float TrueAnomalyEnd,
        TArray<FDynamicMeshVertex>& LineVertices,
        TArray<uint32>& LineIndices
    );

    static FVector ProjectToOrbitalPlane(const FVector& P1, const FVector& CameraPosition, const FVector& OrbitOrigin, const FVector& OrbitalNormal);

    // Used when tesselating segments
    static FVector2D Ellipse(float theta);
    static FVector2D PositiveHyperbola(float theta);
    static FVector2D NegativeHyperbola(float theta);
    static float EllipseAdvancement(float theta, float AdvancementState, float SegmentStart, float SegmentLength, float TrueAnomalyStart, float TrueAnomalyLength);
    static float HyperbolaAdvancement(float theta, float AdvancementState, float SegmentStart, float SegmentLength, float TrueAnomalyStart, float TrueAnomalyLength);

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

    // Use UE's debug draw functionality to sanity check our own tessellation
    void DrawDebug(const FConicSection& conic);

	friend class FConicRendererSceneProxy;
};
