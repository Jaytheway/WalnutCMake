cmake_minimum_required(VERSION 3.28)

# NOTE: for now walnut is setup as a static library,
# the example project is not included, but it can be added
# as a separate executable target that links to the walnut library.

set(WALNUT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# ---- Vulkan (Premake relies on VULKAN_SDK env; find_package will usually pick it up)
find_package(Vulkan REQUIRED)
# Ensure include vars exist across module variants
if (NOT Vulkan_INCLUDE_DIRS AND Vulkan_INCLUDE_DIR)
  set(Vulkan_INCLUDE_DIRS "${Vulkan_INCLUDE_DIR}")
endif()


# ---- glm (Premake: ../vendor/glm)
add_library(glm_header_only INTERFACE)
target_include_directories(glm_header_only INTERFACE "${WALNUT_ROOT}/vendor/glm")

# ---- GLFW (Premake: vendor/glfw)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${WALNUT_ROOT}/vendor/GLFW" "${CMAKE_CURRENT_BINARY_DIR}/glfw")
set(WALNUT_GLFW_TARGET glfw)

# ---- ImGui (Premake: vendor/imgui, but we compile it as a normal CMake library)
set(IMGUI_DIR "${WALNUT_ROOT}/vendor/imgui")

file(GLOB IMGUI_SOURCES
  "${IMGUI_DIR}/*.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
  "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC
  "${IMGUI_DIR}"
  "${IMGUI_DIR}/backends"
  ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(ImGui PUBLIC
  ${WALNUT_GLFW_TARGET}
  ${Vulkan_LIBRARIES}
)
target_compile_definitions(ImGui PUBLIC
  GLFW_INCLUDE_NONE
)

# ---- Common defines like Premake filters
function(walnut_apply_defines tgt)
  if (WIN32)
    target_compile_definitions(${tgt} PRIVATE WL_PLATFORM_WINDOWS)
  endif()

  target_compile_definitions(${tgt} PRIVATE
    $<$<CONFIG:Debug>:WL_DEBUG>
    $<$<CONFIG:Release>:WL_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:WL_RELEASE>
    $<$<CONFIG:MinSizeRel>:WL_DIST>
  )
endfunction()

# ---- Walnut static library (Premake: Walnut/premake5.lua)
file(GLOB_RECURSE WALNUT_SOURCES
  "${WALNUT_ROOT}/Walnut/Source/*.h"
  "${WALNUT_ROOT}/Walnut/Source/*.cpp"
  "${WALNUT_ROOT}/Walnut/Platform/GUI/*.h"
  "${WALNUT_ROOT}/Walnut/Platform/GUI/*.cpp"
)

add_library(Walnut STATIC ${WALNUT_SOURCES})
add_library(Walnut::Walnut ALIAS Walnut)

source_group(TREE "${WALNUT_ROOT}" FILES ${WALNUT_SOURCES})

# C++ version
target_compile_features(Walnut PUBLIC cxx_std_20)

target_include_directories(Walnut PUBLIC
  "${WALNUT_ROOT}/Walnut/Source"
  "${WALNUT_ROOT}/Walnut/Platform/GUI"

  "${WALNUT_ROOT}/vendor/imgui"
  "${WALNUT_ROOT}/vendor/glfw/include"
  "${WALNUT_ROOT}/vendor/stb_image"

  "${WALNUT_ROOT}/vendor/glm"
  "${WALNUT_ROOT}/vendor/spdlog/include"
)

target_link_libraries(Walnut PUBLIC
  ImGui
  ${WALNUT_GLFW_TARGET}
  ${Vulkan_LIBRARIES}
  ${WALNUT_GLM_TARGET}
)

target_link_libraries(Walnut PUBLIC glm_header_only)

target_compile_options(Walnut PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)
walnut_apply_defines(Walnut)

set_target_properties(glfw uninstall update_mappings PROPERTIES FOLDER "Dependencies/WalnutDeps/GLFW3")
set_target_properties(uninstall PROPERTIES FOLDER "Dependencies/WalnutDeps/GLFW3")
set_target_properties(update_mappings PROPERTIES FOLDER "Dependencies/WalnutDeps/GLFW3")


