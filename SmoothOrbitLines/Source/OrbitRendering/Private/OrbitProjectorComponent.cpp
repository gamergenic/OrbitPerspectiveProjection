// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

#include "OrbitProjectorComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Conics/ProjectEllipseToPlane.h"
#include "Conics/ProjectionAngleToTrueAnomaly.h"
#include "Conics/ClipEllipseToFrustum.h"
#include "Conics/ClipHyperbolaToFrustum.h"
#include "GTE/Mathematics/IntrRay3Plane3.h"
#include "OrbitSystemStateComponent.h"
#include "OrbitViewerControllerComponent.h"


// Forward decl's.  No need to dirty the public header with the input/output types.
void GetEllipticalProjectionTrueAnomalies(
    const EllipseProjectionInputs<double>& projInputs,
    const EllipseProjectionOutputs<double>& projOutputs,
    std::vector<ConicSegment<double>>& segmentList,
    std::vector<ConicSegment<double>>& advancementList,
    double Advancement,
    double TrueAnomaly
);

void GetHyperbolicProjectionTrueAnomalies(
    const EllipseProjectionInputs<double>& projInputs,
    const EllipseProjectionOutputs<double>& projOutputs,
    std::vector<ConicSegment<double>>& segmentList,
    std::vector<ConicSegment<double>>& advancementList,
    double Advancement,
    double TrueAnomaly
);


// Sets default values for this component's properties
UOrbitProjectorComponent::UOrbitProjectorComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UOrbitProjectorComponent::BeginPlay()
{
    Super::BeginPlay();

    auto State = GetWorld()->GetGameState();
    for (auto component : State->GetComponents())
    {
        if (!OrbitSystemState) OrbitSystemState = Cast<UOrbitSystemStateComponent>(component);
    }

    auto Controller = GetWorld()->GetFirstPlayerController();
    for (auto component : Controller->GetComponents())
    {
        if (!OrbitViewerController) OrbitViewerController = Cast<UOrbitViewerControllerComponent>(component);
    }


    for (auto component : GetOwner()->GetComponents())
    {
        if (!OrbitCamera) OrbitCamera = Cast<UCameraComponent>(component);
        if (!OrbitSystemState) OrbitSystemState = Cast<UOrbitSystemStateComponent>(component);
    }


    PrimaryComponentTick.TickGroup = ETickingGroup::TG_LastDemotable;
    if (!OrbitCamera || !OrbitSystemState)
    {
        PrimaryComponentTick.bCanEverTick = false;
    }
}


// Called every frame
void UOrbitProjectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TArray<UOrbitingBodyComponent*> ActiveBodies;
    OrbitViewerController->GetActiveBodies(ActiveBodies);

    OrbitArray.Empty();
    for (auto ActiveBody : ActiveBodies)
    {
        if (ActiveBody->IsInertial)
            continue;

        FOrbitItem orbit;
        if (CollectOrbit(ActiveBody, orbit))
        {
            OrbitArray.Add(orbit);
        }
    }
}


bool UOrbitProjectorComponent::CollectOrbit(UOrbitingBodyComponent* ActiveBody, FOrbitItem& orbit)
{
    bool result = !ActiveBody->IsInertial;

    if (result)
    {

        ES_ResultCode ResultCode;
        FOscullatingOrbitGeometry OscillatingGeometry;
        UOrbitalMechanics::ComputeGeometry(ActiveBody->ConicElements, OscillatingGeometry, ResultCode);

        result &= ResultCode == ES_ResultCode::Success;
        if (result)
        {
            orbit.ConicType = ES_ConicType::Ellipse;
            orbit.Color = ActiveBody->LineColor;

            orbit.Center = FFramePosition(-OscillatingGeometry.ae * (Vector3<double>)OscillatingGeometry.p_hat);
            orbit.Axis1 = FFrameVector(OscillatingGeometry.a * (Vector3<double>)OscillatingGeometry.p_hat);
            orbit.Axis2 = FFrameVector(OscillatingGeometry.b * (Vector3<double>)OscillatingGeometry.q_hat);

            // Assumes the orbit is at the solar system barycenter's origin...
            // Child orbits would break here.
            orbit.Focus = FFramePosition();
            orbit.Normal = OscillatingGeometry.w_hat;

            UOrbitalMechanics::ComputeState(ActiveBody->ConicElements, OrbitSystemState->et, orbit.FrameState, ResultCode);
        }
    }

    return result;
}

