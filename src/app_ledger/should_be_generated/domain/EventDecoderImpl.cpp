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

#include "EventDecoderImpl.h"
#include "events/AccountCreatedEvent.h"

namespace gringofts {
namespace ledger {

std::unique_ptr<Event> EventDecoderImpl::decodeEventFromString(const EventMetaData &metaData,
                                                               std::string_view payload) const {
  std::unique_ptr<Event> event;
  switch (metaData.getType()) {
    case ACCOUNT_CREATED_EVENT: {
      event = std::make_unique<AccountCreatedEvent>(metaData.getCreatedTimeInNanos(), std::string(payload));
      break;
    }
    default:return nullptr;
  }
  event->setPartialMetaData(metaData);
  return std::move(event);
}

}  /// namespace ledger
}  /// namespace gringofts
