//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemapRenderer.h"

#include "Frameworks/Flow/Domains/Tilemap/GridFlowTilemap.h"

#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "RenderUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogGridFlowTileRenderer, Log, All);

////////////////////////////// FGridFlowTilemapRenderer //////////////////////////////
class FGridFlowCanvasHelper {
public:
    static UTextureRenderTarget2D* CreateRenderTarget2D(int32 Width, int32 Height, ETextureRenderTargetFormat Format,
                                                        FLinearColor ClearColor, bool bAutoGenerateMipMaps) {
        UTextureRenderTarget2D* NewRenderTarget2D = NewObject<UTextureRenderTarget2D>();
        check(NewRenderTarget2D);
        NewRenderTarget2D->RenderTargetFormat = Format;
        NewRenderTarget2D->ClearColor = ClearColor;
        NewRenderTarget2D->bAutoGenerateMips = bAutoGenerateMipMaps;
        NewRenderTarget2D->InitAutoFormat(Width, Height);
        NewRenderTarget2D->UpdateResourceImmediate(true);

        return NewRenderTarget2D;
    }

    static void BeginDrawCanvasToRenderTarget(UTextureRenderTarget2D* TextureRenderTarget, UCanvas*& Canvas,
                                              FVector2D& Size, FDrawToRenderTargetContext& Context) {
        Context = FDrawToRenderTargetContext();
        Context.RenderTarget = TextureRenderTarget;
        Canvas = NewObject<UCanvas>(GetTransientPackage(), NAME_None);
        Size = FVector2D(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY);

        FTextureRenderTargetResource* RenderTargetResource = TextureRenderTarget->GameThread_GetRenderTargetResource();
        FCanvas* NewCanvas = new FCanvas(
            RenderTargetResource,
            nullptr,
            0, 0, 0,
            GMaxRHIFeatureLevel,
            FCanvas::CDM_ImmediateDrawing);
        Canvas->Init(TextureRenderTarget->SizeX, TextureRenderTarget->SizeY, nullptr, NewCanvas);
        Canvas->Update();

        Context.DrawEvent = new FDrawEvent();

        FName RTName = TextureRenderTarget->GetFName();
        FDrawEvent* DrawEvent = Context.DrawEvent;
        ENQUEUE_RENDER_COMMAND(BeginDrawEventCommand)(
            [RTName, DrawEvent, RenderTargetResource](FRHICommandListImmediate& RHICmdList) {
                RenderTargetResource->FlushDeferredResourceUpdate(RHICmdList);

                BEGIN_DRAW_EVENTF(
                    RHICmdList,
                    DrawCanvasToTarget,
                    (*DrawEvent),
                    *RTName.ToString());
            });
    }

    static void EndDrawCanvasToRenderTarget(UCanvas* Canvas, const FDrawToRenderTargetContext& Context) {

        if (Canvas->Canvas) {
            Canvas->Canvas->Flush_GameThread();
            delete Canvas->Canvas;
            Canvas->Canvas = nullptr;
        }

        if (Context.RenderTarget) {
            FTextureRenderTargetResource* RenderTargetResource = Context
                                                                 .RenderTarget->GameThread_GetRenderTargetResource();
            FDrawEvent* DrawEvent = Context.DrawEvent;
            ENQUEUE_RENDER_COMMAND(CanvasRenderTargetResolveCommand)(
                [RenderTargetResource, DrawEvent](FRHICommandList& RHICmdList) {
                    RHICmdList.CopyToResolveTarget(RenderTargetResource->GetRenderTargetTexture(),
                                                   RenderTargetResource->TextureRHI, FResolveParams());
                    STOP_DRAW_EVENT((*DrawEvent));
                    delete DrawEvent;
                }
            );

            // Remove references to the context now that we've resolved it, to avoid a crash when EndDrawCanvasToRenderTarget is called multiple times with the same context
            // const cast required, as BP will treat Context as an output without the const
            const_cast<FDrawToRenderTargetContext&>(Context) = FDrawToRenderTargetContext();
        }
    }
};

