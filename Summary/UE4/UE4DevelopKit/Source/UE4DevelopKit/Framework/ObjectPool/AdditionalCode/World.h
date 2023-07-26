// UCLASS(customConstructor, config=Engine)
// class ENGINE_API UWorld final : public UObject, public FNetworkNotify
// {
// private:
// 	UPROPERTY(Config)
// 	uint8 bUseObjectPool;
//
// 	UPROPERTY()
// 	UObjectPoolSystem* ObjectPoolSystem;
//
// 	void InitObjectPool();
//
// 	void DestroyObjectPool();
// 	
// public:
// 	UFUNCTION(BlueprintCallable, Category="ObjectPool|World")
// 	UObjectPoolSystem* GetObjectPoolSystem() const { return bUseObjectPool ? ObjectPoolSystem : nullptr; }	
// };
