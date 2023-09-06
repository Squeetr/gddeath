#include "utility.hpp"
#include "GeometryDash.hpp"

#include <mutex>
#include <vector>

using namespace GeometryDash;

//Globals

static std::mutex g_Mtx;
static Callback g_CB;
static uintptr_t g_SfxThing = 0xF680;
static uintptr_t g_destroyPlayer = 0x0020A5B0;

//Hook callback
//Gets called when destroyPlayer() gets called
void __fastcall destroyPlayerCB(
	void* ecx,
	void* edx,
	char const* sfx)
{
	auto original = reinterpret_cast<SfxThing_T>(g_SfxThing);

	g_Mtx.lock();

	if (g_CB)
		g_CB();

	g_Mtx.unlock();

	return original(ecx, sfx);
}

bool GeometryDash::init()
{
	//ASLR bypass
	//https://en.wikipedia.org/wiki/Address_space_layout_randomization

	g_SfxThing += utility::getBaseAddress();
	g_destroyPlayer += utility::getBaseAddress();

	//Hook destroyPlayer()
	//https://en.wikipedia.org/wiki/Hooking

	return utility::doHooking(
		g_destroyPlayer,
		destroyPlayerCB,
		5,
		true);
}

void GeometryDash::installCallback(Callback&& cb)
{
	std::lock_guard lock(g_Mtx);
	g_CB = std::move(cb);
}