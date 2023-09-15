//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerInput.h"
#include "LevelEditor.h"
#include "CustomInputMapping.generated.h"

struct FInputActionKeyMapping;
struct FInputAxisKeyMapping;

UCLASS()
class ADACustomInputConfigBinder : public AActor {
	GENERATED_BODY()

public:
	ADACustomInputConfigBinder();
	bool ProjectContainsMissingInput(TArray<FInputActionKeyMapping>& OutMissingActionMappings, TArray<FInputAxisKeyMapping>& OutMissingAxisMappings) const;
	bool ProjectContainsMissingInput() const;
	void BindMissingInput(bool bShowUserPrompt) const;

public:
	/** List of Action Mappings */
	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TArray<FInputActionKeyMapping> ActionMappings;

	/** List of Axis Mappings */
	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TArray<FInputAxisKeyMapping> AxisMappings;

};


class FDACustomInputConfigBinderHook : public TSharedFromThis<FDACustomInputConfigBinderHook> {
public:
	~FDACustomInputConfigBinderHook();
	void AddHook();
	void RemoveHook();

private:
	void OnMapChanged(UWorld* World, EMapChangeType MapChangeType);

private:
	FDelegateHandle HookHandle;
};

