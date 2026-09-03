#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ArkWarTableTypes.generated.h"

/**
 *	数据表行通用基类：描述文案 + 占位数据
 *	文案中的数据部分以 {0}、{1}、{2}…… 标记，包括纯数字与游戏内变化的数据（X、Y、Z 等）
 *	Data 提供占位符名称，DataGetter 负责在运行期按名称取值
 */
USTRUCT(BlueprintType)
struct FInfoWithData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FText Description;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Data;

	TDelegate<int32(const FString&)> DataGetter;

	TArray<FFormatArgumentData> GetNumericalDatas()
	{
		TArray<FFormatArgumentData> RetVal;
		FFormatArgumentData Arg;

		for (const FString& String : Data)
		{
			Arg.ArgumentName = FString::Printf(TEXT("{%d}"), Data.Find(String));
			Arg.ArgumentValueType = EFormatArgumentType::Int;
			if (String.IsNumeric())
			{
				Arg.ArgumentValueInt = FCString::Atoi64(*String);
			}
			else if (DataGetter.IsBound())
			{
				Arg.ArgumentValueInt = DataGetter.Execute(String);
			}
			else
			{
				Arg.ArgumentValueInt = 0;
			}

			RetVal.Add(MoveTemp(Arg));
		}

		return RetVal;
	};

	virtual ~FInfoWithData() {}
};
