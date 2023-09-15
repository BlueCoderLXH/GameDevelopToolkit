// Tencent is pleased to support the open source community by making UnLua available.
// 
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the MIT License (the "License"); 
// you may not use this file except in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.

#include "UnLuaSettings.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"

int32 UUnLuaSettings::DebugTag = 0;
UUnLuaSettings::UUnLuaSettings(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PreBindClasses.Add(UBlueprintFunctionLibrary::StaticClass());
    PreBindClasses.Add(UAnimNotifyState::StaticClass());
    PreBindClasses.Add(UAnimNotify::StaticClass());
}

void UUnLuaSettings::EnablePrintLogCallUFuntion(bool bInEnable)
{
    if (bInEnable) 
    {
        DebugTag |= (1 << 0);
    }
    else
    {
        DebugTag &= ~(1 << 0);
    }
}

void UUnLuaSettings::EnablePrintLogCallArrayGet(bool bInEnable)
{
    if (bInEnable) 
    {
        DebugTag |= (1 << 1);
    }
    else
    {
        DebugTag &= ~(1 << 1);
    }
}

bool UUnLuaSettings::IsPrintLogCallUFuntion() 
{
    return (DebugTag & (1 << 0)) != 0;
}

bool UUnLuaSettings::IsPrintLogCallArrayGet()
{
    return (DebugTag & (1 << 1)) != 0;
}
