// #pragma once
// #include "CoreMinimal.h"
// #include "GameLinuxForkHandler.generated.h"
//
// /**
//  * Fork handler for dedicated server on linux
//  *
//  * Flow:
//  * - Load all streaming levels on parent process
//  * - Enter 'fork' while circle to wait signal for forking child process
//  * - Unload unnecessary streaming levels for forked chile process
//  */
// UCLASS()
// class GAME_API UGameLinuxForkHandler : public UObject
// {
// 	GENERATED_BODY()
//
// private:
// 	UPROPERTY()
// 	UWorld* CurrentWorld;
//
// 	// Wait-to-load streaming levels count on parent
// 	int32 ParentStreamLevelLoadCount = 0;
//
// 	// Wait-to-unload streaming levels count on child
// 	int32 ChildStreamLevelUnloadCount = 0;
//
// public:
// 	/**
// 	 * @brief Try to start fork child process, this only supports on Linux & GameServer
// 	 * @callergraph This should be called from GameMode, such as 'InitGame' function
// 	 */
// 	void Start();
//
// private:
// 	/**
// 	 * @brief Load all streaming levels on parent process
// 	 */
// 	void ParentLoadAllStreamLevels();
//     
// 	UFUNCTION()
// 	void OnParentLoadStreamLevel(int32 LinkID);
//     
// 	/**
// 	 * @brief Try to enter 'fork' while circle to wait for forking child process 
// 	 */
// 	void TryFork();
//
// 	/**
// 	 * @brief Do some tasks for child process to make it run ok
// 	 *
// 	 * Tasks:
// 	 * - Shut down old 'NetDriver'
// 	 * - ReListen here to solve shared port error from parent process
// 	 * - Unload unnecessary streaming levels
// 	 */
// 	void OnPostForkChild();
//
// 	UFUNCTION()
// 	void OnChildUnloadStreamLevel(int32 LinkID);
//
//     
// 	static uint32 GetPID();
// 	static uint32 GetPPID();
// };
