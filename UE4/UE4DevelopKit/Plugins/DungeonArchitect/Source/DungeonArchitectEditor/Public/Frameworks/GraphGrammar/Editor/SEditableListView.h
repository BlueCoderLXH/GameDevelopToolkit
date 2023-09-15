//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Common/Utils/DungeonEditorUtils.h"
#include "Core/LevelEditor/Customizations/DungeonArchitectStyle.h"

#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Views/SListView.h"

DEFINE_LOG_CATEGORY_STATIC(LogEditableListView, Log, All);

template <typename ItemType>
struct FEditListItemData {
    ItemType Item;
};

template <typename ItemType>
class SEditableListView;

template <typename ItemType>
class FEditableListItemDragDropOp : public FDecoratedDragDropOp {
public:
    DRAG_DROP_OPERATOR_TYPE(FEditableListItemDragDropOp<ItemType>, FDecoratedDragDropOp)

    FEditableListItemDragDropOp(TSharedPtr<FEditListItemData<ItemType>> InItemData, const FGuid& InOwnerListGuid) {
        bAllowDropOnGraph = false;
        OwnerListGuid = InOwnerListGuid;
        ItemData = InItemData;
        DecoratorWidget = SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.ConnectorFeedback.Border"))
			.Content()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
              .AutoWidth()
              .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(NSLOCTEXT("ArrayDragDrop", "PlaceRowHere", "Place Row Here"))
            ]
        ];

        Construct();
    };

    TSharedPtr<SWidget> DecoratorWidget;

    virtual TSharedPtr<SWidget> GetDefaultDecorator() const override {
        return DecoratorWidget;
    }

    TSharedPtr<FEditListItemData<ItemType>> ItemData;
    FGuid OwnerListGuid;
    bool bAllowDropOnGraph;
};


template <typename ItemType>
class SEditableListViewItem : public STableRow<TSharedPtr<ITableRow>> {
public:
    SLATE_BEGIN_ARGS(SEditableListViewItem<ItemType>) {}
    SLATE_END_ARGS()

    typedef STableRow<TSharedPtr<ITableRow>> Super;

public:
    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable,
                   TSharedPtr<SWidget> ContentWidget, TSharedPtr<FEditListItemData<ItemType>> InItemData,
                   TWeakPtr<SEditableListView<ItemType>> InOwner) {
        ItemData = InItemData;
        Owner = InOwner;
        bIsPressed = false;

        STableRow<TSharedPtr<ITableRow>>::Construct(
            STableRow<TSharedPtr<ITableRow>>::FArguments()
            .Padding(3)
            [
                ContentWidget.ToSharedRef()
            ]
            , InOwnerTable);
    }

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {
        FReply BaseResult = Super::OnMouseButtonDown(MyGeometry, MouseEvent);
        if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
            bIsPressed = true;
            BaseResult.DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
        }

        return BaseResult;
    }

    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {
        if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
            bIsPressed = false;
        }

        return Super::OnMouseButtonUp(MyGeometry, MouseEvent);
    }

    virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {
        if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
            if (Owner.IsValid()) {
                TSharedPtr<SEditableListView<ItemType>> OwnerPtr = Owner.Pin();
                TSharedPtr<FEditableListItemDragDropOp<ItemType>> DragDropOp = MakeShareable(
                    new FEditableListItemDragDropOp<ItemType>(ItemData, OwnerPtr->GetGuid()));
                DragDropOp->bAllowDropOnGraph = OwnerPtr->CanDropOnGraph();
                return FReply::Handled().BeginDragDrop(DragDropOp.ToSharedRef());
            }
        }
        return FReply::Handled();
    }

    /** Override OnDragEnter for drag and drop of sockets onto bones */
    virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override {

    }

    /** Override OnDragLeave for drag and drop of sockets onto bones */
    virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override {

    }

    /** Override OnDrop for drag and drop of sockets and meshes onto bones */
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;

private:
    bool bIsPressed;
    TSharedPtr<FEditListItemData<ItemType>> ItemData;
    TWeakPtr<class SEditableListView<ItemType>> Owner;
};

