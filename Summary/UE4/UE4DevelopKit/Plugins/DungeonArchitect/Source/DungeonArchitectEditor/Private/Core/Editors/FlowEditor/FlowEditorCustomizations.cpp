//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/FlowEditorCustomizations.h"

#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/Utils/StringUtils.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraphSchema.h"
#include "Frameworks/Flow/ExecGraph/Nodes/GridFlowExecEdGraphNodes.h"

#include "DetailCategoryBuilder.h"

class UGridFlowExecScriptTaskNode;

void FFlowEditorTaskCustomizations::RegisterTaskNodes(FPropertyEditorModule& PropertyEditorModule) {
    TArray<UClass*> TaskClasses;
    GetTaskClasses(TaskClasses);
    
    for (UClass* TaskClass : TaskClasses) {
        const FName ClassName = TaskClass->GetFName();
        PropertyEditorModule.RegisterCustomClassLayout(ClassName, FOnGetDetailCustomizationInstance::CreateStatic(&FFlowEditorTaskCustomizations::MakeInstance));
    }
}

void FFlowEditorTaskCustomizations::UnregisterTaskNodes(FPropertyEditorModule& PropertyEditorModule) {
    TArray<UClass*> TaskClasses;
    GetTaskClasses(TaskClasses);

    for (UClass* TaskClass : TaskClasses) {
        const FName ClassName = TaskClass->GetFName();
        PropertyEditorModule.UnregisterCustomClassLayout(ClassName);
    }
}

void FFlowEditorTaskCustomizations::GetTaskClasses(TArray<UClass*>& OutTaskClasses) {
    for (TObjectIterator<UClass> ClassIterator; ClassIterator; ++ClassIterator) {
        if (ClassIterator->IsChildOf(UFlowExecTask::StaticClass()) && !ClassIterator->HasAnyClassFlags(CLASS_Abstract)) {
            OutTaskClasses.Add(*ClassIterator);
        }
    }
}

void FFlowEditorTaskCustomizations::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    UFlowExecTask* Task = FDungeonEditorUtils::GetBuilderObject<UFlowExecTask>(&DetailBuilder);
    if (!Task) return;

    UGridFlowExecEdGraphNode_Task* EdNode = Cast<UGridFlowExecEdGraphNode_Task>(Task->GetOuter());  
    if (!EdNode) return;

    const UGridFlowExecEdGraphSchema* GraphSchema = Cast<UGridFlowExecEdGraphSchema>(EdNode->GetGraph()->GetSchema());
    if (!GraphSchema) return;

    TSet<FName> SchemaAllowedDomain;
    for (IFlowDomainWeakPtr DomainPtr : GraphSchema->GetAllowedDomains()) {
        if (DomainPtr.IsValid()) {
            const IFlowDomainPtr Domain = DomainPtr.Pin();
            SchemaAllowedDomain.Add(Domain->GetDomainID());
        }
    }

    // Hide the domain specific properties that do not belong to the schema allowed domains
    for (TFieldIterator<FProperty> It(Task->GetClass()); It; ++It) {
        FProperty* Property = *It;
        if (Property && Property->HasMetaData(TEXT("AllowedDomains"))) {
            FString AllowedDomainsText = Property->GetMetaData(TEXT("AllowedDomains")).TrimStartAndEnd();
            if (AllowedDomainsText.Len() == 0) continue;
            
            TArray<FString> AllowedDomains = FDAStringUtils::Split(AllowedDomainsText, ",");
            bool bShowDomainProperty = false;
            for (const FString& PropAllowedDomain : AllowedDomains) {
                if (SchemaAllowedDomain.Contains(*PropAllowedDomain)) {
                    bShowDomainProperty = true;
                    break;
                }
            }

            if (!bShowDomainProperty) {
                DetailBuilder.HideProperty(Property->GetFName());
            }
        }
    }

    // Add the task extenders
    for (UFlowExecTaskExtender* Extender : Task->Extenders) {
        if (!Extender) continue;
        FString CategoryName = Extender->GetDetailsPanelCategoryName();
        IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(*CategoryName);
        for (TFieldIterator<FProperty> PropIt(Extender->GetClass()); PropIt; ++PropIt) {
            FProperty* Property = *PropIt;
            if (Property && Property->HasAnyPropertyFlags(CPF_Edit)) {
                Category.AddExternalObjectProperty({ Extender }, *Property->GetName());
            }
        }
    } 
}

TSharedRef<IDetailCustomization> FFlowEditorTaskCustomizations::MakeInstance() {
    return MakeShareable(new FFlowEditorTaskCustomizations);
}

