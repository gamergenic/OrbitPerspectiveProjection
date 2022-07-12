// Copyright 2021 Gamergenic. All Rights Reserved.
// Author: chuck@gamergenic.com

using UnrealBuildTool;
using System.Collections.Generic;

public class SmoothOrbitLinesEditorTarget : TargetRules
{
	public SmoothOrbitLinesEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "SmoothOrbitLines", "OrbitalPhysics", "OrbitRendering" } );
	}
}
