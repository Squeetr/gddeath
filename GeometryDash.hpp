#ifndef _GeometryDash_hpp
#define _GeometryDash_hpp

#include <functional>

namespace GeometryDash
{
	typedef void(__thiscall* SfxThing_T)(void*, char const*);

	typedef std::function<void()> Callback;

	bool init();
	void installCallback(Callback&& cb);
}

#endif /* _GeometryDash_hpp */