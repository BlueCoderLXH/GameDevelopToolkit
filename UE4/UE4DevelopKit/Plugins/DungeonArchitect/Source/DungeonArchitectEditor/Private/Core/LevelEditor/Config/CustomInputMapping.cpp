//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Config/CustomInputMapping.h"

#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "DetailCategoryBuilder.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "GameFramework/InputSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "LevelEditor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "UnrealEdMisc.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "ProjectMissingInputBinder"


DEFINE_LOG_CATEGORY_STATIC(LogMissingInputBinder, Log, All);


ADACustomInputConfigBinder::ADACustomInputConfigBinder()
{
	bIsEditorOnlyActor = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>("SceenRoot");
	RootComponent = SceneRoot;

	UBillboardComponent* SpriteComponent = CreateDefaultSubobject<UBillboardComponent>("Sprite");
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->bHiddenInGame = true;
}

bool ADACustomInputConfigBinder::ProjectContainsMissingInput(TArray<FInputActionKeyMapping>& OutMissingActionMappings, TArray<FInputAxisKeyMapping>& OutMissingAxisMappings) const
{
	OutMissingActionMappings.Reset();
	OutMissingAxisMappings.Reset();

	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	for (const FInputActionKeyMapping& ActionMapping : ActionMappings) {
		if (!InputSettings->GetActionMappings().Contains(ActionMapping)) {
			OutMissingActionMappings.Add(ActionMapping);
		}
	}

	for (const FInputAxisKeyMapping& AxisMapping : AxisMappings) {
		if (!InputSettings->GetAxisMappings().Contains(AxisMapping)) {
			OutMissingAxisMappings.Add(AxisMapping);
		}
	}

	const bool bContainsMissingItems = (OutMissingActionMappings.Num() > 0 || OutMissingAxisMappings.Num() > 0);
	return bContainsMissingItems;
}

bool ADACustomInputConfigBinder::ProjectContainsMissingInput() const
{
	TArray<FInputActionKeyMapping> MissingActionMappings;
	TArray<FInputAxisKeyMapping> MissingAxisMappings;
	return ProjectContainsMissingInput(MissingActionMappings, MissingAxisMappings);
}

class FCustomInputMappingModalWindow {
public:
	void Show(const TArray<FInputActionKeyMapping>& MissingActionMappings, const TArray<FInputAxisKeyMapping>& MissingAxisMappings) {
		bProceed = false;

		static const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);

