// OpenAICallSpeech.cpp
#include "OpenAICallSpeech.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"

UOpenAICallSpeech* UOpenAICallSpeech::CreateSpeechRequest(const FString& InputText, const FString& Voice, const FString& Model)
{
	UOpenAICallSpeech* Call = NewObject<UOpenAICallSpeech>();
	Call->Input = InputText;
	Call->Voice = Voice;
	Call->Model = Model;
	return Call;
}

void UOpenAICallSpeech::Activate()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("model"), Model);
	JsonObject->SetStringField(TEXT("input"), Input);
	JsonObject->SetStringField(TEXT("voice"), Voice);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(TEXT("https://yunwu.ai/v1/audio/speech"));
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(OutputString);

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOpenAICallSpeech::OnResponseReceived);
	HttpRequest->ProcessRequest();
}

void UOpenAICallSpeech::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		// 这里假设返回的是音频文件的URL或base64数据
		OnSpeechSynthesisCompleted.Broadcast(true, Response->GetContentAsString());
	}
	else
	{
		OnSpeechSynthesisCompleted.Broadcast(false, Response.IsValid() ? Response->GetContentAsString() : TEXT("No Response"));
	}
}
