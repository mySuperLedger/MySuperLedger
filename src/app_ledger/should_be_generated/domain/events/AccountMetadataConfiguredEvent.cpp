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

#include "AccountMetadataConfiguredEvent.h"

#include <spdlog/spdlog.h>

#include "../common_types.h"

namespace gringofts {
namespace ledger {

AccountMetadataConfiguredEvent::AccountMetadataConfiguredEvent(TimestampInNanos createdTimeInNanos,
                                                               const AccountMetadata &accountMetadata)
    : Event(ACCOUNT_METADATA_CONFIGURED_EVENT, createdTimeInNanos) {
  mAccountMetadata = accountMetadata;
}

AccountMetadataConfiguredEvent::AccountMetadataConfiguredEvent(TimestampInNanos createdTimeInNanos,
                                                               std::string_view eventStr)
    : Event(ACCOUNT_METADATA_CONFIGURED_EVENT, createdTimeInNanos) {
  decodeFromString(eventStr);
}

std::string AccountMetadataConfiguredEvent::encodeToString() const {
  protos::ConfigureAccountMetadata::ConfiguredEvent eventProto;
  eventProto.set_version(mVersion);
  switch (mVersion) {
    case 1: {
      mAccountMetadata.encodeTo(*eventProto.mutable_account_metadata());
      break;
    }
    default: {
      SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
      exit(1);
    }
  }

  return eventProto.SerializeAsString();
}

void AccountMetadataConfiguredEvent::decodeFromString(std::string_view payload) {
  protos::ConfigureAccountMetadata::ConfiguredEvent eventProto;
  eventProto.ParseFromString(std::string(payload));
  mVersion = eventProto.version();
  assert(mVersion > 0);
  switch (mVersion) {
    case 1: {
      mAccountMetadata.initWith(eventProto.account_metadata());
      break;
    }
    default: {
      SPDLOG_ERROR("Cannot recognize this version {}, exiting now", mVersion);
      exit(1);
    }
  }
}

}  /// namespace ledger
}  /// namespace gringofts