// Project conic based on current camera view...
bool UOrbitProjectorComponent::TransformOrbit(const FOrbitItem& orbit, FConicSection& conic)
{
    double fieldOfView = (double)OrbitCamera->FieldOfView / 180 * pi<double>;
    double aspectRatio = (double)OrbitCamera->AspectRatio;
    double distance;
    OrbitViewerController->GetFrameDistance(ProjectionPlaneDistance, distance);

    const FrustumParameters<double> frustumParameters(distance, fieldOfView, aspectRatio);

    // Inputs...
    FFramePosition EyePoint;
    OrbitViewerController->GetFramePosition(OrbitCamera->GetComponentLocation(), EyePoint);
    FFrameVector EyeDirection;
    OrbitViewerController->GetFrameVector(OrbitCamera->GetForwardVector(), EyeDirection);

    // Outputs...
    ProjectionType projectionType;
    FFramePosition ProjectedCenter;
    FFrameVector ProjectedAxis1;
    FFrameVector ProjectedAxis2;
    std::vector<ConicSegment<double>> clipSegments;
    std::vector<ConicSegment<double>> clipTrueAnomalies;

    double TrueAnomaly = orbit.FrameState.Theta / 180 * pi<double>;

    double a = Length((Vector3<double>)orbit.Axis1);
    double b = Length((Vector3<double>)orbit.Axis2);
    double ae = Length((Vector3<double>)orbit.Center);
    double x = orbit.FrameState.r * cos(TrueAnomaly);
    double y = orbit.FrameState.r * sin(TrueAnomaly);
    double TestTheta = atan2(y / b, (x + ae) / a);
    
    double Advancement = 0;


    ProjectToPlane(
        frustumParameters,
        orbit.Center,
        orbit.Axis1,
        orbit.Axis2,
        EyePoint,
        EyeDirection,
        projectionType,
        ProjectedCenter,
        ProjectedAxis1,
        ProjectedAxis2,
        clipSegments,
        clipTrueAnomalies,
        Advancement,
        TestTheta
    );

    switch (projectionType)
    {
    case ProjectionType::Ellipse:
        conic.ConicType = ES_ConicType::Ellipse;
        break;
    case ProjectionType::PositiveHyperbola:
        conic.ConicType = ES_ConicType::PositiveHyperbola;
        break;
    case ProjectionType::NegativeHyperbola:
        conic.ConicType = ES_ConicType::NegativeHyperbola;
        break;

    default:
        conic.ConicType = ES_ConicType::None;
        break;
    }

    bool result = conic.ConicType != ES_ConicType::None;

    if (result)
    {
        OrbitViewerController->GetScenePosition(ProjectedCenter, conic.Center);
        OrbitViewerController->GetSceneVector(ProjectedAxis1, conic.Axis1);
        OrbitViewerController->GetSceneVector(ProjectedAxis2, conic.Axis2);

        OrbitViewerController->GetScenePosition(orbit.Focus, conic.OrbitalPlaneCenter);
        OrbitViewerController->GetSceneVector(orbit.Normal, conic.OrbitalPlaneNormal);

        conic.Color = orbit.Color;

        // Munge from non-UE-specific types to UE-specific types...
        conic.Segments.Empty();

        for (int i = 0; i < clipSegments.size(); ++i)
        {
            // lerp = normalizeRadians0toTwoPi(lerp);

            float p1 = (float)clipSegments[i].SegmentStart;
            float p2 = p1 + (float)clipSegments[i].SegmentLength;
            float ta1 = (float)clipTrueAnomalies[i].SegmentStart;
            float ta2 = ta1 + (float)clipTrueAnomalies[i].SegmentLength;
            float advancement = (float)Advancement;

            if (advancement > p1 && advancement < p2)
            {
                conic.Segments.Add(FVector2D(p1, advancement - p1));
                conic.Segments.Add(FVector2D(advancement, p2- advancement));
                conic.TrueAnomalies.Add(FVector2D(ta1, twopi<float>));
                conic.TrueAnomalies.Add(FVector2D(0, ta2-twopi<float>));
            }
            else
            {
                conic.Segments.Add(FVector2D(p1, (float)clipSegments[i].SegmentLength));
                conic.TrueAnomalies.Add(FVector2D(ta1, (float)clipTrueAnomalies[i].SegmentLength));
            }
        }

        conic.AdvancementState = Advancement;
    }

    return result;
}


