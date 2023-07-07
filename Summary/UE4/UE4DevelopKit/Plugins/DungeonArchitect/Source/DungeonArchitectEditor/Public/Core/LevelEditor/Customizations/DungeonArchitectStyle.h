//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
* Implements the visual style of Dungeon Architect plugin
*/
class FDungeonArchitectStyle {
public:
    static void Initialize();

    static void Shutdown();

    static const ISlateStyle& Get();

    static FName GetStyleSetName();

    class FStyle : public FSlateStyleSet {
    public:
        FStyle();
        void Initialize();

    private:
        FTextBlockStyle NormalText;
    };

private:
    static TSharedRef<class FSlateStyleSet> Create();
    static FString InResource(const FString& RelativePath, const ANSICHAR* Extension);


private:
    static TSharedPtr<class FSlateStyleSet> StyleInstance;
};

