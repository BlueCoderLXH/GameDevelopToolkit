//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"

class UFlowAssetBase;
class UTextureRenderTarget2D;

class DUNGEONARCHITECTEDITOR_API FFlowEditorUtils {
public:
    static void InitializeFlowAsset(UFlowAssetBase* InAsset);
    static void SaveTextureAsAssetThumbnail(const FAssetData& InAsset, int32 ThumbSize, UTextureRenderTarget2D* Texture);
    
    template <typename T>
    static T* AddExecNode(UGridFlowExecScript* ExecScript) {
        T* Node = NewObject<T>(ExecScript->ScriptGraph);
        ExecScript->ScriptGraph->Nodes.Add(Node);
        return Node;
    }

};

