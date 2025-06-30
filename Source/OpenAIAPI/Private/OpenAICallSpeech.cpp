// Copyright Kellan Mythen 2023. All rights Reserved.

#include "OpenAICallSpeech.h"
#include "OpenAIUtils.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

UOpenAICallSpeech::UOpenAICallSpeech() {}
UOpenAICallSpeech::~UOpenAICallSpeech() {}

UOpenAICallSpeech* UOpenAICallSpeech::OpenAICallSpeech(FSpeechSettings speechSettingsInput, FString HostInput)
{
	UOpenAICallSpeech* BPNode = NewObject<UOpenAICallSpeech>();
	BPNode->speechSettings = speechSettingsInput;
	BPNode->Host = HostInput;
	return BPNode;
}

void UOpenAICallSpeech::Activate()
{
	// 构建请求体
	TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject());
	Payload->SetStringField(TEXT("model"), speechSettings.model);
	Payload->SetStringField(TEXT("input"), speechSettings.input);
	Payload->SetStringField(TEXT("voice"), speechSettings.voice);
	Payload->SetStringField(TEXT("response_format"), TEXT("wav"));

	FString PayloadStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&PayloadStr);
	FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	FString url = Host.EndsWith("/") ? Host.LeftChop(1) : Host;
	url += TEXT("/v1/audio/speech");
	HttpRequest->SetURL(url);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetContentAsString(PayloadStr);
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOpenAICallSpeech::OnResponse);
	if (!HttpRequest->ProcessRequest())
	{
		Finished.Broadcast({}, TEXT("Error sending request"), false);
	}
}

void UOpenAICallSpeech::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	if (!WasSuccessful || !Response.IsValid())
	{
		Finished.Broadcast({}, TEXT("Speech API request failed"), false);
		return;
	}

	if (Response->GetResponseCode() != 200)
	{
		Finished.Broadcast({}, Response->GetContentAsString(), false);
		return;
	}

	// 假设返回的是音频文件的二进制内容，保存到本地
	FString SavePath = FPaths::ProjectSavedDir() + "OpenAISpeech.wav";
	if (FFileHelper::SaveArrayToFile(Response->GetContent(), *SavePath))
	{
		FSpeechCompletion Result;
		Result.audioFilePath = SavePath;
		Result.finishReason = "success";
		Finished.Broadcast(Result, TEXT(""), true);
	}
	else
	{
		Finished.Broadcast({}, TEXT("Failed to save audio file"), false);
	}
}
