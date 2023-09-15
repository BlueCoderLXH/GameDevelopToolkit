//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/TestRunner/Widgets/SDATestRunner.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "EditorStyleSet.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/ISlateStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SWidget.h"

#define LOCTEXT_NAMESPACE "SDATestRunner"


namespace {
    const FName DetailsTabId("Details");
    const FName HistogramTabId("Histogram");
}

void SDATestRunner::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderTab,
                              const TSharedPtr<SWindow>& ConstructUnderWindow) {
    TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderTab);
    TSharedRef<FWorkspaceItem> AppMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(
        LOCTEXT("GridFlowPerfMenuGroupName", "Performance Stats"));

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(
        "PropertyEditor");
    const FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea, true);
    PropertyEditor = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    PropertyEditor->SetObject(GetSettingsObject());

    TabManager->RegisterTabSpawner(DetailsTabId,
                                   FOnSpawnTab::CreateRaw(this, &SDATestRunner::HandleTabManagerSpawnTab, DetailsTabId))
              .SetDisplayName(LOCTEXT("DetailsTabTitle", "Perf Settings"))
              .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "SessionFrontEnd.Tabs.Tools"))
              .SetGroup(AppMenuGroup);

    TabManager->RegisterTabSpawner(HistogramTabId,
                                   FOnSpawnTab::CreateRaw(this, &SDATestRunner::HandleTabManagerSpawnTab,
                                                          HistogramTabId))
              .SetDisplayName(LOCTEXT("HistogramTabTitle", "Histogram"))
              .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "SessionFrontEnd.Tabs.Tools"))
              .SetGroup(AppMenuGroup);


    // create tab layout
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("GridFlowPerfLayout_v0.2")
        ->AddArea
        (
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Horizontal)
            ->Split
            (
                // session browser
                FTabManager::NewStack()
                ->AddTab(DetailsTabId, ETabState::OpenedTab)
                ->SetHideTabWell(true)
                ->SetSizeCoefficient(0.25f)
            )
            ->Split
            (
                // applications
                FTabManager::NewStack()
                ->AddTab(HistogramTabId, ETabState::OpenedTab)
                ->SetForegroundTab(HistogramTabId)
                ->SetHideTabWell(true)
            )
        );


    // create & initialize main menu
    FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());

    MenuBarBuilder.AddPullDownMenu(
        LOCTEXT("WindowMenuLabel", "Window"),
        FText::GetEmpty(),
        FNewMenuDelegate::CreateStatic(&SDATestRunner::FillWindowMenu, TabManager),
        "Window"
    );

    ChildSlot
    [
        SNew(SVerticalBox)

        // Menu
        + SVerticalBox::Slot()
          .Padding(0)
          .AutoHeight()
        [
            MenuBarBuilder.MakeWidget()
        ]

        + SVerticalBox::Slot()
          .Padding(0)
          .FillHeight(1.0f)
        [
            SNew(SBorder)
            .BorderImage(FDungeonArchitectStyle::Get().GetBrush("PerfBackground"))
            [
                SNew(SVerticalBox)
                // Toolbar
                + SVerticalBox::Slot()
                  .Padding(2)
                  .AutoHeight()
                [
                    CreateToolbarWidget()
                ]

                // Content
                + SVerticalBox::Slot()
                  .Padding(2)
                  .FillHeight(1.0f)
                [
                    TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()
                ]
            ]
        ]

        // StatusBar
        + SVerticalBox::Slot()
          .Padding(2)
          .AutoHeight()
        [
            SNew(STextBlock)
            .Text(this, &SDATestRunner::GetStatusText)
        ]

    ];
}


void SDATestRunner::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
    SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    if (bIsRunning) {
        TickService();
        if (!IsServiceRunning()) {
            NotifyTestsComplete();
        }
    }
}

TSharedRef<SDockTab> SDATestRunner::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const {
    TSharedRef<SDockTab> DockTab = SNew(SDockTab)
        .TabRole(PanelTab);

    TSharedRef<SWidget> TabWidget = SNullWidget::NullWidget;
    if (TabIdentifier == DetailsTabId) {
        TabWidget = PropertyEditor.ToSharedRef();
    }
    else if (TabIdentifier == HistogramTabId) {
        TabWidget = GetHistogramWidget();
    }

    DockTab->SetContent(TabWidget);
    return DockTab;
}

void SDATestRunner::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager) {
    if (!TabManager.IsValid()) {
        return;
    }

    TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

TSharedRef<SWidget> SDATestRunner::CreateToolbarWidget() {
    return SNew(SBorder)
        .BorderImage(FEditorStyle::GetBrush("ToolBar.Background"))
        [
            SNew(SHorizontalBox)
            // Start / Stop button
            + SHorizontalBox::Slot()
              .AutoWidth()
              .Padding(2)
            [
                SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "ToolBar.Button")
				.OnClicked(this, &SDATestRunner::OnStartStopClicked)
				.ToolTipText(LOCTEXT("GridFlowPerfRecord_Tooltip", "Start/stop Performance Test"))
				.Content()
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SImage)
                        .Image(this, &SDATestRunner::GetStartStopButtonBrush)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(STextBlock)
						.Text(this, &SDATestRunner::GetStartStopButtonLabel)
						.TextStyle(&FDungeonArchitectStyle::Get().GetWidgetStyle<FTextBlockStyle>(
                                            "FlowEditor.Perf.ToolBar.Text"))
						.Justification(ETextJustify::Center)
                    ]
                ]
            ]
        ];
}

FReply SDATestRunner::OnStartStopClicked() {
    if (bIsRunning) {
        HandleStopService();
    }
    else {
        HandleStartService();
    }

    return FReply::Handled();
}

void SDATestRunner::HandleStartService() {
    if (bIsRunning) {
        StopService();
    }

    // Check if the configuration is valid
    FText ErrorMessage;
    if (!ValidateConfiguration(ErrorMessage)) {
        FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage);
        return;
    }
    
    StartService();
    bIsRunning = true;
}

void SDATestRunner::HandleStopService() {
    if (bIsRunning) {
        StopService();
    }
    bIsRunning = false;
}

const FSlateBrush* SDATestRunner::GetStartStopButtonBrush() const {
    return bIsRunning
               ? FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.Icons.Stop")
               : FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.Icons.Play");
}

FText SDATestRunner::GetStartStopButtonLabel() const {
    return bIsRunning
               ? LOCTEXT("StopButtonLabel", "Stop")
               : LOCTEXT("StartButtonLabel", "Start");
}

void SDATestRunner::NotifyTestsComplete() {
    bIsRunning = false;
}

FText SDATestRunner::GetStatusText() const {
    return bIsRunning
               ? LOCTEXT("StatusRunningLabel", "Status: Running...")
               : LOCTEXT("StatusNotRunningLabel", "Status: Not Running");
}

TSharedRef<SWidget> SDATestRunner::GetHistogramWidget() const {
    return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE

