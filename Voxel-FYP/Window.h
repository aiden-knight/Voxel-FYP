#pragma once
#include <cstdint>

struct WindowExtensions
{
	uint32_t count;
	const char** extensions;
};

class Window
{
public:
	Window();
	virtual ~Window();

	virtual void PollEvents() const = 0;
	virtual bool ShouldClose() const = 0;

	virtual WindowExtensions GetRequiredInstanceExtensions() const = 0;
protected:
	bool m_resized;

private:

};