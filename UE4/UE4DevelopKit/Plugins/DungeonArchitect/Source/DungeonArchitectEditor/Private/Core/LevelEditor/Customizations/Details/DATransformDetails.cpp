//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Customizations/Details/DATransformDetails.h"

#include "Core/Editors/ThemeEditor/Graph/EdGraphNode_DungeonActorBase.h"

#include "CoreGlobals.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Math/UnitConversion.h"
#include "Misc/ConfigCacheIni.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "SlateOptMacros.h"
#include "UObject/UnrealType.h"
#include "UnrealEdGlobals.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SRotatorInputBox.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FDATransformDetails"

class FDAScopedSwitchWorldForObject {
public:
    FDAScopedSwitchWorldForObject(UObject* Object)
        : PrevWorld(nullptr) {
        bool bRequiresPlayWorld = false;
        if (GUnrealEd->PlayWorld && !GIsPlayInEditorWorld) {
            UPackage* ObjectPackage = Object->GetOutermost();
            bRequiresPlayWorld = ObjectPackage->HasAnyPackageFlags(PKG_PlayInEditor);
        }

        if (bRequiresPlayWorld) {
            PrevWorld = SetPlayInEditorWorld(GUnrealEd->PlayWorld);
        }
    }

    ~FDAScopedSwitchWorldForObject() {
        if (PrevWorld) {
            RestoreEditorWorld(PrevWorld);
        }
    }

private:
    UWorld* PrevWorld;
};

static UEdGraphNode_DungeonActorBase* GetGraphNodeFromDetailsObject(UObject* InObject) {
    return Cast<UEdGraphNode_DungeonActorBase>(InObject);
}

FDATransformDetails::FDATransformDetails(const TArray<TWeakObjectPtr<UObject>>& InSelectedObjects,
                                         const FSelectedActorInfo& InSelectedActorInfo,
                                         IDetailLayoutBuilder& DetailBuilder)
    : TDANumericUnitTypeInterface(EUnit::Centimeters)
      , SelectedActorInfo(InSelectedActorInfo)
      , SelectedObjects(InSelectedObjects)
      , NotifyHook(DetailBuilder.GetPropertyUtilities()->GetNotifyHook())
      , bPreserveScaleRatio(false)
      , bEditingRotationInUI(false)
      , HiddenFieldMask(0) {
    GConfig->GetBool(TEXT("SelectionDetails"), TEXT("PreserveScaleRatio"), bPreserveScaleRatio, GEditorPerProjectIni);

}

TSharedRef<SWidget> FDATransformDetails::BuildTransformFieldLabel(ETransformField::Type TransformField) {
    FText Label;
    switch (TransformField) {
    case ETransformField::Rotation:
        Label = LOCTEXT("RotationLabel", "Rotation");
        break;
    case ETransformField::Scale:
        Label = LOCTEXT("ScaleLabel", "Scale");
        break;
    case ETransformField::Location:
    default:
        Label = LOCTEXT("LocationLabel", "Location");
        break;
    }

    FMenuBuilder MenuBuilder(true, nullptr, nullptr);

    MenuBuilder.BeginSection(TEXT("TransformType"), FText::Format(LOCTEXT("TransformType", "{0} Type"), Label));

    MenuBuilder.EndSection();


    return
        SNew(SComboButton)
		.ContentPadding(0)
		.ButtonStyle(FEditorStyle::Get(), "NoBorder")
		.ForegroundColor(FSlateColor::UseForeground())
		.MenuContent()
        [
            MenuBuilder.MakeWidget()
        ]
        .ButtonContent()
        [
            SNew(SBox)
            .Padding(FMargin(0.0f, 0.0f, 2.0f, 0.0f))
            [
                SNew(STextBlock)
				.Text(this, &FDATransformDetails::GetTransformFieldText, TransformField)
				.Font(IDetailLayoutBuilder::GetDetailFont())
            ]
        ];
}

FText FDATransformDetails::GetTransformFieldText(ETransformField::Type TransformField) const {
    switch (TransformField) {
    case ETransformField::Location:
        return GetLocationText();
        break;
    case ETransformField::Rotation:
        return GetRotationText();
        break;
    case ETransformField::Scale:
        return GetScaleText();
        break;
    default:
        return FText::GetEmpty();
        break;
    }
}

bool FDATransformDetails::OnCanCopy(ETransformField::Type TransformField) const {
    // We can only copy values if the whole field is set.  If multiple values are defined we do not copy since we are unable to determine the value
    switch (TransformField) {
    case ETransformField::Location:
        return CachedLocation.IsSet();
        break;
    case ETransformField::Rotation:
        return CachedRotation.IsSet();
        break;
    case ETransformField::Scale:
        return CachedScale.IsSet();
        break;
    default:
        return false;
        break;
    }
}

