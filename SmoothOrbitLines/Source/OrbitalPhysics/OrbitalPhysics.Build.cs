// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

using UnrealBuildTool;

public class OrbitalPhysics : ModuleRules
{
    public OrbitalPhysics(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

        PublicIncludePaths.Add("OrbitalPhysics/Public/GTE/");

        if (Target.Type == TargetType.Editor)
        {
            PrivateDefinitions.Add("DYNAMIC_CONIC_ELEMENTS=1");
        }
    }
}
