//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Widgets/SLaunchPadBreadCrumb.h"

#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "EditorStyleSet.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "LaunchPadBreadCrumb"

void SLaunchPadBreadCrumb::Construct(const FArguments& InArgs) {
    BreadCrumbHost = SNew(SHorizontalBox);
    OnNavigation = InArgs._OnNavigation;
    ChildSlot
    [
        BreadCrumbHost.ToSharedRef()
    ];
}

void SLaunchPadBreadCrumb::SetRoot(TSharedPtr<SLaunchPadPage> InPage) {
    PageStack.Reset();
    PushPage(InPage);
}

void SLaunchPadBreadCrumb::PushPage(TSharedPtr<SLaunchPadPage> InPage) {
    PageStack.Push(InPage);
    UpdateWidget();
    OnNavigation.ExecuteIfBound(InPage);
}

void SLaunchPadBreadCrumb::UpdateWidget() {
    BreadCrumbHost->ClearChildren();
    for (int i = 0; i < PageStack.Num(); i++) {
        TSharedPtr<SLaunchPadPage> Page = PageStack[i];
        if (!Page.IsValid()) continue;

        if (i > 0) {
            BreadCrumbHost->AddSlot()
                          .Padding(FMargin(4, 0, 4, 0))
                          .AutoWidth()
            [
                //SNew(STextBlock)
                //.Text(LOCTEXT("BreadCrumbArrow", ">"))
                SNew(SBox)
				.WidthOverride(16)
				.HeightOverride(16)
                [
                    SNew(SImage)
					.ColorAndOpacity(FLinearColor(1, 1, 1, 0.5f))
					.Image(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.Arrows.Generic.Right"))
                ]
            ];
        }
        BreadCrumbHost->AddSlot()
                      .AutoWidth()
        [
            SNew(SBorder)
			.Padding(4)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
            [
                SNew(SHyperlink)
				.Text(FText::FromString(Page->GetTitle()))
				.OnNavigate(this, &SLaunchPadBreadCrumb::OnLinkClicked, Page)
            ]
        ];
    }
}


void SLaunchPadBreadCrumb::OnLinkClicked(TSharedPtr<SLaunchPadPage> InPage) {
    if (PageStack.Num() > 0 && PageStack.Last().IsValid() && PageStack.Last() == InPage) {
        // Currently active link clicked. nothing to do here
        return;
    }

    // Start popping till we reach this page
    while (PageStack.Num() > 0 && PageStack.Last() != InPage) {
        PageStack.Pop();
    }
    UpdateWidget();

    TSharedPtr<SLaunchPadPage> Top = PageStack.Num() > 0 ? PageStack.Last() : nullptr;
    if (Top.IsValid()) {
        OnNavigation.ExecuteIfBound(Top);
    }
}

#undef LOCTEXT_NAMESPACE

