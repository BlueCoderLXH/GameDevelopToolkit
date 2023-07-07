//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Customizations/DungeonArchitectEditorCustomization.h"

#include "Builders/SnapGridFlow/SnapGridFlowEditorUtils.h"
#include "Builders/SnapMap/SnapMapDungeonBuilder.h"
#include "Builders/SnapMap/Utils/SnapMapModuleDBUtils.h"
#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/Dungeon.h"
#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonMesh.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Core/LevelEditor/HelpSystem/DungeonArchitectHelpSystem.h"
#include "Core/Utils/Debug/DungeonDebug.h"
#include "Core/Utils/DungeonEditorViewportProperties.h"
#include "Core/Volumes/DungeonVolume.h"
#include "Frameworks/GraphGrammar/ExecutionGraph/Nodes/EdGraphNode_ExecRuleNode.h"
#include "Frameworks/LevelStreaming/DungeonLevelStreamer.h"
#include "Frameworks/Snap/SnapGridFlow/SnapGridFlowModuleDatabase.h"
#include "Frameworks/Snap/SnapMap/SnapMapModuleDatabase.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetSelection.h"
#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Dialogs/DlgPickPath.h"
#include "EditorActorFolders.h"
#include "IContentBrowserSingleton.h"
#include "IDocumentation.h"
#include "Interfaces/IMainFrameModule.h"
#include "ObjectEditorUtils.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SSpacer.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule"

DEFINE_LOG_CATEGORY(LogDungeonCustomization)

ADungeon* GetDungeon(IDetailLayoutBuilder* DetailBuilder) {
    return FDungeonEditorUtils::GetBuilderObject<ADungeon>(DetailBuilder);
}

void ShowDungeonConfigProperties(IDetailLayoutBuilder& DetailBuilder, UDungeonConfig* Config) {
    if (!Config) return;

    TArray<FName> ImportantAttributes;
    ImportantAttributes.Add("Seed");

    TArray<UObject*> Configs;
    Configs.Add(Config);
    const FString ConfigName = Config->GetName();
    IDetailCategoryBuilder& ConfigCategory = DetailBuilder.EditCategory(*ConfigName);

    // First add the important attributes, so they show up on top of the config properties list
    for (const FName& Attrib : ImportantAttributes) {
        ConfigCategory.AddExternalObjectProperty(Configs, Attrib);
    }

    TArray<FProperty*> ExperimentalProperties;

    for (TFieldIterator<FProperty> PropIt(Config->GetClass()); PropIt; ++PropIt) {
        FProperty* Property = *PropIt;

        if (Property->HasMetaData("Experimental")) {
            ExperimentalProperties.Add(Property);
            continue;
        }

        FName PropertyName(*(Property->GetName()));

        // Make sure we skip the important attrib, since we have already added them above
        if (ImportantAttributes.Contains(PropertyName)) {
            // Already added
            continue;
        }

        ConfigCategory.AddExternalObjectProperty(Configs, PropertyName);
    }

    IDetailCategoryBuilder& ExperimentalCategory = DetailBuilder.EditCategory("Experimental");
    for (FProperty* Property : ExperimentalProperties) {
        const FName PropertyName(*(Property->GetName()));
        ExperimentalCategory.AddExternalObjectProperty(Configs, PropertyName);
    }
}


void ShowLevelStreamingProperties(IDetailLayoutBuilder& DetailBuilder, FDungeonLevelStreamingConfig* StreamingConfig) {
    const FString ConfigName = "Level Streaming";
    IDetailCategoryBuilder& ConfigCategory = DetailBuilder.EditCategory(*ConfigName);

    const TSharedRef<FStructOnScope> ConfigStructRef = MakeShared<FStructOnScope>(
        FDungeonLevelStreamingConfig::StaticStruct(), reinterpret_cast<uint8*>(StreamingConfig));
    ConfigCategory.AddAllExternalStructureProperties(ConfigStructRef);
}

void FDungeonArchitectEditorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");

    Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterBuildDungeon", "build dungeon"))
            .ValueContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_BuildDungeon", "Build Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::RebuildDungeon, &DetailBuilder))
    ];

    Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterDestroyDungeon", "destroy dungeon"))
            .ValueContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_DestroyDungeon", "Destroy Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::DestroyDungeon, &DetailBuilder))
    ];

    Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterRandomizeSeed", "randomize seed"))
            .ValueContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_RandomizeSeed", "Randomize Seed"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::RandomizeSeed, &DetailBuilder))
    ];

    Category.AddCustomRow(LOCTEXT("DungeonCommand_Spacer1", "Spacer 1"))
            .ValueContent()
    [
        SNew(SSpacer)
        .Size(FVector2D(10, 10))
    ];

    Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterOpenHelpSystem", "Help / Support"))
            .ValueContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_OpenHelpSystem", "Help / Support"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectEditorCustomization::OpenHelpSystem, &DetailBuilder))
    ];

    Category.AddCustomRow(LOCTEXT("DungeonCommand_Spacer2", "Spacer 2"))
            .ValueContent()
    [
        SNew(SSpacer)
        .Size(FVector2D(10, 10))
    ];

    ADungeon* Dungeon = GetDungeon(&DetailBuilder);

    ////////////// Builder specific customization ///////////////
    // Show snap configuration customization
    if (Dungeon && Dungeon->GetBuilder()) {
        USnapMapDungeonBuilder* SnapMapBuilder = Cast<USnapMapDungeonBuilder>(Dungeon->GetBuilder());
        if (SnapMapBuilder) {

        }
    }


    ////////////// Config Category //////////////
    if (Dungeon) {
        UDungeonConfig* ConfigObject = Dungeon->GetConfig();
        if (ConfigObject) {
            ShowDungeonConfigProperties(DetailBuilder, ConfigObject);
        }
    }

    ////////////// Level Streaming Category //////////////
    if (Dungeon) {
        UDungeonBuilder* Builder = Dungeon->GetBuilder();
        if (Builder && Builder->SupportsLevelStreaming()) {
            ShowLevelStreamingProperties(DetailBuilder, &Dungeon->LevelStreamingConfig);
        }
    }

    // Hide unsupported properties
    if (Dungeon) {
        UDungeonBuilder* Builder = Dungeon->GetBuilder();
        if (Builder) {
            TArray<FString> PropertyNames;
            PropertyNames.Add("Themes");
            PropertyNames.Add("MarkerEmitters");
            PropertyNames.Add("EventListeners");
            for (const FString& PropertyName : PropertyNames) {
                TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(*PropertyName);
                if (!Builder->SupportsProperty(*PropertyName)) {
                    PropertyHandle->MarkHiddenByCustomization();
                }
            }
        }
    }

    // Show a experimental builder warning message
    if (Dungeon) {
        bool bIsExperimental = false;
        bool bIsEarlyAccess = false;
        UDungeonBuilder* Builder = Dungeon->GetBuilder();

        if (Builder) {
            FString MostDerivedClassName;
            FObjectEditorUtils::GetClassDevelopmentStatus(Builder->GetClass(), bIsExperimental, bIsEarlyAccess,
                                                          MostDerivedClassName);
        }

        const FString BuilderClassName = Builder ? Builder->GetClass()->GetName() : "Builder";

        if (bIsExperimental || bIsEarlyAccess) {

            const FName CategoryName(TEXT("Dungeon"));
            const FText CategoryDisplayName = LOCTEXT("WarningCategoryDisplayName", "Dungeon");
            const FText WarningText = bIsExperimental
                                          ? FText::Format(
                                              LOCTEXT("ExperimentalBuilderWarning",
                                                      "Uses experimental builder: {0}.  This builder is still a work in progress"),
                                              FText::FromString(BuilderClassName))
                                          : FText::Format(
                                              LOCTEXT("EarlyAccessClassWarning",
                                                      "Uses early access builder: {0}.  This builder is still a work in progress"),
                                              FText::FromString(*BuilderClassName));
            const FText SearchString = WarningText;
            const FText Tooltip = FText::Format(
                LOCTEXT("ExperimentalClassTooltip", "{0} is still a work in progress and is not fully supported"),
                FText::FromString(*BuilderClassName));
            const FString ExcerptName = bIsExperimental
                                            ? TEXT("ActorUsesExperimentalClass")
                                            : TEXT("ActorUsesEarlyAccessClass");
            const FSlateBrush* WarningIcon = FEditorStyle::GetBrush(
                bIsExperimental ? "PropertyEditor.ExperimentalClass" : "PropertyEditor.EarlyAccessClass");

            IDetailCategoryBuilder& WarningCategory = DetailBuilder.EditCategory(CategoryName, CategoryDisplayName, ECategoryPriority::Default);
            const FColor BackgroundColor = bIsExperimental ? FColor(255, 90, 0) : FColor(90, 90, 0);

            FDetailWidgetRow& WarningRow = WarningCategory
            .AddCustomRow(SearchString)
            .WholeRowContent()
            [
                SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("SettingsEditor.CheckoutWarningBorder"))
					.BorderBackgroundColor(BackgroundColor)
                [
                    SNew(SHorizontalBox)
					.ToolTip(IDocumentation::Get()->CreateToolTip(Tooltip, nullptr, TEXT("Shared/LevelEditor"), ExcerptName))
					.Visibility(EVisibility::Visible)

                    + SHorizontalBox::Slot()
                      .VAlign(VAlign_Center)
                      .AutoWidth()
                      .Padding(4.0f, 0.0f, 0.0f, 0.0f)
                    [
                        SNew(SImage)
                        .Image(WarningIcon)
                    ]

                    + SHorizontalBox::Slot()
                      .VAlign(VAlign_Center)
                      .AutoWidth()
                      .Padding(4.0f, 0.0f, 0.0f, 0.0f)
                    [
                        SNew(STextBlock)
					.Text(WarningText)
				.Font(IDetailLayoutBuilder::GetDetailFont())
                    ]
                ]
            ];
        }
    }
}


