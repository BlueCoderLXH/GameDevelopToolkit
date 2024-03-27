// Fill out your copyright notice in the Description page of Project Settings.


#include "VMTest.h"
#include "LuaEnv.h"
#include "UnLuaLegacy.h"

TAutoConsoleVariable<int32> CVarVMTest_OutLoops(
	TEXT("vmtest.outloops"),
	100,
	TEXT("vmtest.outloops"),
	ECVF_Default);

TAutoConsoleVariable<int32> CVarVMTest_InLoops(
	TEXT("vmtest.inloops"),
	10000,
	TEXT("vmtest.inloops"),
	ECVF_Default);

AVMTest::AVMTest()
{
	PrimaryActorTick.bCanEverTick = 1;
}

int32 AVMTest::GetInLoops()
{
	return CVarVMTest_InLoops.GetValueOnGameThread();
}

void AVMTest::BeginPlay()
{
	Super::BeginPlay();

	if (bNativizedMode)
	{
		RunBpNativizedWrap();
	}
	else
	{
		RunBpWrap();

		RunCppWrap();

		RunLuaWrap();
	}
}

void AVMTest::RunBpNativizedWrap()
{
	int32 TestResult = 0;

	// Run Blueprint
	{
		QUICK_SCOPE_CYCLE_COUNTER(AVMTest_RunBp_Nativized);

		// QUICK_SCOPE_TIME(AVMTest_RunBp_Nativized);
		for (int32 i = 0; i < CVarVMTest_OutLoops.GetValueOnGameThread(); i++)
		{
			TestResult = RunBp();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AVMTest RunBp(Nativized) Result:%d"), TestResult);	
}

void AVMTest::RunBpWrap()
{
	int32 TestResult = 0;

	// Run Blueprint
	{
		QUICK_SCOPE_CYCLE_COUNTER(AVMTest_RunBp);

		// QUICK_SCOPE_TIME(AVMTest_RunBp);
		for (int32 i = 0; i < CVarVMTest_OutLoops.GetValueOnGameThread(); i++)
		{
			TestResult = RunBp();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AVMTest RunBp(Normal) Result:%d"), TestResult);		
}

void AVMTest::RunCppWrap()
{
	int32 TestResult = 0;

	// Run C++ native
	{
		QUICK_SCOPE_CYCLE_COUNTER(AVMTest_RunCpp);

		// QUICK_SCOPE_TIME(AVMTest_RunCpp);
		for (int32 i = 0; i < CVarVMTest_OutLoops.GetValueOnGameThread(); i++)
		{
			TestResult = RunCpp();
		}
	}
	UE_LOG(LogTemp, Log, TEXT("AVMTest RunCpp Result:%d"), TestResult);	
}

void AVMTest::RunLuaWrap()
{
	int32 TestResult = 0;

	// Run Lua
	{
		QUICK_SCOPE_CYCLE_COUNTER(AVMTest_RunLua);

		// QUICK_SCOPE_TIME(AVMTest_RunLua);
		for (int32 i = 0; i < CVarVMTest_OutLoops.GetValueOnGameThread(); i++)
		{
			TestResult = RunLua();
		}
	}
	UE_LOG(LogTemp, Log, TEXT("AVMTest RunLua Result:%d"), TestResult);	
}

int32 AVMTest::RunCpp()
{
	int32 Sum = 0;
	for (int32 Index = 1; Index <= GetInLoops(); Index++)
	{
		Sum += Index;
	}
	return Sum;
}

int32 AVMTest::RunLua()
{
	lua_State* L = UnLua::GetState();
	UnLua::FLuaRetValues Ret = UnLua::CallTableFunc(L, "VMTest", "RunLua");
	if (Ret.Num() > 0)
	{
		return Ret[0].Value<int32>();
	}
	return -1;
}
