#include "AbilityInputBindingComponent.h"
#include "PHSLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UAbilityInputBindingComponent::UAbilityInputBindingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbilityInputBindingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InputConfig)
	{
		UE_LOG(LogPHSInput, Error,
			TEXT("UAbilityInputBindingComponent on '%s': InputConfig is null. "
			     "Set it in the Blueprint subclass Class Defaults."),
			*GetNameSafe(GetOwner()));
		// Do not return — the controller delegate and timer are still registered
		// in case the config is set at runtime before possession completes.
	}

	// Attempt immediate bind. Succeeds in standalone PIE and most single-player cases
	// where the pawn is already possessed before the GameFeature injects components.
	if (TryBindInputs())
	{
		return;
	}

	// Register for late-possession (multiplayer, delayed spawn, etc.).
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		Character->ReceiveControllerChangedDelegate.AddDynamic(
			this, &UAbilityInputBindingComponent::OnControllerChanged);
		bBoundToControllerChanged = true;
	}

	// Timer fallback: covers the gap between possession and InputComponent readiness.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RetryTimerHandle,
			this,
			&UAbilityInputBindingComponent::RetryBind,
			0.1f,
			/*bLoop=*/ true);
	}
}

void UAbilityInputBindingComponent::RetryBind()
{
	if (!TryBindInputs())
	{
		return;
	}

	// Bind succeeded — cancel timer and unregister controller delegate.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}

	if (bBoundToControllerChanged)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			Character->ReceiveControllerChangedDelegate.RemoveDynamic(
				this, &UAbilityInputBindingComponent::OnControllerChanged);
		}
		bBoundToControllerChanged = false;
	}
}

void UAbilityInputBindingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RetryTimerHandle);
	}

	if (bBoundToControllerChanged)
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			Character->ReceiveControllerChangedDelegate.RemoveDynamic(
				this, &UAbilityInputBindingComponent::OnControllerChanged);
		}
		bBoundToControllerChanged = false;
	}

	// Remove the mapping context so it doesn't persist after the component is destroyed.
	if (InputConfig && InputConfig->InputMappingContext)
	{
		if (const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				if (const ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
						LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						Subsystem->RemoveMappingContext(InputConfig->InputMappingContext);
					}
				}
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UAbilityInputBindingComponent::OnControllerChanged(
	APawn* Pawn, AController* OldController, AController* NewController)
{
	if (!NewController)
	{
		return;
	}

	// Start (or restart) the retry timer rather than binding directly here.
	// InputComponent may not be ready at the exact moment of possession.
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(RetryTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				RetryTimerHandle,
				this,
				&UAbilityInputBindingComponent::RetryBind,
				0.1f,
				/*bLoop=*/ true);
		}
	}
}

bool UAbilityInputBindingComponent::TryBindInputs()
{
	// Guard: don't bind twice if somehow called after a successful bind.
	if (bInputsBound)
	{
		return true;
	}

	if (!InputConfig)
	{
		return false;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC)
	{
		return false;
	}

	UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (!InputComp)
	{
		return false;
	}

	BindAbilityInputs(InputComp, InputConfig);
	return true;
}

void UAbilityInputBindingComponent::BindAbilityInputs(
	UEnhancedInputComponent* Input, const UPHSInputConfig* Config)
{
	if (!Input || !Config)
	{
		UE_LOG(LogPHSInput, Warning,
			TEXT("UAbilityInputBindingComponent::BindAbilityInputs — Input or Config is null."));
		return;
	}

	// Guard against double-binding (e.g. if called manually after automatic bind succeeded).
	if (bInputsBound)
	{
		UE_LOG(LogPHSInput, Warning,
			TEXT("UAbilityInputBindingComponent on '%s': BindAbilityInputs called but inputs are already bound. Ignoring."),
			*GetNameSafe(GetOwner()));
		return;
	}

	// Add the mapping context so key bindings defined in IMC_* assets take effect.
	if (const UInputMappingContext* IMC = Config->InputMappingContext)
	{
		const APawn* Pawn = Cast<APawn>(GetOwner());
		const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
		const ULocalPlayer* LP = PC ? PC->GetLocalPlayer() : nullptr;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LP ? LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr)
		{
			Subsystem->AddMappingContext(IMC, 0);
			UE_LOG(LogPHSInput, Log,
				TEXT("UAbilityInputBindingComponent: Added MappingContext '%s'."), *IMC->GetName());
		}
		else
		{
			UE_LOG(LogPHSInput, Warning,
				TEXT("UAbilityInputBindingComponent on '%s': Could not get EnhancedInputLocalPlayerSubsystem — MappingContext '%s' not added."),
				*GetNameSafe(GetOwner()), *IMC->GetName());
		}
	}

	// Bind each action and cache the tag/ID for the press/release handlers.
	for (const FPHSInputAction& Entry : Config->InputActions)
	{
		if (!Entry.InputAction)
		{
			UE_LOG(LogPHSInput, Warning,
				TEXT("UAbilityInputBindingComponent on '%s': Null InputAction in config '%s' — skipping."),
				*GetNameSafe(GetOwner()), *GetNameSafe(Config));
			continue;
		}

		if (!Entry.InputTag.IsValid())
		{
			UE_LOG(LogPHSInput, Warning,
				TEXT("UAbilityInputBindingComponent on '%s': Invalid InputTag for action '%s' — skipping."),
				*GetNameSafe(GetOwner()), *Entry.InputAction->GetName());
			continue;
		}

		// InputID must match UPHSAbilitySet::GiveToAbilitySystem which uses the same hash.
		const int32 InputID = static_cast<int32>(GetTypeHash(Entry.InputTag));
		InputTagToIDMap.Add(Entry.InputTag, InputID);
		InputActionToTagMap.Add(Entry.InputAction, Entry.InputTag);

		Input->BindAction(Entry.InputAction, ETriggerEvent::Started,
			this, &UAbilityInputBindingComponent::HandleInputActionPressed);
		Input->BindAction(Entry.InputAction, ETriggerEvent::Completed,
			this, &UAbilityInputBindingComponent::HandleInputActionReleased);

		UE_LOG(LogPHSInput, Log,
			TEXT("UAbilityInputBindingComponent: Bound '%s' → Tag '%s' (InputID %d)."),
			*Entry.InputAction->GetName(), *Entry.InputTag.ToString(), InputID);
	}

	bInputsBound = true;
}

void UAbilityInputBindingComponent::HandleInputActionPressed(const FInputActionInstance& Instance)
{
	const UInputAction* InputAction = Instance.GetSourceAction();
	if (!InputAction)
	{
		return;
	}

	const FGameplayTag* Tag = InputActionToTagMap.Find(InputAction);
	if (!Tag || !Tag->IsValid())
	{
		return;
	}

	const int32* InputID = InputTagToIDMap.Find(*Tag);
	if (!InputID)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputPressed(*InputID);
	}
}

void UAbilityInputBindingComponent::HandleInputActionReleased(const FInputActionInstance& Instance)
{
	const UInputAction* InputAction = Instance.GetSourceAction();
	if (!InputAction)
	{
		return;
	}

	const FGameplayTag* Tag = InputActionToTagMap.Find(InputAction);
	if (!Tag || !Tag->IsValid())
	{
		return;
	}

	const int32* InputID = InputTagToIDMap.Find(*Tag);
	if (!InputID)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AbilityLocalInputReleased(*InputID);
	}
}

UAbilitySystemComponent* UAbilityInputBindingComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}