//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEditor.h"
#include "Frameworks/Flow/Domains/AbstractGraph/Core/FlowAbstractItem.h"

#include "UObject/GCObject.h"

class UEdGraphNode;
class SGraphEditor;
class UGridFlowAbstractEdGraph;
class UGridFlowAbstractEdGraphNode;

class FFlowDomainEdAbstractGraph
    : public IFlowDomainEditor
    , public FGCObject
{
public:
    //~ Begin FFlowDomainEditorBase Interface
    virtual FFlowDomainEditorTabInfo GetTabInfo() const override;
    virtual TSharedRef<SWidget> GetContentWidget() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Build(FFlowExecNodeStatePtr State) override;
    virtual void RecenterView(FFlowExecNodeStatePtr State) override;
    //~ End FFlowDomainEditorBase Interface

    //~ Begin FGCObject Interface
    virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
    //~ End FGCObject Interface
    
    void SelectItem(const FGuid& InItemId) const;
    static void GetAllItems(FFlowExecNodeStatePtr State, TArray<UFlowGraphItem*>& OutItems);

    void ClearAllSelections() const;
    void SelectNode(const FVector& InNodeCoord, bool bSelected) const;
    
    class IMediator {
    public:
        virtual ~IMediator() {}
        virtual void OnAbstractNodeSelectionChanged(TArray<UGridFlowAbstractEdGraphNode*> EdNodes) = 0;
        virtual void OnAbstractItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) = 0;
    };

    FORCEINLINE void SetMediator(TSharedPtr<IMediator> InMediator) { Mediator = InMediator; }

private:
    virtual void InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) override;
    virtual IFlowDomainPtr CreateDomain() const override;
    
    void OnItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) const;
    void OnAbstractNodeSelectionChanged(const TSet<class UObject*>& InSelectedObjects) const;
    
private:
    UGridFlowAbstractEdGraph* AbstractGraph = nullptr;
    TSharedPtr<SGraphEditor> AbstractGraphEditor;
    TSharedPtr<class FGridFlowAbstractGraphHandler> AbstractGraphHandler;

    TWeakPtr<IMediator> Mediator;
};

