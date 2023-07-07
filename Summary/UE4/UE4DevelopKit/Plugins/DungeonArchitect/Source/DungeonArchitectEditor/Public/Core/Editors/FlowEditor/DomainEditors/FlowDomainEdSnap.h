//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEditor.h"

class FFlowDomainEdSnap
    : public IFlowDomainEditor
    , public TSharedFromThis<FFlowDomainEdSnap>
{
    //~ Begin FFlowDomainEditorBase Interface
    virtual bool IsVisualEditor() const override { return false; }
    virtual FFlowDomainEditorTabInfo GetTabInfo() const override;
    virtual TSharedRef<SWidget> GetContentWidget() override;
    virtual void Build(FFlowExecNodeStatePtr State) override;
    //~ End FFlowDomainEditorBase Interface

private:
    virtual void InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) override;
    virtual IFlowDomainPtr CreateDomain() const override;
    
};

