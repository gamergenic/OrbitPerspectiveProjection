// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

using UnrealBuildTool;

public class OrbitRendering : ModuleRules
{
    public OrbitRendering(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "MeshDescription",
            "RenderCore",
            "RHI",
            "StaticMeshDescription"
        });

        PublicDependencyModuleNames.Add("OrbitalPhysics");

        PrivateIncludePaths.Add("OrbitRendering/Private/Conics/");
        PublicIncludePaths.Add("OrbitRendering/Public/Conics/");
    }
}
