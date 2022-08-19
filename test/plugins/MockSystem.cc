#include "MockSystem.hh"

#include <gz/plugin/Register.hh>

IGNITION_ADD_PLUGIN(gz::sim::MockSystem, gz::sim::System,
    gz::sim::MockSystem::ISystemConfigure,
    gz::sim::MockSystem::ISystemPreUpdate,
    gz::sim::MockSystem::ISystemUpdate,
    gz::sim::MockSystem::ISystemPostUpdate)

