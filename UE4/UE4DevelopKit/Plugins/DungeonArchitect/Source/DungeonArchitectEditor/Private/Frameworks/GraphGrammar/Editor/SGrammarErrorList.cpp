//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/GraphGrammar/Editor/SGrammarErrorList.h"

#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"
#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidation.h"
#include "Frameworks/GraphGrammar/Editor/GraphGrammarValidationSupport.h"

#include "EditorStyleSet.h"
#include "SlateOptMacros.h"
#include "Widgets/Views/SListView.h"

DEFINE_LOG_CATEGORY_STATIC(LogGrammarErrorList, Log, All);

#define LOCTEXT_NAMESPACE "SGrammarErrorList"

FName SGrammarErrorList::ColumnIdSeverity = "Severity";
FName SGrammarErrorList::ColumnIdMessage = "Message";

/////////////////////////// SGrammarErrorList /////////////////////////// 
void SGrammarErrorList::Construct(const FArguments& InArgs) {
    GrammarEditor = InArgs._GrammarEditor;

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
          .FillHeight(1.0f)
          .Padding(0.0f, 4.0f, 0.0f, 0.0f)
        [
            SNew(SBorder)
					.BorderImage(FDungeonArchitectStyle::Get().GetBrush("DA.SnapEd.GroupBorder"))
					.Padding(0.0f)
            [
                // message list
                SAssignNew(ValidationEntryListView, SListView<FGrammarValidationEntryPtr>)
							.ItemHeight(24.0f)
							.ListItemsSource(&ValidationEntryList)
							.SelectionMode(ESelectionMode::Single)
							.OnGenerateRow(this, &SGrammarErrorList::HandleGenerateRow)
							.OnSelectionChanged(this, &SGrammarErrorList::HandleSelectionChanged)
							.OnMouseButtonDoubleClick(this, &SGrammarErrorList::HandleItemDoubleClicked)
							.HeaderRow
                                                                                          (
                                                                                              SNew(SHeaderRow)

                                                                                              + SHeaderRow::Column(
                                                                                                    ColumnIdSeverity)
                                                                                                .DefaultLabel(
                                                                                                    LOCTEXT(
                                                                                                        "ValidationListSeverityColumnHeader",
                                                                                                        "Severity"))
                                                                                                .FixedWidth(70.0f)

                                                                                              + SHeaderRow::Column(
                                                                                                    ColumnIdMessage)
                                                                                                .DefaultLabel(
                                                                                                    LOCTEXT(
                                                                                                        "ValidationListMessageColumnHeader",
                                                                                                        "Message"))
                                                                                                .FillWidth(1.0f)
                                                                                          )
            ]
        ]
    ];
}

TSharedRef<ITableRow> SGrammarErrorList::HandleGenerateRow(FGrammarValidationEntryPtr InItem,
                                                           const TSharedRef<STableViewBase>& OwnerTable) {
    return SNew(SGrammarErrorListRow, OwnerTable, InItem.ToSharedRef());
}

void SGrammarErrorList::HandleSelectionChanged(FGrammarValidationEntryPtr InItem, ESelectInfo::Type SelectInfo) {

}

void SGrammarErrorList::HandleItemDoubleClicked(FGrammarValidationEntryPtr InItem) {
    if (InItem.IsValid() && InItem->FocusAction.IsValid() && GrammarEditor.IsValid()) {
        FGrammarFocusActionContext FocusContext;
        FocusContext.GrammarEditor = GrammarEditor.Pin();
        InItem->FocusAction->Focus(FocusContext);
    }
}

void SGrammarErrorList::UpdateList(const TArray<FGrammarValidationEntryPtr>& InValidationEntryList) {
    ValidationEntryList.Reset();
    ValidationEntryList.Append(InValidationEntryList);
    ValidationEntryListView->RequestListRefresh();
}

/////////////////////////// SGrammarErrorListRow ///////////////////////////
void SGrammarErrorListRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView,
                                     const FGrammarValidationEntryRef InValidationEntry) {
    ValidationEntry = InValidationEntry;
    SMultiColumnTableRow<FGrammarValidationEntryPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);

}

const FSlateBrush* GetLogIconName(FGrammarValidationEntryPtr ValidationEntry) {
    if (ValidationEntry->LogType == EGrammarLogType::Error) return FEditorStyle::Get().GetBrush("Icons.Error");
    if (ValidationEntry->LogType == EGrammarLogType::Warning) return FEditorStyle::Get().GetBrush("Icons.Warning");
    if (ValidationEntry->LogType == EGrammarLogType::Success) return FDungeonArchitectStyle::Get().GetBrush(
        "DA.SnapEd.SuccessIcon");
    return FEditorStyle::Get().GetBrush("Icons.Info");
}

FText GetLogTypeText(FGrammarValidationEntryPtr ValidationEntry) {
    if (ValidationEntry->LogType == EGrammarLogType::Error) {
        return LOCTEXT("LogTypeError", "Error");
    }
    if (ValidationEntry->LogType == EGrammarLogType::Warning) {
        return LOCTEXT("LogTypeWarn", "Warning");
    }
    if (ValidationEntry->LogType == EGrammarLogType::Success) {
        return LOCTEXT("LogTypeSuccess", "Success");
    }
    return LOCTEXT("LogTypeInfo", "Info");
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SGrammarErrorListRow::GenerateWidgetForColumn(const FName& ColumnName) {
    TSharedPtr<SWidget> CellWidget;

    if (ColumnName == SGrammarErrorList::ColumnIdMessage) {
        CellWidget = SNew(SBox)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(ValidationEntry->Message)
            ];
    }
    else if (ColumnName == SGrammarErrorList::ColumnIdSeverity) {
        const FSlateBrush* Icon = GetLogIconName(ValidationEntry);
        const FText LogTypeText = GetLogTypeText(ValidationEntry);
        CellWidget = SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
              .AutoWidth()
              .VAlign(VAlign_Center)
            [
                SNew(SImage)
                .Image(Icon)
            ]
            + SHorizontalBox::Slot()
              .FillWidth(1.0f)
              .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(LogTypeText)
            ];
    }
    else {
        CellWidget = SNullWidget::NullWidget;
    }
    return SNew(SBox)
        .Padding(2)
        [
            CellWidget.ToSharedRef()
        ];

}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION


#undef LOCTEXT_NAMESPACE

