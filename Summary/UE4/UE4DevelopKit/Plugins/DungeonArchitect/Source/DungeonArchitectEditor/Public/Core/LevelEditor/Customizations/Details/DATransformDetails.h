//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetSelection.h"
#include "DetailLayoutBuilder.h"
#include "IDetailCustomNodeBuilder.h"
#include "UObject/NoExportTypes.h"
#include "Widgets/Input/NumericTypeInterface.h"

namespace ETransformField {
    enum Type {
        Location,
        Rotation,
        Scale
    };
}


/**
* Numeric interface that specifies how to interact with a number in a specific unit.
* Include NumericUnitTypeInterface.inl for symbol definitions.
*/
template <typename NumericType>
struct TDANumericUnitTypeInterface : TDefaultNumericTypeInterface<NumericType> {
    /** The underlying units which the numeric type are specified in. */
    const EUnit UnderlyingUnits;

    /** Optional units that this type interface will be fixed on */
    TOptional<EUnit> FixedDisplayUnits;

    /** Constructor */
    TDANumericUnitTypeInterface(EUnit InUnits) : UnderlyingUnits(InUnits) {
    }

    /** Convert this type to a string */
    virtual FString ToString(const NumericType& Value) const override {
        if (UnderlyingUnits == EUnit::Unspecified) {
            return TDefaultNumericTypeInterface<NumericType>::ToString(Value);
        }

        auto ToUnitString = [this](const FNumericUnit<NumericType>& InNumericUnit) -> FString {
            FString String = TDefaultNumericTypeInterface<NumericType>::ToString(InNumericUnit.Value);
            String += TEXT(" ");
            String += FUnitConversion::GetUnitDisplayString(InNumericUnit.Units);
            return String;
        };

        FNumericUnit<NumericType> FinalValue(Value, UnderlyingUnits);

        if (FixedDisplayUnits.IsSet()) {
            auto Converted = FinalValue.ConvertTo(FixedDisplayUnits.GetValue());
            if (Converted.IsSet()) {
                return ToUnitString(Converted.GetValue());
            }
        }

        return ToUnitString(FinalValue);
    }

    /** Attempt to parse a numeral with our units from the specified string. */
    virtual TOptional<NumericType> FromString(const FString& InString, const NumericType& InExistingValue) override {
        if (UnderlyingUnits == EUnit::Unspecified) {
            return TDefaultNumericTypeInterface<NumericType>::FromString(InString, InExistingValue);
        }

        EUnit DefaultUnits = FixedDisplayUnits.IsSet() ? FixedDisplayUnits.GetValue() : UnderlyingUnits;

        // Always parse in as a double, to allow for input of higher-order units with decimal numerals into integral types (eg, inputting 0.5km as 500m)
        TValueOrError<FNumericUnit<double>, FText> NewValue = FNumericUnit<double>::TryParseExpression(
            *InString, DefaultUnits, InExistingValue);
        if (NewValue.IsValid()) {
            // Convert the number into the correct units
            EUnit SourceUnits = NewValue.GetValue().Units;
            if (SourceUnits == EUnit::Unspecified && FixedDisplayUnits.IsSet()) {
                // Use the default supplied input units
                SourceUnits = FixedDisplayUnits.GetValue();
            }
            return FUnitConversion::Convert(NewValue.GetValue().Value, SourceUnits, UnderlyingUnits);
        }

        return TOptional<NumericType>();
    }

    /** Check whether the specified typed character is valid */
    virtual bool IsCharacterValid(TCHAR InChar) const override {
        return (UnderlyingUnits == EUnit::Unspecified)
                   ? TDefaultNumericTypeInterface<NumericType>::IsCharacterValid(InChar)
                   : InChar != TEXT('\t');
    }

    /** Set up this interface to use a fixed display unit based on the specified value */
    void SetupFixedDisplay(const NumericType& InValue) {
        EUnit DisplayUnit = FUnitConversion::CalculateDisplayUnit(InValue, UnderlyingUnits);
        if (DisplayUnit != EUnit::Unspecified) {
            FixedDisplayUnits = DisplayUnit;
        }
    }
};


/**
 * Manages the Transform section of a details view                    
 */