void UOrbitProjectorComponent::ProjectToPlane(
    const FrustumParameters<double>& frustum,
    const FFramePosition& ellipseCenterPosition,
    const FFrameVector& ellipseAxis1Vector,
    const FFrameVector& ellipseAxis2Vector,
    const FFramePosition& eyePointPosition,
    const FFrameVector& eyeDirection,
    ProjectionType& projectionType,
    FFramePosition& projectedCenterPosition,
    FFrameVector& projectedAxis1Vector,
    FFrameVector& projectedAxis2Vector,
    std::vector<ConicSegment<double>>& segmentList,
    std::vector<ConicSegment<double>>& advancementList,
    double& Advancement,
    double TrueAnomaly
)
{
    // ------------------------------------------------------------------------
    // Inputs
    // Convert from UE friendly representations to generalized
    // ------------------------------------------------------------------------
    Vector3<double> E = eyePointPosition;
    Vector3<double> Ce = ellipseCenterPosition;
    double A = Length((Vector3<double>)ellipseAxis1Vector);
    double B = Length((Vector3<double>)ellipseAxis2Vector);

    Vector3<double> Ue = (Vector3<double>)ellipseAxis1Vector;
    Vector3<double> Ve = (Vector3<double>)ellipseAxis2Vector;
    Normalize(Ue);
    Normalize(Ve);
    Vector3<double> Ne = UnitCross(Ue, Ve);

    Vector3<double> Np = -(Vector3<double>)eyeDirection;
    Normalize(Np);
    const Vector3<double> Cp = eyePointPosition - frustum.z * Np;
    const Vector3<double> Up = UnitCross(Vector3<double>({ 0, 0, 1 }), Np);
    const Vector3<double> Vp = UnitCross(Np, Up);

    EllipseProjectionInputs<double> projInputs(E, Ce, Ne, Ue, Ve, A, B, Cp, Np, Up, Vp, TrueAnomaly);
    EllipseProjectionOutputs<double> projOutputs;

    // ------------------------------------------------------------------------
    // Project the orbit to the plane...
    // ------------------------------------------------------------------------
    ProjectEllipseToPlane(projInputs, projOutputs);

    // ------------------------------------------------------------------------
    // Outputs
    // Convert from generalized to UE friendly representations
    // ------------------------------------------------------------------------
    const Vector<2, double> k = projOutputs.projection.k;
    const Vector<2, double> u_proj = projOutputs.projection.u;
    const Vector<2, double> v_proj = projOutputs.projection.v;
    const double a = projOutputs.projection.a;
    const double b = projOutputs.projection.b;

    projectionType = projOutputs.projectionType;
    Advancement = projOutputs.ThetaLocation;

    segmentList.clear();
    projectedCenterPosition = FFramePosition(Cp + k[0] * Up + k[1] * Vp);

    projectedAxis1Vector = FFrameVector(a * (u_proj[0] * Up + u_proj[1] * Vp));
    projectedAxis2Vector = FFrameVector(b * (v_proj[0] * Up + v_proj[1] * Vp));

    // ------------------------------------------------------------------------
    // Clip the projection to the frustum
    // ------------------------------------------------------------------------
    if (projectionType == ProjectionType::Ellipse)
    {
        // Elliptical!
        bool visible = ClipEllipseToFrustum(frustum, projOutputs.projection, segmentList);

        if (visible)
        {
            GetEllipticalProjectionTrueAnomalies(projInputs, projOutputs, segmentList, advancementList, Advancement, TrueAnomaly);
        }
    }
    else if (projectionType == ProjectionType::PositiveHyperbola || projectionType == ProjectionType::NegativeHyperbola)
    {
        // Hyperbolic!!!
        ClipHyperbolaToFrustum(frustum, projOutputs.projection, projectionType == ProjectionType::PositiveHyperbola, segmentList);

        GetHyperbolicProjectionTrueAnomalies(projInputs, projOutputs, segmentList, advancementList, Advancement, TrueAnomaly);
    }
    else
    {
        // Parabolic... Not worth handing.
    }

}



