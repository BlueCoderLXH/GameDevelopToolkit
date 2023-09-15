//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Config/CustomInputMappingDetails.h"

#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/LevelEditor/Config/CustomInputMapping.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Framework/SlateDelegates.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IDetailPropertyRow.h"
#include "IDocumentation.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "InputSettingsDetails"

/////////////////////////////////////////////// FDACustomInputActionMappingsNodeBuilder ///////////////////////////////////////////////

FDACustomInputActionMappingsNodeBuilder::FDACustomInputActionMappingsNodeBuilder( IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle )
	: DetailLayoutBuilder( InDetailLayoutBuilder )
	, ActionMappingsPropertyHandle( InPropertyHandle )
{
	// Delegate for when the children in the array change
	FSimpleDelegate RebuildChildrenDelegate = FSimpleDelegate::CreateRaw( this, &FDACustomInputActionMappingsNodeBuilder::RebuildChildren );
	ActionMappingsPropertyHandle->SetOnPropertyValueChanged( RebuildChildrenDelegate );
	ActionMappingsPropertyHandle->AsArray()->SetOnNumElementsChanged( RebuildChildrenDelegate );
}

void FDACustomInputActionMappingsNodeBuilder::Tick( float DeltaTime )
{
	if (GroupsRequireRebuild())
	{
		RebuildChildren();
	}
	HandleDelayedGroupExpansion();
}

void FDACustomInputActionMappingsNodeBuilder::GenerateHeaderRowContent( FDetailWidgetRow& NodeRow )
{
	TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton( FSimpleDelegate::CreateSP( this, &FDACustomInputActionMappingsNodeBuilder::AddActionMappingButton_OnClick), 
		LOCTEXT("AddActionMappingToolTip", "Adds Action Mapping") );

	TSharedRef<SWidget> ClearButton = PropertyCustomizationHelpers::MakeEmptyButton( FSimpleDelegate::CreateSP( this, &FDACustomInputActionMappingsNodeBuilder::ClearActionMappingButton_OnClick), 
		LOCTEXT("ClearActionMappingToolTip", "Removes all Action Mappings") );

	NodeRow
	[
		SNew( SHorizontalBox )
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			ActionMappingsPropertyHandle->CreatePropertyNameWidget()
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			AddButton
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			ClearButton
		]
	];
}

void FDACustomInputActionMappingsNodeBuilder::GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder )
{
	RebuildGroupedMappings();

	for (int32 Index = 0; Index < GroupedMappings.Num(); ++Index)
	{
		FDACustomInputMappingSet& MappingSet = GroupedMappings[Index];

		FString GroupNameString(TEXT("ActionMappings."));
		MappingSet.SharedName.AppendString(GroupNameString);
		FName GroupName(*GroupNameString);
		IDetailGroup& ActionMappingGroup = ChildrenBuilder.AddGroup(GroupName, FText::FromName(MappingSet.SharedName));
		MappingSet.DetailGroup = &ActionMappingGroup;

		TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FDACustomInputActionMappingsNodeBuilder::AddActionMappingToGroupButton_OnClick, MappingSet),
			LOCTEXT("AddActionMappingToGroupToolTip", "Adds Action Mapping to Group"));

		TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FDACustomInputActionMappingsNodeBuilder::RemoveActionMappingGroupButton_OnClick, MappingSet),
			LOCTEXT("RemoveActionMappingGroupToolTip", "Removes Action Mapping Group"));

		ActionMappingGroup.HeaderRow()
		[
			SNew( SHorizontalBox )
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride( DACustomInputEditorConstants::TextBoxWidth )
				[
					SNew(SEditableTextBox)
					.Padding(2.0f)
					.Text(FText::FromName(MappingSet.SharedName))
					.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FDACustomInputActionMappingsNodeBuilder::OnActionMappingNameCommitted, MappingSet))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+SHorizontalBox::Slot()
			.Padding(DACustomInputEditorConstants::PropertyPadding)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				AddButton
			]
			+SHorizontalBox::Slot()
			.Padding(DACustomInputEditorConstants::PropertyPadding)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				RemoveButton
			]
		];
	
		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			ActionMappingGroup.AddPropertyRow(MappingSet.Mappings[MappingIndex]).ShowPropertyButtons(false);
		}
	}
}

