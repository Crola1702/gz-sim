/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <sdf/Model.hh>
#include <sdf/Root.hh>
#include <sdf/World.hh>

#include <gz/transport/Node.hh>
#include <gz/utilities/ExtraTestMacros.hh>

#include "gz/sim/EntityComponentManager.hh"
#include "gz/sim/EventManager.hh"
#include "gz/sim/SdfEntityCreator.hh"
#include "gz/sim/Server.hh"
#include "gz/sim/SystemLoader.hh"
#include "gz/sim/Types.hh"
#include "gz/sim/test_config.hh"

#include "gz/sim/components/Model.hh"
#include "gz/sim/components/Name.hh"
#include "gz/sim/components/World.hh"

#include "plugins/MockSystem.hh"
#include "../helpers/EnvTestFixture.hh"

using namespace gz;
using namespace std::chrono_literals;
namespace components = gz::sim::components;

//////////////////////////////////////////////////
class SensorsFixture : public InternalFixture<InternalFixture<::testing::Test>>
{
  protected: void SetUp() override
  {
    InternalFixture::SetUp();

    auto plugin = sm.LoadPlugin("libMockSystem.so",
                                "gz::sim::MockSystem",
                                nullptr);
    EXPECT_TRUE(plugin.has_value());
    this->systemPtr = plugin.value();
    this->mockSystem = static_cast<sim::MockSystem *>(
        systemPtr->QueryInterface<sim::System>());
  }

  public: sim::SystemPluginPtr systemPtr;
  public: sim::MockSystem *mockSystem;

  private: sim::SystemLoader sm;
};

//////////////////////////////////////////////////
void testDefaultTopics()
{
  // TODO(anyone) This should be a new test, but running multiple tests with
  // sensors is not currently working
  std::string prefix{"/world/camera_sensor/model/default_topics/"};
  std::vector<std::string> topics{
      prefix + "link/camera_link/sensor/camera/image",
      prefix + "link/camera_link/sensor/camera/camera_info",
      prefix + "link/gpu_lidar_link/sensor/gpu_lidar/scan",
      prefix + "link/depth_camera_link/sensor/depth_camera/depth_image",
      prefix + "link/depth_camera_link/sensor/depth_camera/camera_info",
      prefix + "link/rgbd_camera_link/sensor/rgbd_camera/image",
      prefix + "link/rgbd_camera_link/sensor/rgbd_camera/depth_image"
  };

  std::vector<transport::MessagePublisher> publishers;
  transport::Node node;

  // Sensors are created in a separate thread, so we sleep here to give them
  // time
  int sleep{0};
  int maxSleep{30};
  for (; sleep < maxSleep && !node.TopicInfo(topics.front(), publishers);
      ++sleep)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  ASSERT_LT(sleep, maxSleep);

  for (const std::string &topic : topics)
  {
    bool result = node.TopicInfo(topic, publishers);

    EXPECT_TRUE(result) << "Could not get topic info for " << topic;
    EXPECT_EQ(1u, publishers.size());
    publishers.clear();
  }
}

/////////////////////////////////////////////////
/// This test checks that that the sensors system handles cases where entities
/// are removed and then added back
TEST_F(SensorsFixture, IGN_UTILS_TEST_DISABLED_ON_MAC(HandleRemovedEntities))
{
  sim::ServerConfig serverConfig;

  const std::string sdfFile = std::string(PROJECT_SOURCE_PATH) +
    "/test/worlds/sensor.sdf";

  serverConfig.SetSdfFile(sdfFile);

  sdf::Root root;
  root.Load(sdfFile);
  const sdf::World *sdfWorld = root.WorldByIndex(0);
  const sdf::Model *sdfModel = sdfWorld->ModelByIndex(0);

  sim::Server server(serverConfig);

  // A pointer to the ecm. This will be valid once we run the mock system
  sim::EntityComponentManager *ecm = nullptr;
  this->mockSystem->preUpdateCallback =
    [&ecm](const sim::UpdateInfo &, sim::EntityComponentManager &_ecm)
    {
      ecm = &_ecm;
    };

  server.AddSystem(this->systemPtr);
  server.Run(true, 50, false);
  ASSERT_NE(nullptr, ecm);

  testDefaultTopics();

  // We won't use the event manager but it's required to create an
  // SdfEntityCreator
  sim::EventManager dummyEventMgr;
  sim::SdfEntityCreator creator(*ecm, dummyEventMgr);

  unsigned int runs = 100;
  unsigned int runIterations = 2;
  for (unsigned int i = 0; i < runs; ++i)
  {
    {
      auto modelEntity = ecm->EntityByComponents(
          components::Model(), components::Name(sdfModel->Name()));
      EXPECT_NE(sim::kNullEntity, modelEntity);

      // Remove the first model in the world
      creator.RequestRemoveEntity(modelEntity, true);
    }

    server.Run(true, runIterations, false);

    {
      auto modelEntity = ecm->EntityByComponents(components::Model(),
          components::Name(sdfModel->Name()));

      // Since the model is removed, we should get a null entity
      EXPECT_EQ(sim::kNullEntity, modelEntity);
    }

    // Create the model again
    auto newModelEntity = creator.CreateEntities(sdfModel);
    creator.SetParent(newModelEntity,
                      ecm->EntityByComponents(components::World()));

    // This would throw if entities that have not been properly removed from the
    // scene manager in the Sensor system
    EXPECT_NO_THROW(server.Run(true, runIterations, false));

    {
      auto modelEntity = ecm->EntityByComponents(components::Model(),
          components::Name(sdfModel->Name()));
      EXPECT_NE(sim::kNullEntity, modelEntity);
    }
  }
}
