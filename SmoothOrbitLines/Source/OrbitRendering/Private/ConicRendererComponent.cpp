// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com
// -----------------------------------------------------------------------------
// ConicRendererComponent.cpp
//
// There is nothing especially relevant or interesting about the pertenent
// algorithm here, this is mostly just an example of a custom mesh renderer.
// It's probably not even a very good example at that, it may be inefficient
// or just plain wrong.  But, it's a reference for interpreting the clipped
// conic segment data as it does render correct results.
// 
// Tested With: UE5 Early Access 
// -----------------------------------------------------------------------------
#include "ConicRendererComponent.h"
#include "Camera/CameraComponent.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "DrawDebugHelpers.h"


// -----------------------------------------------------------------------------
// FConicRendererSceneProxy - a Scene Proxy
// The role of the scene proxy is to interact with the UE rendering system
// on the render thread, while the component executes primarily on the main
// thread, etc.  When we want to call back into the component we'll supply
// any necessary data from the proxy thread and invoke static logic expressed
// by the component.
// -----------------------------------------------------------------------------
class FConicRendererSceneProxy final : public FPrimitiveSceneProxy
{
public:
    SIZE_T GetTypeHash() const override
    {
        static size_t UniquePointer;
        return reinterpret_cast<size_t>(&UniquePointer);
    }

    FConicRendererSceneProxy(UConicRendererComponent* Component, uint32 maxOrbits, uint32 LinesPerConicSegment, float lineThickness)
        : FPrimitiveSceneProxy(Component)
        , VertexFactory(GetScene().GetFeatureLevel(), "FConicRendererSceneProxy")
        , MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
        , MaxOrbits(maxOrbits)
        , LinesPerSegment(LinesPerConicSegment)
        , LineThickness(lineThickness)
    {
        TArray<FDynamicMeshVertex> Vertices;
        TArray<uint32> Indices;

        const int vertsPerSegment = 4;
        const int indicesPerSegment = 6;
        const int maxSegmentsPerOrbit = 4;

        Vertices.SetNum(LinesPerConicSegment * vertsPerSegment * maxSegmentsPerOrbit * maxOrbits, true);
        Indices.SetNum(LinesPerConicSegment * indicesPerSegment * maxSegmentsPerOrbit * maxOrbits, true);

        VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices, 2);
        IndexBuffer.Indices = Indices;

        // Enqueue initialization of render resource
        BeginInitResource(&VertexBuffers.PositionVertexBuffer);
        BeginInitResource(&VertexBuffers.StaticMeshVertexBuffer);
        BeginInitResource(&VertexBuffers.ColorVertexBuffer);
        BeginInitResource(&IndexBuffer);
        BeginInitResource(&VertexFactory);