void FDACustomInputActionMappingsNodeBuilder::AddActionMappingButton_OnClick()
{
	static const FName BaseActionMappingName(*LOCTEXT("NewActionMappingName", "NewActionMapping").ToString());
	static int32 NewMappingCount = 0;
	const FScopedTransaction Transaction(LOCTEXT("AddActionMapping_Transaction", "Add Action Mapping"));

	TArray<UObject*> OuterObjects;
	ActionMappingsPropertyHandle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 1)
	{
		ADACustomInputConfigBinder* InputSettings = CastChecked<ADACustomInputConfigBinder>(OuterObjects[0]);
		InputSettings->Modify();
		ActionMappingsPropertyHandle->NotifyPreChange();

		FName NewActionMappingName;
		bool bFoundUniqueName;
		do
		{
			// Create a numbered name and check whether it's already been used
			NewActionMappingName = FName(BaseActionMappingName, ++NewMappingCount);
			bFoundUniqueName = true;
			for (int32 Index = 0; Index < InputSettings->ActionMappings.Num(); ++Index)
			{
				if (InputSettings->ActionMappings[Index].ActionName == NewActionMappingName)
				{
					bFoundUniqueName = false;
					break;
				}
			}
		}
		while (!bFoundUniqueName);

		DelayedGroupExpansionStates.Emplace(NewActionMappingName, true);
		FInputActionKeyMapping NewMapping(NewActionMappingName);
		InputSettings->ActionMappings.Add(NewMapping);

		ActionMappingsPropertyHandle->NotifyPostChange();
	}
}

void FDACustomInputActionMappingsNodeBuilder::ClearActionMappingButton_OnClick()
{
	ActionMappingsPropertyHandle->AsArray()->EmptyArray();
}

void FDACustomInputActionMappingsNodeBuilder::OnActionMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("RenameActionMapping_Transaction", "Rename Action Mapping"));

	FName NewName = FName(*InName.ToString());
	FName CurrentName = NewName;

	if (MappingSet.Mappings.Num() > 0)
	{
		MappingSet.Mappings[0]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, ActionName))->GetValue(CurrentName);
	}

	if (NewName != CurrentName)
	{
		for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
		{
			MappingSet.Mappings[Index]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, ActionName))->SetValue(NewName);
		}

		if (MappingSet.DetailGroup)
		{
			DelayedGroupExpansionStates.Emplace(NewName, MappingSet.DetailGroup->GetExpansionState());

			// Don't want to save expansion state of old name
			MappingSet.DetailGroup->ToggleExpansion(false);
		}
	}
}

void FDACustomInputActionMappingsNodeBuilder::AddActionMappingToGroupButton_OnClick(const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("AddActionMappingToGroup_Transaction", "Add Action Mapping To Group"));

	TArray<UObject*> OuterObjects;
	ActionMappingsPropertyHandle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 1)
	{
		ADACustomInputConfigBinder* InputSettings = CastChecked<ADACustomInputConfigBinder>(OuterObjects[0]);
		InputSettings->Modify();
		ActionMappingsPropertyHandle->NotifyPreChange();

		DelayedGroupExpansionStates.Emplace(MappingSet.SharedName, true);
		FInputActionKeyMapping NewMapping(MappingSet.SharedName);
		InputSettings->ActionMappings.Add(NewMapping);

		ActionMappingsPropertyHandle->NotifyPostChange();
	}
}

void FDACustomInputActionMappingsNodeBuilder::RemoveActionMappingGroupButton_OnClick(const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveActionMappingGroup_Transaction", "Remove Action Mapping Group"));

	TSharedPtr<IPropertyHandleArray> ActionMappingsArrayHandle = ActionMappingsPropertyHandle->AsArray();

	TArray<int32> SortedIndices;
	for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
	{
		SortedIndices.AddUnique(MappingSet.Mappings[Index]->GetIndexInArray());
	}
	SortedIndices.Sort();

	for (int32 Index = SortedIndices.Num() - 1; Index >= 0; --Index)
	{
		ActionMappingsArrayHandle->DeleteItem(SortedIndices[Index]);
	}
}

