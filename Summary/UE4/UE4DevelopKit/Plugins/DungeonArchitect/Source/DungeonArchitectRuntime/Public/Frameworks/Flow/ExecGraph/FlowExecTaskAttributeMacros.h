//$ Copyright 2015-21, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Utils/Attributes.h"

#define __FLOWTASKATTR_GET_IMPL(Variable, AttrType)	\
	if (InParameterName == #Variable) {	\
		OutValue.AttrType.Set(Variable);	\
		return true;	\
	}

#define __FLOWTASKATTR_GET_IMPL_EX(Variable, AttrType, Value)	\
	if (InParameterName == #Variable) {	\
		OutValue.AttrType.Set(Value);	\
		return true;	\
	}

#define FLOWTASKATTR_GET_INT(Variable)		__FLOWTASKATTR_GET_IMPL(Variable, NumberValue)
#define FLOWTASKATTR_GET_FLOAT(Variable)	__FLOWTASKATTR_GET_IMPL(Variable, NumberValue)
#define FLOWTASKATTR_GET_BOOL(Variable)		__FLOWTASKATTR_GET_IMPL(Variable, BoolValue)
#define FLOWTASKATTR_GET_SIZE(Variable)		__FLOWTASKATTR_GET_IMPL_EX(Variable, SizeValue, FVector2D(Variable.X, Variable.Y))
#define FLOWTASKATTR_GET_VECTOR(Variable)	__FLOWTASKATTR_GET_IMPL_EX(Variable, VectorValue, FVector(Variable.X, Variable.Y, Variable.Z))
#define FLOWTASKATTR_GET_STRING(Variable)	__FLOWTASKATTR_GET_IMPL(Variable, StringValue)
#define FLOWTASKATTR_GET_STRINGARRAY(Variable)	__FLOWTASKATTR_GET_IMPL(Variable, StringArrayValue)


#define __FLOWTASKATTR_SET_IMPL(Variable, AttrType)	\
	if (InParameterName == #Variable && InValue.AttrType.IsSet()) {	\
		Variable = InValue.AttrType.Get();	\
		return true;	\
	}

#define __FLOWTASKATTR_SET_IMPL_EX(Variable, AttrType, Value)	\
	if (InParameterName == #Variable && InValue.AttrType.IsSet()) {	\
		Variable = Value;	\
		return true;	\
	}

#define __FLOWTASKATTR_SET_IMPL2(VariableName, AttrType, Variable)	\
	if (InParameterName == #VariableName && InValue.AttrType.IsSet()) {	\
		Variable = InValue.AttrType.Get();	\
		return true;	\
	}

#define FLOWTASKATTR_SET_INT(Variable)		__FLOWTASKATTR_SET_IMPL_EX(Variable, NumberValue, FMath::RoundToInt(InValue.NumberValue.Get()))
#define FLOWTASKATTR_SET_FLOAT(Variable)	__FLOWTASKATTR_SET_IMPL(Variable, NumberValue)
#define FLOWTASKATTR_SET_BOOL(Variable)		__FLOWTASKATTR_SET_IMPL(Variable, BoolValue)
#define FLOWTASKATTR_SET_SIZEF(Variable)	__FLOWTASKATTR_SET_IMPL(Variable, SizeValue)
#define FLOWTASKATTR_SET_SIZEI(Variable)	__FLOWTASKATTR_SET_IMPL_EX(Variable, SizeValue, FIntPoint(FMath::RoundToInt(InValue.SizeValue.Get().X),FMath::RoundToInt(InValue.SizeValue.Get().Y)))
#define FLOWTASKATTR_SET_VECTORF(Variable)	__FLOWTASKATTR_SET_IMPL(Variable, VectorValue)
#define FLOWTASKATTR_SET_VECTORI(Variable)	__FLOWTASKATTR_SET_IMPL_EX(Variable, VectorValue, FIntVector(FMath::RoundToInt(InValue.VectorValue.Get().X),FMath::RoundToInt(InValue.VectorValue.Get().Y),FMath::RoundToInt(InValue.VectorValue.Get().Z)))
#define FLOWTASKATTR_SET_STRING(Variable)	__FLOWTASKATTR_SET_IMPL(Variable, StringValue)
#define FLOWTASKATTR_SET_STRINGARRAY(Variable)	__FLOWTASKATTR_SET_IMPL(Variable, StringArrayValue)

#define FLOWTASKATTR_SET_PARSE_INT(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseNumber(InSerializedText); FLOWTASKATTR_SET_INT(Variable) }
#define FLOWTASKATTR_SET_PARSE_FLOAT(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseNumber(InSerializedText); FLOWTASKATTR_SET_FLOAT(Variable) }
#define FLOWTASKATTR_SET_PARSE_BOOL(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseBool(InSerializedText); FLOWTASKATTR_SET_BOOL(Variable) }
#define FLOWTASKATTR_SET_PARSE_SIZEF(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseSize(InSerializedText); FLOWTASKATTR_SET_SIZEF(Variable) }
#define FLOWTASKATTR_SET_PARSE_SIZEI(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseSize(InSerializedText); FLOWTASKATTR_SET_SIZEI(Variable) }
#define FLOWTASKATTR_SET_PARSE_VECTORF(Variable)		{ FDAAttribute InValue = FDAAttribute::ParseVector(InSerializedText); FLOWTASKATTR_SET_VECTORF(Variable) }
#define FLOWTASKATTR_SET_PARSE_VECTORI(Variable)		{ FDAAttribute InValue = FDAAttribute::ParseVector(InSerializedText); FLOWTASKATTR_SET_VECTORI(Variable) }
#define FLOWTASKATTR_SET_PARSE_STRING(Variable)			{ FDAAttribute InValue = FDAAttribute::ParseString(InSerializedText); FLOWTASKATTR_SET_STRING(Variable) }
#define FLOWTASKATTR_SET_PARSE_STRINGARRAY(Variable)	{ FDAAttribute InValue = FDAAttribute::ParseStringArray(InSerializedText); FLOWTASKATTR_SET_STRINGARRAY(Variable) }


#define FLOWTASKATTR_SET_PARSEEX_INT(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseNumber(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, NumberValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_FLOAT(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseNumber(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, NumberValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_BOOL(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseBool(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, BoolValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_SIZEF(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseSize(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, SizeValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_SIZEI(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseSize(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, SizeValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_VECTORF(VariableName, Variable)		{ FDAAttribute InValue = FDAAttribute::ParseVector(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, VectorValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_VECTORI(VariableName, Variable)		{ FDAAttribute InValue = FDAAttribute::ParseVector(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, VectorValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_STRING(VariableName, Variable)			{ FDAAttribute InValue = FDAAttribute::ParseString(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, StringValue, Variable) }
#define FLOWTASKATTR_SET_PARSEEX_STRINGARRAY(VariableName, Variable)	{ FDAAttribute InValue = FDAAttribute::ParseStringArray(InSerializedText); __FLOWTASKATTR_SET_IMPL2(VariableName, StringArrayValue, Variable) }