        // Grab material
        Material = Component->GetMaterial(0);
        if (Material == NULL)
        {
            Material = UMaterial::GetDefaultMaterial(MD_Surface);
        }
    }

    virtual ~FConicRendererSceneProxy()
    {
        VertexBuffers.PositionVertexBuffer.ReleaseResource();
        VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
        VertexBuffers.ColorVertexBuffer.ReleaseResource();
        IndexBuffer.ReleaseResource();
        VertexFactory.ReleaseResource();
    }

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
    {
        QUICK_SCOPE_CYCLE_COUNTER(STAT_ConicRendererSceneProxy_GetDynamicMeshElements);

        const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

        auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
            GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
            FLinearColor(0, 0.5f, 1.f)
        );

        Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

        FMaterialRenderProxy* MaterialProxy = NULL;
        if (bWireframe)
        {
            MaterialProxy = WireframeMaterialInstance;
        }
        else
        {
            MaterialProxy = Material->GetRenderProxy();
        }

        int nNumPrimitives = IndexBuffer.Indices.Num() / 3;
        if (!nNumPrimitives) return;
        for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
        {
            if (VisibilityMap & (1 << ViewIndex) && IndexBuffer.Indices.Num() )
            {
                const FSceneView* View = Views[ViewIndex];
                // Draw the mesh.
                FMeshBatch& Mesh = Collector.AllocateMesh();
                FMeshBatchElement& BatchElement = Mesh.Elements[0];
                BatchElement.IndexBuffer = &IndexBuffer;
                Mesh.bWireframe = bWireframe;
                Mesh.VertexFactory = &VertexFactory;
                Mesh.MaterialRenderProxy = MaterialProxy;

                bool bHasPrecomputedVolumetricLightmap;
                FMatrix PreviousLocalToWorld;
                int32 SingleCaptureIndex;
                bool bOutputVelocity;
                GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

                FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
                DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), false);
                BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

                BatchElement.FirstIndex = 0;
                BatchElement.NumPrimitives = nNumPrimitives;
                BatchElement.MinVertexIndex = 0;
                BatchElement.MaxVertexIndex = VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
                Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
                Mesh.Type = PT_TriangleList;
                Mesh.DepthPriorityGroup = SDPG_World;
                Mesh.bCanApplyViewModeOverrides = false;
                Collector.AddMesh(ViewIndex, Mesh);
            }
        }
    }

    virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
    {
        FPrimitiveViewRelevance Result;
        Result.bDrawRelevance = IsShown(View);
        Result.bShadowRelevance = IsShadowCast(View);
        Result.bDynamicRelevance = true;
        Result.bRenderInMainPass = ShouldRenderInMainPass();
        Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
        Result.bRenderCustomDepth = ShouldRenderCustomDepth();
        Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
        MaterialRelevance.SetPrimitiveViewRelevance(Result);
        Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
        return Result;
    }

    virtual bool CanBeOccluded() const override
    {
        // Always Visible.
        return false;
    }

    virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

    uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

    void TesselateConics_RenderThread(
        const FVector& CameraPosition,
        const FVector& CameraDirection,
        const TArray<FConicSection>& Conics,
        const FMatrix& Transform /* Technically, Position and Direction can be obtained by decomposing this, but... */
    )
    {
        check(IsInRenderingThread());

        TArray<FDynamicMeshVertex> Vertices;
        TArray<uint32> Indices;

        UConicRendererComponent::Tessellate(Transform, CameraPosition, CameraDirection, Conics, LineThickness, LinesPerSegment, Vertices, Indices, MaxOrbits);

        // It's probably against the laws of UE rendering to update these vertices in this manner, but
        // the point here is to illustrate the concept over illustration of ue particulars.
        VertexBuffers.InitFromDynamicVertex(&VertexFactory, Vertices, 2);
        IndexBuffer.Indices = Indices;
        IndexBuffer.UpdateRHI();
    }

private:

    UMaterialInterface* Material;

    FStaticMeshVertexBuffers VertexBuffers;
    FDynamicMeshIndexBuffer32 IndexBuffer;
    FLocalVertexFactory VertexFactory;

    FMaterialRelevance MaterialRelevance;
    uint32 MaxOrbits;
    uint32 LinesPerSegment;
    float LineThickness;
};

 
// -----------------------------------------------------------------------------
// UConicRendererComponent - An Actor Component
// Handles the main thread work to be done.  This involves telling the scene
// proxy what it should do the next time it's called upon to interface with
// UE rendering from the render thread
// -----------------------------------------------------------------------------
UConicRendererComponent::UConicRendererComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
}

// Called when the game starts
void UConicRendererComponent::BeginPlay()
{
    Super::BeginPlay();

    for (auto component : GetOwner()->GetComponents())
    {
        if(!Projector) Projector = Cast<UOrbitProjectorComponent>(component);
        if (!OrbitCamera) OrbitCamera = Cast<UCameraComponent>(component);
    }

    PrimaryComponentTick.TickGroup = ETickingGroup::TG_LastDemotable;
    PrimaryComponentTick.bCanEverTick = Projector != nullptr;

    if (!Projector || !OrbitCamera)
    {
        PrimaryComponentTick.bCanEverTick = false;
    }
}


// Called every frame
void UConicRendererComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TArray<FConicSection> Conics;
    Projector->GetConics(Conics);

#if defined(RENDER_DEBUG_LINES) && RENDER_DEBUG_LINES==1
    for (auto Conic : Conics)
    {
        DrawDebug(Conic);
    }
    return;
#endif

    if (SceneProxy)
    {

        // Enqueue command to modify render thread info
        // Render data that changes dynamically at runtime is passed here.
        // The Render proxy is recreated when PIE is running and a value is changed in the editor, though,
        // so tuning points like Line Thicknesses, etc, can be set in the Proxy's constructor instead
        // of passed in the render thread command here.
        FVector CameraLocation = OrbitCamera->GetComponentLocation();
        FVector CameraDirection = OrbitCamera->GetForwardVector();
        FMatrix Transform = GetComponentTransform().ToMatrixWithScale();
        FConicRendererSceneProxy* ConicRendererSceneProxy = (FConicRendererSceneProxy*)SceneProxy;

        ENQUEUE_RENDER_COMMAND(FConicRendererSceneProxy)(
            [ConicRendererSceneProxy, CameraLocation, CameraDirection, Conics, Transform](FRHICommandListImmediate& RHICmdList)
            {
                ConicRendererSceneProxy->TesselateConics_RenderThread(CameraLocation, CameraDirection, Conics, Transform);
            });
    }
}

