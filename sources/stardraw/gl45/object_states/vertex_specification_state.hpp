#pragma once
#include "../gl_headers.hpp"
#include "../types.hpp"
#include "stardraw/api/commands.hpp"
namespace stardraw::gl45
{
    class vertex_specification_state final : public object_state
    {
    public:

        explicit vertex_specification_state();
        ~vertex_specification_state() override;

        [[nodiscard]] bool is_valid() const;
        [[nodiscard]] bool has_index_buffer() const;

        [[nodiscard]] status bind() const;
        [[nodiscard]] status attach_vertex_buffer(const GLuint slot, const GLuint id, const GLintptr offset, const GLsizei stride);
        [[nodiscard]] status attach_index_buffer(GLuint index_buffer_id);

        [[nodiscard]] descriptor_type object_type() const override
        {
            return descriptor_type::VERTEX_SPECIFICATION;
        }

        std::vector<GLuint> vertex_buffers;
        GLuint index_buffer = 0;
        GLuint vertex_array_id = 0;
    };
}
