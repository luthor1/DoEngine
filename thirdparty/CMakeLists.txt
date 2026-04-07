# DoEngine Third-party Dependencies
# Using local folders in thirdparty/ (cloned by setup_deps.ps1)

# 1. GLFW
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
    add_subdirectory(glfw)
else()
    message(WARNING "glfw not found!")
endif()

# 2. EnTT (Header only)
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/entt/src)

# 3. enkiTS (Directly including source to avoid CMake errors)
set(ENKITS_SRC
    enkits/src/TaskScheduler.cpp
    enkits/src/TaskScheduler_c.cpp
)
add_library(enkiTS STATIC ${ENKITS_SRC})
target_include_directories(enkiTS PUBLIC enkits/src)

# 4. Jolt Physics
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jolt/Build)
    set(BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    set(USE_STATIC_CRT OFF CACHE BOOL "" FORCE)
    set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(jolt/Build jolt_build)
else()
    message(WARNING "jolt/Build not found!")
endif()

# 5. NVRHI
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/nvrhi)
    set(NVRHI_WITH_VULKAN ON CACHE BOOL "" FORCE)
    set(NVRHI_WITH_DX11 OFF CACHE BOOL "" FORCE)
    set(NVRHI_WITH_DX12 ON CACHE BOOL "" FORCE) # Windows support
    add_subdirectory(nvrhi)
else()
    message(WARNING "nvrhi not found!")
endif()

# 6. miniaudio (Single Header)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio)
