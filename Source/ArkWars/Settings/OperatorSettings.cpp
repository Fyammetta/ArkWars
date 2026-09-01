

#include "OperatorSettings.h"

UOperatorSettings* UOperatorSettings::Get()
{
	return GetMutableDefault<UOperatorSettings>();
}

UDataTable* UOperatorSettings::GetSkillInfoDataTable() const
{
	check(SkillInfoDataTable.IsValid());
	return SkillInfoDataTable.LoadSynchronous();
}

UDataTable* UOperatorSettings::GetSkillMappingDataTable() const
{
	check(SkillMappingDataTable.IsValid());
	return SkillMappingDataTable.LoadSynchronous();
}

UDataTable* UOperatorSettings::GetOperatorDataTable() const
{
	check(OperatorDataTable.IsValid());
	return OperatorDataTable.LoadSynchronous();
}