class FDATransformDetails : public TSharedFromThis<FDATransformDetails>, public IDetailCustomNodeBuilder,
                            public TDANumericUnitTypeInterface<float> {
public:
    FDATransformDetails(const TArray<TWeakObjectPtr<UObject>>& InSelectedObjects,
                        const FSelectedActorInfo& InSelectedActorInfo, IDetailLayoutBuilder& DetailBuilder);

    /**
     * Caches the representation of the actor transform for the user input boxes                   
     */
    void CacheTransform();

    virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override {
    }

    virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
    virtual bool RequiresTick() const override { return true; }
    virtual FName GetName() const override { return NAME_None; }
    virtual bool InitiallyCollapsed() const override { return false; }
    virtual void Tick(float DeltaTime) override;

    virtual void SetOnRebuildChildren(FSimpleDelegate OnRebuildChildren) override {
    }

    void HideTransformField(const ETransformField::Type InTransformField) {
        HiddenFieldMask |= (1 << InTransformField);
    }

private:
    /** @return Whether the transform details panel should be enabled (editable) or not (read-only / greyed out) */
    bool GetIsEnabled() const;

    /**
     * Sets the selected object(s) translation (relative)
     *
     * @param NewValue The new translation value
     */
    void OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

    /**
     * Sets the selected object(s) rotation (relative)
     *
     * @param NewValue		The new rotation value for the axis
     * @param bCommittted	true if the value was committed, false is the value comes from the slider (int32 due to delegate restrictions)
     * @param Axis			The axis of rotation which changed.  0 = Roll, 1 = Pitch, 2 = Yaw
     */
    void OnSetRotation(float NewValue, bool bCommitted, int32 Axis);

    /**
     * Sets the selected object(s) rotation (relative)
     *
     * @param CommitInfo	Whether or not this was committed from pressing enter or losing focus
     * @param Axis			The axis of rotation which changed.  0 = Roll, 1 = Pitch, 2 = Yaw
     * @param NewValue		The new rotation value for the axis
     */
    void OnRotationCommitted(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

    /**
     * Called when the one of the axis sliders for object rotation begins to change for the first time 
     */
    void OnBeginRotatonSlider();

    /**
     * Called when the one of the axis sliders for object rotation is released
     */
    void OnEndRotationSlider(float NewValue);

    /**
     * Sets the selected object(s) scale (relative)
     *
     * @param NewValue The new scale value
     */
    void OnSetScale(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

    /**
     * Sets the selected object(s) scale or mirrors them (relative)
     * Helper function for OnSetScale and On*ScaleMirrored.
     *
     * @param NewValue					The new scale value - unused if mirroring
     * @param Axis						The axis we are manipulating
     * @param bMirror					Whether to mirror the axis
     * @param TransactionSessionName	the name of the undo/redo transaction
     */
    void ScaleObject(float NewValue, int32 Axis, bool bMirror, const FText& TransactionSessionName);

    /** @return Icon to use in the preserve scale ratio check box */
    const FSlateBrush* GetPreserveScaleRatioImage() const;

    /** @return The state of the preserve scale ratio checkbox */
    ECheckBoxState IsPreserveScaleRatioChecked() const;

    /**
     * Called when the preserve scale ratio checkbox is toggled
     */
    void OnPreserveScaleRatioToggled(ECheckBoxState NewState);

    /**
     * Builds a transform field label
     *
     * @param TransformField The field to build the label for
     * @return The label widget
     */
    TSharedRef<SWidget> BuildTransformFieldLabel(ETransformField::Type TransformField);

    /**
     * Gets display text for a transform field
     *
     * @param TransformField	The field to get the text for
     * @return The text label for TransformField
     */
    FText GetTransformFieldText(ETransformField::Type TransformField) const;

    /**
     * @return the text to display for translation                   
     */
    FText GetLocationText() const;

    /**
     * @return the text to display for rotation                   
     */
    FText GetRotationText() const;

    /**
     * @return the text to display for scale                   
     */
    FText GetScaleText() const;


    /** @return true of copy is enabled for the specified field */
    bool OnCanCopy(ETransformField::Type TransformField) const;

    /**
     * Copies the specified transform field to the clipboard
     */
    void OnCopy(ETransformField::Type TransformField);

    /**
     * Pastes the specified transform field from the clipboard
     */
    void OnPaste(ETransformField::Type TransformField);

    /**
     * Creates a UI action for copying a specified transform field
     */
    FUIAction CreateCopyAction(ETransformField::Type TransformField);

    /**
     * Creates a UI action for pasting a specified transform field
     */
    FUIAction CreatePasteAction(ETransformField::Type TransformField);

    /** Called when the "Reset to Default" button for the location has been clicked */
    FReply OnLocationResetClicked();

    /** Called when the "Reset to Default" button for the rotation has been clicked */
    FReply OnRotationResetClicked();

    /** Called when the "Reset to Default" button for the scale has been clicked */
    FReply OnScaleResetClicked();

    /** Extend the context menu for the X component */
    void ExtendXScaleContextMenu(FMenuBuilder& MenuBuilder);
    /** Extend the context menu for the Y component */
    void ExtendYScaleContextMenu(FMenuBuilder& MenuBuilder);
    /** Extend the context menu for the Z component */
    void ExtendZScaleContextMenu(FMenuBuilder& MenuBuilder);

    /** Called when the X axis scale is mirrored */
    void OnXScaleMirrored();
    /** Called when the Y axis scale is mirrored */
    void OnYScaleMirrored();
    /** Called when the Z axis scale is mirrored */
    void OnZScaleMirrored();

    /** @return The X component of location */
    TOptional<float> GetLocationX() const { return CachedLocation.X; }
    /** @return The Y component of location */
    TOptional<float> GetLocationY() const { return CachedLocation.Y; }
    /** @return The Z component of location */
    TOptional<float> GetLocationZ() const { return CachedLocation.Z; }
    /** @return The visibility of the "Reset to Default" button for the location component */
    EVisibility GetLocationResetVisibility() const {
        // unset means multiple differing values, so show "Reset to Default" in that case
        return CachedLocation.IsSet() && CachedLocation.X.GetValue() == 0.0f && CachedLocation.Y.GetValue() == 0.0f &&
               CachedLocation.Z.GetValue() == 0.0f
                   ? EVisibility::Hidden
                   : EVisibility::Visible;
    }

    /** @return The X component of rotation */
    TOptional<float> GetRotationX() const { return CachedRotation.X; }
    /** @return The Y component of rotation */
    TOptional<float> GetRotationY() const { return CachedRotation.Y; }
    /** @return The Z component of rotation */
    TOptional<float> GetRotationZ() const { return CachedRotation.Z; }
    /** @return The visibility of the "Reset to Default" button for the rotation component */
    EVisibility GetRotationResetVisibility() const {
        // unset means multiple differing values, so show "Reset to Default" in that case
        return CachedRotation.IsSet() && CachedRotation.X.GetValue() == 0.0f && CachedRotation.Y.GetValue() == 0.0f &&
               CachedRotation.Z.GetValue() == 0.0f
                   ? EVisibility::Hidden
                   : EVisibility::Visible;
    }

    /** @return The X component of scale */
    TOptional<float> GetScaleX() const { return CachedScale.X; }
    /** @return The Y component of scale */
    TOptional<float> GetScaleY() const { return CachedScale.Y; }
    /** @return The Z component of scale */
    TOptional<float> GetScaleZ() const { return CachedScale.Z; }
    /** @return The visibility of the "Reset to Default" button for the scale component */
    EVisibility GetScaleResetVisibility() const {
        // unset means multiple differing values, so show "Reset to Default" in that case
        return CachedScale.IsSet() && CachedScale.X.GetValue() == 1.0f && CachedScale.Y.GetValue() == 1.0f &&
               CachedScale.Z.GetValue() == 1.0f
                   ? EVisibility::Hidden
                   : EVisibility::Visible;
    }

    /** Cache a single unit to display all location comonents in */
    void CacheCommonLocationUnits();

private:
    /** A vector where it may optionally be unset */
    struct FOptionalVector {
        /**
         * Sets the value from an FVector                   
         */
        void Set(const FVector& InVec) {
            X = InVec.X;
            Y = InVec.Y;
            Z = InVec.Z;
        }

        /**
         * Sets the value from an FRotator                   
         */
        void Set(const FRotator& InRot) {
            X = InRot.Roll;
            Y = InRot.Pitch;
            Z = InRot.Yaw;
        }

        /** @return Whether or not the value is set */
        bool IsSet() const {
            // The vector is set if all values are set
            return X.IsSet() && Y.IsSet() && Z.IsSet();
        }

        TOptional<float> X;
        TOptional<float> Y;
        TOptional<float> Z;
    };

    FSelectedActorInfo SelectedActorInfo;
    /** Copy of selected actor references in the details view */
    TArray<TWeakObjectPtr<UObject>> SelectedObjects;
    /** Cache translation value of the selected set */
    FOptionalVector CachedLocation;
    /** Cache rotation value of the selected set */
    FOptionalVector CachedRotation;
    /** Cache scale value of the selected set */
    FOptionalVector CachedScale;
    /** Notify hook to use */
    FNotifyHook* NotifyHook;
    /** Whether or not to preserve scale ratios */
    bool bPreserveScaleRatio;
    /** Mapping from object to relative rotation values which are not affected by Quat->Rotator conversions during transform calculations */
    TMap<UObject*, FRotator> ObjectToRelativeRotationMap;
    /** Flag to indicate we are currently editing the rotation in the UI, so we should rely on the cached value in objectToRelativeRotationMap, not the value from the object */
    bool bEditingRotationInUI;
    /** Bitmask to indicate which fields should be hidden (if any) */
    uint8 HiddenFieldMask;
};

