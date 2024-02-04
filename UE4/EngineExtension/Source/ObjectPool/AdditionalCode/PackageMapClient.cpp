void UPackageMapClient::ResetGuidAckStatus(UObject* InObj) const
{
	if (!IsValid(InObj))
	{
		UE_LOG(LogNetPackageMap, Error, TEXT("ResetGuidAckStatus: InObj is invalid!"));
		return;
	}

	const FNetworkGUID* ObjNetGuid = GuidCache->NetGUIDLookup.Find(InObj);
	if (!ObjNetGuid)
	{
		UE_LOG(LogNetPackageMap, Verbose, TEXT("ResetGuidAckStatus: NetGUIDLookup don't contains Obj:%s!"), *(InObj->GetFullName()));
		return;
	}

	ResetGuidAckStatus(*ObjNetGuid);
}

void UPackageMapClient::ResetGuidAckStatus(const FNetworkGUID& InNetworkGUID) const
{
	if (!InNetworkGUID.IsValid())
	{
		UE_LOG(LogNetPackageMap, Error, TEXT("ResetGuidAckStatus: InNetworkGUID:%s is invalid!"), *(InNetworkGUID.ToString()));
		return;
	}

	int32* ExpectedNetGUIDAckStatusPtr = OverrideAckState->NetGUIDAckStatus.Find(InNetworkGUID);
	// if acked, reset it to be not acked
	// if pending or not acked, ignore
	if (ExpectedNetGUIDAckStatusPtr && *ExpectedNetGUIDAckStatusPtr == GUID_PACKET_ACKED)
	{
		*ExpectedNetGUIDAckStatusPtr = GUID_PACKET_NOT_ACKED;
	}
}