TSharedRef<IDetailCustomization> FDungeonArchitectEditorCustomization::MakeInstance() {
    return MakeShareable(new FDungeonArchitectEditorCustomization);
}

FReply FDungeonArchitectEditorCustomization::RebuildDungeon(IDetailLayoutBuilder* DetailBuilder) {
    ADungeon* Dungeon = GetDungeon(DetailBuilder);

    if (!Dungeon) {
        // invalid state
        return FReply::Handled();
    }

    UDungeonBuilder* Builder = Dungeon->GetBuilder();
    if (!Builder) {
        FDungeonEditorUtils::ShowNotification(NSLOCTEXT("DungeonMissingBuilder", "BuilderNotAssigned", "Dungeon Builder not assigned"));
        return FReply::Handled();
    }

    // Check if we can build the dungeon
    {
        FString ErrorMessage;
        if (!Builder->CanBuildDungeon(ErrorMessage)) {
            FDungeonEditorUtils::ShowNotification(FText::FromString(ErrorMessage));
            return FReply::Handled();
        }
    }


    bool bContainValidThemes = false;
    if (Builder->SupportsTheming()) {
        // make sure we have at least one theme defined
        for (UDungeonThemeAsset* Theme : Dungeon->Themes) {
            if (Theme) {
                bContainValidThemes = true;
                break;
            }
        }
    }
    else {
        // themes are not supported. We don't require a theme to be defined
        bContainValidThemes = true;
    }

    if (!bContainValidThemes) {
        // Notify user that a theme has not been assigned
        FDungeonEditorUtils::ShowNotification(NSLOCTEXT("DungeonMissingTheme", "ThemeNotAssigned", "Dungeon Theme Not Assigned"));
        return FReply::Handled();
    }

    FDungeonEditorUtils::BuildDungeon(Dungeon);

    return FReply::Handled();
}

