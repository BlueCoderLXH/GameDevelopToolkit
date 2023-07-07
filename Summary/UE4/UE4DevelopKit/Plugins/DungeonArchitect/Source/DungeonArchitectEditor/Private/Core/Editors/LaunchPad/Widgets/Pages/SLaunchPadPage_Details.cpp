//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Widgets/Pages/SLaunchPadPage_Details.h"

#include "Core/Editors/LaunchPad/Actions/LaunchPadAction.h"
#include "Core/Editors/LaunchPad/Styles/LaunchPadStyle.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void SLaunchPadPage_Details::Construct(const FArguments& InArgs, const FLaunchPadPageLayout_Details& InData) {
    Data = InData;

    TSharedPtr<SVerticalBox> Host = SNew(SVerticalBox);

    // Add the title
    if (Data.Title.Len() > 0) {
        static const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 18);
        Host->AddSlot()
            .Padding(FMargin(0, 0, 0, 8))
            .AutoHeight()
        [
            SNew(STextBlock)
			.Text(FText::FromString(Data.Title))
			.Font(TitleFont)
        ];
    }

    // Add the description
    if (Data.Desc.Len() > 0) {
        Host->AddSlot()
            .Padding(FMargin(0, 0, 0, 8))
            .AutoHeight()
        [
            SNew(STextBlock)
			.Text(FText::FromString(Data.Desc))
			.AutoWrapText(true)
        ];
    }

    // Add the image, if available
    const FSlateBrush* ImageBrush = FDALaunchPadStyle::Get().GetBrush(*Data.Image);
    if (ImageBrush) {
        FVector2D ImageSize = ImageBrush->GetImageSize();
        Host->AddSlot()
            .Padding(FMargin(0, 0, 0, 8))
            .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()[SNullWidget::NullWidget]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SBox)
				.WidthOverride(ImageSize.X)
				.HeightOverride(ImageSize.Y)
                [
                    SNew(SImage)
                    .Image(ImageBrush)
                ]
            ]
            + SHorizontalBox::Slot()[SNullWidget::NullWidget]
        ];
    }

    // Add the actions
    {
        TSharedPtr<SHorizontalBox> ActionsHost = SNew(SHorizontalBox);

        if (Data.Actions.Num() > 0) {
            for (const FLaunchPadPageActionData& ActionData : Data.Actions) {
                ActionsHost->AddSlot()
                           .AutoWidth()
                           .Padding(4)
                [
                    SNew(SLaunchPadAction, ActionData)
                ];
            }

            Host->AddSlot()
                .AutoHeight()
            [
                ActionsHost.ToSharedRef()
            ];
        }
    }

    ChildSlot
    [
        SNew(SBox)
        .Padding(FMargin(10, 0, 0, 0))
        [
            Host.ToSharedRef()
        ]
    ];
}

FString SLaunchPadPage_Details::GetTitle() {
    return Data.Title;
}

