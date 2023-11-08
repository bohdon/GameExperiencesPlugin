// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceActionSet.h"

#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#define LOCTEXT_NAMESPACE "GameExperiences"

UGameExperienceActionSet::UGameExperienceActionSet()
{
}

#if WITH_EDITORONLY_DATA
void UGameExperienceActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UGameExperienceActionSet::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (int32 Idx = 0; Idx < Actions.Num(); ++Idx)
	{
		const UGameFeatureAction* Action = Actions[Idx];
		if (!Action)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("ActionIsNull", "Action {0} is null"), FText::AsNumber(Idx)));
			continue;
		}

		const EDataValidationResult ChildResult = Action->IsDataValid(Context);
		Result = CombineDataValidationResults(Result, ChildResult);
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