void FDATransformDetails::OnCopy(ETransformField::Type TransformField) {
    CacheTransform();

    FString CopyStr;
    switch (TransformField) {
    case ETransformField::Location:
        CopyStr = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), CachedLocation.X.GetValue(), CachedLocation.Y.GetValue(),
                                  CachedLocation.Z.GetValue());
        break;
    case ETransformField::Rotation:
        CopyStr = FString::Printf(TEXT("(Pitch=%f,Yaw=%f,Roll=%f)"), CachedRotation.Y.GetValue(),
                                  CachedRotation.Z.GetValue(), CachedRotation.X.GetValue());
        break;
    case ETransformField::Scale:
        CopyStr = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), CachedScale.X.GetValue(), CachedScale.Y.GetValue(),
                                  CachedScale.Z.GetValue());
        break;
    default:
        break;
    }

    if (!CopyStr.IsEmpty()) {
        FPlatformApplicationMisc::ClipboardCopy(*CopyStr);
    }
}

void FDATransformDetails::OnPaste(ETransformField::Type TransformField) {
    FString PastedText;
    FPlatformApplicationMisc::ClipboardPaste(PastedText);

    switch (TransformField) {
    case ETransformField::Location:
        {
            FVector Location;
            if (Location.InitFromString(PastedText)) {
                FScopedTransaction Transaction(LOCTEXT("PasteLocation", "Paste Location"));
                OnSetLocation(Location.X, ETextCommit::Default, 0);
                OnSetLocation(Location.Y, ETextCommit::Default, 1);
                OnSetLocation(Location.Z, ETextCommit::Default, 2);
            }
        }
        break;
    case ETransformField::Rotation:
        {
            FRotator Rotation;
            PastedText.ReplaceInline(TEXT("Pitch="), TEXT("P="));
            PastedText.ReplaceInline(TEXT("Yaw="), TEXT("Y="));
            PastedText.ReplaceInline(TEXT("Roll="), TEXT("R="));
            if (Rotation.InitFromString(PastedText)) {
                FScopedTransaction Transaction(LOCTEXT("PasteRotation", "Paste Rotation"));
                OnSetRotation(Rotation.Roll, ETextCommit::Default, 0);
                OnSetRotation(Rotation.Pitch, ETextCommit::Default, 1);
                OnSetRotation(Rotation.Yaw, ETextCommit::Default, 2);
            }
        }
        break;
    case ETransformField::Scale:
        {
            FVector Scale;
            if (Scale.InitFromString(PastedText)) {
                FScopedTransaction Transaction(LOCTEXT("PasteScale", "Paste Scale"));
                OnSetScale(Scale.X, ETextCommit::Default, 0);
                OnSetScale(Scale.Y, ETextCommit::Default, 1);
                OnSetScale(Scale.Z, ETextCommit::Default, 2);
            }
        }
        break;
    default:
        break;
    }
}

FUIAction FDATransformDetails::CreateCopyAction(ETransformField::Type TransformField) {
    return
        FUIAction
        (
            FExecuteAction::CreateSP(this, &FDATransformDetails::OnCopy, TransformField),
            FCanExecuteAction::CreateSP(this, &FDATransformDetails::OnCanCopy, TransformField)
        );
}

