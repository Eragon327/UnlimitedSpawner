#include "mod/UnlimitedSpawner.h"
#include "UnlimitedSpawner.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/memory/Hook.h"

using namespace ll::memory_literals;

namespace unlimited_spawner {

LL_AUTO_INSTANCE_HOOK(
    HandlePopulationCap,
    ll::memory::HookPriority::Normal,
    "48 89 5C 24 08 57 48 83 EC 30 41 8B D9 44 8B 89 ? ? ? ? 41 8D 04 19 3D C8 00 00 00"_sig,
    int,
    const void*  mobDatas,
    const void*  conditions,
    unsigned int spawnCount
) {
    int result = origin(mobDatas, conditions, spawnCount);
    UnlimitedSpawner::getInstance().getSelf().getLogger().info("Spawn count: {}", result);
    return result;
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