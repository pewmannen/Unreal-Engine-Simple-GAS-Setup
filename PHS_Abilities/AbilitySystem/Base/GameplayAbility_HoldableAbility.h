#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_ActivationPolicy.h"
#include "GameplayAbility_HoldableAbility.generated.h"

/**
 UGameplayAbility_HoldableAbility
 
 Base class for abilities that need to distinguish between a tap and a hold,
 or need to know the exact hold duration on release.
 
 Inherits UGameplayAbility_ActivationPolicy, so ActivationPolicy is available.
 
 How it works:
 	- UAbilityInputBindingComponent calls AbilityLocalInputPressed/Released on the ASC.
 	- GAS routes those calls to InputPressed/InputReleased on the active ability instance.
 	- This class tracks the timestamp of the press and computes HoldDuration on release.
 	- Blueprint subclasses implement OnInputPressed and OnInputReleased(float HoldDuration).
 
 HoldStartTime baseline:
 	- Set in ActivateAbility as a fallback for the rare case where InputPressed
 		fires after ActivateAbility on the first frame.
 	- Overwritten in InputPressed with the accurate press time.
 
 To port to a new project: no changes needed beyond the module export macro.
 */
UCLASS(Abstract, Blueprintable)
class PHS_API UGameplayAbility_HoldableAbility : public UGameplayAbility_ActivationPolicy
{
	GENERATED_BODY()

public:
	UGameplayAbility_HoldableAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** Called in Blueprint when the player presses the input. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PHS|HoldableAbility")
	void OnInputPressed();

	/**
	 Called in Blueprint when the player releases the input.
	 @param HoldDuration  -Time in seconds between press and release. Minimum 0.
	 							Cast from double to float — sub-millisecond precision not needed.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PHS|HoldableAbility")
	void OnInputReleased(float HoldDuration);

private:
	/**
	 World time (seconds) when the input was last pressed.
	 double to match UWorld::GetTimeSeconds() precision and avoid drift over long sessions.
	 */
	double HoldStartTime = 0.0;
};