#pragma once

#include "CoreMinimal.h"

typedef FName FGameEventType;

/**
 * EGameEventParamType
 */
enum class EGameEventParamType : uint8
{
	None = 0,
	Any,
	Bool,
	Integer,
	Float,
	String,
	Object
};

/**
 * FGameEventParamWrapper
 *
 * Wrap the event parameter with support type
 */
struct GAMEEVENTSYSTEM_API FGameEventParamWrapper
{
	EGameEventParamType Type;

	// Static
	union
	{
		// Value on stack
		bool bValue;
		int64 IntValue;
		double FloatValue;

		// Value on heap
		void* DataPtr = nullptr;		
	};

	// Dynamic
	FString StrValue;

	FGameEventParamWrapper()
	{
		bValue = false;
		IntValue = 0;
		FloatValue = 0;
		DataPtr = nullptr;
		StrValue = "";
		Type = EGameEventParamType::None;		
	}

	~FGameEventParamWrapper()
	{
		bValue = false;
		IntValue = 0;
		FloatValue = 0;
		DataPtr = nullptr;
		StrValue = "";
		Type = EGameEventParamType::None;
	}

	bool GetBool() const
	{
		checkf(Type == EGameEventParamType::Bool, TEXT("FGameEventParamWrapper:GetBool Type:%s isn't a bool"), *GetTypeString());
		return bValue;
	}

	int8 GetByte() const
	{
		checkf(Type == EGameEventParamType::Integer, TEXT("FGameEventParamWrapper:GetInteger Type:%s isn't a int"), *GetTypeString());
		return IntValue;
	}

	int16 GetShort() const
	{
		checkf(Type == EGameEventParamType::Integer, TEXT("FGameEventParamWrapper:GetInteger Type:%s isn't a int"), *GetTypeString());
		return IntValue;		
	}

	int32 GetInt() const
	{
		checkf(Type == EGameEventParamType::Integer, TEXT("FGameEventParamWrapper:GetInteger Type:%s isn't a int"), *GetTypeString());
		return IntValue;
	}

	int64 GetLong() const
	{
		checkf(Type == EGameEventParamType::Integer, TEXT("FGameEventParamWrapper:GetInteger Type:%s isn't a int"), *GetTypeString());
		return IntValue;
	}

	float GetFloat() const
	{
		checkf(Type == EGameEventParamType::Float, TEXT("FGameEventParamWrapper:GetFloat Type:%s isn't a float"), *GetTypeString());
		return FloatValue;
	}

	double GetDouble() const
	{
		checkf(Type == EGameEventParamType::Float, TEXT("FGameEventParamWrapper:GetDouble Type:%s isn't a float"), *GetTypeString());
		return FloatValue;		
	}

	const FString& GetString() const
	{
		checkf(Type == EGameEventParamType::String, TEXT("FGameEventParamWrapper:GetString Type:%s isn't a FString"), *GetTypeString());
		return StrValue;
	}

	template<typename T>
	std::enable_if_t<std::is_base_of<UObject, T>::value, T*> GetUObject() const
	{
		checkf(Type == EGameEventParamType::Object || Type == EGameEventParamType::Any, TEXT("FGameEventParamWrapper:GetObject Type:%s isn't a UObject"), *GetTypeString());
		return static_cast<T*>(DataPtr);
	}

	template<typename T>
	T* GetAny() const
	{
		checkf(Type == EGameEventParamType::Any || Type == EGameEventParamType::Object, TEXT("FGameEventParamWrapper:GetAny Type:%s isn't a Any"), *GetTypeString());
		T* AnyObjectPtr = static_cast<T*>(DataPtr);
		// checkf(AnyObjectPtr, TEXT("FGameEventParamWrapper:GetAny AnyObjectPtr is null!"));
		return AnyObjectPtr;
	}

private:
	FString GetTypeString() const
	{
		FString Ret;
		switch (Type)
		{
		case EGameEventParamType::Any:
			Ret = TEXT("Any");
			break;			
		case EGameEventParamType::Bool:
			Ret = TEXT("Bool");
			break;
		case EGameEventParamType::Integer:
			Ret = TEXT("Integer");
			break;			
		case EGameEventParamType::Float:
			Ret = TEXT("Float");
			break;			
		case EGameEventParamType::String:
			Ret = TEXT("FString");
			break;		
		case EGameEventParamType::Object:
			Ret = TEXT("UObject");
			break;
		case EGameEventParamType::None:
			Ret = TEXT("None");
			break;
		}
		return Ret;
	}
};

