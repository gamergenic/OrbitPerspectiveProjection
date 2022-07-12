// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

using UnrealBuildTool;
using System.Collections.Generic;

public class SmoothOrbitLinesTarget : TargetRules
{
	public SmoothOrbitLinesTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "SmoothOrbitLines", "OrbitalPhysics", "OrbitRendering" } );
    }
}
