// Copyright Kellan Mythen 2023. All rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "OpenAIDefinitions.h"
#include "HttpModule.h"
#include "OpenAICallSpeech.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpeechResponseReceivedPin, const FSpeechCompletion&, Speech, const FString&, errorMessage, bool, Success);

UCLASS()
class OPENAIAPI_API UOpenAICallSpeech : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UOpenAICallSpeech();
	~UOpenAICallSpeech();

	FString Host = TEXT("https://yunwu.ai");

	UPROPERTY(BlueprintAssignable, Category = "OpenAI")
	FOnSpeechResponseReceivedPin Finished;

private:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"), Category = "OpenAI")
	static UOpenAICallSpeech* OpenAICallSpeech(FSpeechSettings speechSettingsInput, FString Host = TEXT("https://yunwu.ai"));

	virtual void Activate() override;
	void OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful);
};
