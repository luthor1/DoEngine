cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(DoEngine LANGUAGES CXX C)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 2. Dependencies
find_package(Vulkan REQUIRED)
add_subdirectory(thirdparty)

# 3. Target and Sources
# Explicitly listing files for build stability on MSVC
set(ENGINE_SOURCES
    main.cpp
    src/Engine/Application.cpp
    src/Core/Window.cpp
    src/Core/PathUtils.cpp
    src/Core/VFS/VFS.cpp
    src/Core/JobSystem/JobSystem.cpp
    src/Graphics/RHI.cpp
    src/Graphics/Renderer.cpp
    src/Audio/AudioSystem.cpp
    src/Physics/PhysicsSystem.cpp
    src/Assets/AssetImporter.cpp
    src/Assets/stb_image_impl.cpp
    src/Assets/AssetRegistry.cpp
    src/ECS/Scene.cpp
    src/Scripting/ScriptEngine.cpp
    src/Editor/Editor.cpp
    src/Editor/ImGui_ImplNVRHI.cpp
    src/Project/ProjectManager.cpp
    # ImGui Core
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    # ImGui Backends
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(DoEngine ${ENGINE_SOURCES})

# 4. Include Directories - Using absolute, QUOTED paths to bypass MSVC path parsing issues with special characters like '+'
target_include_directories(DoEngine PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nvrhi/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/jolt"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/entt/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/enkits/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/miniaudio"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nvrhi/thirdparty/DirectX-Headers/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/spdlog/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui"
)

# 5. Linking - Explicitly link all dependencies
target_link_libraries(DoEngine PRIVATE 
    glfw
    enkiTS
    entt
    Jolt
    miniaudio
    nvrhi
    nvrhi_vk
    nvrhi_d3d12
    Vulkan::Vulkan
)

# 6. MSVC Specific Flags for Modern C++ and EnTT
if(MSVC)
    target_compile_options(DoEngine PRIVATE /std:c++20 /permissive- /utf-8)
endif()

# 6. Global Definitions for Windows/MSVC
target_compile_definitions(DoEngine PRIVATE 
    $<$<CONFIG:Debug>:DO_DEBUG>
    $<$<CONFIG:Release>:DO_RELEASE>
    NVRHI_WITH_VULKAN=1
    VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
    NOMINMAX  # Prevents windows.h from defining macros like PlaySound
    WIN32_LEAN_AND_MEAN
)

# 7. NIHAI CRT ZORLAMASI (Nükleer Seçenek)
# Projeye eklenen tüm hedefleri (bağımlılıklar dahil) gezer ve MSVC_RUNTIME_LIBRARY özelliğini Dinamik (/MD) yapar.
function(force_dynamic_crt)
    get_all_targets(targets_list ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(target ${targets_list})
        set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endforeach()
endfunction()

# Tüm subdirectories eklendikten sonra çalıştırılması için bir makro/fonksiyon yardımcı fonksiyonu
macro(get_all_targets_recursive targets dir)
    get_property(subdirectories DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets} ${subdir})
    endforeach()
    get_property(current_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND ${targets} ${current_targets})
endmacro()

set(all_targets "")
get_all_targets_recursive(all_targets ${CMAKE_CURRENT_SOURCE_DIR})

foreach(target ${all_targets})
    # Sadece MSVC kullanırken ve hedef gerçekten bir kütüphane/executable ise
    if(MSVC)
        set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()
endforeach()
