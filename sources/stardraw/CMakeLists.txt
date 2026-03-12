add_library(stardraw STATIC
        vk13/vk_pipeline.hpp
        vk13/vk_pipeline.cpp
        vk13/vk_render.cpp
        vk13/vk_render.cpp
        vk13/vk_swapchain.hpp
        vk13/vk_swapchain.cpp)

target_sources(stardraw PRIVATE

        internal/window.cpp
        internal/shaders.cpp
        internal/glfw_window.hpp internal/glfw_window.cpp

        gl45/commands_impl.cpp
        gl45/render_context.hpp gl45/render_context.cpp
        gl45/types.hpp
        gl45/window.hpp gl45/window.cpp
        gl45/staging_buffer_uploader.hpp gl45/staging_buffer_uploader.cpp
        gl45/object_states/buffer_state.hpp gl45/object_states/buffer_state.cpp
        gl45/object_states/draw_specification_state.hpp gl45/object_states/draw_specification_state.cpp
        gl45/object_states/shader_state.hpp gl45/object_states/shader_state.cpp
        gl45/object_states/texture_state.hpp gl45/object_states/texture_state.cpp
        gl45/object_states/vertex_specification_state.hpp gl45/object_states/vertex_specification_state.cpp

        vk13/vk_device.hpp vk13/vk_device.cpp
        vk13/simple_window.hpp vk13/simple_window.cpp
)

target_sources(stardraw PUBLIC FILE_SET HEADERS BASE_DIRS ${H_SOURCES_ROOT} FILES
        api/render_context.hpp
        api/types.hpp
        api/descriptors.hpp
        api/commands.hpp
        api/window.hpp
        api/shaders.hpp
        api/shader_parameter_value.hpp
)
message(Vulkan Dirs: ${Vulkan_INCLUDE_DIRS})

target_include_directories(stardraw PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/stardraw>
        ${Vulkan_INCLUDE_DIRS}
)

target_compile_options(stardraw PUBLIC /EHsc)

set_target_properties(stardraw PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(stardraw PRIVATE ${OPENGL_INCLUDE_DIR})

target_link_libraries(stardraw PUBLIC starlib)
target_link_libraries(stardraw PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(stardraw PRIVATE TracyClient)
target_link_libraries(stardraw PRIVATE glad)
target_link_libraries(stardraw PRIVATE slang::slang)
target_link_libraries(stardraw PRIVATE Vulkan::Vulkan)
target_link_libraries(stardraw PRIVATE glfw)
target_link_libraries(stardraw PRIVATE spirv-cross-glsl)


add_executable(StardrawTest ./test/main.cpp ./vk13/simple_window.cpp

        vk13/vk_pipeline.cpp

        vk13/vk_render.cpp

        vk13/vk_swapchain.cpp

        vk13/vk_device.cpp)

target_link_libraries(StardrawTest PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(StardrawTest PUBLIC TracyClient)
target_link_libraries(StardrawTest PUBLIC glad)
target_link_libraries(StardrawTest PUBLIC slang::slang)
target_link_libraries(StardrawTest PUBLIC Vulkan::Vulkan)
target_link_libraries(StardrawTest PUBLIC glfw)
target_link_libraries(StardrawTest PUBLIC spirv-cross-glsl)

