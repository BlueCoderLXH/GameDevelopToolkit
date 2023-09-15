//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEdSnap.h"

#include "Frameworks/Flow/Domains/Snap/SnapFlowDomain.h"

FFlowDomainEditorTabInfo FFlowDomainEdSnap::GetTabInfo() const {
    return FFlowDomainEditorTabInfo();
}

TSharedRef<SWidget> FFlowDomainEdSnap::GetContentWidget() {
    return SNullWidget::NullWidget;
}

void FFlowDomainEdSnap::Build(FFlowExecNodeStatePtr State) {
}

void FFlowDomainEdSnap::InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) {
}

IFlowDomainPtr FFlowDomainEdSnap::CreateDomain() const {
    return MakeShareable(new FSnapFlowDomain);
}

