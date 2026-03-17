#include "GameplayAbility_ActivationPolicy.h"

UGameplayAbility_ActivationPolicy::UGameplayAbility_ActivationPolicy()
{
	// InstancedPerActor is the safest default:
	//  - Allows per-instance state (timers, hold tracking, cooldown state).
	//  - Required by UGameplayAbility_HoldableAbility.
	//  - Costs slightly more memory than NonInstanced but avoids entire categories
	//    of bugs from shared CDO state. Change per subclass only if profiling demands it.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGameplayAbility_ActivationPolicy::ShouldActivateOnGrant() const
{
	return ActivationPolicy == EAbilityActivationPolicy::WhileActive;
}