namespace {
    FLinearColor GetEdgeColor(const FGridFlowTilemapEdge& InEdge) {
        switch(InEdge.EdgeType) {
        case EGridFlowTilemapEdgeType::Wall:
            return FLinearColor(1, 0, 0.5f, 1.0f);
        case EGridFlowTilemapEdgeType::Door:
            return FLinearColor(0, 0, 1.0f, 1.0f);
        case EGridFlowTilemapEdgeType::Fence:
            return FLinearColor(1, 0.2f, 0.0f, 1.0f);
        default:
                return FLinearColor::Black;
        } 
    }
}

UTextureRenderTarget2D* FGridFlowTilemapRenderer::Create(UGridFlowTilemap* Tilemap, const FGridFlowTilemapRendererSettings& Settings) {
    if (!Tilemap) {
        return nullptr;
    }

    int32 TileSize = Settings.TileSize;
    if (Settings.bUseTextureTileSize && Settings.Textures.TileTexture) {
        TileSize = Settings.Textures.TileTexture->GetSurfaceWidth();
    }

    int32 TexWidth = Tilemap->GetWidth() * TileSize + 1;
    int32 TexHeight = Tilemap->GetHeight() * TileSize + 1;

    UTextureRenderTarget2D* Texture = FGridFlowCanvasHelper::CreateRenderTarget2D(
        TexWidth, TexHeight, RTF_RGBA8, Settings.BackgroundColor, false);

    FDrawToRenderTargetContext RenderContext;
    FVector2D CanvasSize;
    UCanvas* Canvas = nullptr;

    FGridFlowCanvasHelper::BeginDrawCanvasToRenderTarget(Texture, Canvas, CanvasSize, RenderContext);

    UTexture* OneWayTileTexL = Settings.Textures.OneWayTileTextureLeft;
    UTexture* OneWayTileTexR = Settings.Textures.OneWayTileTextureRight;
    UTexture* OneWayTileTexU = Settings.Textures.OneWayTileTextureUp;
    UTexture* OneWayTileTexD = Settings.Textures.OneWayTileTextureDown;

    if (!OneWayTileTexL) OneWayTileTexL = Cast<UTexture2D>(StaticLoadObject(
        UTexture2D::StaticClass(), nullptr, TEXT("/DungeonArchitect/Core/Textures/GridFlow/Tilemap/onewaydoor_left")));
    if (!OneWayTileTexR) OneWayTileTexR = Cast<UTexture2D>(StaticLoadObject(
        UTexture2D::StaticClass(), nullptr, TEXT("/DungeonArchitect/Core/Textures/GridFlow/Tilemap/onewaydoor_right")));
    if (!OneWayTileTexU) OneWayTileTexU = Cast<UTexture2D>(StaticLoadObject(
        UTexture2D::StaticClass(), nullptr, TEXT("/DungeonArchitect/Core/Textures/GridFlow/Tilemap/onewaydoor_up")));
    if (!OneWayTileTexD) OneWayTileTexD = Cast<UTexture2D>(StaticLoadObject(
        UTexture2D::StaticClass(), nullptr, TEXT("/DungeonArchitect/Core/Textures/GridFlow/Tilemap/onewaydoor_down")));

    if (!OneWayTileTexL || !OneWayTileTexR || !OneWayTileTexU || !OneWayTileTexD) {
        UE_LOG(LogGridFlowTileRenderer, Warning,
               TEXT("One Way Door Tile texture missing. They will not be indicated in the rendered tilemap"));
    }

    // Render the tiles
    FTexture* TileTextureResource = Settings.Textures.TileTexture
                                        ? Settings.Textures.TileTexture->Resource
                                        : GWhiteTexture;
    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            const FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            bool bCanUseCustomColor = Cell.CellType != EGridFlowTilemapCellType::Door
                && Cell.CellType != EGridFlowTilemapCellType::Wall;

            FLinearColor TileColor = FLinearColor::White;
            if (bCanUseCustomColor && Cell.bUseCustomColor) {
                TileColor = Cell.CustomColor;
                if (Settings.FuncCellSelected(Cell.ChunkCoord)) {
                    TileColor = GetSelectedCellColor(TileColor);
                }
            }
            else {
                TileColor = GetCellColor(Cell);
            }

            if (Cell.CellType == EGridFlowTilemapCellType::Custom) {
                TileColor = Cell.CustomCellInfo.DefaultColor;
            }

            if (!TileTextureResource) {
                continue;
            }

            int32 px = x * TileSize;
            int32 py = y * TileSize;
            FCanvasTileItem TileItem(FVector2D(px, py), TileTextureResource, FVector2D(TileSize, TileSize), TileColor);
            TileItem.BlendMode = SE_BLEND_AlphaBlend;
            Canvas->DrawItem(TileItem);

            if (Cell.bHasOverlay) {
                FLinearColor OverlayColor = Cell.Overlay.Color;
                float OverlaySize = TileSize * 0.5f;
                float ox = px + (TileSize - OverlaySize) * 0.5f;
                float oy = py + (TileSize - OverlaySize) * 0.5f;
                FCanvasTileItem OverlayItem(FVector2D(ox, oy), TileTextureResource, FVector2D(OverlaySize, OverlaySize),
                                            OverlayColor);
                OverlayItem.BlendMode = SE_BLEND_AlphaBlend;
                Canvas->DrawItem(OverlayItem);
            }
        }
    }

    // Render the grid lines
    {
        TArray<FVector4> Lines;
        for (int y = 0; y <= Tilemap->GetHeight(); y++) {
            FVector2D Start(0, y * TileSize);
            FVector2D Size(TexWidth - 1, 1);
            Lines.Add(FVector4(Start.X, Start.Y, Size.X, Size.Y));
        }
        for (int x = 0; x <= Tilemap->GetWidth(); x++) {
            FVector2D Start(x * TileSize, 0);
            FVector2D Size(1, TexHeight - 1);
            Lines.Add(FVector4(Start.X, Start.Y, Size.X, Size.Y));
        }

        for (const FVector4& Line : Lines) {
            FVector2D Start(Line.X, Line.Y);
            FVector2D End(Line.Z, Line.W);

            FCanvasTileItem LineItem(Start, TileTextureResource, End, FLinearColor(0, 0, 0, 0.2f));
            LineItem.BlendMode = SE_BLEND_Translucent;
            Canvas->DrawItem(LineItem);
        }
    }

    // Render the edges
    const FVector2D Thickness = FVector2D(1, 1);
    for (int y = 0; y <= Tilemap->GetHeight(); y++) {
        for (int x = 0; x <= Tilemap->GetWidth(); x++) {
            
            FVector2D Start = FVector2D(x, y)  * TileSize - Thickness;

            {
                const FGridFlowTilemapEdge& EdgeH = Tilemap->GetEdgeH(x, y);
                if (EdgeH.EdgeType != EGridFlowTilemapEdgeType::Empty) {
                    FVector2D SizeH = FVector2D(TileSize, 1) + Thickness * 2;
                    const FLinearColor WallColor = GetEdgeColor(EdgeH);
                    FCanvasTileItem LineItem(Start, TileTextureResource, SizeH, WallColor);
                    //LineItem.BlendMode = SE_BLEND_Translucent;
                    Canvas->DrawItem(LineItem);
                }
            }

            {
                const FGridFlowTilemapEdge& EdgeV = Tilemap->GetEdgeV(x, y);
                if (EdgeV.EdgeType != EGridFlowTilemapEdgeType::Empty) {
                    FVector2D SizeV = FVector2D(1, TileSize) + Thickness * 2;
                    const FLinearColor WallColor = GetEdgeColor(EdgeV);
                    FCanvasTileItem LineItem(Start, TileTextureResource, SizeV, WallColor);
                    //LineItem.BlendMode = SE_BLEND_Opaque;
                    Canvas->DrawItem(LineItem);
                }
            }
        }
    }
    
    // Draw one way door textures
    for (int y = 0; y < Tilemap->GetHeight(); y++) {
        for (int x = 0; x < Tilemap->GetWidth(); x++) {
            const FGridFlowTilemapCell& Cell = Tilemap->Get(x, y);
            const int32 px = x * TileSize;
            const int32 py = y * TileSize;
        
            bool bOneWayDoor = false;
            bool bDrawDoorBackground = false;
            FVector2D DrawOffset = FVector2D::ZeroVector;
            FGridFlowTilemapCellDoorInfo DoorInfo;
            if (Cell.CellType == EGridFlowTilemapCellType::Door && Tilemap->GetDoorMeta(FGridFlowTilemapCoord(Cell.TileCoord), DoorInfo)) {
                bOneWayDoor = DoorInfo.bOneWay;
            }
            else {
                FGridFlowTilemapEdge& EdgeH = Tilemap->GetEdgeH(x, y);
                if (Tilemap->GetDoorMeta(EdgeH.EdgeCoord, DoorInfo)) {
                    bOneWayDoor = DoorInfo.bOneWay;
                    bDrawDoorBackground = true;
                    DrawOffset = -FVector2D(0, TileSize) * 0.5f;
                }
                else {
                    FGridFlowTilemapEdge& EdgeV = Tilemap->GetEdgeV(x, y);
                    if (Tilemap->GetDoorMeta(EdgeV.EdgeCoord, DoorInfo)) {
                        bOneWayDoor = DoorInfo.bOneWay;
                        bDrawDoorBackground = true;
                        DrawOffset = -FVector2D(TileSize, 0) * 0.5f;
                    }
                }
            }
            
            if (bOneWayDoor) {
                if (bDrawDoorBackground) {
                    FLinearColor DoorTileBGColor = FLinearColor::Blue;
                    FCanvasTileItem BGTileItem(FVector2D(px, py) + DrawOffset, TileTextureResource, FVector2D(TileSize, TileSize), DoorTileBGColor);
                    BGTileItem.BlendMode = SE_BLEND_AlphaBlend;
                    Canvas->DrawItem(BGTileItem);
                }
                UTexture* OneWayTileTex = nullptr;
                if (DoorInfo.Angle == 0) OneWayTileTex = OneWayTileTexU;
                else if (DoorInfo.Angle == 90) OneWayTileTex = OneWayTileTexR;
                else if (DoorInfo.Angle == 180) OneWayTileTex = OneWayTileTexD;
                else if (DoorInfo.Angle == 270) OneWayTileTex = OneWayTileTexL;

                if (OneWayTileTex) {
                    FCanvasTileItem OneWayTileItem(FVector2D(px, py) + DrawOffset, OneWayTileTex->Resource,
                                                   FVector2D(TileSize, TileSize), FLinearColor::White);
                    OneWayTileItem.BlendMode = SE_BLEND_AlphaBlend;
                    Canvas->DrawItem(OneWayTileItem);
                }
            }
        }
    }
    
    FGridFlowCanvasHelper::EndDrawCanvasToRenderTarget(Canvas, RenderContext);

    return Texture;
}

FLinearColor FGridFlowTilemapRenderer::GetCellColor(const FGridFlowTilemapCell& Cell) {
    switch (Cell.CellType) {
    case EGridFlowTilemapCellType::Empty:
        return FLinearColor::Transparent;

    case EGridFlowTilemapCellType::Floor:
        return FLinearColor::White;

    case EGridFlowTilemapCellType::Door:
        return FLinearColor::Blue;

    case EGridFlowTilemapCellType::Wall:
        return FLinearColor::Gray;

    default:
        return FLinearColor(0, 0, 0, 1);
    }
}

FLinearColor FGridFlowTilemapRenderer::GetSelectedCellColor(FLinearColor InColor) {
    FLinearColor HSV = InColor.LinearRGBToHSV();
    HSV.G = FMath::Clamp(HSV.G * 3, 0.0f, 1.0f);
    return HSV.HSVToLinearRGB();
}

