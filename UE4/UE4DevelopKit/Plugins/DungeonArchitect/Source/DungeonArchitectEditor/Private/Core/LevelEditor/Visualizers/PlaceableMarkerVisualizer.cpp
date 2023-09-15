//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Visualizers/PlaceableMarkerVisualizer.h"

#include "Frameworks/ThemeEngine/Markers/PlaceableMarker.h"

#include "CanvasItem.h"

namespace {
    struct FMarkerRenderInfo {
        FString Name;
        int32 Width;
        int32 Height;
    };
    
    void RenderMarkerList(FCanvas* Canvas, UFont* Font, const FString& Title, const TArray<FString>& Lines, const FVector2D& BaseLocation, const FVector2D& Offset, int BorderThickness) {
        TArray<FMarkerRenderInfo> Items;
        int32 TotalHeight = 0;
        int32 MaxWidth = 0;
        for (const FString& Line : Lines) {
            FMarkerRenderInfo& Item = Items.AddDefaulted_GetRef();
            Item.Name = Line;
            Font->GetStringHeightAndWidth(Line, Item.Height, Item.Width);
            TotalHeight += Item.Height;
            MaxWidth = FMath::Max(MaxWidth, Item.Width);
        }

        FMarkerRenderInfo TitleInfo;
        TitleInfo.Name = Title;
        Font->GetStringHeightAndWidth(Title, TitleInfo.Height, TitleInfo.Width);
        MaxWidth = FMath::Max(MaxWidth, TitleInfo.Width);

        int32 StartX = BaseLocation.X + Offset.X;
        int32 StartY = BaseLocation.Y + Offset.Y - TotalHeight;

        // Draw the background
        {
            const FVector2D BackgroundStart = FVector2D(StartX, StartY) - FVector2D(BorderThickness);
            const FVector2D BackgroundSize = FVector2D(MaxWidth, TotalHeight) + FVector2D(BorderThickness) * 2;
            FCanvasTileItem BackgroundItem(BackgroundStart, BackgroundSize, FLinearColor(0, 0, 0, 0.5f));
            BackgroundItem.BlendMode = SE_BLEND_Translucent;
            Canvas->DrawItem(BackgroundItem);

            // Draw the title
            const FVector2D TitleSize = FVector2D(BackgroundSize.X, TitleInfo.Height + BorderThickness * 2);
            const FVector2D TitleStart = BackgroundStart - FVector2D(0, TitleSize.Y);
            FCanvasTileItem TitleItem(TitleStart, TitleSize, FLinearColor::Black);
            Canvas->DrawItem(TitleItem);
            
            const FVector2D TitleDrawLocation = TitleStart + FVector2D(BorderThickness);
            FCanvasTextItem TitleTextItem( TitleDrawLocation, FText::FromString( TitleInfo.Name ), Font, FLinearColor::White );
            TitleTextItem.EnableShadow(FLinearColor::Black);
            Canvas->DrawItem( TitleTextItem );

            // Draw the frame border
            FCanvasBoxItem BorderItem(TitleStart, BackgroundSize + FVector2D(0, TitleSize.Y));
            BorderItem.LineThickness = 2;
            BorderItem.SetColor(FLinearColor::Black);
            Canvas->DrawItem(BorderItem);

            // Draw a line
            FVector2D LineStart = BaseLocation;
            FVector2D LineEnd = BackgroundStart + FVector2D(0, BackgroundSize.Y);
            FCanvasLineItem Line(LineStart, LineEnd);
            Line.LineThickness = 2;
            Line.SetColor(FLinearColor::Black);
            Canvas->DrawItem(Line);
        }

        
        for (const FMarkerRenderInfo& Info : Items) {
            const FVector2D Location(StartX, StartY);
            FCanvasTextItem TextItem( Location, FText::FromString( Info.Name ), Font, FLinearColor::Yellow );
            TextItem.EnableShadow(FLinearColor::Black);
            Canvas->DrawItem( TextItem );

            StartY += Info.Height;
        }
        
        
    }
}

void FPlaceableMarkerVisualizer::DrawVisualizationHUD(const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View, FCanvas* Canvas) {
    const UPlaceableMarkerComponent* MarkerComponent = Cast<UPlaceableMarkerComponent>(Component);
    if (!MarkerComponent || !MarkerComponent->MarkerAsset) return;

    
    
    AActor* Owner = Component->GetOwner();
    if (!Owner) return;

    const FVector WorldLocation = Owner->GetActorLocation();
    const FPlane Proj = View->Project(WorldLocation);
    if (Proj.W > 0) {
        const int32 HalfX = 0.5f * Viewport->GetSizeXY().X;
        const int32 HalfY = 0.5f * Viewport->GetSizeXY().Y;
        
        const int32 XPos = HalfX + ( HalfX * Proj.X );
        const int32 YPos = HalfY + ( HalfY * (Proj.Y * -1.f) );

        const FVector2D BaseLocation(XPos, YPos);
        
        UFont* Font = GEngine->GetMediumFont();
        RenderMarkerList(Canvas, Font, MarkerComponent->MarkerAsset->GetName(), MarkerComponent->MarkerAsset->MarkerNames, BaseLocation, FVector2D(20, -25), 5);
    }
}