/**
 * FGameEventData
 *
 * The array of FGameEventParamWrapper
 */
struct GAMEEVENTSYSTEM_API FGameEventData
{
	FGameEventType EventID;

	template<typename T>
	void PushParam(const T& AnyObject)
	{	
		PushParam(const_cast<T*>( &AnyObject));
	}	

	template<typename T>
	void PushParam(T& AnyObject)
	{
		PushParam(&AnyObject);
	}

	template<typename T>
	void PushParam(T&& AnyObject)
	{
		PushParam(&AnyObject);
	}

	template<typename T>
	void PushParam(const T* AnyObject)
	{
		PushParam(const_cast<T*>(AnyObject));
	}

	template<typename T>
	void PushParam(T* AnyObject)
	{
		FGameEventParamWrapper ParamWrapper;

		ParamWrapper.Type = EGameEventParamType::Any;		
		ParamWrapper.DataPtr = AnyObject;
		Params.Add(ParamWrapper);
	}

	void PushParam(std::nullptr_t NullPtr)
	{
		FGameEventParamWrapper ParamWrapper;

		ParamWrapper.Type = EGameEventParamType::Any;		
		ParamWrapper.DataPtr = nullptr;
		Params.Add(ParamWrapper);		
	}

	void PushParam(const UObject* Object)
	{
		PushParam(const_cast<UObject*>(Object));
	}

	void PushParam(UObject* Object)
	{
		FGameEventParamWrapper ParamWrapper;

		ParamWrapper.Type = EGameEventParamType::Object;
		ParamWrapper.DataPtr = Object;
		Params.Add(ParamWrapper);
	}

	void PushParam(bool bValue)
	{
		FGameEventParamWrapper ParamWrapper;
		ParamWrapper.Type = EGameEventParamType::Bool;
		ParamWrapper.bValue = bValue;
		Params.Add(ParamWrapper);
	}

	void PushParam(int8 ByteValue)
	{
		PushParam((int64)ByteValue);
	}
	
	void PushParam(int16 ShortValue)
	{
		PushParam((int64)ShortValue);
	}

	void PushParam(int32 IntValue)
	{
		PushParam((int64)IntValue);
	}

	void PushParam(int64 LongValue)
	{
		FGameEventParamWrapper ParamWrapper;
		ParamWrapper.Type = EGameEventParamType::Integer;
		ParamWrapper.IntValue = LongValue;
		Params.Add(ParamWrapper);
	}

	void PushParam(float FloatValue)
	{
		PushParam((double)FloatValue);
	}	
	
	void PushParam(double DoubleValue)
	{
		FGameEventParamWrapper ParamWrapper;
		ParamWrapper.Type = EGameEventParamType::Float;
		ParamWrapper.FloatValue = DoubleValue;
		Params.Add(ParamWrapper);
	}

	void PushParam(const char* StrValue)
	{
		PushParam(const_cast<char*>(StrValue));
	}

	void PushParam(char* StrValue)
	{
		PushParam(FString(StrValue));
	}

	void PushParam(const TCHAR* StrValue)
	{
		PushParam(const_cast<TCHAR*>(StrValue));
	}

	void PushParam(TCHAR* StrValue)
	{
		PushParam(FString(StrValue));
	}	

	void PushParam(const FString& StrValue)
	{
		PushParam(const_cast<FString&>(StrValue));
	}

	void PushParam(FString& StrValue)
	{
		PushParam(Forward<FString>(StrValue));
	}

	void PushParam(FString&& StrValue)
	{
		FGameEventParamWrapper ParamWrapper;
		ParamWrapper.Type = EGameEventParamType::String;
		ParamWrapper.StrValue = StrValue;
		Params.Add(ParamWrapper);
	}

	void PushParam(const FName& NameValue)
	{
		PushParam(const_cast<FName&>(NameValue));
	}

