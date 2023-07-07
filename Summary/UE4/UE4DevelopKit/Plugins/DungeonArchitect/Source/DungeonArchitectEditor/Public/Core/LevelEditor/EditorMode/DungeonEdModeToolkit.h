//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

class SWidget;
class FEdMode;

class FDungeonEdModeToolkit : public FModeToolkit {
public:
    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

    /** Initializes the dungeon mode toolkit */
    virtual void Init(const TSharedPtr<class IToolkitHost>& InitToolkitHost) override;

    /** IToolkit interface */
    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FEdMode* GetEditorMode() const override;
    virtual TSharedPtr<SWidget> GetInlineContent() const override;

    void SetInlineContent(TSharedPtr<SWidget> Widget);

private:
    TSharedPtr<SWidget> DungeonEdWidget;
};

