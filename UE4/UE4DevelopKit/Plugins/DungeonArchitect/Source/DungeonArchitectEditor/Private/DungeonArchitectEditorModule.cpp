//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "DungeonArchitectEditorModule.h"

#include "Builders/Grid/Customizations/DAGridSpatialConstraintCustomization.h"
#include "Builders/Grid/SpatialConstraints/GridSpatialConstraint2x2.h"
#include "Builders/Grid/SpatialConstraints/GridSpatialConstraint3x3.h"
#include "Builders/Grid/SpatialConstraints/GridSpatialConstraintEdge.h"
#include "Builders/SimpleCity/Customizations/DASimpleCitySpatialConstraintCustomization.h"
#include "Builders/SimpleCity/SpatialConstraints/SimpleCitySpatialConstraint3x3.h"
#include "Core/Common/DungeonArchitectCommands.h"
#include "Core/Common/Utils/DungeonEditorService.h"
#include "Core/Dungeon.h"
#include "Core/Editors/FlowEditor/FlowEditorCommands.h"
#include "Core/Editors/FlowEditor/FlowEditorCustomizations.h"
#include "Core/Editors/FlowEditor/Panels/Viewport/SFlowPreview3DViewportToolbar.h"
#include "Core/Editors/LaunchPad/LaunchPad.h"
#include "Core/Editors/LaunchPad/Styles/LaunchPadStyle.h"
#include "Core/Editors/SnapConnectionEditor/Preview3D/SSnapConnectionPreview3DViewportToolbar.h"
#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditorCommands.h"
#include "Core/Editors/SnapConnectionEditor/SnapConnectionEditorCustomization.h"
#include "Core/Editors/SnapMapEditor/Viewport/SSnapMapEditorViewportToolbar.h"
#include "Core/Editors/ThemeEditor/DungeonArchitectThemeEditor.h"
#include "Core/Editors/ThemeEditor/Widgets/GraphPanelNodeFactory_DungeonProp.h"
#include "Core/Editors/ThemeEditor/Widgets/SDungeonEditorViewportToolbar.h"
#include "Core/LevelEditor/Assets/GridFlow/GridFlowAssetTypeActions.h"
#include "Core/LevelEditor/Assets/PlaceableMarker/PlaceableMarkerAssetBroker.h"
#include "Core/LevelEditor/Assets/PlaceableMarker/PlaceableMarkerAssetTypeActions.h"
#include "Core/LevelEditor/Assets/SnapConnection/SnapConnectionAssetBroker.h"
#include "Core/LevelEditor/Assets/SnapConnection/SnapConnectionAssetTypeActions.h"
#include "Core/LevelEditor/Assets/SnapGridFlow/SnapGridFlowAssetTypeActions.h"
#include "Core/LevelEditor/Assets/SnapMap/ModuleDatabase/SnapMapModuleDBTypeActions.h"
#include "Core/LevelEditor/Assets/SnapMap/SnapMapAssetTypeActions.h"
#include "Core/LevelEditor/Assets/Theme/DungeonThemeAssetTypeActions.h"
#include "Core/LevelEditor/Config/CustomInputMapping.h"
#include "Core/LevelEditor/Config/CustomInputMappingDetails.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectEditorCustomization.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectGraphNodeCustomization.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Core/LevelEditor/EditorMode/DungeonEdMode.h"
#include "Core/LevelEditor/EditorMode/DungeonEdModeHandlerFactory.h"
#include "Core/LevelEditor/Extenders/EditorUIExtender.h"
#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystem.h"
#include "Core/LevelEditor/Placements/DungeonArchitectPlacements.h"
#include "Core/LevelEditor/Visualizers/PlaceableMarkerVisualizer.h"
#include "Core/Utils/Debug/DungeonDebug.h"
#include "Core/Volumes/DungeonVolume.h"
#include "Frameworks/Flow/Domains/AbstractGraph/GridFlowAbstractGraphPanelNodeFactory.h"
#include "Frameworks/Flow/Domains/Tilemap/Graph/TilemapGraphInfrastructure.h"
#include "Frameworks/Flow/ExecGraph/GridFlowExecGraphPanelNodeFactory.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/EdGraphSchema_FlowExec.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/ExecutionGraphConnectionDrawingPolicy.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/GraphPanelNodeFactory_Execution.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/GraphGrammar/RuleGraph/EdGraphSchema_Grammar.h"
#include "Frameworks/GraphGrammar/RuleGraph/GrammarGraphConnectionDrawingPolicy.h"
#include "Frameworks/GraphGrammar/RuleGraph/GraphPanelNodeFactory_Grammar.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionActor.h"
#include "Frameworks/Snap/Lib/Connection/SnapConnectionComponent.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"
#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"
#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

