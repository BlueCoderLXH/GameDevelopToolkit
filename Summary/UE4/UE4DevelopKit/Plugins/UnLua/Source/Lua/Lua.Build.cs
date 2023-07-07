using UnrealBuildTool;
using System.IO;

namespace UnrealBuildTool.Rules {
    public class Lua : ModuleRules {
        public Lua(ReadOnlyTargetRules Target) : base(Target) {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnableUndefinedIdentifierWarnings = false;
            ShadowVariableWarningLevel = WarningLevel.Off;

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "5.4.4"));

            PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine"
            });

            if (Target.Platform == UnrealTargetPlatform.Win64) {
                PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS");
            }
        }
    }
}