FReply FDungeonArchitectEditorCustomization::DestroyDungeon(IDetailLayoutBuilder* DetailBuilder) {
    ADungeon* Dungeon = GetDungeon(DetailBuilder);
    if (Dungeon) {
        FDungeonEditorUtils::SwitchToRealtimeMode();
        Dungeon->DestroyDungeon();
    }

    return FReply::Handled();
}


FReply FDungeonArchitectEditorCustomization::RandomizeSeed(IDetailLayoutBuilder* DetailBuilder) {
    ADungeon* Dungeon = GetDungeon(DetailBuilder);
    if (Dungeon) {
        Dungeon->GetConfig()->Seed = FMath::Rand();
    }

    return FReply::Handled();
}

FReply FDungeonArchitectEditorCustomization::OpenHelpSystem(IDetailLayoutBuilder* DetailBuilder) {
    FDungeonArchitectHelpSystem::LaunchHelpSystemTab();
    return FReply::Handled();
}

void FDungeonArchitectMeshNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");

    Category.AddCustomRow(LOCTEXT("DungeonCommand_FilterBuildDungeon", "build dungeon"))
    .WholeRowContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonCommand_ShowMeshNodeProperties", "Edit Advanced Properties"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectMeshNodeCustomization::EditAdvancedOptions, &DetailBuilder))
    ];
}

FReply FDungeonArchitectMeshNodeCustomization::EditAdvancedOptions(IDetailLayoutBuilder* DetailBuilder) {
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

    if (ObjectsBeingCustomized.Num() > 1) {
        FDungeonEditorUtils::ShowNotification(NSLOCTEXT("TooManyObjects", "TooManyObjects", "Only a single node can be edited at a time"));
        return FReply::Handled();
    }

    UObject* ObjectToEdit = nullptr;
    if (ObjectsBeingCustomized.Num() == 0) {
        ObjectToEdit = ObjectsBeingCustomized[0].Get();
    }

    if (!ObjectToEdit) {
        FDungeonEditorUtils::ShowNotification(NSLOCTEXT("ObjectNotValid", "ObjectNotValid", "Node state is not valid"));
        return FReply::Handled();
    }

    if (UEdGraphNode_DungeonMesh* MeshNode = Cast<UEdGraphNode_DungeonMesh>(ObjectToEdit)) {
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        TMap<UObject*, UObject*> OldToNewObjectMap;
    }

    return FReply::Handled();
}

void FDungeonEditorViewportPropertiesCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    UDungeonEditorViewportProperties* ViewportProperties = FDungeonEditorUtils::GetBuilderObject<UDungeonEditorViewportProperties>(&DetailBuilder);
    if (ViewportProperties) {
        ShowDungeonConfigProperties(DetailBuilder, ViewportProperties->DungeonConfig);
    }
}

TSharedRef<IDetailCustomization> FDungeonEditorViewportPropertiesCustomization::MakeInstance() {
    return MakeShareable(new FDungeonEditorViewportPropertiesCustomization);
}

/////////////////////////////// Volume Customization //////////////////////////
void FDungeonArchitectVolumeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Dungeon");
    Category.AddCustomRow(LOCTEXT("DungeonVolumeCommand_FilterBuildDungeon", "build dungeon"))
    .ValueContent()
    [
        SNew(SButton)
			.Text(LOCTEXT("DungeonVolumeCommand_BuildDungeon", "Rebuild Dungeon"))
			.OnClicked(FOnClicked::CreateStatic(&FDungeonArchitectVolumeCustomization::RebuildDungeon, &DetailBuilder))
    ];
}


