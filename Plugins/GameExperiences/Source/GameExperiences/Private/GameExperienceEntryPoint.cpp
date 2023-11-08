// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameExperienceEntryPoint.h"

#include "CommonSessionSubsystem.h"


// UGameExperienceEntryPointUIData
// -------------------------------

TArray<UGameExperienceEntryPoint*> UGameExperienceEntryPointUIData::SortGameExperienceEntryPoints(const TArray<UGameExperienceEntryPoint*>& EntryPoints)
{
	TArray<UGameExperienceEntryPoint*> Result = EntryPoints;

	// remove null entries
	Result.RemoveAll([](const UGameExperienceEntryPoint* EntryPoint) { return EntryPoint == nullptr; });

	Result.Sort([](const UGameExperienceEntryPoint& EntryPointA, const UGameExperienceEntryPoint& EntryPointB)
	{
		if (EntryPointA.UIData && EntryPointB.UIData)
		{
			return EntryPointA.UIData->SortOrder < EntryPointB.UIData->SortOrder;
		}
		return false;
	});
	return Result;
}


// UGameExperienceEntryPoint
// -------------------------

UCommonSession_HostSessionRequest* UGameExperienceEntryPoint::CreateOnlineHostSessionRequest() const
{
	UCommonSession_HostSessionRequest* Request = NewObject<UCommonSession_HostSessionRequest>();
	Request->MapID = Level;
	Request->ExtraArgs = ExtraArgs;

	if (GameExperience.IsValid())
	{
		// options strings do not allow the ':' symbol, use just the primary asset name
		Request->ExtraArgs.Add(TEXT("Experience"), GameExperience.PrimaryAssetName.ToString());
	}

	Request->OnlineMode = ECommonSessionOnlineMode::Online;
	Request->ModeNameForAdvertisement = GetPrimaryAssetId().PrimaryAssetName.ToString();
	Request->MaxPlayerCount = MaxPlayerCount;

	return Request;
}

UCommonSession_HostSessionRequest* UGameExperienceEntryPoint::CreateOfflineHostSessionRequest() const
{
	UCommonSession_HostSessionRequest* Request = NewObject<UCommonSession_HostSessionRequest>();
	Request->MapID = Level;
	Request->ExtraArgs = ExtraArgs;

	if (GameExperience.IsValid())
	{
		// options strings do not allow the ':' symbol, use just the primary asset name
		Request->ExtraArgs.Add(TEXT("Experience"), GameExperience.PrimaryAssetName.ToString());
	}

	Request->OnlineMode = ECommonSessionOnlineMode::Offline;

	return Request;
}
