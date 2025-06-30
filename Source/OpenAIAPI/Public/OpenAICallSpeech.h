// OpenAICallSpeech.h
// 文字转语音API接口声明
#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "OpenAICallSpeech.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeechSynthesisCompleted, bool, bSuccess, const FString&, AudioUrlOrError);

UCLASS(BlueprintType)
class OPENAIAPI_API UOpenAICallSpeech : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "OpenAI|Speech")
	static UOpenAICallSpeech* CreateSpeechRequest(const FString& InputText, const FString& Voice = TEXT("alloy"), const FString& Model = TEXT("gpt-4o-mini-tts"));

	UFUNCTION(BlueprintCallable, Category = "OpenAI|Speech")
	void Activate();

	UPROPERTY(BlueprintAssignable)
	FOnSpeechSynthesisCompleted OnSpeechSynthesisCompleted;

	// 请求参数
	FString Input;
	FString Voice;
	FString Model;
};
