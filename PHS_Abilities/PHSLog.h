#pragma once

#include "Logging/LogMacros.h"

/**
 * PHSLog.h
 *
 * Centralized log categories for ProjectHellshift.
 *
 * Usage:
 *   UE_LOG(LogPHS, Log,     TEXT("..."));
 *   UE_LOG(LogPHSAbility,  Warning, TEXT("..."));
 *   UE_LOG(LogPHSInput,    Error,   TEXT("..."));
 *
 * Declare in your .cpp: DEFINE_LOG_CATEGORY(LogPHS);
 * Or use the convenience macros below which resolve to the right category.
 *
 * To port to a new project: rename the PROJECTHELLSHIFT_API macro to match
 * the new project's module export macro, then update the log category names
 * if desired. Everything else is self-contained.
 */

PROJECTHELLSHIFT_API DECLARE_LOG_CATEGORY_EXTERN(LogPHS,        Log, All);
PROJECTHELLSHIFT_API DECLARE_LOG_CATEGORY_EXTERN(LogPHSAbility, Log, All);
PROJECTHELLSHIFT_API DECLARE_LOG_CATEGORY_EXTERN(LogPHSInput,   Log, All);