void UOrbitProjectorComponent::GetConics(TArray<FConicSection>& Conics)
{
    Conics.Empty();

    for (auto Orbit : OrbitArray)
    {
        FConicSection conic;
        TransformOrbit(Orbit, conic);
        Conics.Add(conic);
    }
}



void GetEllipticalProjectionTrueAnomalies(
    const EllipseProjectionInputs<double>& projInputs,
    const EllipseProjectionOutputs<double>& projOutputs,
    std::vector<ConicSegment<double>>& segmentList,
    std::vector<ConicSegment<double>>& advancementList,
    double Advancement,
    double TrueAnomaly
)
{
    advancementList.clear();

    if (segmentList.size() == 0)
    {
        segmentList.push_back(ConicSegment<double>(Advancement, twopi<double>));
        advancementList.push_back(ConicSegment<double>(0, twopi<double>));
    }
    else
    {
        for (int i = 0; i < segmentList.size(); ++i)
        {
            auto segment = segmentList[i];
            double segmentStart = segment.SegmentStart;
            double segmentLength = segment.SegmentLength;
            double segmentEnd = segmentStart + segmentLength;
            double trueAnomaly1 = 0;
            double trueAnomaly2 = 0;
            ProjectionAngleToTrueAnomaly(projInputs, projOutputs, segmentStart, trueAnomaly1);
            ProjectionAngleToTrueAnomaly(projInputs, projOutputs, segmentEnd, trueAnomaly2);

            trueAnomaly1 -= TrueAnomaly;
            trueAnomaly2 -= TrueAnomaly;

            if (trueAnomaly2 <= trueAnomaly1) trueAnomaly2 += twopi<double>;
            double trueAnomalyLength = trueAnomaly2 - trueAnomaly1;
            if (trueAnomaly1 < 0) trueAnomaly1 += twopi<double>;

            // Make sure true anomaly isn't in the wrap around portion...
            if ((segmentEnd >= twopi<double>) && (Advancement >= segmentStart - twopi<double>) && (Advancement < segmentEnd - twopi<double>))
            {
                segmentList[i].SegmentStart -= twopi<double>;
            }

            advancementList.push_back(ConicSegment<double>(trueAnomaly1, trueAnomalyLength));
        }
    }
}

void GetHyperbolicProjectionTrueAnomalies(
    const EllipseProjectionInputs<double>& projInputs,
    const EllipseProjectionOutputs<double>& projOutputs,
    std::vector<ConicSegment<double>>& segmentList,
    std::vector<ConicSegment<double>>& advancementList,
    double Advancement,
    double TrueAnomaly
)
{
    advancementList.clear();

    for (int i = 0; i < segmentList.size(); ++i)
    {
        auto segment = segmentList[i];
        double segmentStart = segment.SegmentStart;
        double segmentLength = segment.SegmentLength;
        double segmentEnd = segmentStart + segmentLength;
        double trueAnomaly1 = 0;
        double trueAnomaly2 = 0;
        ProjectionAngleToTrueAnomaly(projInputs, projOutputs, segmentStart, trueAnomaly1);
        ProjectionAngleToTrueAnomaly(projInputs, projOutputs, segmentEnd, trueAnomaly2);

        trueAnomaly1 -= TrueAnomaly;
        trueAnomaly2 -= TrueAnomaly;

        if (trueAnomaly2 <= trueAnomaly1) trueAnomaly2 += twopi<double>;
        double trueAnomalyLength = trueAnomaly2 - trueAnomaly1;
        if (trueAnomaly1 < 0) trueAnomaly1 = 0;

        advancementList.push_back(ConicSegment<double>(trueAnomaly1, trueAnomalyLength));
    }
}


