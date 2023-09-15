// #include "OnlineGameModeBase.h"
//
// void AOnlineGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) {
// 	Super::InitGame(MapName, Options, ErrorMessage);
//     
// 	// #if PLATFORM_LINUX && UE_SERVER
// 	ForkHandler = NewObject<UGameLinuxForkHandler>(GetWorld());
// 	if (ForkHandler)
// 	{
// 		ForkHandler->Start();
// 	}
// 	// #endif
// }