FUIAction FDATransformDetails::CreatePasteAction(ETransformField::Type TransformField) {
    return
        FUIAction(FExecuteAction::CreateSP(this, &FDATransformDetails::OnPaste, TransformField));
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FDATransformDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) {
    FSlateFontInfo FontInfo = IDetailLayoutBuilder::GetDetailFont();

    const bool bHideLocationField = (HiddenFieldMask & (1 << ETransformField::Location)) != 0;
    const bool bHideRotationField = (HiddenFieldMask & (1 << ETransformField::Rotation)) != 0;
    const bool bHideScaleField = (HiddenFieldMask & (1 << ETransformField::Scale)) != 0;

    // Location
    if (!bHideLocationField) {
        TSharedPtr<INumericTypeInterface<float>> TypeInterface;
        if (FUnitConversion::Settings().ShouldDisplayUnits()) {
            TypeInterface = SharedThis(this);
        }

        ChildrenBuilder.AddCustomRow(LOCTEXT("LocationFilter", "Location"))
                       .CopyAction(CreateCopyAction(ETransformField::Location))
                       .PasteAction(CreatePasteAction(ETransformField::Location))
                       .NameContent()
                       .HAlign(HAlign_Left)
                       .VAlign(VAlign_Center)
            [
                BuildTransformFieldLabel(ETransformField::Location)
            ]
            .ValueContent()
            .MinDesiredWidth(125.0f * 3.0f)
            .MaxDesiredWidth(125.0f * 3.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .FillWidth(1)
                  .VAlign(VAlign_Center)
                [
                    SNew(SVectorInputBox)
				.X(this, &FDATransformDetails::GetLocationX)
				.Y(this, &FDATransformDetails::GetLocationY)
				.Z(this, &FDATransformDetails::GetLocationZ)
				.bColorAxisLabels(true)
				.AllowResponsiveLayout(true)
				.IsEnabled(this, &FDATransformDetails::GetIsEnabled)
				.OnXCommitted(this, &FDATransformDetails::OnSetLocation, 0)
				.OnYCommitted(this, &FDATransformDetails::OnSetLocation, 1)
				.OnZCommitted(this, &FDATransformDetails::OnSetLocation, 2)
				.Font(FontInfo)
				.TypeInterface(TypeInterface)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    // Just take up space for alignment
                    SNew(SBox)
                    .WidthOverride(18.0f)
                ]
                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .AutoWidth()
                [
                    SNew(SButton)
				.OnClicked(this, &FDATransformDetails::OnLocationResetClicked)
				.Visibility(this, &FDATransformDetails::GetLocationResetVisibility)
				.ContentPadding(FMargin(5.f, 0.f))
				.ToolTipText(LOCTEXT("ResetToDefaultToolTip", "Reset to Default"))
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
				.Content()
                    [
                        SNew(SImage)
                        .Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
                    ]
                ]
            ];
    }

    // Rotation
    if (!bHideRotationField) {
        TSharedPtr<INumericTypeInterface<float>> TypeInterface;
        if (FUnitConversion::Settings().ShouldDisplayUnits()) {
            TypeInterface = MakeShareable(new TDANumericUnitTypeInterface<float>(EUnit::Degrees));
        }

        ChildrenBuilder.AddCustomRow(LOCTEXT("RotationFilter", "Rotation"))
                       .CopyAction(CreateCopyAction(ETransformField::Rotation))
                       .PasteAction(CreatePasteAction(ETransformField::Rotation))
                       .NameContent()
                       .HAlign(HAlign_Left)
                       .VAlign(VAlign_Center)
            [
                BuildTransformFieldLabel(ETransformField::Rotation)
            ]
            .ValueContent()
            .MinDesiredWidth(125.0f * 3.0f)
            .MaxDesiredWidth(125.0f * 3.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .FillWidth(1)
                  .VAlign(VAlign_Center)
                [
                    SNew(SRotatorInputBox)
				.AllowSpin(SelectedObjects.Num() == 1)
				.Roll(this, &FDATransformDetails::GetRotationX)
				.Pitch(this, &FDATransformDetails::GetRotationY)
				.Yaw(this, &FDATransformDetails::GetRotationZ)
				.AllowResponsiveLayout(true)
				.bColorAxisLabels(true)
				.IsEnabled(this, &FDATransformDetails::GetIsEnabled)
				.OnBeginSliderMovement(this, &FDATransformDetails::OnBeginRotatonSlider)
				.OnEndSliderMovement(this, &FDATransformDetails::OnEndRotationSlider)
				.OnRollChanged(this, &FDATransformDetails::OnSetRotation, false, 0)
				.OnPitchChanged(this, &FDATransformDetails::OnSetRotation, false, 1)
				.OnYawChanged(this, &FDATransformDetails::OnSetRotation, false, 2)
				.OnRollCommitted(this, &FDATransformDetails::OnRotationCommitted, 0)
				.OnPitchCommitted(this, &FDATransformDetails::OnRotationCommitted, 1)
				.OnYawCommitted(this, &FDATransformDetails::OnRotationCommitted, 2)
				.TypeInterface(TypeInterface)
				.Font(FontInfo)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    // Just take up space for alignment
                    SNew(SBox)
                    .WidthOverride(18.0f)
                ]
                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .AutoWidth()
                [
                    SNew(SButton)
				.OnClicked(this, &FDATransformDetails::OnRotationResetClicked)
				.Visibility(this, &FDATransformDetails::GetRotationResetVisibility)
				.ContentPadding(FMargin(5.f, 0.f))
				.ToolTipText(LOCTEXT("ResetToDefaultToolTip", "Reset to Default"))
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
				.Content()
                    [
                        SNew(SImage)
                        .Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
                    ]
                ]
            ];
    }

    // Scale
    if (!bHideScaleField) {
        ChildrenBuilder.AddCustomRow(LOCTEXT("ScaleFilter", "Scale"))
                       .CopyAction(CreateCopyAction(ETransformField::Scale))
                       .PasteAction(CreatePasteAction(ETransformField::Scale))
                       .NameContent()
                       .HAlign(HAlign_Left)
                       .VAlign(VAlign_Center)
            [
                BuildTransformFieldLabel(ETransformField::Scale)
            ]
            .ValueContent()
            .MinDesiredWidth(125.0f * 3.0f)
            .MaxDesiredWidth(125.0f * 3.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .FillWidth(1.0f)
                [
                    SNew(SVectorInputBox)
				.X(this, &FDATransformDetails::GetScaleX)
				.Y(this, &FDATransformDetails::GetScaleY)
				.Z(this, &FDATransformDetails::GetScaleZ)
				.bColorAxisLabels(true)
				.AllowResponsiveLayout(true)
				.IsEnabled(this, &FDATransformDetails::GetIsEnabled)
				.OnXCommitted(this, &FDATransformDetails::OnSetScale, 0)
				.OnYCommitted(this, &FDATransformDetails::OnSetScale, 1)
				.OnZCommitted(this, &FDATransformDetails::OnSetScale, 2)
				.ContextMenuExtenderX(this, &FDATransformDetails::ExtendXScaleContextMenu)
				.ContextMenuExtenderY(this, &FDATransformDetails::ExtendYScaleContextMenu)
				.ContextMenuExtenderZ(this, &FDATransformDetails::ExtendZScaleContextMenu)
				.Font(FontInfo)
                ]
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .MaxWidth(18.0f)
                [
                    // Add a checkbox to toggle between preserving the ratio of x,y,z components of scale when a value is entered
                    SNew(SCheckBox)
				.IsChecked(this, &FDATransformDetails::IsPreserveScaleRatioChecked)
				.IsEnabled(this, &FDATransformDetails::GetIsEnabled)
				.OnCheckStateChanged(this, &FDATransformDetails::OnPreserveScaleRatioToggled)
				.Style(FEditorStyle::Get(), "TransparentCheckBox")
				.ToolTipText(LOCTEXT("PreserveScaleToolTip",
                                     "When locked, scales uniformly based on the current xyz scale values so the object maintains its shape in each direction when scaled"))
                    [
                        SNew(SImage)
					.Image(this, &FDATransformDetails::GetPreserveScaleRatioImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
                    ]
                ]
                + SHorizontalBox::Slot()
                  .VAlign(VAlign_Center)
                  .AutoWidth()
                [
                    SNew(SButton)
				.OnClicked(this, &FDATransformDetails::OnScaleResetClicked)
				.Visibility(this, &FDATransformDetails::GetScaleResetVisibility)
				.ContentPadding(FMargin(5.f, 0.f))
				.ToolTipText(LOCTEXT("ResetToDefaultToolTip", "Reset to Default"))
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
				.Content()
                    [
                        SNew(SImage)
                        .Image(FEditorStyle::GetBrush("PropertyWindow.DiffersFromDefault"))
                    ]
                ]
            ];
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FDATransformDetails::Tick(float DeltaTime) {
    CacheTransform();
    if (!FixedDisplayUnits.IsSet()) {
        CacheCommonLocationUnits();
    }
}

void FDATransformDetails::CacheCommonLocationUnits() {
    float LargestValue = 0.f;
    if (CachedLocation.X.IsSet() && CachedLocation.X.GetValue() > LargestValue) {
        LargestValue = CachedLocation.X.GetValue();
    }
    if (CachedLocation.Y.IsSet() && CachedLocation.Y.GetValue() > LargestValue) {
        LargestValue = CachedLocation.Y.GetValue();
    }
    if (CachedLocation.Z.IsSet() && CachedLocation.Z.GetValue() > LargestValue) {
        LargestValue = CachedLocation.Z.GetValue();
    }

    SetupFixedDisplay(LargestValue);
}

bool FDATransformDetails::GetIsEnabled() const {
    return !GEditor->HasLockedActors() || SelectedActorInfo.NumSelected == 0;
}

const FSlateBrush* FDATransformDetails::GetPreserveScaleRatioImage() const {
    return bPreserveScaleRatio
               ? FEditorStyle::GetBrush(TEXT("GenericLock"))
               : FEditorStyle::GetBrush(TEXT("GenericUnlock"));
}

ECheckBoxState FDATransformDetails::IsPreserveScaleRatioChecked() const {
    return bPreserveScaleRatio ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void FDATransformDetails::OnPreserveScaleRatioToggled(ECheckBoxState NewState) {
    bPreserveScaleRatio = (NewState == ECheckBoxState::Checked) ? true : false;
    GConfig->SetBool(TEXT("SelectionDetails"), TEXT("PreserveScaleRatio"), bPreserveScaleRatio, GEditorPerProjectIni);
}

FText FDATransformDetails::GetLocationText() const {
    return LOCTEXT("Location", "Location");
}

FText FDATransformDetails::GetRotationText() const {
    return LOCTEXT("Rotation", "Rotation");
}

FText FDATransformDetails::GetScaleText() const {
    return LOCTEXT("Scale", "Scale");
}

FReply FDATransformDetails::OnLocationResetClicked() {
    const FText TransactionName = LOCTEXT("ResetLocation", "Reset Location");
    FScopedTransaction Transaction(TransactionName);

    //UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(SelectedObjects[0].Get());
    //const FVector Data = Node ? Node->Offset.GetLocation() : FVector();
    const FVector Data = FVector::ZeroVector;

    OnSetLocation(Data[0], ETextCommit::Default, 0);
    OnSetLocation(Data[1], ETextCommit::Default, 1);
    OnSetLocation(Data[2], ETextCommit::Default, 2);

    return FReply::Handled();
}

FReply FDATransformDetails::OnRotationResetClicked() {
    const FText TransactionName = LOCTEXT("ResetRotation", "Reset Rotation");
    FScopedTransaction Transaction(TransactionName);

    //UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(SelectedObjects[0].Get());
    //const FRotator Data = Node ? FRotator(Node->Offset.GetRotation()) : FRotator();
    const FRotator Data(0, 0, 0);

    OnSetRotation(Data.Roll, true, 0);
    OnSetRotation(Data.Pitch, true, 1);
    OnSetRotation(Data.Yaw, true, 2);

    return FReply::Handled();
}

FReply FDATransformDetails::OnScaleResetClicked() {
    const FText TransactionName = LOCTEXT("ResetScale", "Reset Scale");
    FScopedTransaction Transaction(TransactionName);

    //UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(SelectedObjects[0].Get());
    //const FVector Data = Node ? Node->Offset.GetScale3D() : FVector();
    const FVector Data(1, 1, 1);


    ScaleObject(Data[0], 0, false, TransactionName);
    ScaleObject(Data[1], 1, false, TransactionName);
    ScaleObject(Data[2], 2, false, TransactionName);

    return FReply::Handled();
}

void FDATransformDetails::ExtendXScaleContextMenu(FMenuBuilder& MenuBuilder) {
    MenuBuilder.BeginSection("ScaleOperations", LOCTEXT("ScaleOperations", "Scale Operations"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("MirrorValueX", "Mirror X"),
        LOCTEXT("MirrorValueX_Tooltip", "Mirror scale value on the X axis"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FDATransformDetails::OnXScaleMirrored), FCanExecuteAction())
    );
    MenuBuilder.EndSection();
}

void FDATransformDetails::ExtendYScaleContextMenu(FMenuBuilder& MenuBuilder) {
    MenuBuilder.BeginSection("ScaleOperations", LOCTEXT("ScaleOperations", "Scale Operations"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("MirrorValueY", "Mirror Y"),
        LOCTEXT("MirrorValueY_Tooltip", "Mirror scale value on the Y axis"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FDATransformDetails::OnYScaleMirrored), FCanExecuteAction())
    );
    MenuBuilder.EndSection();
}

void FDATransformDetails::ExtendZScaleContextMenu(FMenuBuilder& MenuBuilder) {
    MenuBuilder.BeginSection("ScaleOperations", LOCTEXT("ScaleOperations", "Scale Operations"));
    MenuBuilder.AddMenuEntry(
        LOCTEXT("MirrorValueZ", "Mirror Z"),
        LOCTEXT("MirrorValueZ_Tooltip", "Mirror scale value on the Z axis"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &FDATransformDetails::OnZScaleMirrored), FCanExecuteAction())
    );
    MenuBuilder.EndSection();
}

void FDATransformDetails::OnXScaleMirrored() {
    FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Mouse);
    ScaleObject(1.0f, 0, true, LOCTEXT("MirrorActorScaleX", "Mirror actor scale X"));
}

void FDATransformDetails::OnYScaleMirrored() {
    FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Mouse);
    ScaleObject(1.0f, 1, true, LOCTEXT("MirrorActorScaleY", "Mirror actor scale Y"));
}

void FDATransformDetails::OnZScaleMirrored() {
    FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Mouse);
    ScaleObject(1.0f, 2, true, LOCTEXT("MirrorActorScaleZ", "Mirror actor scale Z"));
}

