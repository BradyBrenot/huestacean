#include "backend/backend.h"

Backend::Backend() :
	stopRequested(false),
	threadStarted(false),
	threadIsRunning(false),
	thread(),
	lock(),
	rooms(),
	deviceProviders()
{

}

void Backend::Start()
{

}

bool Backend::HasThreadStarted()
{
	return false;
}

bool Backend::IsThreadRunning()
{
	return false;
}

void Backend::Stop()
{

}