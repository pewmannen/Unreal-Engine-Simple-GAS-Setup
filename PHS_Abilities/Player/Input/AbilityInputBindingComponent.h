#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "PHSInputConfig.h"
#include "AbilityInputBindingComponent.generated.h"

class UEnhancedInputComponent;
class UAbilitySystemComponent;
class UInputAction;
struct FInputActionInstance;

/**
 UAbilityInputBindingComponent
 
 Injected onto a character (typically via a GameFeature Add Components action)
 to route Enhanced Input action events to GAS ability activation via
 AbilityLocalInputPressed / AbilityLocalInputReleased.
 
 Setup:
 - Create a Blueprint subclass (e.g. BP_AbilityInputBindingComponent).
 - Set InputConfig in the Blueprint Class Defaults.
 - Add the Blueprint class (not the C++ class) to the GameFeature action.
 
 InputID contract:
 	Both this component and UPHSAbilitySet derive InputID via GetTypeHash(InputTag).
 	The tag string must be identical on both sides — mismatches silently fail.
 
 Timing strategy:
 1. Attempts immediate bind on BeginPlay (succeeds in most PIE/standalone cases).
 2. If the PlayerController or its InputComponent isn't ready, registers a
 	ReceiveControllerChangedDelegate for late-possession (multiplayer).
 3. Runs a 0.1s repeating timer as a fallback for the window between possession
 	and InputComponent initialization.
 4. Timer and delegate both clean themselves up on first successful bind.
 
 To port to a new project: rename PHS_API. No other changes needed.
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PHS_API UAbilityInputBindingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityInputBindingComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 Input config that maps InputActions to GameplayTags.
	 Must be set in the Blueprint subclass Class Defaults before injection.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PHS|Input")
	TObjectPtr<UPHSInputConfig> InputConfig = nullptr;

	/**
	 Manually bind all inputs from a given config.
	 Normally called internally by TryBindInputs, but exposed for
	 advanced use cases (e.g. runtime config swapping).
	 Safe to call only after a valid PlayerController and InputComponent exist.
	 */
	UFUNCTION(BlueprintCallable, Category = "PHS|Input")
	void BindAbilityInputs(UEnhancedInputComponent* Input, const UPHSInputConfig* Config);

protected:
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

private:
	/**
	 * Attempts to resolve the controller and input component and bind all actions.
	 * Returns true on success (bindings applied), false if prerequisites aren't ready yet.
	 * Must only be called after BeginPlay.
	 */
	bool TryBindInputs();

	/** Called by the retry timer until TryBindInputs succeeds. */
	UFUNCTION()
	void RetryBind();

	/** Fires when the pawn receives a new controller (multiplayer / late possession). */
	UFUNCTION()
	void OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	/** Called by Enhanced Input on action Started event. Routes to AbilityLocalInputPressed. */
	void HandleInputActionPressed(const FInputActionInstance& Instance);

	/** Called by Enhanced Input on action Completed event. Routes to AbilityLocalInputReleased. */
	void HandleInputActionReleased(const FInputActionInstance& Instance);

	/**
	 * Maps InputAction → GameplayTag (for lookup on input event).
	 * Uses raw pointer as map key — TObjectPtr is not safe as a TMap key across engine versions.
	 */
	UPROPERTY()
	TMap<TObjectPtr<const UInputAction>, FGameplayTag> InputActionToTagMap;

	/** Maps GameplayTag → InputID (cached GetTypeHash result). */
	UPROPERTY()
	TMap<FGameplayTag, int32> InputTagToIDMap;

	/** Retry timer handle. Active only between BeginPlay and first successful bind. */
	FTimerHandle RetryTimerHandle;

	/** True if we've registered with ReceiveControllerChangedDelegate and need to clean up. */
	bool bBoundToControllerChanged = false;

	/** True once BindAbilityInputs has been called successfully. Guards against double-binding. */
	bool bInputsBound = false;
};