/**
 * Cache the entire transform at it is seen by the input boxes so we dont have to iterate over the selected actors multiple times                   
 */
void FDATransformDetails::CacheTransform() {
    FVector CurLoc;
    FRotator CurRot;
    FVector CurScale;

    for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
        TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
        if (ObjectPtr.IsValid()) {
            UObject* Object = ObjectPtr.Get();
            UEdGraphNode_DungeonActorBase* GraphNode = Cast<UEdGraphNode_DungeonActorBase>(Object);

            FVector Loc;
            FRotator Rot;
            FVector Scale;
            if (GraphNode) {
                //Loc = SceneComponent->RelativeLocation;
                //Rot = (bEditingRotationInUI && !Object->IsTemplate()) ? ObjectToRelativeRotationMap.FindOrAdd(Object) : SceneComponent->RelativeRotation;
                //Scale = SceneComponent->RelativeScale3D;
                Loc = GraphNode->Offset.GetLocation();
                Rot = FRotator(GraphNode->Offset.GetRotation());
                Scale = GraphNode->Offset.GetScale3D();

                if (ObjectIndex == 0) {
                    // Cache the current values from the first actor to see if any values differ among other actors
                    CurLoc = Loc;
                    CurRot = Rot;
                    CurScale = Scale;

                    CachedLocation.Set(Loc);
                    CachedRotation.Set(Rot);
                    CachedScale.Set(Scale);
                }
                else if (CurLoc != Loc || CurRot != Rot || CurScale != Scale) {
                    // Check which values differ and unset the different values
                    CachedLocation.X = Loc.X == CurLoc.X && CachedLocation.X.IsSet() ? Loc.X : TOptional<float>();
                    CachedLocation.Y = Loc.Y == CurLoc.Y && CachedLocation.Y.IsSet() ? Loc.Y : TOptional<float>();
                    CachedLocation.Z = Loc.Z == CurLoc.Z && CachedLocation.Z.IsSet() ? Loc.Z : TOptional<float>();

                    CachedRotation.X = Rot.Roll == CurRot.Roll && CachedRotation.X.IsSet()
                                           ? Rot.Roll
                                           : TOptional<float>();
                    CachedRotation.Y = Rot.Pitch == CurRot.Pitch && CachedRotation.Y.IsSet()
                                           ? Rot.Pitch
                                           : TOptional<float>();
                    CachedRotation.Z = Rot.Yaw == CurRot.Yaw && CachedRotation.Z.IsSet() ? Rot.Yaw : TOptional<float>();

                    CachedScale.X = Scale.X == CurScale.X && CachedScale.X.IsSet() ? Scale.X : TOptional<float>();
                    CachedScale.Y = Scale.Y == CurScale.Y && CachedScale.Y.IsSet() ? Scale.Y : TOptional<float>();
                    CachedScale.Z = Scale.Z == CurScale.Z && CachedScale.Z.IsSet() ? Scale.Z : TOptional<float>();

                    // If all values are unset all values are different and we can stop looking
                    const bool bAllValuesDiffer = !CachedLocation.IsSet() && !CachedRotation.IsSet() && !CachedScale.
                        IsSet();
                    if (bAllValuesDiffer) {
                        break;
                    }
                }
            }
        }
    }
}