TSharedRef<IDetailCustomization> FDungeonArchitectVolumeCustomization::MakeInstance() {
    return MakeShareable(new FDungeonArchitectVolumeCustomization);
}

FReply FDungeonArchitectVolumeCustomization::RebuildDungeon(IDetailLayoutBuilder* DetailBuilder) {
    ADungeonVolume* Volume = FDungeonEditorUtils::GetBuilderObject<ADungeonVolume>(DetailBuilder);
    if (Volume && Volume->Dungeon) {
        FDungeonEditorUtils::BuildDungeon(Volume->Dungeon);
    }
    return FReply::Handled();
}

void FDungeonPropertyChangeListener::Initialize() {
    FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(SharedThis(this), &FDungeonPropertyChangeListener::OnPropertyChanged);
}

void FDungeonPropertyChangeListener::OnPropertyChanged(UObject* Object, struct FPropertyChangedEvent& Event) {
    if (Object->IsA(ADungeon::StaticClass())) {
        const FName PropertyName = (Event.Property != nullptr) ? Event.Property->GetFName() : NAME_None;
        if (PropertyName == GET_MEMBER_NAME_CHECKED(ADungeon, BuilderClass)) {
            // Refresh the properties window
            UE_LOG(LogDungeonCustomization, Log, TEXT("Dungeon builder class changed"));
            FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
            TArray<UObject*> ObjectList;
            ObjectList.Add(Object);
            PropertyEditorModule.UpdatePropertyViews(ObjectList);
        }
    }
}

//////////////// FDungeonDebugCustomization ////////////////////
void FDungeonDebugCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Debug Commands");
    for (int i = 0; i < 10; i++) {
        FText Caption = FText::FromString("Command " + FString::FromInt(i));

        Category.AddCustomRow(Caption)
        .ValueContent()
        [
            SNew(SButton)
				.Text(Caption)
				.OnClicked(FOnClicked::CreateStatic(&FDungeonDebugCustomization::ExecuteCommand, &DetailBuilder, i))
        ];
    }
}

TSharedRef<IDetailCustomization> FDungeonDebugCustomization::MakeInstance() {
    return MakeShareable(new FDungeonDebugCustomization);
}

FReply FDungeonDebugCustomization::ExecuteCommand(IDetailLayoutBuilder* DetailBuilder, int32 CommandID) {
    ADungeonDebug* Debug = FDungeonEditorUtils::GetBuilderObject<ADungeonDebug>(DetailBuilder);
    if (Debug) {
        Debug->ExecuteDebugCommand(CommandID);
    }

    if (CommandID == 8) {
        IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
        TArray<FString> SelectedPaths;
        ContentBrowserSingleton.GetSelectedPathViewFolders(SelectedPaths);
        for (const FString& SelectedPath : SelectedPaths) {
            UE_LOG(LogDungeonCustomization, Log, TEXT("Selected Path: %s"), *SelectedPath);
        }
    }

    return FReply::Handled();
}

//////////////// FSnapModuleDatabaseCustomization ////////////////////
void FSnapModuleDatabaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& SnapMapCategory = DetailBuilder.EditCategory("Module Cache");

    SnapMapCategory.AddCustomRow(LOCTEXT("SnapMapCommand_FilterRebuildModuleCache", "Build Module Cache"))
    .WholeRowContent()
    [
        SNew(SBorder)
			.BorderImage(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.RoundDarkBorder"))
			.BorderBackgroundColor(FColor(32, 32, 32))
			.Padding(FMargin(10, 10))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("SnapMap_RebuildCacheHelpText",
                                  "Rebuild the cache whenever you change the module list below or if the module level files have been changed in any way.\nThis needs to be done in the editor before building the dungeon at runtime to speed things up\n"))
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
					.Text(LOCTEXT("SnapMapCommand_RebuildModuleCache", "Build Module Cache"))
					.OnClicked(FOnClicked::CreateStatic(&FSnapModuleDatabaseCustomization::BuildDatabaseCache,
                                                        &DetailBuilder))
            ]
        ]
    ];
}

