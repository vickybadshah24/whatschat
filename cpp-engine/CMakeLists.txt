cmake_minimum_required(VERSION 3.16)
project(whatsapp_ai_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies ────────────────────────────────────────────────────────────
# libcurl  (HTTP client for Claude API calls)
find_package(CURL REQUIRED)

# nlohmann/json  (header-only JSON library)
# Auto-download if not found via FetchContent
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
  DOWNLOAD_NO_EXTRACT TRUE
  DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann
)
FetchContent_MakeAvailable(nlohmann_json)

# ── Sources ──────────────────────────────────────────────────────────────────
add_executable(whatsapp_engine
    main.cpp
    config.cpp
    contact_filter.cpp
    chat_history.cpp
    claude_api.cpp
    http_server.cpp
)

target_include_directories(whatsapp_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CURL_INCLUDE_DIRS}
)

target_link_libraries(whatsapp_engine PRIVATE
    ${CURL_LIBRARIES}
)

# Release build flags
target_compile_options(whatsapp_engine PRIVATE
    $<$<CONFIG:Release>:-O2>
)
