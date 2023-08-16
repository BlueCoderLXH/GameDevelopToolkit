// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Engine : ModuleRules
{
	public Engine(ReadOnlyTargetRules Target) : base(Target)
	{
		...
		
		PublicIncludePaths.AddRange(new string[]
		{
			"Runtime/Engine/Classes/ObjectPool"
		});
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"Runtime/Engine/Classes/ObjectPool"
			}
		);
		
		...
	}
}