bool FDACustomInputActionMappingsNodeBuilder::GroupsRequireRebuild() const
{
	for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
	{
		const FDACustomInputMappingSet& MappingSet = GroupedMappings[GroupIndex];
		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			FName ActionName;
			MappingSet.Mappings[MappingIndex]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, ActionName))->GetValue(ActionName);
			if (MappingSet.SharedName != ActionName)
			{
				return true;
			}
		}
	}
	return false;
}

void FDACustomInputActionMappingsNodeBuilder::RebuildGroupedMappings()
{
	GroupedMappings.Empty();

	TSharedPtr<IPropertyHandleArray> ActionMappingsArrayHandle = ActionMappingsPropertyHandle->AsArray();

	uint32 NumMappings;
	ActionMappingsArrayHandle->GetNumElements(NumMappings);
	for (uint32 Index = 0; Index < NumMappings; ++Index)
	{
		TSharedRef<IPropertyHandle> ActionMapping = ActionMappingsArrayHandle->GetElement(Index);
		FName ActionName;
		FPropertyAccess::Result Result = ActionMapping->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, ActionName))->GetValue(ActionName);

		if (Result == FPropertyAccess::Success)
		{
			int32 FoundIndex = INDEX_NONE;
			for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
			{
				if (GroupedMappings[GroupIndex].SharedName == ActionName)
				{
					FoundIndex = GroupIndex;
					break;
				}
			}
			if (FoundIndex == INDEX_NONE)
			{
				FoundIndex = GroupedMappings.Num();
				GroupedMappings.AddZeroed();
				GroupedMappings[FoundIndex].SharedName = ActionName;
			}
			GroupedMappings[FoundIndex].Mappings.Add(ActionMapping);
		}
	}
}

void FDACustomInputActionMappingsNodeBuilder::HandleDelayedGroupExpansion()
{
	if (DelayedGroupExpansionStates.Num() > 0)
	{
		for (auto GroupState : DelayedGroupExpansionStates)
		{
			for (auto& MappingSet : GroupedMappings)
			{
				if (MappingSet.SharedName == GroupState.Key)
				{
					MappingSet.DetailGroup->ToggleExpansion(GroupState.Value);
					break;
				}
			}
		}
		DelayedGroupExpansionStates.Empty();
	}
}

/////////////////////////////////////////////// FDACustomInputAxisMappingsNodeBuilder ///////////////////////////////////////////////

FDACustomInputAxisMappingsNodeBuilder::FDACustomInputAxisMappingsNodeBuilder( IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle )
	: DetailLayoutBuilder( InDetailLayoutBuilder )
	, AxisMappingsPropertyHandle( InPropertyHandle )
{
	// Delegate for when the children in the array change
	FSimpleDelegate RebuildChildrenDelegate = FSimpleDelegate::CreateRaw( this, &FDACustomInputAxisMappingsNodeBuilder::RebuildChildren );
	AxisMappingsPropertyHandle->SetOnPropertyValueChanged( RebuildChildrenDelegate );
	AxisMappingsPropertyHandle->AsArray()->SetOnNumElementsChanged( RebuildChildrenDelegate );
}

void FDACustomInputAxisMappingsNodeBuilder::Tick( float DeltaTime )
{
	if (GroupsRequireRebuild())
	{
		RebuildChildren();
	}
	HandleDelayedGroupExpansion();
}

void FDACustomInputAxisMappingsNodeBuilder::GenerateHeaderRowContent( FDetailWidgetRow& NodeRow )
{
	TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton( FSimpleDelegate::CreateSP( this, &FDACustomInputAxisMappingsNodeBuilder::AddAxisMappingButton_OnClick), 
		LOCTEXT("AddAxisMappingToolTip", "Adds Axis Mapping") );

	TSharedRef<SWidget> ClearButton = PropertyCustomizationHelpers::MakeEmptyButton( FSimpleDelegate::CreateSP( this, &FDACustomInputAxisMappingsNodeBuilder::ClearAxisMappingButton_OnClick), 
		LOCTEXT("ClearAxisMappingToolTip", "Removes all Axis Mappings") );

	NodeRow
	[
		SNew( SHorizontalBox )
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			AxisMappingsPropertyHandle->CreatePropertyNameWidget()
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			AddButton
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			ClearButton
		]
	];
}