template <typename ItemType>
class SEditableListView : public SCompoundWidget {
public:
    typedef typename TListTypeTraits<ItemType>::NullableType NullableItemType;
    typedef typename TSlateDelegates<NullableItemType>::FOnSelectionChanged FOnSelectionChanged;
    DECLARE_DELEGATE(FOnAddItem)
    DECLARE_DELEGATE_OneParam(FOnDeleteItem, ItemType)
    DECLARE_DELEGATE_RetVal(const TArray<ItemType>*, FGetListSource)
    DECLARE_DELEGATE_RetVal_OneParam(FText, FGetItemTextDelegate, ItemType)
    DECLARE_DELEGATE_RetVal_OneParam(TSharedPtr<SWidget>, FCreateItemWidget, ItemType)
    DECLARE_DELEGATE_TwoParams(FOnReorderItem, ItemType, ItemType)

public:
    SLATE_BEGIN_ARGS(SEditableListView)
            : _Title("")
              , _IconBrush(nullptr)
              , _BorderFlashColor(FLinearColor::Red)
              , _AllowDropOnGraph(false)
              , _GetListSource()
              , _OnSelectionChanged()
              , _OnAddItem()
              , _OnDeleteItem()
              , _OnReorderItem()
              , _GetItemText()
              , _CreateItemWidget() {
        }