// What do we want?  SCENE PROXIES!
// When do we want them?  NOW!
// [Repeat until fullfillment]
FPrimitiveSceneProxy* UConicRendererComponent::CreateSceneProxy()
{
    FPrimitiveSceneProxy* Proxy = new FConicRendererSceneProxy(this, MaxOrbits, LinesPerConicSegment, LineThickness);
    return Proxy;
}

int32 UConicRendererComponent::GetNumMaterials() const
{
    return 1;
}


FBoxSphereBounds UConicRendererComponent::CalcBounds(const FTransform& LocalToWorld) const
{
    // Always visible...
    FBoxSphereBounds NewBounds;

    // There may be negative consequences to this, once the algorithm is finalized we should compute a real bounds.
    // An important open questions, depending on choices we make when integrating with the rest of our rendering
    // would be whether we will leave the vertices on the projection plane or project them back to the orbit plane.
    // ('this' = saying our extends are pretty much the universe...)
    NewBounds.BoxExtent = FVector(1e15, 1e15, 1e15);
    NewBounds.Origin = FVector(0, 0, 0);
    NewBounds.SphereRadius = 1e15;

    return NewBounds;
}


// Main Tessellation entry point.
// This is a static method, all data the algorithm uses should come in as a parameter (eg streamed
// from main thread to render thread in the command queue.)
// We intend to primarily call this from the render thread.
void UConicRendererComponent::Tessellate(
    const FMatrix& LocalToWorld,
    const FVector& CameraPosition,
    const FVector& CameraDirection,
    const TArray <FConicSection>& Conics,
    float LineThickness,
    uint32 LinesPerSegment,
    TArray<FDynamicMeshVertex>& LineVertices,
    TArray<uint32>& LineIndices,
    int MaxOrbits
)
{
    LineVertices.Empty();
    LineIndices.Empty();

    int nCountOfOrbits = Conics.Num();
    
    // We don't handle more than a static number of orbits, but the least we can do is
    // log an error for the case and not crash.
    // There are innumerable alternatives.  Please feel free to spend your own time implementing
    // the one of your choice :-D.
    if (nCountOfOrbits > MaxOrbits)
    {
        static bool AlreadyLoggedMaxOrbitError = false;
        if (!AlreadyLoggedMaxOrbitError)
        {
            UE_LOG(LogTemp, Error, TEXT("MaxOrbits = %d, but there are %d orbits rendering.  Increase MaxOrbits on Pawn's ConicRendererComponent to fix this."), MaxOrbits, Conics.Num());
            AlreadyLoggedMaxOrbitError = true;
        }
        nCountOfOrbits = MaxOrbits;
    }

    for (int i = 0; i < nCountOfOrbits; ++i)
    {
        Tessellate(LocalToWorld, CameraPosition, CameraDirection, Conics[i], LineThickness, LinesPerSegment, LineVertices, LineIndices);
    }
}


void UConicRendererComponent::Tessellate(
    const FMatrix& LocalToWorld,
    const FVector& CameraPosition,
    const FVector& CameraDirection,
    const FConicSection& Conic,
    float LineThickness,
    uint32 LinesPerSegment,
    TArray<FDynamicMeshVertex>& LineVertices,
    TArray<uint32>& LineIndices
)
{
    const int segmentCount = Conic.Segments.Num();
    
    for (int i = 0; i < segmentCount; ++i)
    {
        Tessellate(Conic.AdvancementState, LocalToWorld, CameraPosition, CameraDirection, Conic.ConicType, Conic.Color, Conic.Center, Conic.Axis1, Conic.Axis2, Conic.OrbitalPlaneCenter, Conic.OrbitalPlaneNormal, LineThickness, LinesPerSegment, Conic.Segments[i].X, Conic.Segments[i].Y, Conic.TrueAnomalies[i].X, Conic.TrueAnomalies[i].Y, LineVertices, LineIndices);
    }
}

