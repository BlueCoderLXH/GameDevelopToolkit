//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEditor.h"

class SFlowDomainEdViewport;
class USphereComponent;
class UTextRenderComponent;
class AStaticMeshActor;

class FFlowDomainEdAbstractGraph3D
    : public IFlowDomainEditor
    , public FGCObject
    , public TSharedFromThis<FFlowDomainEdAbstractGraph3D>
{
public:
    //~ Begin FFlowDomainEditorBase Interface
    virtual FFlowDomainEditorTabInfo GetTabInfo() const override;
    virtual TSharedRef<SWidget> GetContentWidget() override;
    virtual void Build(FFlowExecNodeStatePtr State) override;
    virtual void RecenterView(FFlowExecNodeStatePtr State) override;
    virtual void Tick(float DeltaTime) override;
    //~ End FFlowDomainEditorBase Interface

    //~ Begin FGCObject Interface
    virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
    //~ End FGCObject Interface
    
    class IMediator {
    public:
        virtual ~IMediator() {}
        //virtual void OnAbstractNodeSelectionChanged(TArray<UGridFlowAbstractEdGraphNode*> EdNodes) = 0;
        //virtual void OnAbstractItemWidgetClicked(const FGuid& InItemId, bool bDoubleClicked) = 0;
    };

    FORCEINLINE void SetMediator(TSharedPtr<IMediator> InMediator) { Mediator = InMediator; }

private:
    virtual void InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) override;
    virtual IFlowDomainPtr CreateDomain() const override;
    void OnActorSelectionChanged(AActor* InActor);
    void OnActorDoubleClicked(AActor* InActor);
    
private:
    TSharedPtr<SFlowDomainEdViewport> Viewport;
    TSharedPtr<SWidget> ContentWidget;
    TWeakPtr<IMediator> Mediator;
    TWeakObjectPtr<class UFDAbstractNodePreview> SelectedNode;
    AStaticMeshActor* Skybox = nullptr;
};

