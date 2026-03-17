

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "AddGameplayCuePath.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Add Gameplay Cue Path")) // THIS makes it show up in the GameFeature fragment dropdown
class PHS_API UAddGameplayCuePath : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	// Path to the folder with cue notifies
	UPROPERTY(EditAnywhere, Category = "Gameplay Cues")
	FDirectoryPath GameplayCueNotifyPath;

protected:
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;

private:
	FString RegisteredPath;
	
};
