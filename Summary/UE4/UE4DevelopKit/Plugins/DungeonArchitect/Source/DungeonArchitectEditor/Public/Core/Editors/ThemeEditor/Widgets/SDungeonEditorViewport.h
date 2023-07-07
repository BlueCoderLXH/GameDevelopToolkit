//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "Core/Utils/DungeonEditorViewportProperties.h"
#include "Frameworks/ThemeEngine/DungeonThemeAsset.h"

#include "SEditorViewport.h"

/**
* StaticMesh Editor Preview viewport widget
*/
class DUNGEONARCHITECTEDITOR_API SDungeonEditorViewport : public SEditorViewport, public FGCObject,
                                                          public FDungeonEditorViewportPropertiesListener {
public:
    SLATE_BEGIN_ARGS(SDungeonEditorViewport) {}
        SLATE_ARGUMENT(TWeakPtr<FDungeonArchitectThemeEditor>, DungeonEditor)
        SLATE_ARGUMENT(UDungeonThemeAsset*, ObjectToEdit)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    ~SDungeonEditorViewport();

    // FGCObject interface
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    // End of FGCObject interface

    /** Set the parent tab of the viewport for determining visibility */
    void SetParentTab(TSharedRef<SDockTab> InParentTab) { ParentTab = InParentTab; }

    void RebuildMeshes();

    // SWidget Interface
    virtual void Tick(const FGeometry& AllottedGeometry, double InCurrentTime, float InDeltaTime) override;
    // End of SWidget Interface

    EVisibility GetToolbarVisibility() const;

    virtual void OnPropertyChanged(FString PropertyName, UDungeonEditorViewportProperties* Properties) override;

    /* The nodes that were modified and requires a clean rebuild by the scene provider */
    void SetNodesToRebuild(const TSet<FName>& NodeIds);

    TSharedPtr<class FAdvancedPreviewScene> GetAdvancedPreview() const { return PreviewScene; }

protected:
    /** SEditorViewport interface */
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual EVisibility OnGetViewportContentVisibility() const override;
    virtual void BindCommands() override;
    virtual void OnFocusViewportToSelection() override;
    virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

    void PerformMeshRebuild();

    void OnShowPropertyDungeon();
    void OnShowPropertySkylight();
    void OnShowPropertyDirectionalLight();
    void OnShowPropertyAtmosphericFog();
    void OnToggleDebugData();

    void HandleForceRebuildPreviewLayout();
    bool HandleForceRebuildPreviewLayoutIsChecked() const;
    bool HandleToggleDebugDataIsChecked() const;

    void ShowObjectProperties(UObject* Object, bool bForceRefresh = false);
    void ListenToConfigChanges(UDungeonConfig* Config);

    void CreateDungeonBuilder(TSubclassOf<UDungeonBuilder> BuilderClass);

    void DestroyDungeonActors();
    void CleanupModifiedNodeObjects();

private:
    /** Determines the visibility of the viewport. */
    virtual bool IsVisible() const override;
    UDungeonModel* CreateModelInstance(UObject* Outer);
    UDungeonConfig* CreateConfigInstance(UObject* Outer);
    UDungeonQuery* CreateQueryInstance(UObject* Outer);

    /*
    /////// FDungeonSceneProvider Implementation ///////
    virtual void AddStaticMesh(UDungeonMesh* Mesh, const FTransform& transform, const FName& NodeId) override;
    virtual void AddLight(UPointLightComponent* LightTemplate, const FTransform& transform, const FName& NodeId) override;
    virtual void AddParticleSystem(UParticleSystem* ParticleTemplate, const FTransform& transform, const FName& NodeId) override;
    virtual void AddActorFromTemplate(UClass* ClassTemplate, const FTransform& transform, const FName& NodeId) override;
    ////////////////////////////////////////////////////
    */

private:
    TWeakPtr<FDungeonArchitectThemeEditor> DungeonEditorPtr;
    UDungeonThemeAsset* ObjectToEdit = nullptr;

    /** The parent tab where this viewport resides */
    TWeakPtr<SDockTab> ParentTab;

    /** Level viewport client */
    TSharedPtr<class FDungeonEditorViewportClient> EditorViewportClient;

    /** The scene for this viewport. */
    TSharedPtr<class FAdvancedPreviewScene> PreviewScene;

    UDungeonBuilder* DungeonBuilder = nullptr;
    TSharedPtr<class FDungeonSceneProvider> SceneProvider;

    bool bRequestRebuildPreviewMesh = false;

    bool bDrawDebugData = false;
    bool bForcePreviewRebuildLayout = false;

    USkyLightComponent* Skylight = nullptr;
    USkyAtmosphereComponent* AtmosphericFog = nullptr;

    // The nodes that were modified and requires a clean rebuild by the scene provider
    TSet<FName> NodeObjectsToRebuild;

    UDungeonModel* DungeonModel = nullptr;
    UDungeonQuery* DungeonQuery = nullptr;
};

