#include "CoreMinimal.h"
#include "GameBaseMode.h"
#include "GameLinuxForkHandler.h"
#include "OnlineGameModeBase.generated.h"

/**
 * 联机GameMode基类(联机模式专用)
 */
class AGamePlayerController;
UCLASS()
class GAME_API AOnlineGameModeBase : public AGameBaseMode
{
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FName> RequiredSubStreamLevels;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

private:
	// ***** Fork Begin
	UPROPERTY()
	UGameLinuxForkHandler* ForkHandler;
	// ***** Fork End 
}
