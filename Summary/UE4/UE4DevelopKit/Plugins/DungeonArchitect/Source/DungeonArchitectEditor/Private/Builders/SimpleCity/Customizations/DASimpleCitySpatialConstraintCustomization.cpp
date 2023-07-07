//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SimpleCity/Customizations/DASimpleCitySpatialConstraintCustomization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorActorFolders.h"
#include "IDetailsView.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule"


void FDASimpleCityConstraintCustomizationBase::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
                                                               FDetailWidgetRow& HeaderRow,
                                                               IPropertyTypeCustomizationUtils& CustomizationUtils) {
    CellDataProperty = PropertyHandle->GetChildHandle("Cells");
    CreateWidget(HeaderRow);

}

void FDASimpleCityConstraintCustomizationBase::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
                                                                 IDetailChildrenBuilder& ChildBuilder,
                                                                 IPropertyTypeCustomizationUtils& CustomizationUtils) {

}

TArray<UObject*> FDASimpleCityConstraintCustomizationBase::GetSelectedObjects(IDetailLayoutBuilder& DetailBuilder) {
    TArray<UObject*> Result;

    for (auto SelectedObject : DetailBuilder.GetDetailsView()->GetSelectedObjects()) {
        Result.Add(SelectedObject.Get());
    }

    return Result;
}

FReply FDASimpleCityConstraintCustomizationBase::OnCellClicked(int32 index) {
    auto ConstraintProperty = GetCellConstraintElement(index);
    if (ConstraintProperty.IsValid() && ConstraintProperty->IsValidHandle()) {
        uint8 Constraint;
        ConstraintProperty->GetValue(Constraint);
        Constraint = (Constraint + 1) % 5; // We have 3 states
        ConstraintProperty->SetValue(Constraint);
    }
    return FReply::Handled();
}

FText FDASimpleCityConstraintCustomizationBase::GetButtonText(int32 index) const {
    TSharedPtr<IPropertyHandle> ConstraintProperty = GetCellConstraintElement(index);
    if (!ConstraintProperty.IsValid() || !ConstraintProperty->IsValidHandle()) {
        return NSLOCTEXT("Invalid", "Invalid", "Invalid");
    }

    uint8 Constraint;
    ConstraintProperty->GetValue(Constraint);
    return GetConstraintName(Constraint);
}

FText FDASimpleCityConstraintCustomizationBase::GetConstraintName(uint8 value) const {
    const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ESimpleCitySpatialCellOccupation"), true);
    if (!EnumPtr) return NSLOCTEXT("Invalid", "Invalid", "Invalid");
    return EnumPtr->GetDisplayNameTextByIndex(value);
}

TSharedPtr<IPropertyHandle> FDASimpleCityConstraintCustomizationBase::GetCellConstraintElement(uint32 index) const {
    auto CellItemProperty = GetCellElement(index);
    if (!CellItemProperty.IsValid() || !CellItemProperty->IsValidHandle()) {
        return nullptr;
    }
    return CellItemProperty->GetChildHandle("OccupationConstraint");
}

TSharedPtr<IPropertyHandle> FDASimpleCityConstraintCustomizationBase::GetCellElement(uint32 index) const {
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
TSharedRef<IPropertyTypeCustomization> FDASimpleCityConstraintCustomization3x3::MakeInstance() {
    return MakeShareable(new FDASimpleCityConstraintCustomization3x3);
}

void FDASimpleCityConstraintCustomization3x3::CreateWidget(FDetailWidgetRow& HeaderRow) {

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
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 0)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 0))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 1)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 1))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 2)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 2))
            ]
        ]

        //////////// ROW 2 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 3)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 3))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNullWidget::NullWidget
                //SNew(SButton)
                //.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 4)
                //.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 4))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 5)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 5))
            ]

        ]

        //////////// ROW 3 ////////////
        + SVerticalBox::Slot()
        .AutoHeight()[
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 6)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 6))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
			.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 7)
			.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 7))
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.33f)[
                SNew(SButton)
				.Text_Raw(this, &FDASimpleCityConstraintCustomizationBase::GetButtonText, 8)
				.OnClicked(FOnClicked::CreateRaw(this, &FDASimpleCityConstraintCustomizationBase::OnCellClicked, 8))
            ]
        ]
    ];
}

#undef LOCTEXT_NAMESPACE

