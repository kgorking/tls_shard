cmake_minimum_required (VERSION 3.28)

include(FetchContent)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (tls)
add_library(tls)

target_sources(tls PUBLIC FILE_SET CXX_MODULES FILES
	"tls.cppm"
	"shard.cppm"
	"shard_retain.cppm")

# TODO: Add tests and install targets if needed.
enable_testing()
add_subdirectory ("unittest")
