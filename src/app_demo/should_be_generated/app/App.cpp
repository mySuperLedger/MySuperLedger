/************************************************************************
Copyright 2019-2020 eBay Inc.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
**************************************************************************/

#include "App.h"

#include "../../../infra/es/store/RaftCommandEventStore.h"
#include "../../../infra/es/store/ReadonlyRaftCommandEventStore.h"
#include "../../../infra/monitor/Monitorable.h"
#include "../../../infra/raft/RaftBuilder.h"
#include "../../../infra/raft/metrics/RaftMonitorAdaptor.h"

namespace gringofts {
namespace demo {
using protos::IncreaseRequest;

App::App(const char *configPath) : mIsShutdown(false) {
  SPDLOG_INFO("current working dir: {}", FileUtil::currentWorkingDir());

  INIReader reader(configPath);
  if (reader.ParseError() < 0) {
    SPDLOG_WARN("Cannot load config file {}, exiting", configPath);
    throw std::runtime_error("Cannot load config file");
  }

  app::AppInfo::init(reader);

  mCrypto = std::make_shared<gringofts::CryptoUtil>();
  mCrypto->init(reader);

  initDeploymentMode(reader);

  initMonitor(reader);

  initMemoryPool(reader);

  initCommandEventStore(reader);

  initBenchmark(reader);

  std::string snapshotDir = reader.Get("snapshot", "dir", "UNKNOWN");
  assert(snapshotDir != "UNKNOWN");

  auto commandEventDecoder = std::make_shared<app::CommandEventDecoderImpl<EventDecoderImpl, CommandDecoderImpl>>();

  const auto &appVersion = app::AppInfo::appVersion();
  assert(appVersion == "v2");
  mEventApplyLoop =
      std::make_shared<app::EventApplyLoop<v2::RocksDBBackedAppStateMachine>>(
          reader, commandEventDecoder, std::move(mReadonlyCommandEventStoreForEventApplyLoop), snapshotDir);
  mCommandProcessLoop = std::make_unique<CommandProcessLoop<v2::MemoryBackedAppStateMachine>>(
      reader,
      commandEventDecoder,
      mDeploymentMode,
      mEventApplyLoop,
      mCommandQueue,
      std::move(mReadonlyCommandEventStoreForCommandProcessLoop),
      mCommandEventStore,
      snapshotDir,
      mFactory);

  mRequestReceiver = ::std::make_unique<RequestReceiver>(reader, app::AppInfo::gatewayPort(), mCommandQueue);
  mNetAdminServer = ::std::make_unique<app::NetAdminServer>(reader, mEventApplyLoop);
  mPostServer = std::make_unique<BundleExposePublisher>(reader, std::move(mReadonlyCommandEventStoreForPostServer));
}

App::~App() {
  SPDLOG_INFO("deleting app");
}

void App::initDeploymentMode(const INIReader &reader) {
  std::string mode = reader.Get("app", "deployment.mode", "standalone");
  assert(mode == "distributed");
  mDeploymentMode = DeploymentMode::Distributed;

  SPDLOG_INFO("Deployment mode is {}", mode);
}

void App::initMonitor(const INIReader &reader) {
  int monitorPort = reader.GetInteger("monitor", "port", -1);
  assert(monitorPort > 0);
  auto &server = gringofts::Singleton<santiago::Server>::getInstance("0.0.0.0", monitorPort);

  auto &appInfo = Singleton<santiago::AppInfo>::getInstance();
  auto appName = "demoApp";
  auto appVersion = "demoVersion";
  auto appEnv = reader.Get("app", "env", "unknown");
  appInfo.setAppInfo(appName, appVersion, appEnv);

  auto startTime = TimeUtil::currentTimeInNanos();
  appInfo.gauge("start_time_gauge", {}).set(startTime);

  server.Registry(appInfo);
  server.Registry(gringofts::Singleton<gringofts::MonitorCenter>::getInstance());
  SPDLOG_INFO("Init monitor with app name : {} , app version : {}, app env : {}, start time : {}",
              appName,
              appVersion,
              appEnv,
              startTime);
}

void App::initMemoryPool(const INIReader &reader) {
  if (gringofts::PerfConfig::getInstance().getMemoryPoolType() == "monotonic") {
    mFactory = std::make_shared<gringofts::PMRContainerFactory>(
        "PMRFactory",
        std::make_unique<gringofts::MonotonicPMRMemoryPool>(
            "monotonicPool",
            gringofts::PerfConfig::getInstance().getMaxMemoryPoolSizeInMB()));
  } else {
    mFactory = std::make_shared<gringofts::PMRContainerFactory>(
        "PMRFactory",
        std::make_unique<gringofts::NewDeleteMemoryPool>(
            "newDeletePool"));
  }
}

void App::initCommandEventStore(const INIReader &reader) {
  std::string storeType = reader.Get("cluster", "persistence.type", "UNKNOWN");
  assert(storeType == "raft");
  assert(mDeploymentMode == DeploymentMode::Distributed);

  std::string configPath = reader.Get("cluster", "raft.config.path", "UNKNOWN");
  assert(configPath != "UNKNOWN");

  std::shared_ptr<app::CommandEventDecoderImpl<EventDecoderImpl, CommandDecoderImpl>> commandEventDecoder =
      std::make_shared<app::CommandEventDecoderImpl<EventDecoderImpl, CommandDecoderImpl>>();
  auto myNodeId = gringofts::app::AppInfo::getMyNodeId();
  auto myClusterInfo = gringofts::app::AppInfo::getMyClusterInfo();
  mRaftImpl = raft::buildRaftImpl(configPath.c_str(), myNodeId, myClusterInfo);
  auto metricsAdaptor = std::make_shared<RaftMonitorAdaptor>(mRaftImpl);
  enableMonitorable(metricsAdaptor);
  mCommandEventStore = std::make_shared<RaftCommandEventStore>(mRaftImpl, mCrypto);
  mReadonlyCommandEventStoreForCommandProcessLoop = nullptr;
  mReadonlyCommandEventStoreForEventApplyLoop = std::make_unique<ReadonlyRaftCommandEventStore>(mRaftImpl,
                                                                                                commandEventDecoder,
                                                                                                commandEventDecoder,
                                                                                                mCrypto,
                                                                                                true);
  mReadonlyCommandEventStoreForPostServer = std::make_unique<ReadonlyRaftCommandEventStore>(mRaftImpl,
                                                                                            commandEventDecoder,
                                                                                            commandEventDecoder,
                                                                                            mCrypto,
                                                                                            false);
}

void App::initBenchmark(const INIReader &reader) {
  mRunBenchmark = reader.GetBoolean("benchmark", "enable", false);
  mTotalRequestCnt = reader.GetInteger("benchmark", "total.cnt", 0);
  assert(!mRunBenchmark || mTotalRequestCnt > 0);
}

void App::startRequestReceiver() {
  mRequestReceiver->start();
}

void App::startNetAdminServer() {
  mNetAdminServerThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "NetAdmin");
    mNetAdminServer->run();
  });
}

