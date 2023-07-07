//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Brushes/SlateImageBrush.h"
#include "Styling/SlateBrush.h"

struct FLaunchPadTextResource {
    FString Path;
    FString Value;
};

typedef TSharedPtr<struct FLaunchPadTextResource> FLaunchPadTextResourcePtr;

class ILaunchPadDataSource {
public:
    virtual ~ILaunchPadDataSource() {
    }

    virtual FLaunchPadTextResourcePtr GetText(const FString& InPath) = 0;
    const FSlateBrush* GetImageThumb(const FString& InPath);
    const FSlateBrush* GetImage(const FString& InPath);
    const FSlateBrush* GetImage(const FString& InPath, const FVector2D& InImageSize);

protected:
    virtual const FSlateBrush* GetImageImpl(const FString& InPath, const FVector2D& InImageSize) = 0;
};

typedef TSharedPtr<class ILaunchPadDataSource> ILaunchPadDataSourcePtr;

class FLaunchPadResourceFolderDataSource : public ILaunchPadDataSource {
public:
    FLaunchPadResourceFolderDataSource();
    virtual FLaunchPadTextResourcePtr GetText(const FString& InPath) override;

protected:
    virtual const FSlateBrush* GetImageImpl(const FString& InPath, const FVector2D& InImageSize) override;

private:
    FString BaseTextDir;
    FString BaseImageDir;
    TMap<uint32, FSlateImageBrush> Brushes;
};

