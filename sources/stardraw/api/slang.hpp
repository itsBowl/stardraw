#pragma once

#include "types.hpp"

namespace stardraw
{
    struct shader_macro
    {
        std::string name;
        std::string value;
    };

    struct shader_data
    {
        void* data;
        uint32_t data_size;
    };

    [[nodiscard]] status init_slang_session();
    [[nodiscard]] status load_slang_module(const std::string_view& name, const std::string_view& source);
    [[nodiscard]] status link_slang_shader(const std::string& shader_name, const std::string& entry_point_module, const std::string& entry_point_name, const std::vector<std::string>& additional_modules);
    [[nodiscard]] status link_slang_shader(const std::string& shader_name, const std::string& entry_point_module, const std::string& entry_point_name);
    [[nodiscard]] status get_shader_data(const std::string& shader_name, const graphics_api& api, shader_data& out_shader_data);
}

