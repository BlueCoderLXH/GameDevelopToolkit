//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

#define DECLARE_FLOW_TAB_FACTORY(NAME)			\
class FSnapMapEditorTabFactory_##NAME : public FWorkflowTabFactory {	\
public:	\
	FSnapMapEditorTabFactory_##NAME(TSharedPtr<class FSnapMapEditor> InFlowEditor, TSharedPtr<SWidget> InWidget);	\
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;	\
	virtual FText GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const override;	\
	virtual TSharedRef<SDockTab> SpawnTab(const FWorkflowTabSpawnInfo& Info) const override;	\
protected:	\
	TWeakPtr<class FSnapMapEditor> FlowEditorPtr;	\
	TWeakPtr<SWidget> WidgetPtr;	\
};

///////////////////////////////////////////////////////////////
#define DEFINE_FLOW_TAB_FACTORY_BASE(NAME, CAPTION, ICON, TOOLTIP_MENU, TOOLTIP_TAB)	\
FSnapMapEditorTabFactory_##NAME::FSnapMapEditorTabFactory_##NAME(TSharedPtr<FSnapMapEditor> InFlowEditor, TSharedPtr<SWidget> InWidget)		\
	: FWorkflowTabFactory(FSnapMapEditorTabs::NAME##ID, InFlowEditor)		\
	, FlowEditorPtr(InFlowEditor)		\
	, WidgetPtr(InWidget)		\
{		\
	bIsSingleton = true;		\
	TabLabel = CAPTION;		\
	TabIcon = FSlateIcon(FEditorStyle::GetStyleSetName(), ICON);		\
	ViewMenuDescription = CAPTION;		\
	ViewMenuTooltip = TOOLTIP_MENU;		\
}		\
TSharedRef<SWidget> FSnapMapEditorTabFactory_##NAME::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const		\
{		\
	check (WidgetPtr.IsValid());	\
	return WidgetPtr.Pin().ToSharedRef();		\
}		\
FText FSnapMapEditorTabFactory_##NAME::GetTabToolTipText(const FWorkflowTabSpawnInfo& Info) const		\
{		\
	return TOOLTIP_TAB;		\
}
///////////////////////////////////////////////////////////////
#define DEFINE_FLOW_TAB_FACTORY(NAME, CAPTION, ICON, TOOLTIP_MENU, TOOLTIP_TAB)	\
	DEFINE_FLOW_TAB_FACTORY_BASE(NAME, CAPTION, ICON, TOOLTIP_MENU, TOOLTIP_TAB)	\
	TSharedRef<SDockTab> FSnapMapEditorTabFactory_##NAME::SpawnTab(const FWorkflowTabSpawnInfo& Info) const { return FWorkflowTabFactory::SpawnTab(Info); }

///////////////////////////////////////////////////////////////
#define DEFINE_FLOW_TAB_FACTORY_CUSTOM_WIDGET(NAME, CAPTION, ICON, TOOLTIP_MENU, TOOLTIP_TAB)	\
	DEFINE_FLOW_TAB_FACTORY_BASE(NAME, CAPTION, ICON, TOOLTIP_MENU, TOOLTIP_TAB)


	