void UConicRendererComponent::Tessellate(
    const float AdvancementState,
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
    float SegmentLength,
    float TrueAnomalyStart,
    float TrueAnomalyLength,
    TArray<FDynamicMeshVertex>& LineVertices,
    TArray<uint32>& LineIndices
)
{

    // 'Conic' defines the 2D shape of our conic section.
    // The possibilities of 'Circle' and 'Parabola' are not supported.  These are singular special cases
    // which are possible to contrive but do not happen in general useage.
    FVector2D(*Conic)(float theta) = nullptr;


    switch (ConicType)
    {
    case ES_ConicType::Ellipse:
        Conic = UConicRendererComponent::Ellipse;
        break;
    case ES_ConicType::NegativeHyperbola:
        Conic = UConicRendererComponent::NegativeHyperbola;
        break;
    case ES_ConicType::PositiveHyperbola:
        Conic = UConicRendererComponent::PositiveHyperbola;
        break;
    }

    if (Conic != nullptr)
    {
        // The is not the greatest line tessellator in the world.   (It's just a tribute.)
        // There's a tradeoff between feeding the processor excessive computational gymnastics,
        // conceptual clarity, and tessellation perfection with many possible solutions each
        // optimized differently.
        float cumulativeDistance = 0;
        FDynamicMeshVertex Vertex;
        Vertex.Color = Color;

        float thetaA = SegmentStart;
        float advancementAngle = TrueAnomalyStart;
        FVector2D c0 = Conic(thetaA);
        FVector A = Center + c0.X * Axis1 + c0.Y * Axis2;
        float segmentLength = SegmentLength / (float)LinesPerSegment;
        float anomalyLength = TrueAnomalyLength / (float)LinesPerSegment;

        for (uint32 i = 0; i <= LinesPerSegment; ++i)
        {
            int baseIndex = LineVertices.Num();

            float thetaB = thetaA + segmentLength;
            FVector2D c1 = Conic(thetaB);

            FVector B = Center + c1.X * Axis1 + c1.Y * Axis2;

            FVector AtoB = B - A;

            const FVector TangentX = AtoB.GetSafeNormal();
            const FVector TangentZ = CameraDirection.GetSafeNormal();
            const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

            Vertex.SetTangents((FVector3f)TangentX, (FVector3f)TangentY, (FVector3f)TangentZ);

            FVector Perp = TangentY;
            Perp *= LineThickness * 0.5f;

            float AdvancementCoordinate = FMath::Clamp(advancementAngle / twopi<float>, 0.1f, 1.f);

            // So, like...  The vertices are treated as if they're in local space to the renderer...
            // which, we've attached to the camera and we're moving it all around.
            // But, we've computed them in world space... So, we need to handle this one way or another.
            // Moving them from the local space of the (moving) renderer to world space is fine for now.
            // Otherwise, we'd have to guarantee the local to world transform for it is identity, or
            // handle this in a vertex shader, or .... bleh.

            // It's kinda rediculous because in a way we're going from world space to clip space to world space to clip space
            // TODO: Fix that.

            // For clarity:
            const FVector WorldProjectionPlanePositionA1 = A + Perp;
            const FVector LocalPositionA1 = LocalToWorld.InverseTransformPosition(WorldProjectionPlanePositionA1);
            Vertex.Position = (FVector3f)LocalPositionA1;
            Vertex.TextureCoordinate[0] = FVector2f(cumulativeDistance, 1);
            Vertex.TextureCoordinate[1] = FVector2f(AdvancementCoordinate, 1);
            LineVertices.Add(Vertex);

            const FVector WorldProjectionPlanePositionA2 = A - Perp;
            const FVector LocalPositionA2 = LocalToWorld.InverseTransformPosition(WorldProjectionPlanePositionA2);
            Vertex.Position = (FVector3f)LocalPositionA2;
            Vertex.TextureCoordinate[0] = FVector2f(cumulativeDistance, 0);
            Vertex.TextureCoordinate[1] = FVector2f(AdvancementCoordinate, 0);
            LineVertices.Add(Vertex);

            cumulativeDistance += AtoB.Size();

            if (i > 0)
            {
                LineIndices.Add(baseIndex + 1 - 2);
                LineIndices.Add(baseIndex + 2 - 2);
                LineIndices.Add(baseIndex + 0 - 2);
                LineIndices.Add(baseIndex + 3 - 2);
                LineIndices.Add(baseIndex + 1 - 2);
                LineIndices.Add(baseIndex + 2 - 2);
            }

            A = B;
            thetaA += segmentLength;
            advancementAngle += anomalyLength;
        }
    }
}

FVector UConicRendererComponent::ProjectToOrbitalPlane(const FVector& P1, const FVector& CameraPosition, const FVector& OrbitOrigin, const FVector& OrbitalNormal)
{
    FVector RayDirection = P1 - CameraPosition;

    auto factor = CameraPosition.Dot(OrbitOrigin);
    factor -= (OrbitOrigin.X + OrbitOrigin.Y + OrbitOrigin.Z);
    factor /= RayDirection.Dot(OrbitalNormal);

    FVector P = CameraPosition - factor * RayDirection;

    return P;
}


