//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDetailCustomization.h"
#include "Layout/Margin.h"
#include "PropertyHandle.h"

class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IDetailGroup;
class IDetailLayoutBuilder;

namespace DACustomInputEditorConstants
{
	const FMargin PropertyPadding(2.0f, 0.0f, 2.0f, 0.0f);
	const float TextBoxWidth = 200.0f;
	const float ScaleBoxWidth = 50.0f;
}

struct FDACustomInputMappingSet
{
	FName SharedName;
	IDetailGroup* DetailGroup;
	TArray<TSharedRef<IPropertyHandle>> Mappings;
};

class FDACustomInputActionMappingsNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FDACustomInputActionMappingsNodeBuilder>
{
public:
	FDACustomInputActionMappingsNodeBuilder( IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle );

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRebuildChildren  ) override { OnRebuildChildren = InOnRebuildChildren; } 
	virtual bool RequiresTick() const override { return true; }
	virtual void Tick( float DeltaTime ) override;
	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override;
	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override;
	virtual bool InitiallyCollapsed() const override { return true; };
	virtual FName GetName() const override { return FName(TEXT("ActionMappings")); }

private:
	void AddActionMappingButton_OnClick();
	void ClearActionMappingButton_OnClick();
	void OnActionMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FDACustomInputMappingSet MappingSet);
	void AddActionMappingToGroupButton_OnClick(const FDACustomInputMappingSet MappingSet);
	void RemoveActionMappingGroupButton_OnClick(const FDACustomInputMappingSet MappingSet);

	bool GroupsRequireRebuild() const;
	void RebuildGroupedMappings();
	void RebuildChildren()
	{
		OnRebuildChildren.ExecuteIfBound();
	}
	/** Makes sure that groups have their expansion set after any rebuilding */
	void HandleDelayedGroupExpansion();

private:
	/** Called to rebuild the children of the detail tree */
	FSimpleDelegate OnRebuildChildren;

	/** Associated detail layout builder */
	IDetailLayoutBuilder* DetailLayoutBuilder;

	/** Property handle to associated action mappings */
	TSharedPtr<IPropertyHandle> ActionMappingsPropertyHandle;

	TArray<FDACustomInputMappingSet> GroupedMappings;

	TArray<TPair<FName, bool>> DelayedGroupExpansionStates;
};

class FDACustomInputAxisMappingsNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FDACustomInputAxisMappingsNodeBuilder>
{
public:
	FDACustomInputAxisMappingsNodeBuilder( IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle );

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRebuildChildren  ) override { OnRebuildChildren = InOnRebuildChildren; } 
	virtual bool RequiresTick() const override { return true; }
	virtual void Tick( float DeltaTime ) override;
	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override;
	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override;
	virtual bool InitiallyCollapsed() const override { return true; };
	virtual FName GetName() const override { return FName(TEXT("AxisMappings")); }

private:
	void AddAxisMappingButton_OnClick();
	void ClearAxisMappingButton_OnClick();
	void OnAxisMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FDACustomInputMappingSet MappingSet);
	void AddAxisMappingToGroupButton_OnClick(const FDACustomInputMappingSet MappingSet);
	void RemoveAxisMappingGroupButton_OnClick(const FDACustomInputMappingSet MappingSet);

	bool GroupsRequireRebuild() const;
	void RebuildGroupedMappings();
	void RebuildChildren()
	{
		OnRebuildChildren.ExecuteIfBound();
	}
	/** Makes sure that groups have their expansion set after any rebuilding */
	void HandleDelayedGroupExpansion();

private:
	/** Called to rebuild the children of the detail tree */
	FSimpleDelegate OnRebuildChildren;

	/** Associated detail layout builder */
	IDetailLayoutBuilder* DetailLayoutBuilder;

	/** Property handle to associated axis mappings */
	TSharedPtr<IPropertyHandle> AxisMappingsPropertyHandle;

	TArray<FDACustomInputMappingSet> GroupedMappings;

	TArray<TPair<FName, bool>> DelayedGroupExpansionStates;
};

class FDACustomInputBinderCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** ILayoutDetails interface */
	virtual void CustomizeDetails( class IDetailLayoutBuilder& DetailBuilder ) override;

private:
	FReply OnAddClicked(IDetailLayoutBuilder* DetailBuilder);
};

