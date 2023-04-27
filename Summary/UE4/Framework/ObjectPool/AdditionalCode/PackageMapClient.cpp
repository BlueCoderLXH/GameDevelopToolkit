void UPackageMapClient::ResetGuidAckStatus(UObject* InObj) const
{
	if (!IsValid(InObj))
	{
		return;
	}

	const FNetworkGUID* ObjNetGuid = GuidCache->NetGUIDLookup.Find(InObj);
	if (!ObjNetGuid)
	{
		return;
	}

	ResetGuidAckStatus(*ObjNetGuid);
}

void UPackageMapClient::ResetGuidAckStatus(const FNetworkGUID& InNetworkGUID) const
{
	if (!InNetworkGUID.IsValid())
	{
		return;
	}

	int32* ExpectedNetGUIDAckStatus = OverrideAckState->NetGUIDAckStatus.Find(InNetworkGUID);
	if (ExpectedNetGUIDAckStatus)
	{
		*(ExpectedNetGUIDAckStatus) = GUID_PACKET_NOT_ACKED;
	}
}