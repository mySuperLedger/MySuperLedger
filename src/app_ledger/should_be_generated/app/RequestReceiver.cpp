/************************************************************************
Copyright 2022 MySuperLedger
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

#include "RequestReceiver.h"

#include "calldatas/RequestCallData.h"

namespace gringofts {
namespace ledger {

RequestReceiver::RequestReceiver(const INIReader &reader,
                                 uint32_t port,
                                 BlockingQueue<std::shared_ptr<Command>> &commandQueue)  // NOLINT(runtime/references)
    : mCommandQueue(commandQueue) {
  mIpPort = "0.0.0.0:" + std::to_string(port);
  assert(mIpPort != "UNKNOWN");
  mTlsConfOpt = TlsUtil::parseTlsConf(reader, "tls");
}

void RequestReceiver::startListen() {
  if (mIsShutdown) {
    SPDLOG_WARN("Receiver is already down. Will not run again.");
    return;
  }

  std::string server_address(mIpPort);

  ::grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address,
                           TlsUtil::buildServerCredentials(mTlsConfOpt));
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&mService);

  for (uint64_t i = 0; i < mConcurrency; ++i) {
    mCompletionQueues.emplace_back(builder.AddCompletionQueue());
  }

  mServer = builder.BuildAndStart();
  SPDLOG_INFO("Server listening on {}", server_address);

  // Spawn a new CallData instance to serve new clients.
  for (uint64_t i = 0; i < mPreSpawn; ++i) {
    for (uint64_t j = 0; j < mConcurrency; ++j) {
      new CreateAccountCallData(&mService, mCompletionQueues[j].get(), mCommandQueue);
    }
  }
}

void RequestReceiver::start() {
  startListen();
  // start receive threads
  for (uint64_t i = 0; i < mConcurrency; ++i) {
    mRcvThreads.emplace_back([this, i]() {
      std::string threadName = (std::string("RcvThread_") + std::to_string(i));
      pthread_setname_np(pthread_self(), threadName.c_str());
      handleRpcs(i);
    });
  }
}

void RequestReceiver::stop() {
  if (mIsShutdown) {
    SPDLOG_INFO("Server is already down");
  } else {
    mIsShutdown = true;
    mServer->Shutdown();
    for (uint64_t i = 0; i < mConcurrency; ++i) {
      mCompletionQueues[i]->Shutdown();
      /// drain completion queue.
      void *tag;
      bool ok;
      while (mCompletionQueues[i]->Next(&tag, &ok)) { ; }
    }
    // join threads
    for (uint64_t i = 0; i < mConcurrency; ++i) {
      if (mRcvThreads[i].joinable()) {
        mRcvThreads[i].join();
      }
    }
  }
}

void RequestReceiver::handleRpcs(uint64_t i) {
  void *tag;  // uniquely identifies a request.
  bool ok;
  // Block waiting to read the next event from the completion queue. The
  // event is uniquely identified by its tag, which in this case is the
  // memory address of a CallData instance.
  // The return value of Next should always be checked. This return value
  // tells us whether there is any kind of event or cq_ is shutting down.
  while (mCompletionQueues[i]->Next(&tag, &ok)) {
    auto *callData = static_cast<RequestHandle *>(tag);
    if (ok) {
      callData->proceed();
    } else if (!mIsShutdown) {
      callData->failOver();
    }
  }
}

}  // namespace ledger
}  // namespace gringofts
