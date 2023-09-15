//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Attributes.h"

#include "Textures/SlateIcon.h"

class SWidget;
class IDetailsView;

struct FFlowDomainEditorTabInfo {
    FName TabID;
    FText DisplayName;
    FSlateIcon Icon;
};

typedef TSharedPtr<class FFlowExecNodeState> FFlowExecNodeStatePtr;
typedef TSharedPtr<class IFlowDomain> IFlowDomainPtr;
typedef TWeakPtr<class IFlowDomain> IFlowDomainWeakPtr;

class IFlowDomainEditor {
public:
    virtual ~IFlowDomainEditor() {}
    
    void Initialize(TSharedPtr<IDetailsView> PropertyEditor);
    IFlowDomainPtr GetDomain() const { return Domain; }
    FName GetDomainID() const;

    // IFlowDomainEditor Interface
    virtual bool IsVisualEditor() const { return true; }
    virtual FFlowDomainEditorTabInfo GetTabInfo() const = 0;
    virtual TSharedRef<SWidget> GetContentWidget() = 0;
    virtual void Build(FFlowExecNodeStatePtr State) = 0;
    
    virtual void Tick(float DeltaTime) {}
    virtual void RecenterView(FFlowExecNodeStatePtr State) {}
    virtual bool CanSaveThumbnail() const { return false; }
    virtual void SaveThumbnail(const struct FAssetData& InAsset, int32 ThumbSize) {}
    // End IFlowDomainEditor Interface

private:
    virtual void InitializeImpl(TSharedPtr<IDetailsView> PropertyEditor) = 0;
    virtual IFlowDomainPtr CreateDomain() const = 0;
    
protected:
    IFlowDomainPtr Domain;
};

