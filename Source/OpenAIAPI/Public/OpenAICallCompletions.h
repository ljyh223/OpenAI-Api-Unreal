// Copyright Kellan Mythen 2023. All rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OpenAIDefinitions.h"
#include "HttpModule.h"
#include "OpenAICallCompletions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnGptResponseRecievedPin, const TArray<FCompletion>&, completions, const FString&, errorMessage, const FCompletionInfo&, completionInfo, bool, Success);

/**
 * 
 */
UCLASS()
class OPENAIAPI_API UOpenAICallCompletions : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UOpenAICallCompletions();
	~UOpenAICallCompletions();

	EOACompletionsEngineType engine = EOACompletionsEngineType::TEXT_DAVINCI_002;
	FString prompt = "";
	FCompletionSettings settings;
	FString Host = TEXT("https://api.openai.com");

	UPROPERTY(BlueprintAssignable, Category = "OpenAI")
		FOnGptResponseRecievedPin Finished;

private:
	OpenAIValueMapping mapping;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "OpenAI", meta=(DeprecatedFunction, DeprecationMessage="Function has been deprecated, Please use OpenAICallChat instead"))
		static UOpenAICallCompletions* OpenAICallCompletions(EOACompletionsEngineType engine, FString prompt, FCompletionSettings settings, FString Host = TEXT("https://api.openai.com"));

	virtual void Activate() override;
	void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
};