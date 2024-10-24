cmake_minimum_required (VERSION 3.28)

# Download Catch2
FetchContent_Declare(catch2
  GIT_REPOSITORY "https://github.com/catchorg/catch2.git"
  GIT_TAG "v3.5.3"
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/catch2"
)
FetchContent_MakeAvailable(catch2)


macro(make_test)
	add_executable (${ARGV0} "${ARGV0}.cpp")
	target_link_libraries(${ARGV0} PUBLIC tls Catch2::Catch2WithMain)
	add_test(${ARGV0} ${ARGV0})

if (MSVC)
	#target_compile_options(${ARGV0} PRIVATE  /bigobj) # thanks hierarchy.cpp
	#target_compile_options(${ARGV0} PRIVATE /RTCs /fsanitize=address)
else()
	target_compile_options(${ARGV0} PRIVATE -fsanitize=undefined,address)
	target_link_options(${ARGV0} PRIVATE -fsanitize=undefined,address)
endif()
endmacro()

make_test (test_shard)
