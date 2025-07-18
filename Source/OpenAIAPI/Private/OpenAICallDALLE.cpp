﻿// Copyright Kellan Mythen 2023. All rights Reserved.


#include "OpenAICallDALLE.h"
#include "OpenAIUtils.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "OpenAIParser.h"


UOpenAICallDALLE::UOpenAICallDALLE()
{
}

UOpenAICallDALLE::~UOpenAICallDALLE()
{
}

UOpenAICallDALLE* UOpenAICallDALLE::OpenAICallDALLE(EOAImageSize imageSizeInput, FString promptInput, int32 numImagesInput, FString HostInput)
{
	UOpenAICallDALLE* BPNode = NewObject<UOpenAICallDALLE>();
	BPNode->imageSize = imageSizeInput;
	BPNode->prompt = promptInput;
	BPNode->numImages = numImagesInput;
	BPNode->Host = HostInput;
	return BPNode;
}

void UOpenAICallDALLE::Activate()
{
	FString _apiKey;
	FString _host = Host;
	if (UOpenAIUtils::getUseApiKeyFromEnvironmentVars())
	{
		_apiKey = UOpenAIUtils::GetEnvironmentVariable(TEXT("OPENAI_API_KEY"));
		FString EnvHost = UOpenAIUtils::GetEnvironmentVariable(TEXT("OPENAI_API_HOST"));
		if (!EnvHost.IsEmpty())
		{
			_host = EnvHost;
		}
	}
	else
	{
		_apiKey = UOpenAIUtils::getApiKey();
		FString ConfigHost = UOpenAIUtils::getApiHost();
		if (!ConfigHost.IsEmpty())
		{
			_host = ConfigHost;
		}
	}
	if (!Host.IsEmpty())
	{
		_host = Host;
	}

	// checking parameters are valid
	if (_apiKey.IsEmpty())
	{
		Finished.Broadcast({}, TEXT("Api key is not set"), false);
	} else if (prompt.IsEmpty())
	{
		Finished.Broadcast({}, TEXT("Prompt is empty"), false);
	} else if (numImages < 1 || numImages > 10)
	{
		Finished.Broadcast({}, TEXT("NumImages must be set to a value between 1 and 10"), false);
	}
	
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	FString url = _host.EndsWith("/") ? _host.LeftChop(1) : _host;
	url += TEXT("/v1/images/generations");
	HttpRequest->SetURL(url);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Authorization"), "Bearer " + _apiKey);

	// build payload
	TSharedPtr<FJsonObject> _payloadObject = MakeShareable(new FJsonObject());
	_payloadObject->SetStringField(TEXT("prompt"), prompt);
	_payloadObject->SetNumberField(TEXT("n"), numImages);
	_payloadObject->SetStringField(TEXT("size"), imageResolution);

	// convert payload to string
	FString _payload;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&_payload);
	FJsonSerializer::Serialize(_payloadObject.ToSharedRef(), Writer);

	// commit request
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetContentAsString(_payload);

	if (HttpRequest->ProcessRequest())
	{
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UOpenAICallDALLE::OnResponse);
	}
	else
	{
		Finished.Broadcast({}, ("Error sending request"), false);
	}
}

void UOpenAICallDALLE::OnResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool WasSuccessful)
{
	if (!WasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Error processing request. \n%s \n%s"), *Response->GetContentAsString(), *Response->GetURL());
		if (Finished.IsBound())
		{
			Finished.Broadcast({}, *Response->GetContentAsString(), false);
		}
		return;
	}

	TSharedPtr<FJsonObject> responseObject;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(reader, responseObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());
		bool err = responseObject->HasField("error");
		
		if (err)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());
			Finished.Broadcast({}, TEXT("Api error"), false);
			return;
		}

		OpenAIParser parser(settings);
		TArray<FString> _out;

		auto GeneratedImagesObject = responseObject->GetArrayField(TEXT("data"));
		for (auto& elem : GeneratedImagesObject)
		{
		_out.Add(parser.ParseGeneratedImage(*elem->AsObject()));
		}

		Finished.Broadcast(_out, "", true);
	}
}