TSharedRef<IDetailCustomization> FSnapModuleDatabaseCustomization::MakeInstance() {
    return MakeShareable(new FSnapModuleDatabaseCustomization);
}

FReply FSnapModuleDatabaseCustomization::BuildDatabaseCache(IDetailLayoutBuilder* DetailBuilder) {
    USnapMapModuleDatabase* ModuleDatabase = FDungeonEditorUtils::GetBuilderObject<USnapMapModuleDatabase>(DetailBuilder);
    if (ModuleDatabase) {
        FSnapMapModuleDBUtils::BuildModuleDatabaseCache(ModuleDatabase);
    }

    return FReply::Handled();
}

//////////////// FSnapGridFlowModuleDatabaseCustomization ////////////////////
void FSnapGridFlowModuleDatabaseCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    IDetailCategoryBuilder& ModuleCacheCategory = DetailBuilder.EditCategory("Module Cache", FText::GetEmpty(), ECategoryPriority::Variable);

    ModuleCacheCategory.AddCustomRow(LOCTEXT("SnapGridFlowCommand_FilterRebuildModuleCache", "Build Module Cache"))
    .WholeRowContent()
    [
        SNew(SBorder)
            .BorderImage(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.RoundDarkBorder"))
            .BorderBackgroundColor(FColor(32, 32, 32))
            .Padding(FMargin(10, 10))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(LOCTEXT("SnapGridFlow_RebuildCacheHelpText",
                                  "Rebuild the cache whenever you change the module list below or if the module level files have been changed in any way.\nThis needs to be done in the editor before building the dungeon at runtime to speed things up\n"))
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
                    .Text(LOCTEXT("SnapGridFlowCommand_RebuildModuleCache", "Build Module Cache"))
                    .OnClicked(FOnClicked::CreateStatic(&FSnapGridFlowModuleDatabaseCustomization::BuildDatabaseCache,
                                                        &DetailBuilder))
            ]
        ]
    ];

    IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory("Tools", FText::GetEmpty(), ECategoryPriority::Important);
    ToolsCategory.AddCustomRow(LOCTEXT("SnapGridFlowCommand_FilterToolsHeader", "Tools"))
            .WholeRowContent()
    [
        SNew(SBorder)
            .BorderImage(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.RoundDarkBorder"))
            .BorderBackgroundColor(FColor(32, 32, 32))
            .Padding(FMargin(10, 10))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SButton)
                    .Text(LOCTEXT("SnapGridFlowCommand_ToolAddModulesFromDir", "Add All Modules from Directory"))
                    .OnClicked(FOnClicked::CreateRaw(this, &FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir, &DetailBuilder))
            ]
        ]
    ];
}

void FSnapGridFlowModuleDatabaseCustomization::AddReferencedObjects(FReferenceCollector& Collector) {
    if (DirImportSettings) {
        Collector.AddReferencedObject(DirImportSettings);
    }
}

TSharedRef<IDetailCustomization> FSnapGridFlowModuleDatabaseCustomization::MakeInstance() {
    return MakeShareable(new FSnapGridFlowModuleDatabaseCustomization);
}

FReply FSnapGridFlowModuleDatabaseCustomization::BuildDatabaseCache(IDetailLayoutBuilder* DetailBuilder) {
    USnapGridFlowModuleDatabase* ModuleDatabase = FDungeonEditorUtils::GetBuilderObject<USnapGridFlowModuleDatabase>(DetailBuilder);
    if (ModuleDatabase) {
        FSnapGridFlowEditorUtils::BuildModuleDatabaseCache(ModuleDatabase);
    }

    return FReply::Handled();
}