void App::startProcessCommandLoop() {
  mCommandProcessLoopThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "CommandProcLoop");
    assert(mDeploymentMode == DeploymentMode::Distributed);
    mCommandProcessLoop->runDistributed();
  });
}

void App::startEventApplyLoop() {
  mEventApplyLoopThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "EventApplyLoop");
    mEventApplyLoop->run();
  });
}

void App::startPersistLoop() {
  mPersistLoopThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "CmdEvtStoreMain");
    mCommandEventStore->run();
  });
}

void App::startPostServerLoop() {
  mPostServerThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "PostServerMain");
    if (mPostServer != nullptr) {
      mPostServer->run();
    }
  });
}

void App::startBenchmark() {
  mBenchmarkThread = std::thread([this]() {
    pthread_setname_np(pthread_self(), "Benchmark");
    IncreaseRequest request;
    unsigned int seed = 1234;
    uint64_t cnt = 0;
    auto startTimeInNanos = TimeUtil::currentTimeInNanos();
    while (true) {
      try {
        if (mRaftImpl->getRaftRole() != RaftRole::Leader) {
          usleep(1'000);  // sleep 1ms
          continue;
        }
        cnt++;
        request.set_value(rand_r(&seed));
        auto createdTimeInNanos = TimeUtil::currentTimeInNanos();
        auto command = std::make_shared<IncreaseCommand>(createdTimeInNanos, request);
        command->setCreatorId(app::AppInfo::subsystemId());
        command->setGroupId(app::AppInfo::groupId());
        command->setGroupVersion(app::AppInfo::groupVersion());
        mCommandQueue.enqueue(command);
        if (cnt == mTotalRequestCnt) {
          auto elapsedTimeInNanos = TimeUtil::currentTimeInNanos() - startTimeInNanos;
          SPDLOG_INFO("Complete sending {} requests, took {}ms", cnt, elapsedTimeInNanos / 1'000'000);
          break;
        }
      }
      catch (const QueueStoppedException &e) {
        SPDLOG_WARN(e.what());
      }
    }
  });
}

void App::run() {
  if (mIsShutdown) {
    SPDLOG_WARN("App is already down. Will not run again.");
  } else {
    assert(mDeploymentMode == DeploymentMode::Distributed);

    // now run the threads
    startRequestReceiver();
    startNetAdminServer();
    startPersistLoop();
    startEventApplyLoop();
    startProcessCommandLoop();
    startPostServerLoop();
    if (mRunBenchmark) {
      startBenchmark();
      mBenchmarkThread.join();
    }

    // wait for all threads to exit
    mPostServerThread.join();
    mCommandProcessLoopThread.join();
    mEventApplyLoopThread.join();
    mPersistLoopThread.join();
    mNetAdminServerThread.join();
    mServerThread.join();
  }
}

void App::shutdown() {
  if (mIsShutdown) {
    SPDLOG_INFO("App is already down");
  } else {
    mIsShutdown = true;

    // shutdown all threads
    mCommandQueue.shutdown();
    mRequestReceiver->stop();
    mNetAdminServer->shutdown();
    mCommandProcessLoop->shutdown();
    mEventApplyLoop->shutdown();
    mCommandEventStore->shutdown();
    if (mPostServer != nullptr) {
      mPostServer->shutdown();
    }
  }
}

}  /// namespace demo
}  /// namespace gringofts