FVector2D UConicRendererComponent::Ellipse(float theta)
{
    return FVector2D(cos(theta), sin(theta));
}

FVector2D UConicRendererComponent::PositiveHyperbola(float theta)
{
    return FVector2D(cosh(theta), sinh(theta));
}

FVector2D UConicRendererComponent::NegativeHyperbola(float theta)
{
    return FVector2D(-cosh(theta), -sinh(theta));
}

void UConicRendererComponent::DrawDebug(const FConicSection& conic)
{
#if defined(RENDER_DEBUG_LINES) && RENDER_DEBUG_LINES==1
    FColor color2 = conic.Color;
    FColor color3 = conic.Color;
    color2.B = 128;
    color3.G = 128;

    if (conic.ConicType == ES_ConicType::Ellipse)
    {

        DrawDebugLine(
            GetOwner()->GetWorld(),
            conic.Center,
            conic.Center + conic.Axis1,
            color2,
            false, -1.f, 0, 1.f
        );

        DrawDebugLine(
            GetOwner()->GetWorld(),
            conic.Center,
            conic.Center + conic.Axis2,
            color3,
            false, -1.f, 0, 1.f
        );
        for (int i = 0; i < conic.Segments.Num(); ++i)
        {
            double theta1 = conic.Segments[i].X;
            double theta2 = conic.Segments[i].Y;

            if (theta1 > theta2) theta1 -= 2. * pi<double>;

            for (int j = 0; j < 256; ++j)
            {
                double f0 = FMath::Lerp(theta1, theta2, (double)j / (double)256);
                float f1 = FMath::Lerp(theta1, theta2, (double)(j + 1) / (double)256);


                DrawDebugLine(
                    GetOwner()->GetWorld(),
                    conic.Center + cos(f0) * conic.Axis1 + sin(f0) * conic.Axis2,
                    conic.Center + cos(f1) * conic.Axis1 + sin(f1) * conic.Axis2,
                    conic.Color,
                    false, -1.f, 0, 1.f
                );
            }

            DrawDebugSphere(GetOwner()->GetWorld(),
                conic.Center + cos(theta1) * conic.Axis1 + sin(theta1) * conic.Axis2,
                3.f, 16, conic.Color
            );
            DrawDebugBox(GetOwner()->GetWorld(),
                conic.Center + cos(theta2) * conic.Axis1 + sin(theta2) * conic.Axis2,
                FVector(3.f, 3.f, 3.f), conic.Color
            );
        }
    }
    else if (conic.ConicType == ES_ConicType::PositiveHyperbola || conic.ConicType == ES_ConicType::NegativeHyperbola)
    {
        double sign = conic.ConicType == ES_ConicType::NegativeHyperbola ? -1. : 1.;

        DrawDebugLine(
            GetOwner()->GetWorld(),
            conic.Center,
            conic.Center + sign * conic.Axis1,
            color2,
            false, -1.f, 0, 1.f
        );

                DrawDebugLine(
            GetOwner()->GetWorld(),
            conic.Center,
            conic.Center + sign * conic.Axis2,
                    color3,
            false, -1.f, 0, 1.f
        );


        for (int i = 0; i < conic.Segments.Num(); ++i)
        {
            double theta1 = conic.Segments[i].X;
            double theta2 = conic.Segments[i].Y;

            for (int j = 0; j < 256; ++j)
            {
                float f0 = FMath::Lerp(theta1, theta2, (double)j / (double)256);
                float f1 = FMath::Lerp(theta1, theta2, (double)(j + 1) / (double)256);

                DrawDebugLine(
                    GetOwner()->GetWorld(),
                    conic.Center + sign * cosh(f0) * conic.Axis1 + sign * sinh(f0) * conic.Axis2,
                    conic.Center + sign * cosh(f1) * conic.Axis1 + sign * sinh(f1) * conic.Axis2,
                    conic.Color,
                    false, -1.f, 0, 1.f
                );
            }

            DrawDebugSphere(GetOwner()->GetWorld(),
                conic.Center + sign * cosh(theta1) * conic.Axis1 + sign * sinh(theta1) * conic.Axis2,
                3.f, 16, conic.Color
            );
            DrawDebugBox(GetOwner()->GetWorld(),
                conic.Center + sign * cosh(theta2) * conic.Axis1 + sign * sinh(theta2) * conic.Axis2,
                FVector(3.f, 3.f, 3.f), conic.Color
            );
        }
    }
#endif
}

