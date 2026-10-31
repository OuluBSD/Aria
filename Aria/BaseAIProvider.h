#ifndef _Aria_BaseAIProvider_h_
#define _Aria_BaseAIProvider_h_

#include <Core/Core.h>

class BaseAIProvider {
public:
	virtual ~BaseAIProvider() {}
	virtual String Generate(const String& prompt, const String& context = "", const String& output_format = "text") = 0;
};

#endif
