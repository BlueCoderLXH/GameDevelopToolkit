// Fill out your copyright notice in the Description page of Project Settings.


#include "UDKGameInstance.h"

#include "UE4DevelopKit.h"
#include "UnLuaBase.h"

void UUDKGameInstance::Init()
{
	Super::Init();

	InitLua();
}

void UUDKGameInstance::InitLua()
{
	if (!UnLua::Startup())
	{
		UE_LOG(LogUDK, Error, TEXT("Failed to call 'UnLua::Startup'"));
		return;
	}

	lua_State* L = UnLua::GetState();
	if (!UnLua::RunFile(L, TEXT("Preload.lua")))
	{
		UE_LOG(LogUDK, Error, TEXT("Failed to call 'Preload.lua'"));
		return;
	}

	UE_LOG(LogUDK, Log, TEXT("Initialize Lua OK"));
}