        SLATE_ARGUMENT(FString, Title)
        SLATE_ARGUMENT(const FSlateBrush*, IconBrush)
        SLATE_ARGUMENT(FLinearColor, BorderFlashColor)
        SLATE_ARGUMENT(bool, AllowDropOnGraph)
        SLATE_EVENT(FGetListSource, GetListSource)
        SLATE_EVENT(FOnSelectionChanged, OnSelectionChanged)
        SLATE_EVENT(FOnAddItem, OnAddItem)
        SLATE_EVENT(FOnDeleteItem, OnDeleteItem)
        SLATE_EVENT(FOnReorderItem, OnReorderItem)
        SLATE_EVENT(FGetItemTextDelegate, GetItemText)
        SLATE_EVENT(FCreateItemWidget, CreateItemWidget)
    SLATE_END_ARGS()

public:
    /** SCompoundWidget functions */
    void Construct(const FArguments& InArgs) {
        GetListSource = InArgs._GetListSource;
        OnAddItem = InArgs._OnAddItem;
        OnDeleteItem = InArgs._OnDeleteItem;
        GetItemText = InArgs._GetItemText;
        OnSelectionChanged = InArgs._OnSelectionChanged;
        CreateItemWidget = InArgs._CreateItemWidget;
        OnReorderItem = InArgs._OnReorderItem;
        IconBrush = InArgs._IconBrush;
        Title = InArgs._Title;
        bAllowDropOnGraph = InArgs._AllowDropOnGraph;

        ListGuid = FGuid::NewGuid();

        Flasher.FlashDuration = 0.75f;
        Flasher.Widget = this->AsShared();
        Flasher.FlashColor = InArgs._BorderFlashColor;

        ItemListView = SNew(SListView<TSharedPtr<FEditListItemData<ItemType>>>)
			.ItemHeight(24)
			.ListItemsSource(&CachedItems)
			.SelectionMode(ESelectionMode::Single)
			.OnSelectionChanged(this, &SEditableListView<ItemType>::OnSelectionChangedCallback)
			.OnGenerateRow(this, &SEditableListView<ItemType>::GenerateItemListRow)
			.OnMouseButtonClick(this, &SEditableListView<ItemType>::OnMouseButtonClick);

        TSharedPtr<SWidget> ContentWidget = SNew(SBorder)
			.BorderImage(FDungeonArchitectStyle::Get().GetBrush("DungeonArchitect.RoundDarkBorder"))
			.BorderBackgroundColor(this, &SEditableListView::GetBorderColor)
        [
            SNew(SVerticalBox)

            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                ItemListView.ToSharedRef()
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SBox)
                .HeightOverride(32)
                [
                    SNew(SButton)
						.ButtonStyle(
                                     &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                         "DungeonArchitect.FlatButton.Blue"))
						.OnClicked(this, &SEditableListView<ItemType>::OnAddItemClicked)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
                    [
                        SNew(SImage)
                        .Image(FEditorStyle::Get().GetBrush("Plus"))
                    ]
                ]
            ]
        ];

        // Wrap around a title if specified
        if (Title.Len() > 0) {
            ContentWidget = SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(4))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                  .AutoHeight()
                  .Padding(10)
                [
                    SNew(STextBlock)
					.Text(FText::FromString(Title))
				.Font(FDungeonArchitectStyle::Get().GetFontStyle("DungeonArchitect.ListView.LargeFont"))
                ]

                // original content
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    ContentWidget.ToSharedRef()
                ]
            ];
        }

        this->ChildSlot
        [
            ContentWidget.ToSharedRef()
        ];

        RefreshListView();
    }

    FSlateColor GetBorderColor() const {
        if (Flasher.IsFlashing()) {
            return Flasher.GetFlashCurveColor();
        }
        return FSlateColor(FColor(32, 32, 32));
    }

    void RefreshListView() {
        if (!GetListSource.IsBound()) {
            return;
        }

        const TArray<ItemType>* Items = GetListSource.Execute();
        if (!Items) {
            return;
        }

        CachedItems.Reset();
        TArray<TSharedPtr<FEditListItemData<ItemType>>> OldSelectedItems = ItemListView->GetSelectedItems();
        TSharedPtr<FEditListItemData<ItemType>> NewSelectedItem = nullptr;
        for (ItemType Item : *Items) {
            TSharedPtr<FEditListItemData<ItemType>> ItemData = MakeShareable(new FEditListItemData<ItemType>);
            ItemData->Item = Item;
            CachedItems.Add(ItemData);

            if (OldSelectedItems.Num() > 0 && OldSelectedItems[0]->Item == ItemData->Item) {
                NewSelectedItem = ItemData;
            }
        }

        if (!NewSelectedItem.IsValid() && CachedItems.Num() > 0) {
            NewSelectedItem = CachedItems[0];
        }
        ItemListView->SetSelection(NewSelectedItem);
        ItemListView->RequestListRefresh();
    }

    ItemType GetSelectedItem() const {
        TArray<ItemType> SelectedItems = GetSelectedItems();
        return SelectedItems.Num() > 0 ? SelectedItems[0] : nullptr;
    }

    TArray<ItemType> GetSelectedItems() const {
        TArray<TSharedPtr<FEditListItemData<ItemType>>> SelectedItems = ItemListView->GetSelectedItems();
        TArray<ItemType> Result;
        for (TSharedPtr<FEditListItemData<ItemType>> Item : SelectedItems) {
            Result.Add(Item->Item);
        }
        return Result;
    }

    void ProcessItemReordering(ItemType Source, ItemType Dest) {
        if (OnReorderItem.IsBound()) {
            OnReorderItem.Execute(Source, Dest);
        }
        RefreshListView();
    }

    FGuid GetGuid() const { return ListGuid; }

    void SetItemSelection(ItemType ItemData) {
        if (ItemData == GetSelectedItem()) {
            // Already selected
            return;
        }
        TSharedPtr<FEditListItemData<ItemType>> SelectedItem;
        for (TSharedPtr<FEditListItemData<ItemType>> CachedItem : CachedItems) {
            if (CachedItem->Item == ItemData) {
                SelectedItem = CachedItem;
                break;
            }
        }
        ItemListView->ClearSelection();
        if (SelectedItem.IsValid()) {
            ItemListView->SetItemSelection(SelectedItem, true);
            ItemListView->RequestScrollIntoView(SelectedItem);
        }
    }

    void Flash() {
        Flasher.Flash();
    }

    void Focus() {
        FSlateApplication::Get().SetAllUserFocus(ItemListView);
    }

    bool CanDropOnGraph() const { return bAllowDropOnGraph; }