#include "AssetToolsModule.h"
#include "ComponentAssetBroker.h"
#include "ComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "IAssetTools.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule"


class FDungeonArchitectEditorModule : public IDungeonArchitectEditorModule {
public:

    /** IModuleInterface implementation */
    virtual void StartupModule() override {
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FDungeonArchitectEditorModule::OnPostEngineInit);
        
        FDungeonEditorThumbnailPool::Create();
        RegisterCommands();
        InitializeStyles();

        UIExtender.Extend();
        HelpSystem.Initialize();

        // Add a category for the dungeon architect assets
        {
            IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
            DungeonAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Dungeon Architect")), LOCTEXT("DungeonArchitectAssetCategory", "Dungeon Architect"));
        }

        // Register the details customization
        {
            FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		    RegisterCustomClassLayout<ADungeon, FDungeonArchitectEditorCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<ADungeonVolume, FDungeonArchitectVolumeCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<UEdGraphNode_DungeonActorBase, FDungeonArchitectVisualGraphNodeCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<UEdGraphNode_ExecRuleNode, FDAExecRuleNodeCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<UDungeonEditorViewportProperties, FDungeonEditorViewportPropertiesCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<ADungeonDebug, FDungeonDebugCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<ASnapConnectionActor, FSnapConnectionActorCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<USnapMapModuleDatabase, FSnapModuleDatabaseCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<USnapGridFlowModuleDatabase, FSnapGridFlowModuleDatabaseCustomization>(PropertyEditorModule);
		    RegisterCustomClassLayout<ADACustomInputConfigBinder, FDACustomInputBinderCustomization>(PropertyEditorModule);
		    FFlowEditorTaskCustomizations::RegisterTaskNodes(PropertyEditorModule);
		    
            RegisterCustomPropertyTypeLayout<FGridSpatialConstraint3x3Data, FDAGridConstraintCustomization3x3>(PropertyEditorModule);
            RegisterCustomPropertyTypeLayout<FGridSpatialConstraint2x2Data, FDAGridConstraintCustomization2x2>(PropertyEditorModule);
            RegisterCustomPropertyTypeLayout<FGridSpatialConstraintEdgeData, FDAGridConstraintCustomizationEdge>(PropertyEditorModule);
            RegisterCustomPropertyTypeLayout<FSimpleCitySpatialConstraint3x3Data, FDASimpleCityConstraintCustomization3x3>(PropertyEditorModule);

            PropertyEditorModule.NotifyCustomizationModuleChanged();
        }

        // Register the editor mode handlers for the dungeon builders
        FDungeonEdModeHandlerFactory::Register();

