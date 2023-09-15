//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/FlowDomainEditor.h"

#include "Frameworks/Flow/Domains/FlowDomain.h"

void IFlowDomainEditor::Initialize(TSharedPtr<IDetailsView> PropertyEditor) {
    Domain = CreateDomain();
    InitializeImpl(PropertyEditor);
}

FName IFlowDomainEditor::GetDomainID() const {
    check(Domain.IsValid());
    return Domain->GetDomainID();
}

