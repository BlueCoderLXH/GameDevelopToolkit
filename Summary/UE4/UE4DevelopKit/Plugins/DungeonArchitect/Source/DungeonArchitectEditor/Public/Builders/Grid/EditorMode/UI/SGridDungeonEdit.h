//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/LevelEditor/EditorMode/IDungeonEdTool.h"

class UGridDungeonEdModeHandler;

class SGridDungeonEdit : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SGridDungeonEdit) {}
    SLATE_END_ARGS()

public:
    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs);

    ~SGridDungeonEdit();

    // SWidget interface
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget interface

protected:
    bool GetIsPropertyVisible(const struct FPropertyAndParent& PropertyAndParent) const;

private:
    /** Creates the toolbar. */
    TSharedRef<SWidget> BuildToolBar();

    /** Clears all the tools selection by setting them to false. */
    void ClearAllToolSelection();

    /** Binds UI commands for the toolbar. */
    void BindCommands();

    void OnToolSelectedPaint();
    void OnToolSelectedRectangle();
    void OnToolSelectedBorder();

    bool IsToolActive_Paint();
    bool IsToolActive_Rectangle();
    bool IsToolActive_Border();
    bool IsToolActive(FDungeonEdToolID ToolType);

    void SwitchTool(FDungeonEdToolID ToolType);

    UGridDungeonEdModeHandler* GetModeHandler();

private:
    /** Command list for binding functions for the toolbar. */
    TSharedPtr<FUICommandList> UICommandList;

    TSharedPtr<class IDetailsView> DetailsPanel;
};

