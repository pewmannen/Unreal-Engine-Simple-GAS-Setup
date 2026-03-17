#include "GameplayAbility_HoldableAbility.h"

UGameplayAbility_HoldableAbility::UGameplayAbility_HoldableAbility()
{
	// InstancedPerActor is required — we store per-instance hold state (HoldStartTime).
	// This is already set by the parent constructor but re-stated here for clarity
	// since subclasses must not change it to NonInstanced or InstancedPerExecution.
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGameplayAbility_HoldableAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Record activation time as a baseline. InputPressed may not have fired yet
	// on the first frame (e.g. instant-activation abilities). HoldStartTime will
	// be overwritten with the accurate press time when InputPressed fires.
	if (const UWorld* World = GetWorld())
	{
		HoldStartTime = World->GetTimeSeconds();
	}
}

void UGameplayAbility_HoldableAbility::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);

	// Accurate press timestamp — overrides the ActivateAbility baseline.
	if (const UWorld* World = GetWorld())
	{
		HoldStartTime = World->GetTimeSeconds();
	}

	OnInputPressed();
}

void UGameplayAbility_HoldableAbility::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	// GetWorld() should always be valid for an active ability, but guard defensively.
	// If null, HoldDuration is 0 rather than a garbage value.
	const double HoldEndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : HoldStartTime;

	// Cast double → float intentionally: hold durations beyond ~16 minutes of precision
	// are not meaningful for gameplay. The float is sufficient for all practical use.
	const float HoldDuration = static_cast<float>(FMath::Max(0.0, HoldEndTime - HoldStartTime));

	OnInputReleased(HoldDuration);
}