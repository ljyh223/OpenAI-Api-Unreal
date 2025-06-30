// Copyright (c) 2024 tobenot, See LICENSE in the project root for license information.


// UOpenAICallEmbedding.cpp

#include "OpenAICallEmbedding.h"
#include "OpenAIUtils.h"

UOpenAICallEmbedding::UOpenAICallEmbedding()
	: OpenAIEmbeddingInstance(nullptr)
{
}

UOpenAICallEmbedding::~UOpenAICallEmbedding()
{
	if (OpenAIEmbeddingInstance)
	{
		OpenAIEmbeddingInstance->ConditionalBeginDestroy();
	}
}

UOpenAICallEmbedding* UOpenAICallEmbedding::OpenAICallEmbedding(const FEmbeddingSettings& EmbeddingSettingsInput, FString HostInput)
{
	UOpenAICallEmbedding* BPNode = NewObject<UOpenAICallEmbedding>();
	BPNode->EmbeddingSettings = EmbeddingSettingsInput;
	BPNode->Host = HostInput;
	return BPNode;
}

void UOpenAICallEmbedding::Activate()
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

	if (!OpenAIEmbeddingInstance)
	{
		OpenAIEmbeddingInstance = UOpenAIEmbedding::CreateEmbeddingInstance();
	}

	OpenAIEmbeddingInstance->Init(EmbeddingSettings);

	OpenAIEmbeddingInstance->OnResponseReceived.BindDynamic(this, &UOpenAICallEmbedding::OnResponse);

	OpenAIEmbeddingInstance->StartEmbedding();
}

void UOpenAICallEmbedding::OnResponse(const FEmbeddingResult& Result, const FString& ErrorMessage, bool Success)
{
	OpenAIEmbeddingInstance->OnResponseReceived.Unbind();
	Finished.Broadcast(Result, ErrorMessage, Success);
	OpenAIEmbeddingInstance->ConditionalBeginDestroy();
	OpenAIEmbeddingInstance = nullptr;
}