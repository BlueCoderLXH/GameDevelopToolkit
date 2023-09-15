//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class SWidget;
struct FLaunchPadPageActionData;
class USnapGridFlowModuleDatabase;

class FLaunchPadActions {
public:
    static void Execute(const FLaunchPadPageActionData& Data, TSharedPtr<SWidget> HostWidget);
    static void Exec_OpenURL(const FString& URL);

private:
    static void Exec_OpenFolder(const FString& InPath);
    static bool Exec_OpenScene(const FString& InPath);
    static bool Exec_OpenTheme(const FString& InPath);
    static bool Exec_OpenSnapFlow(const FString& InPath);
    static bool Exec_OpenGridFlow(const FString& InPath);
    static bool Exec_OpenSnapGridFlow(const FString& InPath, USnapGridFlowModuleDatabase* InModuleDatabase = nullptr);
    static bool Exec_CloneScene(const FString& InPath);
    static bool Exec_CloneSceneAndBuild(const FString& InPath);
    static bool Exec_CloneTheme(const FString& InPath);
    static bool Exec_CloneSnapFlow(const FString& InPath);
    static bool Exec_CloneGridFlow(const FString& InPath);
    static bool Exec_CloneSnapGridFlow(const FString& InPath);
    static void Exec_Documentation(const FString& InPath);
    static void Exec_Video(const FString& InPath);
    static void Exec_LauncherURL(const FString& InPath);
    static void Exec_AddStarterContent(TSharedPtr<SWidget> HostWidget);


};

