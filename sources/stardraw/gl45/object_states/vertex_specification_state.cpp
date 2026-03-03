#include "vertex_specification_state.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
namespace stardraw::gl45
{
    vertex_specification_state::vertex_specification_state()
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Create vertex specification");
        glCreateVertexArrays(1, &vertex_array_id);
    }

    vertex_specification_state::~vertex_specification_state()
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Delete vertex specification");
        glDeleteVertexArrays(1, &vertex_array_id);
    }

    bool vertex_specification_state::is_valid() const
    {
        ZoneScoped;
        if (vertex_array_id == 0) return false;
        for (const GLuint buffer : vertex_buffers)
        {
            if (!glIsBuffer(buffer)) return false;
        }

        if (index_buffer != 0 && !glIsBuffer(index_buffer)) return false;

        return true;
    }

    bool vertex_specification_state::has_index_buffer() const
    {
        return index_buffer != 0;
    }

    status vertex_specification_state::bind() const
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Bind vertex specification");
        glBindVertexArray(vertex_array_id);
        return status_type::SUCCESS;
    }

    status vertex_specification_state::attach_vertex_buffer(const GLuint slot, const GLuint id, const GLintptr offset, const GLsizei stride)
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Attach vertex buffer to vertex specification");
        glVertexArrayVertexBuffer(vertex_array_id, slot, id, offset, stride);
        vertex_buffers.push_back(id);
        return status_type::SUCCESS;
    }

    status vertex_specification_state::attach_index_buffer(const GLuint index_buffer_id)
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Attach index buffer to vertex specification");
        glVertexArrayElementBuffer(vertex_array_id, index_buffer_id);
        index_buffer = index_buffer_id;
        return status_type::SUCCESS;
    }
}