void FDACustomInputAxisMappingsNodeBuilder::GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder )
{
	RebuildGroupedMappings();

	for (int32 Index = 0; Index < GroupedMappings.Num(); ++Index)
	{
		FDACustomInputMappingSet& MappingSet = GroupedMappings[Index];

		FString GroupNameString(TEXT("AxisMappings."));
		MappingSet.SharedName.AppendString(GroupNameString);
		FName GroupName(*GroupNameString);
		IDetailGroup& AxisMappingGroup = ChildrenBuilder.AddGroup(GroupName, FText::FromName(MappingSet.SharedName));
		MappingSet.DetailGroup = &AxisMappingGroup;

		TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FDACustomInputAxisMappingsNodeBuilder::AddAxisMappingToGroupButton_OnClick, MappingSet),
			LOCTEXT("AddAxisMappingToGroupToolTip", "Adds Axis Mapping to Group"));

		TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FDACustomInputAxisMappingsNodeBuilder::RemoveAxisMappingGroupButton_OnClick, MappingSet),
			LOCTEXT("RemoveAxisMappingGroupToolTip", "Removes Axis Mapping Group"));

		AxisMappingGroup.HeaderRow()
		[
			SNew( SHorizontalBox )
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew( SBox )
				.WidthOverride( DACustomInputEditorConstants::TextBoxWidth )
				[
					SNew(SEditableTextBox)
					.Padding(2.0f)
					.Text(FText::FromName(MappingSet.SharedName))
					.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FDACustomInputAxisMappingsNodeBuilder::OnAxisMappingNameCommitted, MappingSet))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+SHorizontalBox::Slot()
			.Padding(DACustomInputEditorConstants::PropertyPadding)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				AddButton
			]
			+SHorizontalBox::Slot()
			.Padding(DACustomInputEditorConstants::PropertyPadding)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				RemoveButton
			]
		];

		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			AxisMappingGroup.AddPropertyRow(MappingSet.Mappings[MappingIndex]).ShowPropertyButtons(false);
		}
	}
}

void FDACustomInputAxisMappingsNodeBuilder::AddAxisMappingButton_OnClick()
{
	static const FName BaseAxisMappingName(*LOCTEXT("NewAxisMappingName", "NewAxisMapping").ToString());
	static int32 NewMappingCount = 0;
	const FScopedTransaction Transaction(LOCTEXT("AddAxisMapping_Transaction", "Add Axis Mapping"));

	TArray<UObject*> OuterObjects;
	AxisMappingsPropertyHandle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 1)
	{
		ADACustomInputConfigBinder* InputSettings = CastChecked<ADACustomInputConfigBinder>(OuterObjects[0]);
		InputSettings->Modify();
		AxisMappingsPropertyHandle->NotifyPreChange();

		FName NewAxisMappingName;
		bool bFoundUniqueName;
		do
		{
			// Create a numbered name and check whether it's already been used
			NewAxisMappingName = FName(BaseAxisMappingName, ++NewMappingCount);
			bFoundUniqueName = true;
			for (int32 Index = 0; Index < InputSettings->AxisMappings.Num(); ++Index)
			{
				if (InputSettings->AxisMappings[Index].AxisName == NewAxisMappingName)
				{
					bFoundUniqueName = false;
					break;
				}
			}
		}
		while (!bFoundUniqueName);

		DelayedGroupExpansionStates.Emplace(NewAxisMappingName, true);
		FInputAxisKeyMapping NewMapping(NewAxisMappingName);
		InputSettings->AxisMappings.Add(NewMapping);

		AxisMappingsPropertyHandle->NotifyPostChange();
	}
}

void FDACustomInputAxisMappingsNodeBuilder::ClearAxisMappingButton_OnClick()
{
	AxisMappingsPropertyHandle->AsArray()->EmptyArray();
}

