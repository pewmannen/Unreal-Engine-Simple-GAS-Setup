#include "PHSInputConfig.h"
#include "PHSLog.h"

UPHSInputConfig::UPHSInputConfig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UInputAction* UPHSInputConfig::FindInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FPHSInputAction& Action : InputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogPHSInput, Warning, TEXT("UPHSInputConfig '%s': No InputAction found for tag '%s'."),
			*GetNameSafe(this), *InputTag.ToString());
	}

	return nullptr;
}