		TSharedPtr<SWidget> ActionsWidget = SNullWidget::NullWidget;
		if (MissingActionMappings.Num() > 0) {
			TSharedPtr<SVerticalBox> ActionName = SNew(SVerticalBox);
			TSharedPtr<SVerticalBox> ActionKey = SNew(SVerticalBox);

			for (const FInputActionKeyMapping& MissingActionMapping : MissingActionMappings) {
				ActionName->AddSlot() [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(FText::FromName(MissingActionMapping.ActionName))
                ];

				FString Value = MissingActionMapping.Key.ToString();
				if (MissingActionMapping.bShift) Value += " [Shift]";
				if (MissingActionMapping.bCtrl) Value += " [Ctrl]";
				if (MissingActionMapping.bCmd) Value += " [Cmd]";
				if (MissingActionMapping.bAlt) Value += " [Alt]";

				ActionKey->AddSlot() [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(FText::FromString(Value))
                ];
			}

			ActionsWidget =
				SNew(SBorder)
                .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .BorderBackgroundColor(FLinearColor(0, 0, 0, 0.25f))
                [
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("InputBindingActions", "The following Action mappings would be added:"))
					]
					+SVerticalBox::Slot()
					.AutoHeight()
	                [
                		SNew(SBorder)
                		[
			                SNew(SHorizontalBox)
			                +SHorizontalBox::Slot() [ ActionName.ToSharedRef() ]
			                +SHorizontalBox::Slot() [ ActionKey.ToSharedRef() ]
		                ]
	                ]
	            ];
		}

		TSharedPtr<SWidget> AxisWidget = SNullWidget::NullWidget;
		if (MissingAxisMappings.Num() > 0) {
			TSharedPtr<SVerticalBox> AxisName = SNew(SVerticalBox);
			TSharedPtr<SVerticalBox> AxisKey = SNew(SVerticalBox);
			TSharedPtr<SVerticalBox> AxisScale = SNew(SVerticalBox);
			
			for (const FInputAxisKeyMapping& MissingAxisMapping : MissingAxisMappings) {
				AxisName->AddSlot() [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(FText::FromName(MissingAxisMapping.AxisName))
                ];
				AxisKey->AddSlot() [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(FText::FromString(MissingAxisMapping.Key.ToString()))
                ];
				FString ScaleText = FString::Printf(TEXT("%0.2f"), MissingAxisMapping.Scale);
				AxisScale->AddSlot() [
                    SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(FText::FromString(ScaleText))
                ];	
			}
			
			AxisWidget =
                SNew(SBorder)
                .BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
                .BorderBackgroundColor(FLinearColor(0, 0, 0, 0.25f))
                [
	                SNew(SVerticalBox)
	                +SVerticalBox::Slot()
	                .AutoHeight()
	                [
	                    SNew(STextBlock)
	                    .AutoWrapText(true)
	                    .Text(LOCTEXT("InputBindingAxis", "The following Axis mappings would be added:"))
	                ]
	                +SVerticalBox::Slot()
	                .AutoHeight()
	                [
	                    SNew(SBorder)
	                    [
	                        SNew(SHorizontalBox)
	                        +SHorizontalBox::Slot() [ AxisName.ToSharedRef() ]
	                        +SHorizontalBox::Slot() [ AxisKey.ToSharedRef() ]
	                        +SHorizontalBox::Slot() [ AxisScale.ToSharedRef() ]
	                    ]
	                ]
	            ];
		}
		
		
		
		const float Padding = 5.0f;
		const TSharedPtr<SWidget> Host =
            SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(10)
			.AutoHeight()
			[
				SNew(STextBlock)
                .AutoWrapText(true)
				.Text(LOCTEXT("InputBindingTitle", "Add Missing Input Bindings"))
				.Font(TitleFont)
				.ShadowOffset(FVector2D(1, 2))
				.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.5f))
				.Justification(ETextJustify::Center)
			]
			+SVerticalBox::Slot()
			.Padding(Padding)
	        .AutoHeight()
			[
				SNew(STextBlock)
                .AutoWrapText(true)
	            .Text(LOCTEXT("AddMissingInputPromptKey", "The Project contains missing Input Mappings that this demo requires.  Dungeon Architect can add these input mappings for you in the project settings.  Proceed?"))
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(Padding)
			[
				SNew(SScrollBox)
				+SScrollBox::Slot()
				.HAlign(HAlign_Fill)
				[
					ActionsWidget.ToSharedRef()
				]
				+SScrollBox::Slot()
				.HAlign(HAlign_Fill)
				.Padding(0, 5, 0, 0)
				[
					AxisWidget.ToSharedRef()
				]
			]
			+SVerticalBox::Slot()
	        .AutoHeight()
			.Padding(Padding)
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
                    .OnClicked_Raw(this, &FCustomInputMappingModalWindow::ButtonPressed_OK)
                ]
                +SHorizontalBox::Slot()
                .Padding(Padding)
                .AutoWidth()
                [
                    SNew(SButton)
                    .Text(NSLOCTEXT("SGFModuleDB", "ButtonCancel", "Cancel"))
                    .OnClicked_Raw(this, &FCustomInputMappingModalWindow::ButtonPressed_Cancel)
                ]
			];
		

		TSharedRef<SWindow> WindowFrame = SNew(SWindow)
            .Title(NSLOCTEXT("SGFModuleDB", "DirImportTitle", "Input Binding Registration"))
            .AutoCenter(EAutoCenter::PreferredWorkArea)
            .SizingRule(ESizingRule::UserSized)
            .ClientSize(FVector2D(350, 350))
            .MinWidth(300)
            .MinHeight(300);
		WindowFrame->SetContent(Host.ToSharedRef());
                
		TSharedPtr<SWindow> ParentWindow;
		if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
		{
			IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
			ParentWindow = MainFrame.GetParentWindow();
		}

		Window = WindowFrame;
		FSlateApplication::Get().AddModalWindow(WindowFrame, ParentWindow, false);
	}

	bool ShouldProceed() const { return bProceed; }