void FDACustomInputAxisMappingsNodeBuilder::OnAxisMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("RenameAxisMapping_Transaction", "Rename Axis Mapping"));

	FName NewName = FName(*InName.ToString());
	FName CurrentName = NewName;

	if (MappingSet.Mappings.Num() > 0)
	{
		MappingSet.Mappings[0]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, AxisName))->GetValue(CurrentName);
	}

	if (NewName != CurrentName)
	{
		for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
		{
			MappingSet.Mappings[Index]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, AxisName))->SetValue(NewName);
		}

		if (MappingSet.DetailGroup)
		{
			DelayedGroupExpansionStates.Emplace(NewName, MappingSet.DetailGroup->GetExpansionState());

			// Don't want to save expansion state of old name
			MappingSet.DetailGroup->ToggleExpansion(false);
		}
	}
}

void FDACustomInputAxisMappingsNodeBuilder::AddAxisMappingToGroupButton_OnClick(const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("AddAxisMappingToGroup_Transaction", "Add Axis Mapping To Group"));

	TArray<UObject*> OuterObjects;
	AxisMappingsPropertyHandle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 1)
	{
		ADACustomInputConfigBinder* InputSettings = CastChecked<ADACustomInputConfigBinder>(OuterObjects[0]);
		InputSettings->Modify();
		AxisMappingsPropertyHandle->NotifyPreChange();

		DelayedGroupExpansionStates.Emplace(MappingSet.SharedName, true);
		FInputAxisKeyMapping NewMapping(MappingSet.SharedName);
		InputSettings->AxisMappings.Add(NewMapping);

		AxisMappingsPropertyHandle->NotifyPostChange();
	}
}

void FDACustomInputAxisMappingsNodeBuilder::RemoveAxisMappingGroupButton_OnClick(const FDACustomInputMappingSet MappingSet)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveAxisMappingGroup_Transaction", "Remove Axis Mapping Group"));

	TSharedPtr<IPropertyHandleArray> AxisMappingsArrayHandle = AxisMappingsPropertyHandle->AsArray();

	TArray<int32> SortedIndices;
	for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
	{
		SortedIndices.AddUnique(MappingSet.Mappings[Index]->GetIndexInArray());
	}
	SortedIndices.Sort();

	for (int32 Index = SortedIndices.Num() - 1; Index >= 0; --Index)
	{
		AxisMappingsArrayHandle->DeleteItem(SortedIndices[Index]);
	}
}

bool FDACustomInputAxisMappingsNodeBuilder::GroupsRequireRebuild() const
{
	for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
	{
		const FDACustomInputMappingSet& MappingSet = GroupedMappings[GroupIndex];
		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			FName AxisName;
			MappingSet.Mappings[MappingIndex]->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, AxisName))->GetValue(AxisName);
			if (MappingSet.SharedName != AxisName)
			{
				return true;
			}
		}
	}
	return false;
}

void FDACustomInputAxisMappingsNodeBuilder::RebuildGroupedMappings()
{
	GroupedMappings.Empty();

	TSharedPtr<IPropertyHandleArray> AxisMappingsArrayHandle = AxisMappingsPropertyHandle->AsArray();

	uint32 NumMappings;
	AxisMappingsArrayHandle->GetNumElements(NumMappings);
	for (uint32 Index = 0; Index < NumMappings; ++Index)
	{
		TSharedRef<IPropertyHandle> AxisMapping = AxisMappingsArrayHandle->GetElement(Index);
		FName AxisName;
		FPropertyAccess::Result Result = AxisMapping->GetChildHandle(GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, AxisName))->GetValue(AxisName);

		if (Result == FPropertyAccess::Success)
		{
			int32 FoundIndex = INDEX_NONE;
			for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
			{
				if (GroupedMappings[GroupIndex].SharedName == AxisName)
				{
					FoundIndex = GroupIndex;
					break;
				}
			}
			if (FoundIndex == INDEX_NONE)
			{
				FoundIndex = GroupedMappings.Num();
				GroupedMappings.AddZeroed();
				GroupedMappings[FoundIndex].SharedName = AxisName;
			}
			GroupedMappings[FoundIndex].Mappings.Add(AxisMapping);
		}
	}
}

