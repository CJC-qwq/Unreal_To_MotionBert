#include "MotionBERTLiveLink.h"

#define LOCTEXT_NAMESPACE "FMotionBERTLiveLinkModule"

DEFINE_LOG_CATEGORY(LogMotionBERTLiveLink);

void FMotionBERTLiveLinkModule::StartupModule()
{
}

void FMotionBERTLiveLinkModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMotionBERTLiveLinkModule, MotionBERTLiveLink)
