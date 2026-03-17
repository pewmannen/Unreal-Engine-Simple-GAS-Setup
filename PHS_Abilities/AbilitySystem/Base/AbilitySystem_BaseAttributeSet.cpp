#include "PHS_Abilities/AbilitySystem/Base/AbilitySystem_BaseAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UAbilitySystem_BaseAttributeSet::UAbilitySystem_BaseAttributeSet()
{
	/*
	Health = MaxHealth = 100.f;
	Stamina = MaxStamina = 100.f;
	Shield = MaxShield = 100.f;
	SwordBlock = MaxSwordBlock = 0.f;
	*/
	
	// Health
	InitMaxHealth(100.f);
	InitHealth(100.f);

	// Stamina
	InitMaxStamina(100.f);
	InitStamina(100.f);

	// Shield
	InitMaxShield(100.f);
	InitShield(100.f);

	// Sword Block
	InitMaxSwordBlock(100.f);
	InitSwordBlock(0.f);
}

void UAbilitySystem_BaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp Health between 0 and MaxHealth
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Clamp Stamina between 0 and MaxStamina
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}

	if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		// Clamp Shield between 0 and MaxShield
		SetShield(FMath::Clamp(GetShield(), 0.f, GetMaxShield()));
	}

	if (Data.EvaluatedData.Attribute == GetSwordBlockAttribute())
	{
		// Clamp SwordBlock between 0 and MaxShield
		SetSwordBlock(FMath::Clamp(GetSwordBlock(), 0.f, GetMaxSwordBlock()));
	}
}

// Health
void UAbilitySystem_BaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, Health, OldValue);
}

void UAbilitySystem_BaseAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, MaxHealth, OldValue);
}

// Stamina
void UAbilitySystem_BaseAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, Stamina, OldValue);
}

void UAbilitySystem_BaseAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, MaxStamina, OldValue);
}

// Shield
void UAbilitySystem_BaseAttributeSet::OnRep_Shield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, Shield, OldValue);
}

void UAbilitySystem_BaseAttributeSet::OnRep_MaxShield(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, MaxShield, OldValue);
}

// Sword Block
void UAbilitySystem_BaseAttributeSet::OnRep_SwordBlock(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, SwordBlock, OldValue);
}

void UAbilitySystem_BaseAttributeSet::OnRep_MaxSwordBlock(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAbilitySystem_BaseAttributeSet, MaxSwordBlock, OldValue);
}

void UAbilitySystem_BaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Health
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	// Stamina
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	// Shield
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, MaxShield, COND_None, REPNOTIFY_Always);
	// Sword Block
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, SwordBlock, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAbilitySystem_BaseAttributeSet, MaxSwordBlock, COND_None, REPNOTIFY_Always);
}