FReply FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir(IDetailLayoutBuilder* DetailBuilder) {
    USnapGridFlowModuleDatabase* ModuleDatabase = FDungeonEditorUtils::GetBuilderObject<USnapGridFlowModuleDatabase>(DetailBuilder);
    if (ModuleDatabase) {
        
        static FString Path = "/Game/";
        TSharedRef<SDlgPickPath> SelectPathDlg =
            SNew(SDlgPickPath)
            .Title(LOCTEXT("ChooseImportRootContentPath", "Select the directory that contains the modules"))
            .DefaultPath(FText::FromString(Path));
        
        if (SelectPathDlg->ShowModal() != EAppReturnType::Cancel) {
            const FString ImportPath = SelectPathDlg->GetPath().ToString();
            // Ask the user if we want to rotate these modules while stitching
            {
                if (!DirImportSettings) {
                    DirImportSettings = NewObject<USnapGridFlowModuleDBImportSettings>();
                }
                
                FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
                const FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
                TSharedRef<IDetailsView> PropertyEditor = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
                PropertyEditor->SetObject(DirImportSettings);
                
                TSharedRef<SWindow> Window = SNew(SWindow)
                    .Title(NSLOCTEXT("SGFModuleDB", "DirImportTitle", "Register modules from directory"))
                    .AutoCenter(EAutoCenter::PreferredWorkArea)
                    .SizingRule(ESizingRule::UserSized)
                    .ClientSize(FVector2D(350, 350))
                    .MinWidth(300)
                    .MinHeight(300);


                const float Padding = 5.0f;
                const TSharedPtr<SWidget> Host =
                    SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                    .Padding(Padding)
                    .AutoHeight()
                    [
                        SNew(STextBlock)
                        .Text(NSLOCTEXT("SGFModuleDB", "DirImportDesc", "Module Settings"))
                    ]
                    +SVerticalBox::Slot()
                    .Padding(Padding)
                    .FillHeight(1.0f)
                    [
                        PropertyEditor
                    ]
                    +SVerticalBox::Slot()
                    .Padding(Padding)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        +SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        [
                            SNullWidget::NullWidget
                        ]
                        +SHorizontalBox::Slot()
                        .Padding(Padding)
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(NSLOCTEXT("SGFModuleDB", "ButtonOK", "OK"))
                            .OnClicked_Raw(this, &FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir_ButtonOK, DirImportSettings, ModuleDatabase, ImportPath)
                        ]
                        +SHorizontalBox::Slot()
                        .Padding(Padding)
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(NSLOCTEXT("SGFModuleDB", "ButtonCancel", "Cancel"))
                            .OnClicked_Raw(this, &FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir_ButtonCancel)
                        ]
                    ];


                Window->SetContent(Host.ToSharedRef());
                
                TSharedPtr<SWindow> ParentWindow;
                if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
                {
                    IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
                    ParentWindow = MainFrame.GetParentWindow();
                }

                DirImportSettingsWindow = Window;
                FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);
            }
        }
    }
    
    return FReply::Handled();
}

FReply FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir_ButtonOK(USnapGridFlowModuleDBImportSettings* InSettings, USnapGridFlowModuleDatabase* ModuleDatabase, FString ImportPath) {
    if (InSettings && ModuleDatabase) {
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
        TArray<FAssetData> AssetDataList;
        AssetRegistry.GetAssetsByPath(*ImportPath, AssetDataList, true, true);
        static const FName MapClassName = UWorld::StaticClass()->GetFName();
        bool bModified = false;
        for (const FAssetData& AssetData : AssetDataList) {
            if (AssetData.AssetClass == MapClassName) {
                FSnapGridFlowModuleDatabaseItem& ModuleInfo = ModuleDatabase->Modules.AddDefaulted_GetRef();
                ModuleInfo.Level = AssetData.GetAsset();
                ModuleInfo.Category = InSettings->Category;
                ModuleInfo.bAllowRotation = InSettings->bAllowRotation;
                    
                bModified = true;
            } 
        }

        if (bModified) {
            ModuleDatabase->Modify();
        }
    }
    
    // Close the settings window
    if (DirImportSettingsWindow.IsValid()) {
        DirImportSettingsWindow.Pin()->RequestDestroyWindow();
        DirImportSettingsWindow.Reset();
    }
    return FReply::Handled();
}