void FDATransformDetails::OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis) {
    bool bBeganTransaction = false;

    for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
        TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
        if (ObjectPtr.IsValid()) {
            UObject* Object = ObjectPtr.Get();
            UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(Object);

            if (Node) {
                FVector OldRelativeLocation = Node->Offset.GetLocation();
                FVector RelativeLocation = OldRelativeLocation;
                float OldValue = RelativeLocation[Axis];
                if (OldValue != NewValue) {
                    if (!bBeganTransaction) {
                        GEditor->BeginTransaction(LOCTEXT("OnSetLocation", "Set location"));
                        bBeganTransaction = true;
                    }

                    //if (Object->HasAnyFlags(RF_DefaultSubObject))
                    //{
                    //	// Default subobjects must be included in any undo/redo operations
                    //	Object->SetFlags(RF_Transactional);
                    //}

                    //// Begin a new movement event which will broadcast delegates before and after the actor moves
                    //FScopedObjectMovement ActorMoveEvent( Object );

                    //FDAScopedSwitchWorldForObject WorldSwitcher( Object );

                    FProperty* OffsetProperty = PropertyAccessUtil::FindPropertyByName(
                        "Offset", UEdGraphNode_DungeonActorBase::StaticClass());
                    Object->PreEditChange(OffsetProperty);

                    if (NotifyHook) {
                        NotifyHook->NotifyPreChange(OffsetProperty);
                    }

                    RelativeLocation[Axis] = NewValue;

                    Node->Offset.SetLocation(RelativeLocation);


                    FPropertyChangedEvent PropertyChangedEvent(OffsetProperty);
                    Object->PostEditChangeProperty(PropertyChangedEvent);

                    if (NotifyHook) {
                        NotifyHook->NotifyPostChange(PropertyChangedEvent, OffsetProperty);
                    }
                }

            }
        }
    }

    if (bBeganTransaction) {
        GEditor->EndTransaction();
    }

    CacheTransform();

    GUnrealEd->UpdatePivotLocationForSelection();
    GUnrealEd->SetPivotMovedIndependently(false);
    GUnrealEd->RedrawLevelEditingViewports();
}