void FDACustomInputAxisMappingsNodeBuilder::HandleDelayedGroupExpansion()
{
	if (DelayedGroupExpansionStates.Num() > 0)
	{
		for (auto GroupState : DelayedGroupExpansionStates)
		{
			for (auto& MappingSet : GroupedMappings)
			{
				if (MappingSet.SharedName == GroupState.Key)
				{
					MappingSet.DetailGroup->ToggleExpansion(GroupState.Value);
					break;
				}
			}
		}
		DelayedGroupExpansionStates.Empty();
	}
}


/////////////////////////////////////////////// FDACustomInputBinderCustomization ///////////////////////////////////////////////

TSharedRef<IDetailCustomization> FDACustomInputBinderCustomization::MakeInstance()
{
	return MakeShareable(new FDACustomInputBinderCustomization);
}

void FDACustomInputBinderCustomization::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
	static const FName BindingsCategory = TEXT("Bindings");
	static const FName ActionMappings = GET_MEMBER_NAME_CHECKED(ADACustomInputConfigBinder, ActionMappings);
	static const FName AxisMappings = GET_MEMBER_NAME_CHECKED(ADACustomInputConfigBinder, AxisMappings);

	IDetailCategoryBuilder& MappingsDetailCategoryBuilder = DetailBuilder.EditCategory(BindingsCategory);

	MappingsDetailCategoryBuilder.AddCustomRow(LOCTEXT("Mappings_Title", "Action Axis Mappings"))
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.AutoWrapText(true)
			.Text(LOCTEXT("Mappings_Description", "Provide action and axis mappings here that are required by the game project.  Click \'Add Missing Inputs\' button to update the input settings"))
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			IDocumentation::Get()->CreateAnchor(FString("Gameplay/Input"))
		]
	];

	// Custom Action Mappings
	const TSharedPtr<IPropertyHandle> ActionMappingsPropertyHandle = DetailBuilder.GetProperty(ActionMappings, ADACustomInputConfigBinder::StaticClass());
	ActionMappingsPropertyHandle->MarkHiddenByCustomization();

	const TSharedRef<FDACustomInputActionMappingsNodeBuilder> ActionMappingsBuilder = MakeShareable( new FDACustomInputActionMappingsNodeBuilder( &DetailBuilder, ActionMappingsPropertyHandle ) );
	MappingsDetailCategoryBuilder.AddCustomBuilder(ActionMappingsBuilder);

	// Custom Axis Mappings
	const TSharedPtr<IPropertyHandle> AxisMappingsPropertyHandle = DetailBuilder.GetProperty(AxisMappings, ADACustomInputConfigBinder::StaticClass());
	AxisMappingsPropertyHandle->MarkHiddenByCustomization();

	const TSharedRef<FDACustomInputAxisMappingsNodeBuilder> AxisMappingsBuilder = MakeShareable( new FDACustomInputAxisMappingsNodeBuilder( &DetailBuilder, AxisMappingsPropertyHandle ) );
	MappingsDetailCategoryBuilder.AddCustomBuilder(AxisMappingsBuilder);
	
	MappingsDetailCategoryBuilder.AddCustomRow(LOCTEXT("AddInputCommand_Filter", "Add Missing Input"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("AddInputCommand", "Add Missing Input"))
			.OnClicked(FOnClicked::CreateRaw(this, &FDACustomInputBinderCustomization::OnAddClicked, &DetailBuilder))
		];

	DetailBuilder.HideCategory("Transform");
	DetailBuilder.HideCategory("Rendering");
	DetailBuilder.HideCategory("Input");
	DetailBuilder.HideCategory("Actor");
	DetailBuilder.HideCategory("LOD");
	DetailBuilder.HideCategory("Cooking");
}

FReply FDACustomInputBinderCustomization::OnAddClicked(IDetailLayoutBuilder* DetailBuilder)
{
	ADACustomInputConfigBinder* InputBinder = FDungeonEditorUtils::GetBuilderObject<ADACustomInputConfigBinder>(DetailBuilder);
	InputBinder->BindMissingInput(true);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

