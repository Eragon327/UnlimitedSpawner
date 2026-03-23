#include "UnlimitedSpawner.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/memory/Hook.h"

#include "mc/_HeaderOutputPredefine.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/ActorType.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/chunk/ChunkSource.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/actor/Mob.h"
#include "mc/world/level/Spawner.h"
#include <cstdint>
#include <string>
#include <vector>

#include "mc/entity/systems/ActorLegacyTickSystem.h"

#define MAX_COUNT 200

using namespace ll::memory_literals;

namespace {
struct mDimension {
    DimensionType dimId;
    unsigned int  loadedChunkCount;
    unsigned int  mobCount;
};
std::vector<mDimension> dimensions;
mDimension* searchDimension(DimensionType dimId) {
    auto it = std::find_if(dimensions.begin(), dimensions.end(), [dimId](const mDimension& d) {
        return d.dimId.id == dimId.id;
    });
    if (it != dimensions.end()) {
        return &(*it);
    } else {
        return nullptr;
    }
}
}

namespace unlimited_spawner {

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

LL_AUTO_TYPE_INSTANCE_HOOK(
    DimensionInit,
    ll::memory::HookPriority::Normal,
    Dimension,
    &Dimension::$init,
    void,
    ::br::worldgen::StructureSetRegistry const& structureSetRegistry
) {
    origin(structureSetRegistry);
    // 检查有没有这个维度的记录
    if (searchDimension(this->getDimensionId()) == nullptr) {
        // 没有记录，添加一个新的
        dimensions.push_back({this->getDimensionId(), 0, 0});
    } else {
        std::string name = this->mName;
        UnlimitedSpawner::getInstance().getSelf().getLogger().error("Dimension {} already exists in the tracking list!", name);
    }
}

LL_AUTO_TYPE_INSTANCE_HOOK(
    MobCtor,
    ll::memory::HookPriority::Normal,
    Mob,
    &Mob::$ctor,
    void*,
    ActorDefinitionGroup*            definitions,
    ActorDefinitionIdentifier const& definitionName,
    EntityContext&                   entityContext
) {
    Mob* mob = static_cast<Mob*>(origin(definitions, definitionName, entityContext));
    if (mob->mNaturallySpawned) {
        auto dim = searchDimension(mob->getDimensionId());
        if (dim) {
            dim->mobCount++;
            std::string name = mob->getDimension().mName;
            UnlimitedSpawner::getInstance().getSelf().getLogger().info(
                "Mob spawned in dimension {}, current count: {}",
                name,
                dim->mobCount
            );
        } else {
            std::string name = mob->getDimension().mName;
            UnlimitedSpawner::getInstance().getSelf().getLogger().error(
                "Dimension {} not found in the tracking list when spawning mob!",
                name
            );
        }
    }
    return mob;
}

LL_AUTO_TYPE_INSTANCE_HOOK(MobRemove, ll::memory::HookPriority::Normal, Actor, &Actor::$remove, void) {
    if (this->isType(ActorType::Mob)) {
        Mob* mob = static_cast<Mob*>(static_cast<Actor*>(this));
        if (mob->mNaturallySpawned) {
            auto dim = searchDimension(mob->getDimensionId());
            if (dim) {
                dim->mobCount--;
                std::string name = mob->getDimension().mName;
                UnlimitedSpawner::getInstance().getSelf().getLogger().info(
                    "Mob removed in dimension {}, current count: {}",
                    name,
                    dim->mobCount
                );
            } else {
                std::string name = mob->getDimension().mName;
                UnlimitedSpawner::getInstance().getSelf().getLogger().error(
                    "Dimension {} not found in the tracking list when removing mob!",
                    name
                );
            }
        }
    }
    origin();
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
