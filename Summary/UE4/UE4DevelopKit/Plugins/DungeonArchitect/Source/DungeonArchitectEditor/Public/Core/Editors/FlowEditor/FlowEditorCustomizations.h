//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FFlowEditorTaskCustomizations : public IDetailCustomization {
public:
    static void RegisterTaskNodes(class FPropertyEditorModule& PropertyEditorModule);
    static void UnregisterTaskNodes(class FPropertyEditorModule& PropertyEditorModule);
    
    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    // End of IDetailCustomization interface

    static TSharedRef<IDetailCustomization> MakeInstance();

private:
    static void GetTaskClasses(TArray<UClass*>& OutTaskClasses);
};