        // Register asset types
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FDungeonThemeAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapMapAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FGridFlowFlowAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapConnectionAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapMapModuleDBTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapGridFlowAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapGridFlowModuleBoundsAssetTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FSnapGridFlowModuleDatabaseTypeActions));
        RegisterAssetTypeAction(AssetTools, MakeShareable(new FPlaceableMarkerAssetTypeActions));

        // Register custom graph nodes
        RegisterVisualNodeFactory(MakeShareable(new FGraphPanelNodeFactory_DungeonProp));
        RegisterVisualNodeFactory(MakeShareable(new FGraphPanelNodeFactory_Grammar));
        RegisterVisualNodeFactory(MakeShareable(new FGraphPanelNodeFactory_Execution));
        RegisterVisualNodeFactory(MakeShareable(new FGridFlowExecGraphPanelNodeFactory));
        RegisterVisualNodeFactory(MakeShareable(new FGridFlowAbstractGraphPanelNodeFactory));
        RegisterVisualNodeFactory(MakeShareable(new FGridFlowTilemapGraphPanelNodeFactory));
        //SGraphNode_DungeonPropMesh

        // Register the asset brokers (used for asset to component mapping) 
        RegisterAssetBroker<FSnapConnectionAssetBroker, USnapConnectionComponent>(true, true);
        RegisterAssetBroker<FPlaceableMarkerAssetBroker, UPlaceableMarkerComponent>(true, true);

        // Register the dungeon draw editor mode
        FEditorModeRegistry::Get().RegisterMode<FEdModeDungeon>(
            FEdModeDungeon::EM_Dungeon, NSLOCTEXT("EditorModes", "DungeonDrawMode", "Draw Dungeon"),
            FSlateIcon(FDungeonArchitectStyle::GetStyleSetName(), "DungeonArchitect.TabIcon", "DungeonArchitect.TabIcon.Small"),
            true, 400
        );

        // Hook on to the map change event to bind any missing inputs that the samples require
        InputBinderHook = MakeShareable(new FDACustomInputConfigBinderHook);
        InputBinderHook->AddHook();
        
        // Track dungeon actor property change events to handle advanced dungeon details
        DungeonPropertyChangeListener = MakeShareable(new FDungeonPropertyChangeListener);
        DungeonPropertyChangeListener->Initialize();

        UEdGraphSchema_Grammar::GrammarGraphSupport = new FEditorGrammarGraphSupport();
        UEdGraphSchema_FlowExec::ExecGraphSupport = new FEditorFlowExecGraphSupport();

        // Create and editor service, so the runtime module can access it
        IDungeonEditorService::Set(MakeShareable(new FDungeonEditorService));

        DungeonItemPlacements.Initialize();
        ProtoToolsMeshPlacements.Initialize();
        ProtoToolsMaterialPlacements.Initialize();
        FLaunchPadSystem::Register();
    }
    
    void OnPostEngineInit() {
        RegisterComponentVisualizer<UPlaceableMarkerComponent, FPlaceableMarkerVisualizer>();
    }

    virtual void ShutdownModule() override {
        FCoreDelegates::OnPostEngineInit.RemoveAll(this);
        
        FLaunchPadSystem::Unregister();
        ProtoToolsMeshPlacements.Release();
        DungeonItemPlacements.Release();
        ProtoToolsMaterialPlacements.Release();
        UIExtender.Release();
        HelpSystem.Release();

        // Unregister all the asset types that we registered
        if (FModuleManager::Get().IsModuleLoaded("AssetTools")) {
            IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
            for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index) {
                AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
            }
        }
        CreatedAssetTypeActions.Empty();

        // Unregister all the visual node factories
        for (TSharedPtr<FGraphPanelNodeFactory> VisualNodeFactory : CreatedVisualNodeFactories) {
            FEdGraphUtilities::UnregisterVisualNodeFactory(VisualNodeFactory);
        }
        CreatedVisualNodeFactories.Empty();

        FEditorModeRegistry::Get().UnregisterMode(FEdModeDungeon::EM_Dungeon);

        delete UEdGraphSchema_Grammar::GrammarGraphSupport;
        UEdGraphSchema_Grammar::GrammarGraphSupport = nullptr;

        // Unregister property editor customizations
        if (FModuleManager::Get().IsModuleLoaded("PropertyEditor")) {
            FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
            UnregisterCustomClassLayout(PropertyEditorModule);
            UnregisterCustomPropertyTypeLayout(PropertyEditorModule);
            FFlowEditorTaskCustomizations::UnregisterTaskNodes(PropertyEditorModule);
        }

        if (InputBinderHook.IsValid()) {
            InputBinderHook->RemoveHook();
            InputBinderHook = nullptr;
        }
        
        UnregisterAssetBrokers();
        UnregisterComponentVisualizers();
        UnregisterCommands();
        ShutdownStyles();
    }

    virtual EAssetTypeCategories::Type GetDungeonAssetCategoryBit() const override {
        return DungeonAssetCategoryBit;
    }

public:
    TSharedPtr<FDungeonPropertyChangeListener> DungeonPropertyChangeListener;

