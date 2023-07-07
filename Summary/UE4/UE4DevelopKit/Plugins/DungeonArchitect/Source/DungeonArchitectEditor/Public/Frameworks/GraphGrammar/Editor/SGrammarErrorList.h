//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableRow.h"

typedef TSharedPtr<struct FGrammarValidationEntry> FGrammarValidationEntryPtr;
typedef TSharedRef<struct FGrammarValidationEntry> FGrammarValidationEntryRef;

class SGrammarEditor;

class SGrammarErrorList : public SCompoundWidget {
public:

    SLATE_BEGIN_ARGS(SGrammarErrorList) {}
        SLATE_ARGUMENT(TWeakPtr<SGrammarEditor>, GrammarEditor)
    SLATE_END_ARGS()

public:

    void Construct(const FArguments& InArgs);
    void UpdateList(const TArray<FGrammarValidationEntryPtr>& InValidationEntryList);

    static FName ColumnIdSeverity;
    static FName ColumnIdMessage;

private:
    TSharedRef<ITableRow> HandleGenerateRow(FGrammarValidationEntryPtr InItem,
                                            const TSharedRef<STableViewBase>& OwnerTable);
    void HandleSelectionChanged(FGrammarValidationEntryPtr InItem, ESelectInfo::Type SelectInfo);
    void HandleItemDoubleClicked(FGrammarValidationEntryPtr InItem);

private:
    TArray<FGrammarValidationEntryPtr> ValidationEntryList;
    TSharedPtr<SListView<FGrammarValidationEntryPtr>> ValidationEntryListView;
    TWeakPtr<SGrammarEditor> GrammarEditor;
};


class SGrammarErrorListRow : public SMultiColumnTableRow<FGrammarValidationEntryPtr> {
public:
    SLATE_BEGIN_ARGS(SGrammarErrorList) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView,
                   FGrammarValidationEntryRef InValidationEntry);

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
    FGrammarValidationEntryPtr ValidationEntry;
};

