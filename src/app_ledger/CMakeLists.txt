project(app_ledger)
MESSAGE(STATUS "processing app_ledger")

#################################################################################################
#
# BEGIN src for app_ledger
#################################################################################################

# Generated sources for ledger app
set(proto_generated_dir "generated/grpc")
file(MAKE_DIRECTORY ${proto_generated_dir})

set(proto_file_list
        should_be_generated/domain/protos/publishEvents.proto
        should_be_generated/domain/protos/ledger.proto)

ADD_PROTO_SET(ledger_proto_library "${proto_file_list}" ${proto_generated_dir})

## src list
# app_ledger
set(APP_LEDGER_AUTOGEN
        AppStateMachine.cpp
        should_be_generated/app/App.cpp
        should_be_generated/app/RequestReceiver.cpp
        should_be_generated/domain/CommandDecoderImpl.cpp
        should_be_generated/domain/EventDecoderImpl.cpp
        should_be_generated/domain/IncreaseCommand.cpp
        should_be_generated/domain/ProcessedEvent.cpp
        should_be_generated/domain/post/BundleExposePublisher.cpp)

set(APP_LEDGER_SRC
        v2/RocksDBBackedAppStateMachine.cpp
        execution/IncreaseApplier.cpp
        execution/IncreaseHandler.cpp)

#################################################################################################
#
# END src for app_ledger
#################################################################################################


#################################################################################################
#
# BEGIN lib & executables for app_ledger
#################################################################################################

add_library(app_ledger STATIC
        ${APP_LEDGER_AUTOGEN}
        ${APP_LEDGER_SRC})

add_dependencies(app_ledger ledger_proto_library)
target_link_libraries(app_ledger ledger_proto_library)

# executables
add_executable(LedgerApp
        should_be_generated/app/Main.cpp)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    #for macos
    target_link_libraries(LedgerApp app_ledger gringofts_app_util gringofts_infra -Wl -lgrpc++_reflection)
else()
    target_link_libraries(LedgerApp app_ledger gringofts_app_util gringofts_infra -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed)
endif()

#################################################################################################
#
# END lib & executables for app_ledger
#################################################################################################