private:
    static void RegisterCommands() {
        FDungeonArchitectCommands::Register();
        FDungeonEditorViewportCommands::Register();
        FSnapMapEditorViewportCommands::Register();
        FSnapConnectionEditorCommands::Register();
        FSnapConnectionEditorViewportCommands::Register();
        FFlowEditorCommands::Register();
        FGridFlowEditorViewportCommands::Register();
        FDALevelToolbarCommands::Register();
    }

    static void UnregisterCommands() {
        FDungeonArchitectCommands::Unregister();
        FDungeonEditorViewportCommands::Unregister();
        FSnapMapEditorViewportCommands::Unregister();
        FSnapConnectionEditorCommands::Unregister();
        FSnapConnectionEditorViewportCommands::Unregister();
        FFlowEditorCommands::Unregister();
        FGridFlowEditorViewportCommands::Unregister();
        FDALevelToolbarCommands::Unregister();
    }

    static void InitializeStyles() {
        FDungeonArchitectStyle::Initialize();
        FDALaunchPadStyle::Initialize();
    }

    static void ShutdownStyles() {
        FDungeonArchitectStyle::Shutdown();
        FDALaunchPadStyle::Shutdown();
    }

    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action) {
        AssetTools.RegisterAssetTypeActions(Action);
        CreatedAssetTypeActions.Add(Action);
    }

    void RegisterVisualNodeFactory(TSharedRef<FGraphPanelNodeFactory> VisualNodeFactory) {
        FEdGraphUtilities::RegisterVisualNodeFactory(VisualNodeFactory);
        CreatedVisualNodeFactories.Add(VisualNodeFactory);
    }

    template<typename TComponent, typename TVisualizer>
    void RegisterComponentVisualizer() {
        RegisterComponentVisualizer(TComponent::StaticClass()->GetFName(),MakeShareable(new TVisualizer));
    }
    
    void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer) {
        if (GUnrealEd) {
            GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
        }

        RegisteredComponentClassNames.Add(ComponentClassName);

        if (Visualizer.IsValid()) {
            Visualizer->OnRegister();
        }
    }

    void UnregisterComponentVisualizers() {
        if(GUnrealEd) {
            for(const FName& ClassName : RegisteredComponentClassNames) {
                GUnrealEd->UnregisterComponentVisualizer(ClassName);
            }
            RegisteredComponentClassNames.Reset();
        }
    }

    template<typename TClass, typename TCustomization>
    void RegisterCustomClassLayout(FPropertyEditorModule& PropertyEditorModule) {
        const FName ClassName = TClass::StaticClass()->GetFName();
        PropertyEditorModule.RegisterCustomClassLayout(ClassName, FOnGetDetailCustomizationInstance::CreateStatic(&TCustomization::MakeInstance));
        CustomLayoutClassNames.Add(ClassName);
    }

    void UnregisterCustomClassLayout(FPropertyEditorModule& PropertyEditorModule) {
        for (const FName& ClassName : CustomLayoutClassNames) {
            PropertyEditorModule.UnregisterCustomClassLayout(ClassName);
        }
        CustomLayoutClassNames.Reset();
    }

    template<typename TClass, typename TCustomization>
    void RegisterCustomPropertyTypeLayout(FPropertyEditorModule& PropertyEditorModule) {
        const FName ClassName = TClass::StaticStruct()->GetFName();
        PropertyEditorModule.RegisterCustomPropertyTypeLayout(ClassName, FOnGetPropertyTypeCustomizationInstance::CreateStatic(&TCustomization::MakeInstance));
        CustomPropertyTypeLayoutNames.Add(ClassName);
    }

    void UnregisterCustomPropertyTypeLayout(FPropertyEditorModule& PropertyEditorModule) {
        for (const FName& ClassName : CustomPropertyTypeLayoutNames) {
            PropertyEditorModule.UnregisterCustomPropertyTypeLayout(ClassName);
        }
        CustomPropertyTypeLayoutNames.Reset();
    }
    
    template<typename TBroker, typename TComponent>
    void RegisterAssetBroker(bool bSetAsPrimary, bool bMapComponentForAssets) {
        TSharedPtr<IComponentAssetBroker> AssetBroker = MakeShareable(new TBroker);
        FComponentAssetBrokerage::RegisterBroker(AssetBroker, TComponent::StaticClass(), true, true);
        AssetBrokers.Add(AssetBroker);
    }

    void UnregisterAssetBrokers() {
        if (UObjectInitialized()) {
            // Unregister the component brokers
            for (TSharedPtr<IComponentAssetBroker> AssetBroker : AssetBrokers) {
                if (AssetBroker.IsValid()) {
                    FComponentAssetBrokerage::UnregisterBroker(AssetBroker);
                }
            }
        }
    }
    

private:
    /** All created asset type actions.  Cached here so that we can unregister them during shutdown. */
    TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
    TArray<TSharedPtr<FGraphPanelNodeFactory>> CreatedVisualNodeFactories;
    TArray<TSharedPtr<IComponentAssetBroker>> AssetBrokers;
	TArray<FName> RegisteredComponentClassNames;
    TArray<FName> CustomLayoutClassNames;
    TArray<FName> CustomPropertyTypeLayoutNames;

    EAssetTypeCategories::Type DungeonAssetCategoryBit = EAssetTypeCategories::None;

    FDungeonItemsPlacements DungeonItemPlacements;
    FDAProtoToolsMeshPlacements ProtoToolsMeshPlacements;
    FDAProtoToolsMaterialPlacements ProtoToolsMaterialPlacements;
    FEditorUIExtender UIExtender;
    FDungeonArchitectHelpSystem HelpSystem;
    TSharedPtr<FDACustomInputConfigBinderHook> InputBinderHook;
};

IMPLEMENT_MODULE(FDungeonArchitectEditorModule, DungeonArchitectEditor)


#undef LOCTEXT_NAMESPACE

