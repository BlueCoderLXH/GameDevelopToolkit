//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/SnapGridFlow/SnapGridFlowEditorUtils.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleBounds.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"
#include "Frameworks/Snap/SnapModuleDBBuilder.h"

namespace {
    class FMDBGridFlowModulePolicy {
    public:
        FBox CalculateBounds(ULevel* Level) const {
            FBox LevelBounds = FBox(ForceInit);
            for (AActor* Actor : Level->Actors) {
                if (ASnapGridFlowModuleBoundsActor* BoundsActor = Cast<ASnapGridFlowModuleBoundsActor>(Actor)) {
                    USnapGridFlowModuleBoundsAsset* BoundsAsset = BoundsActor->BoundsComponent->ModuleBounds.LoadSynchronous();
                    const FVector ModuleSize = BoundsAsset->ChunkSize * FMathUtils::ToVector(BoundsActor->BoundsComponent->NumChunks);;
                    const FBox BaseBounds = FBox(FVector::ZeroVector, ModuleSize);
                    const FTransform BaseTransform(FRotator::ZeroRotator, BoundsActor->GetActorLocation());
                    LevelBounds = BaseBounds.TransformBy(BaseTransform);
                    break;
                }
            }
            if (!LevelBounds.IsValid) {
                // TODO: Notify the user
                LevelBounds = FBox(ForceInitToZero);
            }
            return LevelBounds;
        }
        
        void Initialize(FSnapGridFlowModuleDatabaseItem& ModuleItem, const ULevel* Level, const UObject* InModuleDB) {
            ASnapGridFlowModuleBoundsActor* BoundsActor = nullptr;
            for (AActor* Actor : Level->Actors) {
                if (ASnapGridFlowModuleBoundsActor* ModuleBoundsActor = Cast<ASnapGridFlowModuleBoundsActor>(Actor)) {
                    BoundsActor = ModuleBoundsActor;
                    break;
                }
            }
            
            ModuleItem.NumChunks = BoundsActor ? BoundsActor->BoundsComponent->NumChunks : FIntVector(1, 1, 1);

            ChunkSize = FVector::ZeroVector;
            if (const USnapGridFlowModuleDatabase* SGFModuleDB = Cast<USnapGridFlowModuleDatabase>(InModuleDB)) {
                if (SGFModuleDB->ModuleBoundsAsset) {
                    ChunkSize = SGFModuleDB->ModuleBoundsAsset->ChunkSize;
                }
            }
        }
        
        void PostProcess(FSnapGridFlowModuleDatabaseItem& ModuleItem, const ULevel* Level) const {
            if (ChunkSize == FVector::ZeroVector) {
                // invalid state. This is probably because the user didn't specify the module bounds asset in the module db editor
                return;
            }
            
            if (ModuleItem.bAllowRotation) {
                ModuleItem.RotatedAssemblies.SetNum(4);
                FSGFModuleAssemblyBuilder::Build(ChunkSize, ModuleItem, ModuleItem.RotatedAssemblies[0]);
                for (int i = 1; i < 4; i++) {
                    FSGFModuleAssemblyBuilder::Rotate90CW(ModuleItem.RotatedAssemblies[i - 1], ModuleItem.RotatedAssemblies[i]);
                }
            }
            else {
                ModuleItem.RotatedAssemblies.SetNum(1);
                FSGFModuleAssemblyBuilder::Build(ChunkSize, ModuleItem, ModuleItem.RotatedAssemblies[0]);
            }

            // Build the available marker list
            ModuleItem.AvailableMarkers.Reset();
            for (AActor* const Actor : Level->Actors) {
                if (APlaceableMarkerActor* MarkerActor = Cast<APlaceableMarkerActor>(Actor)) {
                    if (MarkerActor->PlaceableMarkerComponent && MarkerActor->PlaceableMarkerComponent->MarkerAsset) {
                        UPlaceableMarkerAsset* MarkerAsset = MarkerActor->PlaceableMarkerComponent->MarkerAsset;
                        int32& Count = ModuleItem.AvailableMarkers.FindOrAdd(MarkerAsset);
                        Count++;
                    }
                }
            }
        }

    private:
        FVector ChunkSize;
    };

}


void FSnapGridFlowEditorUtils::BuildModuleDatabaseCache(USnapGridFlowModuleDatabase* InDatabase) {
    if (!InDatabase) return;

    typedef TSnapModuleDatabaseBuilder<
            FSnapGridFlowModuleDatabaseItem,
            FSnapGridFlowModuleDatabaseConnectionInfo,
            FMDBGridFlowModulePolicy,
            SnapModuleDatabaseBuilder::TDefaultConnectionPolicy<FSnapGridFlowModuleDatabaseConnectionInfo>
    > FSnapGridFlowDatabaseBuilder;
    
    FSnapGridFlowDatabaseBuilder::Build(InDatabase->Modules, InDatabase);

    InDatabase->Modify();
}