FReply FSnapGridFlowModuleDatabaseCustomization::HandleTool_AddModulesFromDir_ButtonCancel() {
    if (DirImportSettingsWindow.IsValid()) {
        DirImportSettingsWindow.Pin()->RequestDestroyWindow();
        DirImportSettingsWindow.Reset();
    }
    return FReply::Handled();
}

///////////////////////////////////// FDAExecRuleNodeCustomization /////////////////////////////////////

void FDAExecRuleNodeCustomization::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& DetailBuilder) {
    CachedDetailBuilder = DetailBuilder;
    CustomizeDetails(*DetailBuilder);
}

void FDAExecRuleNodeCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    UEdGraphNode_ExecRuleNode* Node = FDungeonEditorUtils::GetBuilderObject<UEdGraphNode_ExecRuleNode>(&DetailBuilder);
    if (!Node) {
        return;
    }

    TSharedPtr<IPropertyHandle> ExecutionModeProperty = DetailBuilder.GetProperty(
        GET_MEMBER_NAME_CHECKED(UEdGraphNode_ExecRuleNode, ExecutionMode), UEdGraphNode_ExecRuleNode::StaticClass());
    ExecutionModeProperty->SetOnPropertyValueChanged(
        FSimpleDelegate::CreateSP(this, &FDAExecRuleNodeCustomization::OnExecutionModeChanged, Node));

    TSharedRef<FStructOnScope> ConfigRef = MakeShared<FStructOnScope>(FRuleNodeExecutionModeConfig::StaticStruct(),
                                                                      (uint8*)&Node->ExecutionConfig);

    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Execution Config",
                                                                  LOCTEXT("GrammarCategoryLabel", "Execution Config"),
                                                                  ECategoryPriority::Uncommon);
    ERuleNodeExecutionMode ExecutionMode = Node->ExecutionMode;
    if (ExecutionMode == ERuleNodeExecutionMode::RunWithProbability) {
        Category.AddExternalStructureProperty(
            ConfigRef, GET_MEMBER_NAME_CHECKED(FRuleNodeExecutionModeConfig, RunProbability),
            EPropertyLocation::Common);
    }
    else if (ExecutionMode == ERuleNodeExecutionMode::Iterate) {
        Category.AddExternalStructureProperty(
            ConfigRef, GET_MEMBER_NAME_CHECKED(FRuleNodeExecutionModeConfig, IterationCount),
            EPropertyLocation::Common);
    }
    else if (ExecutionMode == ERuleNodeExecutionMode::IterateRange) {
        Category.AddExternalStructureProperty(
            ConfigRef, GET_MEMBER_NAME_CHECKED(FRuleNodeExecutionModeConfig, IterationCountMin),
            EPropertyLocation::Common);
        Category.AddExternalStructureProperty(
            ConfigRef, GET_MEMBER_NAME_CHECKED(FRuleNodeExecutionModeConfig, IterationCountMax),
            EPropertyLocation::Common);
    }
}

void FDAExecRuleNodeCustomization::OnExecutionModeChanged(UEdGraphNode_ExecRuleNode* Node) {
    IDetailLayoutBuilder* DetailBuilder = CachedDetailBuilder.Pin().Get();
    if (DetailBuilder) {
        DetailBuilder->ForceRefreshDetails();
    }
}

TSharedRef<IDetailCustomization> FDAExecRuleNodeCustomization::MakeInstance() {
    return MakeShareable(new FDAExecRuleNodeCustomization);
}

#undef LOCTEXT_NAMESPACE

