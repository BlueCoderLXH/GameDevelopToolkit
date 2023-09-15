#include "UnLuaEx.h"
#include "LuaCore.h"
#include "Engine/World.h"
#include "UMG/Public/Blueprint/UserWidget.h"

#if WITH_EDITOR
#include "Editor/UnrealEdEngine.h"
#include "Engine/GameEngine.h"
#endif

static UUserWidget* LoadUI(const FSoftClassPath& Path) {
	TSubclassOf<UUserWidget> WidgetClass = Path.TryLoadClass<UUserWidget>();
	UWorld* World = nullptr;

#if WITH_EDITOR
	UGameInstance* GameInstance = nullptr;
	if (GEngine->IsEditor()) {
		World = Cast<UUnrealEdEngine>(GEngine)->PlayWorld;
	} else {
		World = Cast<UGameEngine>(GEngine)->GetGameWorld();
	}
#else
	World = GWorld;
#endif

	return IsValid(World) ? CreateWidget<UUserWidget, UWorld>(World, WidgetClass) : nullptr;
}

static UUserWidget* LoadUIByPath(const FString& Path) {
    FSoftClassPath ClassPath(Path);
    if (ClassPath.IsValid())
        return LoadUI(ClassPath);
    return nullptr;
}

static UUserWidget* LoadUIAndSetName(const FSoftClassPath& Path, const FName& InName) {
	TSubclassOf<UUserWidget> WidgetClass = Path.TryLoadClass<UUserWidget>();
	UWorld* World = nullptr;

#if WITH_EDITOR
	UGameInstance* GameInstance = nullptr;
	if (GEngine->IsEditor()) {
		World = Cast<UUnrealEdEngine>(GEngine)->PlayWorld;
	} else {
		World = Cast<UGameEngine>(GEngine)->GetGameWorld();
	}
#else
	World = GWorld;
#endif

	return IsValid(World) ? CreateWidget<UUserWidget, UWorld>(World, WidgetClass, InName) : nullptr;
}

EXPORT_FUNCTION(UUserWidget*, LoadUI, const FSoftClassPath&)
EXPORT_FUNCTION(UUserWidget*, LoadUIByPath, const FString&)
EXPORT_FUNCTION(UUserWidget*, LoadUIAndSetName, const FSoftClassPath&, const FName&)
