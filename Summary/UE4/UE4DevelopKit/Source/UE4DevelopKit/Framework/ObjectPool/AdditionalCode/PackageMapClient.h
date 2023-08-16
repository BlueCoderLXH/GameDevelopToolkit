UCLASS(transient)
class ENGINE_API UPackageMapClient : public UPackageMap
{
	void ResetGuidAckStatus(UObject* InObj) const;
	void ResetGuidAckStatus(const FNetworkGUID& InNetworkGUID) const;	
}