void FDATransformDetails::OnSetRotation(float NewValue, bool bCommitted, int32 Axis) {
    // OnSetRotation is sent from the slider or when the value changes and we dont have slider and the value is being typed.
    // We should only change the value on commit when it is being typed
    const bool bAllowSpin = SelectedObjects.Num() == 1;

    if (bAllowSpin || bCommitted) {
        bool bBeganTransaction = false;
        for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
            TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
            if (ObjectPtr.IsValid()) {
                UObject* Object = ObjectPtr.Get();
                UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(Object);

                if (Node) {
                    const bool bIsEditingTemplateObject = Object->IsTemplate();

                    FRotator CurrentComponentRotation = FRotator(Node->Offset.GetRotation());
                    // Intentionally make a copy, we don't want the FRotator& below to directly edit the component's values!
                    FRotator& RelativeRotation = (bEditingRotationInUI && !bIsEditingTemplateObject)
                                                     ? ObjectToRelativeRotationMap.FindOrAdd(Object)
                                                     : CurrentComponentRotation;
                    FRotator OldRelativeRotation = RelativeRotation;

                    float& ValueToChange = Axis == 0
                                               ? RelativeRotation.Roll
                                               : Axis == 1
                                               ? RelativeRotation.Pitch
                                               : RelativeRotation.Yaw;

                    if (ValueToChange != NewValue) {
                        if (!bBeganTransaction && bCommitted) {
                            GEditor->BeginTransaction(LOCTEXT("OnSetRotation", "Set rotation"));

                            bBeganTransaction = true;
                        }

                        //FDAScopedSwitchWorldForObject WorldSwitcher( Object );

                        FProperty* OffsetProperty = PropertyAccessUtil::FindPropertyByName(
                            "Offset", UEdGraphNode_DungeonActorBase::StaticClass());
                        if (bCommitted && !bEditingRotationInUI) {
                            if (Object->HasAnyFlags(RF_DefaultSubObject)) {
                                // Default subobjects must be included in any undo/redo operations
                                Object->SetFlags(RF_Transactional);
                            }

                            Object->PreEditChange(OffsetProperty);
                        }

                        if (NotifyHook) {
                            NotifyHook->NotifyPreChange(OffsetProperty);
                        }

                        ValueToChange = NewValue;

                        Node->Offset.SetRotation(RelativeRotation.Quaternion());

                        FPropertyChangedEvent PropertyChangedEvent(OffsetProperty,
                                                                   !bCommitted && bEditingRotationInUI
                                                                       ? EPropertyChangeType::Interactive
                                                                       : EPropertyChangeType::ValueSet);

                        if (NotifyHook) {
                            NotifyHook->NotifyPostChange(PropertyChangedEvent, OffsetProperty);
                        }

                        if (bCommitted) {
                            if (!bEditingRotationInUI) {
                                Object->PostEditChangeProperty(PropertyChangedEvent);
                            }

                            if (!bIsEditingTemplateObject) {
                                // The actor is done moving
                                GEditor->BroadcastEndObjectMovement(*Object);
                            }
                        }
                    }
                }
            }
        }

        if (bCommitted && bBeganTransaction) {
            GEditor->EndTransaction();
        }

        GUnrealEd->UpdatePivotLocationForSelection();
        GUnrealEd->SetPivotMovedIndependently(false);
        // Redraw
        GUnrealEd->RedrawLevelEditingViewports();
    }
}

