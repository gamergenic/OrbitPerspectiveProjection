// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

using UnrealBuildTool;

public class SmoothOrbitLines : ModuleRules
{
    public SmoothOrbitLines(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        PrivateDependencyModuleNames.Add("OrbitalPhysics");
        PrivateDependencyModuleNames.Add("OrbitRendering");

        if (Target.Type == TargetType.Editor)
        {
            // Enable live tuning of conic elements in editor
            PrivateDefinitions.Add("DYNAMIC_CONIC_ELEMENTS=1");
        }
    }
}
