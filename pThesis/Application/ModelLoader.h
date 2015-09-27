#pragma once

#include "../CoreSystems/CoreStd.h"

class ModelLoader
{
private:
protected:
public:

private:
protected:
public:
	virtual ~ModelLoader() {}

	virtual bool VLoad(const std::string& _file) = 0;
};

