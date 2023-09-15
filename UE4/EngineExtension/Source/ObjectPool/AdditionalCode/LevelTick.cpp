/**
* Update the level after a variable amount of time, DeltaSeconds, has passed.
* All child actors are ticked after their owners have been ticked.
*/
void UWorld::Tick( ELevelTick TickType, float DeltaSeconds )
{
	SCOPE_TIME_GUARD(TEXT("UWorld::Tick"));
	SCOPED_NAMED_EVENT(UWorld_Tick, FColor::Orange);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(WorldTickMisc);
	CSV_SCOPED_SET_WAIT_STAT(WorldTickMisc);

	if (GIntraFrameDebuggingGameThread)
	{
		return;
	}

	TickObjectPool(DeltaSeconds);

	...
}
