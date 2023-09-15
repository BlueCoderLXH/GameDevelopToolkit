//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/Grid/Customizations/DAGridSpatialConstraintCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorActorFolders.h"
#include "IDetailsView.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule"


void FDAGridConstraintCustomizationBase::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                         FDetailWidgetRow& HeaderRow,
                                                         IPropertyTypeCustomizationUtils& CustomizationUtils) {
    CellDataProperty = PropertyHandle->GetChildHandle("Cells");
    CreateWidget(HeaderRow);

}

void FDAGridConstraintCustomizationBase::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
                                                           IDetailChildrenBuilder& ChildBuilder,
                                                           IPropertyTypeCustomizationUtils& CustomizationUtils) {

}

TArray<UObject*> FDAGridConstraintCustomizationBase::GetSelectedObjects(IDetailLayoutBuilder& DetailBuilder) {
    TArray<UObject*> Result;
    for (auto SelectedObject : DetailBuilder.GetDetailsView()->GetSelectedObjects()) {
        Result.Add(SelectedObject.Get());
    }

    return Result;
}

FReply FDAGridConstraintCustomizationBase::OnCellClicked(int32 index) {
    auto ConstraintProperty = GetCellConstraintElement(index);
    if (ConstraintProperty.IsValid() && ConstraintProperty->IsValidHandle()) {
        uint8 Constraint;
        ConstraintProperty->GetValue(Constraint);
        Constraint = (Constraint + 1) % 3; // We have 3 states
        ConstraintProperty->SetValue(Constraint);
    }
    return FReply::Handled();
}

FText FDAGridConstraintCustomizationBase::GetButtonText(int32 index) const {
    TSharedPtr<IPropertyHandle> ConstraintProperty = GetCellConstraintElement(index);
    if (!ConstraintProperty.IsValid() || !ConstraintProperty->IsValidHandle()) {
        return NSLOCTEXT("Invalid", "Invalid", "Invalid");
    }

    uint8 Constraint;
    ConstraintProperty->GetValue(Constraint);
    return GetConstraintName(Constraint);
}

FText FDAGridConstraintCustomizationBase::GetConstraintName(uint8 value) const {
    const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EGridSpatialCellOccupation"), true);
    if (!EnumPtr) return NSLOCTEXT("Invalid", "Invalid", "Invalid");
    return EnumPtr->GetDisplayNameTextByIndex(value);
}

TSharedPtr<IPropertyHandle> FDAGridConstraintCustomizationBase::GetCellConstraintElement(uint32 index) const {
    auto CellItemProperty = GetCellElement(index);
    if (!CellItemProperty.IsValid() || !CellItemProperty->IsValidHandle()) {
        return nullptr;
    }
    return CellItemProperty->GetChildHandle("OccupationConstraint");
}

TSharedPtr<IPropertyHandle> FDAGridConstraintCustomizationBase::GetCellElement(uint32 index) const {
    if (!CellDataProperty.IsValid()) return nullptr;
    auto ArrayHandle = CellDataProperty->AsArray();
    uint32 NumItems;
    ArrayHandle->GetNumElements(NumItems);
    if (index < 0 || index >= NumItems) {
        return nullptr;
    }
    return ArrayHandle->GetElement(index);
}

/////////////////////////////////// 3x3 Layout ///////////////////////////////////////
TSharedRef<IPropertyTypeCustomization> FDAGridConstraintCustomization3x3::MakeInstance() {
    return MakeShareable(new FDAGridConstraintCustomization3x3);
}

void FDAGridConstraintCustomization3x3::CreateWidget(FDetailWidgetRow& HeaderRow) {

    HeaderRow.WholeRowContent()
    [
        SNew(SVerticalBox)

        //////////// ROW 1 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 0)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 0))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 1)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 1))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 2)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 2))
            ]
        ]

        //////////// ROW 2 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 3)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 3))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNullWidget::NullWidget
                //SNew(SButton)
                //.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 4)
                //.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 4))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 5)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 5))
            ]

        ]

        //////////// ROW 3 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 6)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 6))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 7)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 7))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
				.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 8)
				.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 8))
            ]
        ]
    ];
}

/////////////////////////////////// 2x2 Layout ///////////////////////////////////////
TSharedRef<IPropertyTypeCustomization> FDAGridConstraintCustomization2x2::MakeInstance() {
    return MakeShareable(new FDAGridConstraintCustomization2x2);
}

void FDAGridConstraintCustomization2x2::CreateWidget(FDetailWidgetRow& HeaderRow) {
    HeaderRow.WholeRowContent()[
        SNew(SVerticalBox)

        //////////// ROW 1 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.5f)[
                SNew(SButton)
				.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 0)
				.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 0))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.5f)[
                SNew(SButton)
				.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 1)
				.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 1))
            ]
        ]

        //////////// ROW 2 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.5f)[
                SNew(SButton)
				.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 2)
				.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 2))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.5f)[
                SNew(SButton)
				.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 3)
				.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 3))
            ]

        ]
    ];

}

/////////////////////////////////// Edge Layout ///////////////////////////////////////
TSharedRef<IPropertyTypeCustomization> FDAGridConstraintCustomizationEdge::MakeInstance() {
    return MakeShareable(new FDAGridConstraintCustomizationEdge);
}

void FDAGridConstraintCustomizationEdge::CreateWidget(FDetailWidgetRow& HeaderRow) {

    HeaderRow.WholeRowContent()[
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .FillWidth(0.5f)[
            SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 0)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 0))
        ]
        + SHorizontalBox::Slot()
        .FillWidth(0.5f)[
            SNew(SButton)
			.Text_Raw(this, &FDAGridConstraintCustomizationBase::GetButtonText, 1)
			.OnClicked(FOnClicked::CreateRaw(this, &FDAGridConstraintCustomizationBase::OnCellClicked, 1))
        ]
    ];
}

#undef LOCTEXT_NAMESPACE

