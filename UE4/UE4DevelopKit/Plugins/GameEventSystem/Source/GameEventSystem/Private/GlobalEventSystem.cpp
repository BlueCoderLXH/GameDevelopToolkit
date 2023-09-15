#include "GlobalEventSystem.h"

TArray<FGameEventType> FGlobalEventSystem::EventIDs;

int FGlobalEventSystem::ReceiveEventFromLua(lua_State* L)
{
	FGameEventData EventData;
	if (!ParseEventData(L, EventData))
	{
		return 0;
	}

	DispatchToCpp(EventData);
	return 1;
}

bool FGlobalEventSystem::CheckLuaParams(lua_State* L)
{
	const int32 NumParams = lua_gettop(L);
	if (NumParams < 1)
	{
		UE_LOG(LogGameEventSystem, Error, TEXT("CheckLuaParams: There should be at least one param for 'EventID'"));
		return false;
	}
	
	// The first param should always be 'EventID'
	if (lua_type(L, 1) != LUA_TSTRING)
	{
		UE_LOG(LogGameEventSystem, Error, TEXT("CheckLuaParams: The first param should be a string as the 'EventID' Param"));
		return false;
	}

	for (int32 i = 2; i <= NumParams; i++)
	{
		const int32 ParamType = lua_type(L, i);
		switch (ParamType)
		{
		case LUA_TBOOLEAN:			
		case LUA_TNUMBER:	
		case LUA_TSTRING:		
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
			break;
		default:
			UE_LOG(LogGameEventSystem, Error, TEXT("CheckLuaParams: the type:%d of Param[%d] is unsupported!"), ParamType, i);
			return false;
		}
	}

	return true;
}

bool FGlobalEventSystem::ParseEventData(lua_State* L, FGameEventData& EventData)
{
	if (!CheckLuaParams(L))
	{
		return false;
	}
	
	EventData.EventID = FGameEventType(lua_tostring(L, 1));
	if (!CheckEventID(EventData.EventID))
	{
		return false;
	}

	const int32 NumParams = lua_gettop(L);
	// Parse the event data params
	for (int32 i = 2; i <= NumParams; i++)
	{
		const int32 ParamType = lua_type(L, i);
		if (ParamType == LUA_TBOOLEAN)
		{
			const bool bValue = lua_toboolean(L, i) ? true : false;
			EventData.PushParam(bValue);
		}
		else if (ParamType == LUA_TNUMBER)
		{
			if (lua_isinteger(L, i))
			{
				const int64 IntValue = lua_tointeger(L, i);
				EventData.PushParam(IntValue);
			}
			else
			{
				const double FloatValue = lua_tonumber(L, i);
				EventData.PushParam(FloatValue);					
			}
		}
		else if (ParamType == LUA_TSTRING)
		{
			const char* StrValue = lua_tostring(L, i);
			EventData.PushParam(StrValue);
		}
		else if (ParamType == LUA_TUSERDATA || ParamType == LUA_TLIGHTUSERDATA)
		{
			if (void* ContainerPtr = UnLua::GetScriptContainerPointer(L, i))
			{
				EventData.PushParam(ContainerPtr);
			}
			else if (UObject* ObjectPtr = UnLua::GetUObject(L, i, true))
			{
				EventData.PushParam(ObjectPtr);
			}
			else
			{
				void* UserDataPtr = UnLua::GetPointer(L, i);
				EventData.PushParam(UserDataPtr);
			}
		}
		else
		{
			// Never allow passing the unsupported type of params
			UE_LOG(LogGameEventSystem, Error, TEXT("ParseEventData: the type:%d of Param[%d] is unsupported!"), ParamType, i);
		}
	}

	return true;
}