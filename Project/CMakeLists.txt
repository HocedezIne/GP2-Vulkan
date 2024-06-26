file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/resources" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} -g ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)


set(SOURCES
    "main.cpp"
    "vulkanbase/VulkanBase.h" 
    "vulkanbase/VulkanUtil.h"
    # Add other source files here
    "labwork/Week01.cpp"
    "labwork/Week02.cpp" 
    "labwork/Week03.cpp" 
    "labwork/Week04.cpp" 
    "labwork/Week05.cpp"
    "labwork/Week06.cpp"
    "GP2_Vertex.h" 
    "GP2_Shader.h"  
    "GP2_CommandPool.h" "GP2_CommandPool.cpp" 
    "CommandBuffer.h" "CommandBuffer.cpp" 
    "GP2_Mesh.h"  
    "GP2_Buffer.h" "GP2_Buffer.cpp" 
    "GP2_GraphicsPipeline2D.h" "GP2_GraphicsPipeline3D.h" 
    "GP2_DescriptorPool.h" 
    "GP2_UniformBufferObject.h" 
    "GP2_ImageBuffer.h" "GP2_ImageBuffer.cpp" 
    "GP2_DepthBuffer.h" "GP2_DepthBuffer.cpp" 
    "GP2_PBRSpecularPipeline.h" "GP2_PBRMetalnessPipeline.h" "GP2_PBRBasePipeline.h" 
    "jsonParser.h")

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty")
set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${GLSL_SOURCE_FILES}  "vulkanbase/VulkanUtil.cpp" "labwork/Week03.cpp")
add_dependencies(${PROJECT_NAME} Shaders)
# Link libraries
# target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${THIRD_PARTY_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)
