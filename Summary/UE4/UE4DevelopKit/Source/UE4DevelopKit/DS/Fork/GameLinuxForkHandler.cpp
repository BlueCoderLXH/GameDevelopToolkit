// #include "GameLinuxForkHandler.h"
//
// #include "OnlineGameModeBase.h"
// #include "Engine/LevelStreaming.h"
// #include "Kismet/GameplayStatics.h"
//
// uint32 UGameLinuxForkHandler::GetPID()
// {
//     return FGenericPlatformProcess::GetCurrentProcessId();
// }
//
// uint32 UGameLinuxForkHandler::GetPPID()
// {
// #if PLATFORM_LINUX && UE_SERVER
//     return getppid();
// #else
//     return 0;
// #endif
// }
//
// void UGameLinuxForkHandler::Start()
// {
//     CurrentWorld = GetWorld();
//     check(IsValid(CurrentWorld));
//
//     ParentLoadAllStreamLevels();
// }
//
// void UGameLinuxForkHandler::ParentLoadAllStreamLevels()
// {
//     for (ULevelStreaming* StreamLevel : CurrentWorld->GetStreamingLevels())
//     {
//         if (StreamLevel)
//         {
//             ParentStreamLevelLoadCount++;
//
//             const FName& StreamLevelPackageName = StreamLevel->GetWorldAssetPackageFName();
//             UE_LOG(LogGameMode, Log, TEXT("[P] UGameLinuxForkHandler::ParentLoadAllStreamLevels PID:%d load streamlevel:%s"),
//                 GetPID(), *(StreamLevelPackageName.ToString()));
//
//             FLatentActionInfo StreamLevelAction;
//             StreamLevelAction.CallbackTarget = this;
//             StreamLevelAction.ExecutionFunction = TEXT("OnParentLoadStreamLevel");
//             StreamLevelAction.Linkage = ParentStreamLevelLoadCount;
//             StreamLevelAction.UUID = ParentStreamLevelLoadCount;
//             
//             UGameplayStatics::LoadStreamLevel(this, StreamLevelPackageName, true, false, StreamLevelAction);
//         }
//     }
//
//     UE_LOG(LogGameMode, Log, TEXT("[P] UGameLinuxForkHandler::ParentLoadAllStreamLevels PID:%d Prepare load streamlevel count:%d"),
//         GetPID(), ParentStreamLevelLoadCount);
// }
//
// void UGameLinuxForkHandler::OnParentLoadStreamLevel(int32 LinkID)
// {
//     ParentStreamLevelLoadCount--;
//
//     if (!ParentStreamLevelLoadCount)
//     {
//         // Fork child process when streaming levels finish loading
//         TryFork();
//     }
// }
//
// void UGameLinuxForkHandler::TryFork()
// {
//     bool bUseFork = bUseFork = FParse::Param(FCommandLine::Get(), TEXT("UseFork"));
//     UE_LOG(LogGameMode, Log, TEXT("[P] UGameLinuxForkHandler::TryFork UseFork:%s!"), bUseFork ? TEXT("True") : TEXT("False"));
//     if (!bUseFork)
//     {
//         return;
//     }
//     
//     const FGenericPlatformProcess::EWaitAndForkResult ForkResult = FPlatformProcess::WaitAndFork();
//     switch (ForkResult)
//     {
//     case FGenericPlatformProcess::EWaitAndForkResult::Error:
//         UE_LOG(LogGameMode, Fatal, TEXT("[P] UGameLinuxForkHandler::TryFork Failed to fork child process!"));
//         break;
//         
//     case FGenericPlatformProcess::EWaitAndForkResult::Parent:
//         RequestEngineExit(TEXT("[Parent] GameServer normally quit."));
//         UE_LOG(LogGameMode, Log, TEXT("[P] UGameLinuxForkHandler::TryFork parent process out!"));
//         break;
//         
//     case FGenericPlatformProcess::EWaitAndForkResult::Child:
//         OnPostForkChild();
//         break;
//     }
// }
//
// void UGameLinuxForkHandler::OnPostForkChild()
// {
//     UE_LOG(LogGameMode, Log, TEXT("[C] UGameLinuxForkHandler::OnPostForkChild PPID:%d PID:%d!"), GetPPID(), GetPID());
//
//     // 1. Shut down old 'NetDriver' first
//     GEngine->ShutdownWorldNetDriver(CurrentWorld);
//
//     FURL NewUrl;
//     NewUrl.Port = 20520;
//     // 2. ReListen here to solve shared port error from parent process
//     CurrentWorld->Listen(NewUrl);
//
//     AOnlineGameModeBase* OnlineGameMode = Cast<AOnlineGameModeBase>(CurrentWorld->GetAuthGameMode());
//     if (!OnlineGameMode)
//     {
//         UE_LOG(LogGameMode, Fatal, TEXT("[C] UGameLinuxForkHandler::OnPostForkChild PPID:%d PID:%d!"), GetPPID(), GetPID());
//         return;
//     }
//
//     // 3. Unload unnecessary streaming levels
//     const TArray<ULevelStreaming*>& AllStreamLevels = CurrentWorld->GetStreamingLevels();
//     for (const ULevelStreaming* StreamLevel : AllStreamLevels)
//     {
//         const FName& StreamLevelName = StreamLevel->GetWorldAssetPackageFName();
//
//         UE_LOG(LogGameMode, Log, TEXT("[C] UGameLinuxForkHandler::OnPostForkChild streamlevel name:%s WorldAsset:%s Package:%s"),
//             *(StreamLevel->GetName()), *(StreamLevel->GetWorldAssetPackageName()), *(StreamLevel->PackageNameToLoad.ToString()));
//
//         if (!OnlineGameMode->RequiredSubStreamLevels.Contains(StreamLevelName))
//         {
//             FLatentActionInfo StreamLevelAction;
//             StreamLevelAction.CallbackTarget = this;
//             StreamLevelAction.ExecutionFunction = TEXT("OnChildUnloadStreamLevel");
//             StreamLevelAction.Linkage = ChildStreamLevelUnloadCount++;
//             StreamLevelAction.UUID = ChildStreamLevelUnloadCount;
//             
//             UGameplayStatics::UnloadStreamLevel(this, StreamLevelName, StreamLevelAction, false);            
//         
//             UE_LOG(LogGameMode, Log, TEXT("[C] UGameLinuxForkHandler::OnPostForkChild Remove streamlevel:%s"), *(StreamLevelName.ToString()));
//         }
//     }
//
//     UE_LOG(LogGameMode, Log, TEXT("[C] UGameLinuxForkHandler::OnPostForkChild Unload stream level count:%d"), ChildStreamLevelUnloadCount);
// }
//
// void UGameLinuxForkHandler::OnChildUnloadStreamLevel(int32 LinkID)
// {
//     ChildStreamLevelUnloadCount--;
//     if (!ChildStreamLevelUnloadCount)
//     {
//         // Now it's ok for child DS process connecting clients
//         UE_LOG(LogGameMode, Log, TEXT("[C] UGameLinuxForkHandler::OnChildUnloadStreamLevel Finish"));
//     }
// }