void FDATransformDetails::OnRotationCommitted(float NewValue, ETextCommit::Type CommitInfo, int32 Axis) {
    OnSetRotation(NewValue, true, Axis);

    CacheTransform();
}

void FDATransformDetails::OnBeginRotatonSlider() {
    bEditingRotationInUI = true;

    bool bBeganTransaction = false;
    for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
        TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
        if (ObjectPtr.IsValid()) {
            UObject* Object = ObjectPtr.Get();

            // Start a new transation when a rotator slider begins to change
            // We'll end it when the slider is release
            // NOTE: One transaction per change, not per actor
            if (!bBeganTransaction) {
                GEditor->BeginTransaction(LOCTEXT("OnSetRotation", "Set rotation"));

                bBeganTransaction = true;
            }

            UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(Object);
            if (Node) {
                //FDAScopedSwitchWorldForObject WorldSwitcher( Object );

                if (Object->HasAnyFlags(RF_DefaultSubObject)) {
                    // Default subobjects must be included in any undo/redo operations
                    Object->SetFlags(RF_Transactional);
                }

                FProperty* OffsetProperty = PropertyAccessUtil::FindPropertyByName(
                    "Offset", UEdGraphNode_DungeonActorBase::StaticClass());
                Object->PreEditChange(OffsetProperty);

                // Add/update cached rotation value prior to slider interaction
                ObjectToRelativeRotationMap.FindOrAdd(Object) = Node->Offset.GetRotation().Rotator();
            }
        }
    }

    // Just in case we couldn't start a new transaction for some reason
    if (!bBeganTransaction) {
        GEditor->BeginTransaction(LOCTEXT("OnSetRotation", "Set actor rotation"));
    }
}

void FDATransformDetails::OnEndRotationSlider(float NewValue) {
    bEditingRotationInUI = false;

    for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
        TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
        if (ObjectPtr.IsValid()) {
            UObject* Object = ObjectPtr.Get();
            UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(Object);
            if (Node) {
                //FDAScopedSwitchWorldForObject WorldSwitcher( Object );

                FProperty* OffsetProperty = PropertyAccessUtil::FindPropertyByName(
                    "Offset", UEdGraphNode_DungeonActorBase::StaticClass());
                FPropertyChangedEvent PropertyChangedEvent(OffsetProperty);
                Object->PostEditChangeProperty(PropertyChangedEvent);
            }
        }
    }

    GEditor->EndTransaction();

    // Redraw
    GUnrealEd->RedrawLevelEditingViewports();
}

void FDATransformDetails::OnSetScale(const float NewValue, ETextCommit::Type CommitInfo, int32 Axis) {
    ScaleObject(NewValue, Axis, false, LOCTEXT("OnSetScale", "Set actor scale"));
}

