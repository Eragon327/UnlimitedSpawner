#include "UnlimitedSpawner.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/memory/Hook.h"
#include "mc/world/level/Spawner.h"

using namespace ll::memory_literals;

#define MAX_COUNT 200

namespace unlimited_spawner {
namespace {
auto& logger = UnlimitedSpawner::getInstance().getSelf().getLogger();
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    HandlePopulationCap,
    ll::memory::HookPriority::Normal,
    Spawner,
    "48 89 5C 24 08 57 48 83 EC 30 41 8B D9 44 8B 89 ? ? ? ? 41 8D 04 19 3D C8 00 00 00"_sig,
    int,
    void const*  mobDatas,
    void const*  conditions,
    int          spawnCount
) {
    origin(mobDatas, conditions, spawnCount);
    int currentCount = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x220);
    if (currentCount + spawnCount > MAX_COUNT) {
        return std::max(0, MAX_COUNT - currentCount);
    } else {
        return spawnCount;
    }
}

UnlimitedSpawner& UnlimitedSpawner::getInstance() {
    static UnlimitedSpawner instance;
    return instance;
}

bool UnlimitedSpawner::load() { return true; }
bool UnlimitedSpawner::enable() { return true; }
bool UnlimitedSpawner::disable() { return true; }


} // namespace unlimited_spawner

LL_REGISTER_MOD(unlimited_spawner::UnlimitedSpawner, unlimited_spawner::UnlimitedSpawner::getInstance());
