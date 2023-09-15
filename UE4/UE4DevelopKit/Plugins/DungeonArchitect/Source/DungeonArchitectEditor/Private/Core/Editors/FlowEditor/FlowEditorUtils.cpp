//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/FlowEditorUtils.h"

#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecEdGraph.h"
#include "Frameworks/Flow/FlowAsset.h"

#include "AssetToolsModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "ObjectTools.h"

void FFlowEditorUtils::InitializeFlowAsset(UFlowAssetBase* InAsset) {
    InAsset->ExecEdGraph = NewObject<UGridFlowExecEdGraph>(InAsset, UGridFlowExecEdGraph::StaticClass(), NAME_None,
                                                           RF_Transactional);
    InAsset->ExecScript = NewObject<UGridFlowExecScript>(InAsset, "ExecScript");
    FGridFlowExecScriptCompiler::Compile(InAsset->ExecEdGraph, InAsset->ExecScript);
}

void FFlowEditorUtils::SaveTextureAsAssetThumbnail(const FAssetData& InAsset, int32 ThumbSize, UTextureRenderTarget2D* RTT) {
    TArray<FColor> ThumbBitmap;
    {
        if (!RTT) return;

        const int32 TexWidth = RTT->GetSurfaceWidth();
        const int32 TexHeight = RTT->GetSurfaceHeight();

        TArray<FColor> SrcBitmap;

        FTextureRenderTargetResource* RTTResource = RTT->GameThread_GetRenderTargetResource();

        ENQUEUE_RENDER_COMMAND(UpdateThumbnailRTCommand)(
            [RTTResource](FRHICommandListImmediate& RHICmdList) {
                // Copy (resolve) the rendered thumbnail from the render target to its texture
                RHICmdList.CopyToResolveTarget(
                    RTTResource->GetRenderTargetTexture(), // Source texture
                    RTTResource->TextureRHI, // Dest texture
                    FResolveParams()); // Resolve parameters
            });

        FRenderCommandFence RenderFence;
        RenderFence.BeginFence();
        RenderFence.Wait();

        RTTResource->ReadPixels(SrcBitmap);
        check(SrcBitmap.Num() == TexWidth * TexHeight);
        
        FImageUtils::CropAndScaleImage(TexWidth, TexHeight, ThumbSize, ThumbSize, SrcBitmap, ThumbBitmap);
    }


    //setup actual thumbnail
    FObjectThumbnail TempThumbnail;
    TempThumbnail.SetImageSize(ThumbSize, ThumbSize);
    TArray<uint8>& ThumbnailByteArray = TempThumbnail.AccessImageData();

    // Copy scaled image into destination thumb
    const int32 MemorySize = ThumbSize * ThumbSize * sizeof(FColor);
    ThumbnailByteArray.AddUninitialized(MemorySize);
    FMemory::Memcpy(&(ThumbnailByteArray[0]), &(ThumbBitmap[0]), MemorySize);

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    //check if each asset should receive the new thumb nail
    {
        //assign the thumbnail and dirty
        const FString ObjectFullName = InAsset.GetFullName();
        const FString PackageName = InAsset.PackageName.ToString();

        UPackage* AssetPackage = FindObject<UPackage>(nullptr, *PackageName);
        if (ensure(AssetPackage)) {
            FObjectThumbnail* NewThumbnail = ThumbnailTools::CacheThumbnail(
                ObjectFullName, &TempThumbnail, AssetPackage);
            if (ensure(NewThumbnail)) {
                //we need to indicate that the package needs to be re-saved
                AssetPackage->MarkPackageDirty();

                // Let the content browser know that we've changed the thumbnail
                NewThumbnail->MarkAsDirty();

                // Signal that the asset was changed if it is loaded so thumbnail pools will update
                if (InAsset.IsAssetLoaded()) {
                    InAsset.GetAsset()->PostEditChange();
                }

                //Set that thumbnail as a valid custom thumbnail so it'll be saved out
                NewThumbnail->SetCreatedAfterCustomThumbsEnabled();
            }
        }
    }
}