void FDATransformDetails::ScaleObject(float NewValue, int32 Axis, bool bMirror, const FText& TransactionSessionName) {
    FProperty* OffsetProperty = PropertyAccessUtil::FindPropertyByName(
        "Offset", UEdGraphNode_DungeonActorBase::StaticClass());

    bool bBeganTransaction = false;
    for (int32 ObjectIndex = 0; ObjectIndex < SelectedObjects.Num(); ++ObjectIndex) {
        TWeakObjectPtr<UObject> ObjectPtr = SelectedObjects[ObjectIndex];
        if (ObjectPtr.IsValid()) {
            UObject* Object = ObjectPtr.Get();
            UEdGraphNode_DungeonActorBase* Node = GetGraphNodeFromDetailsObject(Object);
            if (Node) {
                FVector OldRelativeScale = Node->Offset.GetScale3D();
                FVector RelativeScale = OldRelativeScale;
                if (bMirror) {
                    NewValue = -RelativeScale[Axis];
                }
                float OldValue = RelativeScale[Axis];
                if (OldValue != NewValue) {
                    if (!bBeganTransaction) {
                        // Begin a transaction the first time an actors scale is about to change.
                        // NOTE: One transaction per change, not per actor
                        GEditor->BeginTransaction(TransactionSessionName);
                        bBeganTransaction = true;
                    }

                    //FDAScopedSwitchWorldForObject WorldSwitcher( Object );

                    if (Object->HasAnyFlags(RF_DefaultSubObject)) {
                        // Default subobjects must be included in any undo/redo operations
                        Object->SetFlags(RF_Transactional);
                    }

                    // Begin a new movement event which will broadcast delegates before and after the actor moves
                    FScopedObjectMovement ActorMoveEvent(Object);

                    Object->PreEditChange(OffsetProperty);

                    if (NotifyHook) {
                        NotifyHook->NotifyPreChange(OffsetProperty);
                    }

                    // Set the new value for the corresponding axis
                    RelativeScale[Axis] = NewValue;

                    if (bPreserveScaleRatio) {
                        // Account for the previous scale being zero.  Just set to the new value in that case?
                        float Ratio = OldValue == 0.0f ? NewValue : NewValue / OldValue;

                        // Change values on axes besides the one being directly changed
                        switch (Axis) {
                        case 0:
                            RelativeScale.Y *= Ratio;
                            RelativeScale.Z *= Ratio;
                            break;
                        case 1:
                            RelativeScale.X *= Ratio;
                            RelativeScale.Z *= Ratio;
                            break;
                        case 2:
                            RelativeScale.X *= Ratio;
                            RelativeScale.Y *= Ratio;
                        }
                    }

                    Node->Offset.SetScale3D(RelativeScale);

                    // Build property chain so the actor knows whether we changed the X, Y or Z
                    FEditPropertyChain PropertyChain;

                    if (!bPreserveScaleRatio) {
                        UStruct* VectorStruct = FindObjectChecked<UScriptStruct>(
                            UObject::StaticClass()->GetOutermost(), TEXT("Vector"), false);
                        FProperty* VectorValueProperty = nullptr;
                        switch (Axis) {
                        case 0:
                            VectorValueProperty = static_cast<FFloatProperty*>(VectorStruct->FindPropertyByName(
                                TEXT("X")));
                            break;
                        case 1:
                            VectorValueProperty = static_cast<FFloatProperty*>(VectorStruct->FindPropertyByName(
                                TEXT("Y")));
                            break;
                        case 2:
                            VectorValueProperty = static_cast<FFloatProperty*>(VectorStruct->FindPropertyByName(
                                TEXT("Z")));
                        }

                        PropertyChain.AddHead(VectorValueProperty);
                    }
                    PropertyChain.AddHead(OffsetProperty);

                    FPropertyChangedEvent PropertyChangedEvent(OffsetProperty, EPropertyChangeType::ValueSet);
                    FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
                    Object->PostEditChangeChainProperty(PropertyChangedChainEvent);

                    // For backwards compatibility, as for some reason PostEditChangeChainProperty calls PostEditChangeProperty with the property set to "X" not "RelativeScale3D"
                    // (it does that for all vector properties, and I don't want to be the one to change that)
                    if (!bPreserveScaleRatio) {
                        Object->PostEditChangeProperty(PropertyChangedEvent);
                    }
                    else {
                        // If the other scale values have been updated, make sure we update the transform now (as the tick will be too late)
                        // so they appear correct when their EditedText is fetched from the delegate.
                        CacheTransform();
                    }

                    if (NotifyHook) {
                        NotifyHook->NotifyPostChange(PropertyChangedEvent, OffsetProperty);
                    }
                }
            }
        }
    }

    if (bBeganTransaction) {
        GEditor->EndTransaction();
    }

    CacheTransform();

    // Redraw
    GUnrealEd->RedrawLevelEditingViewports();
}

#undef LOCTEXT_NAMESPACE

