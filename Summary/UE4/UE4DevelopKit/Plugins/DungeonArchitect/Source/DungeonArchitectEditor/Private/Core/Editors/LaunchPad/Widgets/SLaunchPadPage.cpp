//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/LaunchPad/Widgets/SLaunchPadPage.h"

#include "Core/Editors/LaunchPad/Data/LaunchPadData.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadDataParsers.h"
#include "Core/Editors/LaunchPad/Data/LaunchPadResource.h"
#include "Core/Editors/LaunchPad/Widgets/Pages/SLaunchPadPage_CardGrid.h"
#include "Core/Editors/LaunchPad/Widgets/Pages/SLaunchPadPage_Details.h"

DEFINE_LOG_CATEGORY_STATIC(LogLaunchPadPage, Log, All);

#define LOCTEXT_NAMESPACE "LaunchPadPages"

///////////////////////// FLaunchPadPageWidgetFactory ///////////////////////// 
namespace {
    template <typename TWidget, typename TData>
    TSharedPtr<SLaunchPadPage> CreateWidget(TSharedPtr<FJsonObject> JsonObject) {
        TData LayoutData;
        if (FLaunchPadDataParser::Parse(JsonObject, LayoutData)) {
            return SNew(TWidget, LayoutData);
        }
        UE_LOG(LogLaunchPadPage, Error, TEXT("Failed to parse page layout data"));
        return nullptr;
    }
}

TSharedPtr<SLaunchPadPage> FLaunchPadPageWidgetFactory::Create(ILaunchPadDataSourcePtr InDataSource,
                                                               const FString& InPath) {
    FLaunchPadTextResourcePtr Resource = InDataSource->GetText(InPath);
    if (!Resource.IsValid()) {
        UE_LOG(LogLaunchPadPage, Error, TEXT("Cannot download page data. Page Path: [%s]"), *InPath);
        return nullptr;
    }

    TSharedPtr<FJsonObject> JsonObject = FLaunchPadDataParser::ParseJson(Resource->Value);
    if (!JsonObject.IsValid()) {
        UE_LOG(LogLaunchPadPage, Error, TEXT("Failed to parse page json data. Page Path: [%s]"), *InPath);
        return nullptr;
    }

    FString Layout = JsonObject->GetStringField("layout");
    if (Layout == "CardGrid") {
        return CreateWidget<SLaunchPadPage_CardGrid, FLaunchPadPageLayout_CardGrid>(JsonObject);
    }
    if (Layout == "Details") {
        return CreateWidget<SLaunchPadPage_Details, FLaunchPadPageLayout_Details>(JsonObject);
    }
    UE_LOG(LogLaunchPadPage, Error, TEXT("Unsupported page layout type: [%s], Page Path: [%s]"), *Layout, *InPath);
    return nullptr;
}

#undef LOCTEXT_NAMESPACE