private:
	FReply ButtonPressed_OK() {
		bProceed = true;
		if (Window.IsValid()) {
			Window.Pin()->RequestDestroyWindow();
			Window.Reset();
		}
		return FReply::Handled();
	}
	
	FReply ButtonPressed_Cancel() {
		bProceed = false;
		if (Window.IsValid()) {
			Window.Pin()->RequestDestroyWindow();
			Window.Reset();
		}
		return FReply::Handled();
	}

private:
	TWeakPtr<SWindow> Window;
	bool bProceed = false;
};

void ADACustomInputConfigBinder::BindMissingInput(bool bShowUserPrompt) const
{
	UE_LOG(LogMissingInputBinder, Log, TEXT("Binding missing inputs"));

	TArray<FInputActionKeyMapping> MissingActionMappings;
	TArray<FInputAxisKeyMapping> MissingAxisMappings;
	if (!ProjectContainsMissingInput(MissingActionMappings, MissingAxisMappings)) {
		// Already up to date
		if (bShowUserPrompt) {
			FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::Ok, LOCTEXT("AddMissingInputAlreadyUpToDateKey", "Input settings are already up to date. No changes were done"));
		}
		return;
	}

	
	bool bProceed = false;
	if (bShowUserPrompt) {
		FCustomInputMappingModalWindow Window;
		Window.Show(MissingActionMappings, MissingAxisMappings);
		bProceed = Window.ShouldProceed();
	}
	else {
		// No user prompt. Proceed by default
		bProceed = true;
	}
	
	if (!bProceed) return;

	if (ActionMappings.Num() > 0 || AxisMappings.Num() > 0) {
		UInputSettings* InputSettings = UInputSettings::GetInputSettings();
		for (const FInputActionKeyMapping& ActionMapping : ActionMappings) {
			InputSettings->AddActionMapping(ActionMapping, false);
		}

		for (const FInputAxisKeyMapping& AxisMapping : AxisMappings) {
			InputSettings->AddAxisMapping(AxisMapping, false);
		}

		InputSettings->ForceRebuildKeymaps();
		InputSettings->SaveKeyMappings();
		InputSettings->UpdateDefaultConfigFile();

		FEditorDelegates::OnActionAxisMappingsChanged.Broadcast();
	}

	if (bShowUserPrompt) {
		FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::Ok, LOCTEXT("AddMissingInputResultKey", "Project Input configuration updated successfully"));
	}
}


FDACustomInputConfigBinderHook::~FDACustomInputConfigBinderHook()
{
	RemoveHook();
}

void FDACustomInputConfigBinderHook::AddHook()
{
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	HookHandle = LevelEditor.OnMapChanged().AddSP(SharedThis(this), &FDACustomInputConfigBinderHook::OnMapChanged);
}

void FDACustomInputConfigBinderHook::RemoveHook()
{
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditor.OnMapChanged().Remove(HookHandle);
}

void FDACustomInputConfigBinderHook::OnMapChanged(UWorld* World, EMapChangeType MapChangeType)
{
	if (MapChangeType == EMapChangeType::LoadMap) {
		ADACustomInputConfigBinder* InputBinder = nullptr;
		for (TActorIterator<ADACustomInputConfigBinder> It(World); It; ++It) {
			InputBinder = *It;
		}

		if (InputBinder && InputBinder->ProjectContainsMissingInput()) {
			InputBinder->BindMissingInput(true);
		}
	}
}

#undef LOCTEXT_NAMESPACE

