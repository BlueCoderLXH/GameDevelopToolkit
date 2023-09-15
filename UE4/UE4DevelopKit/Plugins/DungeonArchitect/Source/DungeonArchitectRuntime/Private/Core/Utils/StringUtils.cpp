//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Utils/StringUtils.h"


TArray<FString> FDAStringUtils::Split(const FString& InText, const FString& InSeparator) {
    TArray<FString> Result;
    FString Left, Right;
    FString RemainingText = InText;
    while (RemainingText.Split(",", &Left, &Right)) {
        Result.Add(Left);
        RemainingText = Right;
    }
    Result.Add(RemainingText);
    return Result;
}

