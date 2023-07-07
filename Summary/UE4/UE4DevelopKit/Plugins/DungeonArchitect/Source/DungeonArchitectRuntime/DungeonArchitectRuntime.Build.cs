//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

namespace UnrealBuildTool.Rules
{
    public class DungeonArchitectRuntime : ModuleRules
    {
        public DungeonArchitectRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            bLegacyPublicIncludePaths = false;
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            ShadowVariableWarningLevel = WarningLevel.Error;
            
            PublicIncludePaths.AddRange(
                new string[]
                {
                    // ... add public include paths required here ...
                }
            );

            PrivateIncludePaths.AddRange(
                new[]
                {
                    "DungeonArchitectRuntime/Private",
                    "DungeonArchitectRuntime/Public"
                    // ... add other private include paths required here ...
                }
            );

            PublicDependencyModuleNames.AddRange(
                new[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Foliage",
                    "RenderCore",
                    "Landscape",
                    "AssetRegistry",
                    "NavigationSystem",
                    "AIModule",
                    "RHI"
                    // ... add other public dependencies that you statically link with here ...
                }
            );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    // ... add private dependencies that you statically link with here ...
                }
            );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
                    // ... add any modules that your module loads dynamically here ...
                }
            );
        }
    }
}