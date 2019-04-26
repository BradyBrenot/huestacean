#pragma once

#include "rep_frontend.h"
#include "backend/backend.h"

#include <memory>

class Frontend : public FrontendSimpleSource
{
	Q_OBJECT

public:
	explicit Frontend(std::shared_ptr<Backend> inBackend);

public:
	virtual ~Frontend() {}


private:
	std::shared_ptr<Backend> m_Backend;
};