private:
    TSharedRef<ITableRow> GenerateItemListRow(TSharedPtr<FEditListItemData<ItemType>> InItem,
                                              const TSharedRef<STableViewBase>& OwnerTable) {
        TSharedPtr<SWidget> TextContentWidget;

        if (CreateItemWidget.IsBound()) {
            TextContentWidget = CreateItemWidget.Execute(InItem->Item);
        }
        if (!TextContentWidget.IsValid()) {
            TextContentWidget = SNew(STextBlock)
				.Text(this, &SEditableListView<ItemType>::OnGetItemText, InItem)
				.Font(FDungeonArchitectStyle::Get().GetFontStyle("DungeonArchitect.ListView.LargeFont"));
        }

        TSharedPtr<SWidget> ContentWidget;
        if (IconBrush) {
            ContentWidget = SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                  .AutoWidth()
                  .VAlign(VAlign_Center)
                [
                    SNew(SImage)
                    .Image(IconBrush)
                ]
                + SHorizontalBox::Slot()
                  .FillWidth(1.0f)
                  .VAlign(VAlign_Center)
                  .Padding(10, 0, 0, 0)
                [
                    TextContentWidget.ToSharedRef()
                ];
        }
        else {
            ContentWidget = TextContentWidget;
        }

        ContentWidget = SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
              .Padding(10, 10)
              .FillWidth(1.0)
            [
                ContentWidget.ToSharedRef()
            ]

            + SHorizontalBox::Slot()
              .Padding(10, 10)
              .AutoWidth()
            [
                SNew(SBorder)
				.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.BorderBackgroundColor(FColor::Transparent)
                [
                    SNew(SButton)
				.ButtonStyle(
                                     &FDungeonArchitectStyle::Get().GetWidgetStyle<FButtonStyle>(
                                         "DungeonArchitect.Button.Close"))
			.OnClicked(this, &SEditableListView<ItemType>::OnDeleteItemClicked, InItem)
                ]
            ];

        return SNew(SEditableListViewItem<ItemType>, OwnerTable, ContentWidget, InItem, SharedThis(this));
    }

    FText OnGetItemText(TSharedPtr<FEditListItemData<ItemType>> InItem) const {
        if (GetItemText.IsBound()) {
            return GetItemText.Execute(InItem->Item);
        }
        return FText::FromString("Unknown");
    }

    void OnSelectionChangedCallback(TSharedPtr<FEditListItemData<ItemType>> ItemData, ESelectInfo::Type SelectInfo) {
        if (OnSelectionChanged.IsBound()) {
            OnSelectionChanged.Execute(ItemData.IsValid() ? ItemData->Item : nullptr, SelectInfo);
        }
    }

    void OnMouseButtonClick(TSharedPtr<FEditListItemData<ItemType>> ItemData) {
        if (OnSelectionChanged.IsBound()) {
            OnSelectionChanged.Execute(ItemData.IsValid() ? ItemData->Item : nullptr, ESelectInfo::OnMouseClick);
        }
    }


    FReply OnDeleteItemClicked(TSharedPtr<FEditListItemData<ItemType>> InItem) {
        if (OnDeleteItem.IsBound()) {
            OnDeleteItem.Execute(InItem->Item);
        }

        RefreshListView();

        return FReply::Handled();
    }

    FReply OnAddItemClicked() {
        if (OnAddItem.IsBound()) {
            OnAddItem.Execute();
        }

        RefreshListView();

        return FReply::Handled();
    }

private:
    TSharedPtr<SListView<TSharedPtr<FEditListItemData<ItemType>>>> ItemListView;
    TArray<TSharedPtr<FEditListItemData<ItemType>>> CachedItems;

    FOnAddItem OnAddItem;
    FOnDeleteItem OnDeleteItem;
    FOnSelectionChanged OnSelectionChanged;
    FOnReorderItem OnReorderItem;
    FGetItemTextDelegate GetItemText;
    FGetListSource GetListSource;
    FCreateItemWidget CreateItemWidget;
    const FSlateBrush* IconBrush;
    FString Title;
    FGuid ListGuid;
    FWidgetFlasher Flasher;
    bool bAllowDropOnGraph;
};


template <typename ItemType>
FReply SEditableListViewItem<ItemType>::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    TSharedPtr<SEditableListView<ItemType>> OwnerPtr = Owner.Pin();
    TSharedPtr<FEditableListItemDragDropOp<ItemType>> ItemDropOp = DragDropEvent.GetOperationAs<
        FEditableListItemDragDropOp<ItemType>>();
    if (OwnerPtr.IsValid() && ItemDropOp.IsValid() && ItemDropOp->ItemData.IsValid() && ItemDropOp->OwnerListGuid ==
        OwnerPtr->GetGuid()) {
        ItemType ItemSource = ItemDropOp->ItemData->Item;
        ItemType ItemDest = ItemData->Item;
        if (ItemSource != ItemDest && Owner.IsValid()) {
            OwnerPtr->ProcessItemReordering(ItemSource, ItemDest);
            return FReply::Handled();
        }
    }

    return FReply::Unhandled();
}