	void PushParam(FName& NameValue)
	{
		PushParam(Forward<FName>(NameValue));
	}

	void PushParam(FName&& NameValue)
	{
		PushParam(NameValue.ToString());
	}

	void PushParam(const FText& TextValue)
	{
		PushParam(const_cast<FText&>(TextValue));
	}

	void PushParam(FText& TextValue)
	{
		PushParam(Forward<FText>(TextValue));
	}

	void PushParam(FText&& TextValue)
	{
		PushParam(TextValue.ToString());
	}

	bool GetBool(const int32 Index) const
	{
		return GetParam(Index).GetBool();
	}

	int8 GetByte(const int32 Index) const
	{
		return GetParam(Index).GetByte();
	}

	int16 GetShort(const int32 Index) const
	{
		return GetParam(Index).GetShort();
	}	

	int32 GetInt(const int32 Index) const
	{
		return GetParam(Index).GetInt();
	}

	int64 GetLong(const int32 Index) const
	{
		return GetParam(Index).GetLong();
	}	

	float GetFloat(const int32 Index) const
	{
		return GetParam(Index).GetFloat();
	}

	double GetDouble(const int32 Index) const
	{
		return GetParam(Index).GetDouble();
	}	

	const FString& GetString(const int32 Index) const
	{
		return GetParam(Index).GetString();
	}

	FName GetName(const int32 Index) const
	{
		return FName(GetParam(Index).GetString());
	}

	FText GetText(const int32 Index) const
	{
		return FText::FromString(GetParam(Index).GetString());
	}

	template<typename T>
	std::enable_if_t<std::is_base_of<UObject, T>::value, T*> GetUObject(const int32 Index) const
	{
		return GetParam(Index).GetUObject<T>();
	}

	template<typename T>
	T* GetAny(const int32 Index) const
	{
		return GetParam(Index).GetAny<T>();
	}

	template<typename T>
	T& GetAnyObject(const int32 Index) const
	{
		T* AnyPtr = GetParam(Index).GetAny<T>();
		checkf(AnyPtr, TEXT("FGameEventData:GetAnyObject AnyPtr is null!"));
		return *(AnyPtr);
	}	

	FString ToString() const
	{
		FString Ret = "{ ";
		
		for (int32 i = 0; i < Params.Num(); i++)
		{
			auto DataParam = Params[i];
			switch (DataParam.Type)
			{
			case EGameEventParamType::None:
				Ret += TEXT("None:");
				break;
			case EGameEventParamType::Any:
				Ret += FString::Printf(TEXT("Any:%p"), DataParam.DataPtr);
				break;
			case EGameEventParamType::Bool:
				{
					const bool bValue = DataParam.GetBool();
					Ret += FString::Printf(TEXT("Bool:%s"), bValue ? TEXT("true") : TEXT("false"));
				}
				break;
			case EGameEventParamType::Integer:
				{
					const int32 IntValue = DataParam.GetInt();
					Ret += FString::Printf(TEXT("Integer:%d"), IntValue);
				}
				break;
			case EGameEventParamType::Float:
				{
					const float FValue = DataParam.GetFloat();
					Ret += FString::Printf(TEXT("Float:%f"), FValue);					
				}
				break;
			case EGameEventParamType::String:
				{
					FString StrValue = DataParam.GetString();
					Ret += FString::Printf(TEXT("String:%s"), *StrValue);					
				}
				break;
			case EGameEventParamType::Object:
				{
					UObject* Object = DataParam.GetUObject<UObject>();
					Ret += FString::Printf(TEXT("Object:%s_%s"), *(Object->GetClass()->GetName()), *(Object->GetName()));					
				}
				break;
			}

			if (i < Params.Num() - 1)
			{
				Ret += ", ";
			}
		}
		
		Ret += " }";

		return Ret;
	}

	void Clear()
	{
		Params.Reset();
	}

	~FGameEventData()
	{
		Params.Empty();
	}

private:
	const FGameEventParamWrapper& GetParam(const int32 Index) const
	{
		checkf(Params.IsValidIndex(Index), TEXT("GetParam Index:%d is out of params bounds:%d"), Index, Params.Num());
		return Params[Index];
	}
	
	TArray<FGameEventParamWrapper> Params;
};