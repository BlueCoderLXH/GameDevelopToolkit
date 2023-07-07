//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class ISlateStyle;

/**
* Implements the visual style of Dungeon Architect plugin
*/
class FDungeonArchitectHelpSystemStyle {
public:
    static void Initialize();

    static void Shutdown();

    /** @return The Slate style set for Fortnite Editor */
    static const ISlateStyle& Get();

    static FName GetStyleSetName();

private:
    static TSharedRef<class FSlateStyleSet> Create();
    static FString InResource(const FString& RelativePath, const ANSICHAR* Extension);

private:

    static TSharedPtr<class FSlateStyleSet> StyleInstance;
};

