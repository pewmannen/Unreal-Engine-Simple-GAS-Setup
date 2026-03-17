
#include "PHS_Abilities/GameFeatureActions/AddGameplayCuePath.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"

void UAddGameplayCuePath::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		const FString PathStr = GameplayCueNotifyPath.Path;
		if (!PathStr.IsEmpty())
		{
			RegisteredPath = PathStr;
			CueManager->AddGameplayCueNotifyPath(PathStr);
		}
	}
}

void UAddGameplayCuePath::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		if (!RegisteredPath.IsEmpty())
		{
			CueManager->RemoveGameplayCueNotifyPath(RegisteredPath);
		}
	}

	RegisteredPath.Empty();
}
