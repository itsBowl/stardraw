#include "texture_state.hpp"
#include <format>
#include <spirv_glsl.hpp>

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

namespace stardraw::gl45
{
    GLenum texture_type_to_gl_target(const texture_shape type, const bool multisample, const bool is_array)
    {
        switch (type)
        {
            case texture_shape::_1D: return is_array ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
            case texture_shape::_2D: return multisample ? (is_array ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_MULTISAMPLE) : (is_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);
            case texture_shape::_3D: return GL_TEXTURE_3D;
            case texture_shape::CUBE_MAP: return is_array ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
        }
        return 0;
    }

    GLenum texture_data_type_to_gl_format(const texture_data_type type)
    {
        switch (type)
        {
            case texture_data_type::DEPTH_32: return GL_DEPTH_COMPONENT32;
            case texture_data_type::DEPTH_24: return GL_DEPTH_COMPONENT24;
            case texture_data_type::DEPTH_16: return GL_DEPTH_COMPONENT16;
            case texture_data_type::DEPTH_32_STENCIL_8: return GL_DEPTH32F_STENCIL8;
            case texture_data_type::DEPTH_24_STENCIL_8: return GL_DEPTH24_STENCIL8;
            case texture_data_type::STENCIL_8: return GL_STENCIL_INDEX8;
            case texture_data_type::R_8: return GL_R8;
            case texture_data_type::RG_8: return GL_RG8;
            case texture_data_type::RGB_8: return GL_RGB8;
            case texture_data_type::RGBA_8: return GL_RGBA8;
            case texture_data_type::SRGB_8: return GL_SRGB8;
            case texture_data_type::SRGBA_8: return GL_SRGB8_ALPHA8;
            case texture_data_type::R_U8: return GL_R8UI;
            case texture_data_type::RG_U8: return GL_RG8UI;
            case texture_data_type::RGB_U8: return GL_RGB8UI;
            case texture_data_type::RGBA_U8: return GL_RGBA8UI;
            case texture_data_type::R_U16: return GL_R16UI;
            case texture_data_type::RG_U16: return GL_RG16UI;
            case texture_data_type::RGB_U16: return GL_RGB16UI;
            case texture_data_type::RGBA_U16: return GL_RGBA16UI;
            case texture_data_type::R_U32: return GL_R32UI;
            case texture_data_type::RG_U32: return GL_RG32UI;
            case texture_data_type::RGB_U32: return GL_RGB32UI;
            case texture_data_type::RGBA_U32: return GL_RGBA32UI;
            case texture_data_type::R_I8: return GL_R8I;
            case texture_data_type::RG_I8: return GL_RG8I;
            case texture_data_type::RGB_I8: return GL_RGB8I;
            case texture_data_type::RGBA_I8: return GL_RGBA8I;
            case texture_data_type::R_I16: return GL_R16I;
            case texture_data_type::RG_I16: return GL_RG16I;
            case texture_data_type::RGB_I16: return GL_RGB16I;
            case texture_data_type::RGBA_I16: return GL_RGBA16I;
            case texture_data_type::R_I32: return GL_R32I;
            case texture_data_type::RG_I32: return GL_RG32I;
            case texture_data_type::RGB_I32: return GL_RGB32I;
            case texture_data_type::RGBA_I32: return GL_RGBA32I;
            case texture_data_type::R_F16: return GL_R16F;
            case texture_data_type::RG_F16: return GL_RG16F;
            case texture_data_type::RGB_F16: return GL_RGB16F;
            case texture_data_type::RGBA_F16: return GL_RGBA16F;
            case texture_data_type::R_F32: return GL_R32F;
            case texture_data_type::RG_F32: return GL_RG32F;
            case texture_data_type::RGB_F32: return GL_RGB32F;
            case texture_data_type::RGBA_F32: return GL_RGBA32F;
        }

        return 0;
    }

    bool is_texture_data_type_integer(const texture_data_type type)
    {
        switch (type)
        {
            case texture_data_type::DEPTH_32_STENCIL_8:
            case texture_data_type::DEPTH_24_STENCIL_8:
            case texture_data_type::R_U8:
            case texture_data_type::RG_U8:
            case texture_data_type::RGB_U8:
            case texture_data_type::RGBA_U8:
            case texture_data_type::R_U16:
            case texture_data_type::RG_U16:
            case texture_data_type::RGB_U16:
            case texture_data_type::RGBA_U16:
            case texture_data_type::R_U32:
            case texture_data_type::RG_U32:
            case texture_data_type::RGB_U32:
            case texture_data_type::RGBA_U32:
            case texture_data_type::R_I8:
            case texture_data_type::RG_I8:
            case texture_data_type::RGB_I8:
            case texture_data_type::RGBA_I8:
            case texture_data_type::R_I16:
            case texture_data_type::RG_I16:
            case texture_data_type::RGB_I16:
            case texture_data_type::RGBA_I16:
            case texture_data_type::R_I32:
            case texture_data_type::RG_I32:
            case texture_data_type::RGB_I32:
            case texture_data_type::RGBA_I32:
            {
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    bool is_sampling_config_valid_for_type(const texture_data_type type, const texture_sampling_conifg& sampling_config)
    {
        const bool is_integer_type = is_texture_data_type_integer(type);
        if (!is_integer_type) return true;

        if (sampling_config.upscale_filter != texture_filtering_mode::NEAREST) return false;
        if (sampling_config.downscale_filter != texture_filtering_mode::NEAREST) return false;
        if (sampling_config.mipmap_filter != texture_filtering_mode::NEAREST) return false;

        return true;
    }

    texture_state::texture_state(const texture_descriptor& desc, status& out_status)
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Create texture object");

        num_texture_mipmap_levels = desc.format.mipmap_levels;
        num_texture_array_layers = desc.format.texture_layers;

        const bool has_msaa = desc.format.msaa != texture_msaa_level::NONE;
        const bool can_type_be_array = desc.format.shape != texture_shape::_3D;
        const bool can_shape_be_multisample = desc.format.shape == texture_shape::_2D || desc.format.shape == texture_shape::_3D;
        const bool is_array = (desc.format.shape != texture_shape::CUBE_MAP && num_texture_array_layers > 1) || num_texture_array_layers > 6;

        gl_texture_target = texture_type_to_gl_target(desc.format.shape, has_msaa, is_array);
        gl_texture_format = texture_data_type_to_gl_format(desc.format.data_type);

        const u32 width = desc.format.width;
        const u32 height = desc.format.height;
        const u32 depth = desc.format.depth;
        const u8 multisample_count = static_cast<u8>(desc.format.msaa);

        if (num_texture_mipmap_levels <= 0)
        {
            out_status = {status_type::INVALID, std::format("Texture '{0}' has zero mipmap layers", desc.identifier().name)};
            return;
        }

        if (num_texture_array_layers <= 0)
        {
            out_status = {status_type::INVALID, std::format("Texture '{0}' has zero texture layers", desc.identifier().name)};
            return;
        }

        if (!can_shape_be_multisample && has_msaa)
        {
            out_status = {status_type::INVALID, std::format("Texture '{0}' shape cannot have MSAA, but msaa level is not zero", desc.identifier().name)};
            return;
        }

        if (!can_type_be_array && is_array)
        {
            out_status = {status_type::INVALID, std::format("Texture '{0}' shape cannot be an array texture, but has more than 1 texture layer", desc.identifier().name)};
            return;
        }

        if (desc.format.shape == texture_shape::CUBE_MAP && num_texture_array_layers % 6 != 0)
        {
            out_status = {status_type::INVALID, std::format("Texture '{0}' is a cubemap array texture, but number of texture layers is not a multiple of 6", desc.identifier().name)};
            return;
        }

        if (!is_sampling_config_valid_for_type(desc.format.data_type, desc.default_sampling_config))
        {
            out_status = {status_type::INVALID, std::format("Provided default sampling config isn't valid for texture '{0}' (are you trying to use linear filtering on an integer texture?)", desc.identifier().name)};
            return;
        }

        glCreateTextures(gl_texture_target, 1, &texture_id);

        if (texture_id == 0)
        {
            out_status = {status_type::BACKEND_ERROR, std::format("Creating texture {0} failed", desc.identifier().name)};
            return;
        }

        switch (desc.format.shape)
        {
            case texture_shape::_1D:
            {
                if (num_texture_array_layers > 1) glTextureStorage2D(texture_id, num_texture_mipmap_levels, gl_texture_format, width, num_texture_array_layers);
                else glTextureStorage1D(texture_id, num_texture_mipmap_levels, gl_texture_format, width);
                break;
            }
            case texture_shape::_3D:
            {
                glTextureStorage3D(texture_id, num_texture_mipmap_levels, gl_texture_format, width, height, depth);
                break;
            }
            case texture_shape::_2D:
            case texture_shape::CUBE_MAP:
            {
                if (num_texture_array_layers > 1 && has_msaa)
                {
                    //It's unclear whether the last parameter (fixed sample locations) is actually used by any implementations, so for now I'm not supporting customizing it.
                    glTextureStorage3DMultisample(texture_id, multisample_count, gl_texture_format, width, height, num_texture_array_layers, GL_TRUE);
                    break;
                }

                if (num_texture_array_layers > 1)
                {
                    glTextureStorage3D(texture_id, num_texture_mipmap_levels, gl_texture_format, width, height, num_texture_array_layers);
                    break;
                }

                if (has_msaa)
                {
                    //It's unclear whether the last parameter (fixed sample locations) is actually used by any implementations, so for now I'm not supporting customizing it.
                    glTextureStorage2DMultisample(texture_id, multisample_count, gl_texture_format, width, height, GL_TRUE);
                    break;
                }

                glTextureStorage2D(texture_id, num_texture_mipmap_levels, gl_texture_format, width, height);
                break;
            }
        }

        texture_sampling_conifg sampling_config = desc.default_sampling_config;
        sampling_config.mipmap_max_level = std::min(sampling_config.mipmap_max_level, num_texture_mipmap_levels - 1);
        out_status = set_sampling_config(sampling_config);
    }

    texture_state::texture_state(const texture_state* original, const texture_descriptor& desc, status& out_status)
    {
        const status compatibility_status = original->is_view_compatible(desc);
        if (is_status_error(compatibility_status))
        {
            out_status = compatibility_status;
            return;
        }

        num_texture_mipmap_levels = 1 + desc.format.mipmap_levels;
        num_texture_array_layers = desc.format.texture_layers;

        const bool has_msaa = desc.format.msaa != texture_msaa_level::NONE;
        const bool can_type_be_array = desc.format.shape != texture_shape::_3D;
        const bool can_shape_be_multisample = desc.format.shape == texture_shape::_2D || desc.format.shape == texture_shape::_3D;
        const bool is_array = (desc.format.shape != texture_shape::CUBE_MAP && num_texture_array_layers > 1) || num_texture_array_layers > 6;

        gl_texture_target = texture_type_to_gl_target(desc.format.shape, desc.format.msaa != texture_msaa_level::NONE, is_array);
        gl_texture_format = texture_data_type_to_gl_format(desc.format.data_type);

        if (num_texture_mipmap_levels <= 0)
        {
            out_status = {status_type::INVALID, std::format("Texture view '{0}' includes zero mipmap layers", desc.identifier().name)};
            return;
        }

        if (num_texture_array_layers <= 0)
        {
            out_status = {status_type::INVALID, std::format("Texture view '{0}' includes zero texture layers", desc.identifier().name)};
            return;
        }

        if (!can_shape_be_multisample && has_msaa)
        {
            out_status = {status_type::INVALID, std::format("Texture view '{0}' shape cannot have MSAA, but msaa level is not zero", desc.identifier().name)};
            return;
        }

        if (!can_type_be_array && is_array)
        {
            out_status = {status_type::INVALID, std::format("Texture view '{0}' shape cannot be an array texture, but includes too many texture layers", desc.identifier().name)};
            return;
        }

        if (desc.format.shape == texture_shape::CUBE_MAP && num_texture_array_layers % 6 != 0)
        {
            out_status = {status_type::INVALID, std::format("Texture view '{0}' shape is cubemap, but number of texture layers included is not a multiple of 6", desc.identifier().name)};
            return;
        }

        if (!is_sampling_config_valid_for_type(desc.format.data_type, desc.default_sampling_config))
        {
            out_status = {status_type::INVALID, std::format("Provided default sampling config isn't valid for texture view '{0}' (are you trying to use linear filtering on an integer texture?)", desc.identifier().name)};
            return;
        }

        glGenTextures(1, &texture_id);

        if (texture_id == 0)
        {
            out_status = {status_type::BACKEND_ERROR, std::format("Creating texture view {0} failed", desc.identifier().name)};
            return;
        }

        glTextureView(texture_id, gl_texture_target, original->texture_id, gl_texture_format, desc.format.view_texture_base_mipmap, desc.format.mipmap_levels, desc.format.view_texture_base_array_index, num_texture_array_layers);

        texture_sampling_conifg sampling_config = desc.default_sampling_config;
        sampling_config.mipmap_max_level = std::min(sampling_config.mipmap_max_level, num_texture_mipmap_levels - 1);
        out_status = set_sampling_config(sampling_config);
    }

    texture_state::~texture_state()
    {
        ZoneScoped;
        TracyGpuZone("[Stardraw] Delete texture object");
        glDeleteTextures(1, &texture_id);
    }

    GLenum gl_filter_mode_from_modes(const texture_filtering_mode filter, const texture_filtering_mode mipmap_selection)
    {
        const u8 option = static_cast<u8>(filter) + static_cast<u8>(mipmap_selection) * 2;
        switch (option)
        {
            case 0: return GL_NEAREST_MIPMAP_NEAREST; //Nearest nearest
            case 1: return GL_LINEAR_MIPMAP_NEAREST;  //Linear nearest
            case 2: return GL_NEAREST_MIPMAP_LINEAR;  //Nearest linear
            case 3: return GL_LINEAR_MIPMAP_LINEAR;   //Linear linear
            default: return -1;
        }
    }

    inline void set_texture_border_color(const GLuint texture_id, const texture_border_color border_color)
    {
        switch (border_color)
        {
            case texture_border_color::INTEGER_BLACK:
            {
                constexpr std::array col = {0, 0, 0, 1};
                glTextureParameteriv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
            case texture_border_color::INTEGER_WHITE:
            {
                constexpr std::array col = {1, 1, 1, 1};
                glTextureParameteriv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
            case texture_border_color::INTEGER_TRANSPARENT:
            {
                constexpr std::array col = {0, 0, 0, 0};
                glTextureParameteriv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
            case texture_border_color::FLOAT_BLACK:
            {
                constexpr std::array col = {0.f, 0.f, 0.f, 1.f};
                glTextureParameterfv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
            case texture_border_color::FLOAT_WHITE:
            {
                constexpr std::array col = {1.f, 1.f, 1.f, 1.f};
                glTextureParameterfv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
            case texture_border_color::FLOAT_TRANSPARENT:
            {
                constexpr std::array col = {0.f, 0.f, 0.f, 0.f};
                glTextureParameterfv(texture_id, GL_TEXTURE_BORDER_COLOR, col.data());
                break;
            }
        }
    }

    GLenum gl_wrapping_mode(const texture_wrapping_mode mode)
    {
        switch (mode)
        {
            case texture_wrapping_mode::CLAMP: return GL_CLAMP;
            case texture_wrapping_mode::REPEAT: return GL_REPEAT;
            case texture_wrapping_mode::MIRROR: return GL_MIRRORED_REPEAT;
            case texture_wrapping_mode::BORDER: return GL_CLAMP_TO_BORDER;
        }
        return -1;
    }

    GLenum gl_swizzle_mode(const texture_swizzle swizzle)
    {
        switch (swizzle)
        {
            case texture_swizzle::RED: return GL_RED;
            case texture_swizzle::GREEN: return GL_GREEN;
            case texture_swizzle::BLUE: return GL_BLUE;
            case texture_swizzle::ALPHA: return GL_ALPHA;
        }
        return -1;
    }

    bool texture_state::is_valid() const
    {
        return texture_id != 0;
    }

    bool is_view_format_compatible(const GLenum source_format, const GLenum view_format)
    {
        const std::array<std::vector<GLenum>, 12> compatible_sets = {
            std::vector<GLenum> {GL_RGBA32F, GL_RGBA32UI, GL_RGBA32I},
            {GL_RGB32F, GL_RGB32UI, GL_RGB32I},
            {GL_RGBA16F, GL_RG32F, GL_RGBA16UI, GL_RG32UI, GL_RGBA16I, GL_RG32I, GL_RGBA16, GL_RGBA16_SNORM},
            {GL_RGB16, GL_RGB16_SNORM, GL_RGB16F, GL_RGB16UI, GL_RGB16I},
            {GL_RG16F, GL_R11F_G11F_B10F, GL_R32F, GL_RGB10_A2UI, GL_RGBA8UI, GL_RG16UI, GL_R32UI, GL_RGBA8I, GL_RG16I, GL_R32I, GL_RGB10_A2, GL_RGBA8, GL_RG16, GL_RGBA8_SNORM, GL_RG16_SNORM, GL_SRGB8_ALPHA8, GL_RGB9_E5},
            {GL_RGB8, GL_RGB8_SNORM, GL_SRGB8, GL_RGB8UI, GL_RGB8I},
            {GL_R16F, GL_RG8UI, GL_R16UI, GL_RG8I, GL_R16I, GL_RG8, GL_R16, GL_RG8_SNORM, GL_R16_SNORM},
            {GL_R8UI, GL_R8I, GL_R8, GL_R8_SNORM},
            {GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1},
            {GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2},
            {GL_COMPRESSED_RGBA_BPTC_UNORM, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM},
            {GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT}
        };

        const auto set_ptr = std::ranges::find_if(compatible_sets, [view_format](const std::vector<GLenum>& set)
        {
            return std::ranges::contains(set, view_format);
        });

        if (set_ptr == compatible_sets.end()) return false;
        return std::ranges::contains(*set_ptr, source_format);
    }

    bool is_view_target_compatible(const GLenum source_target, const GLenum view_target)
    {
        const std::array<std::vector<GLenum>, 11> compatible_sets = {
            std::vector<GLenum> {GL_TEXTURE_1D, GL_TEXTURE_1D_ARRAY},
            {GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY},
            {GL_TEXTURE_3D},
            {GL_TEXTURE_CUBE_MAP, GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP_ARRAY},
            {GL_TEXTURE_RECTANGLE},
            {GL_TEXTURE_1D_ARRAY, GL_TEXTURE_1D},
            {GL_TEXTURE_2D_ARRAY, GL_TEXTURE_2D},
            {GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_CUBE_MAP},
            {GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY},
            {GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_2D_MULTISAMPLE},
        };

        const auto set_ptr = std::ranges::find_if(compatible_sets, [source_target](const std::vector<GLenum>& set)
        {
            return set[0] == source_target; //Target compatibility is not 100% commutable - a 2D texture can view into a cube map, but not vice versa.
        });

        if (set_ptr == compatible_sets.end()) return false;
        return std::ranges::contains(*set_ptr, view_target);
    }

    status texture_state::is_view_compatible(const texture_descriptor& view_descriptor) const
    {
        const u32 min_view_mipmap = view_descriptor.format.view_texture_base_mipmap;
        const u32 max_view_mipmap = min_view_mipmap + view_descriptor.format.mipmap_levels - 1;

        const u32 min_view_layer = view_descriptor.format.view_texture_base_array_index;
        const u32 max_view_layer = min_view_mipmap + view_descriptor.format.texture_layers - 1;

        const bool is_array = (view_descriptor.format.shape != texture_shape::CUBE_MAP && view_descriptor.format.texture_layers > 1) || view_descriptor.format.texture_layers > 6;

        if (min_view_mipmap + 1 > num_texture_mipmap_levels) return {status_type::RANGE_OVERFLOW, std::format("Texture view '{0}' is not compatible with source - source has {1} mipmap levels, but requesting index {2}", view_descriptor.identifier().name, num_texture_mipmap_levels, min_view_mipmap)};
        if (max_view_mipmap + 1 > num_texture_mipmap_levels) return {status_type::RANGE_OVERFLOW, std::format("Texture view '{0}' is not compatible with source - source has {1} mipmap levels, but requesting index {2}", view_descriptor.identifier().name, num_texture_mipmap_levels, max_view_mipmap)};
        if (min_view_layer + 1 > num_texture_array_layers) return {status_type::RANGE_OVERFLOW, std::format("Texture view '{0}' is not compatible with source - source has {1} texture layers, but requesting index {2}", view_descriptor.identifier().name, num_texture_array_layers, min_view_layer)};
        if (max_view_layer + 1 > num_texture_array_layers) return {status_type::RANGE_OVERFLOW, std::format("Texture view '{0}' is not compatible with source - source has {1} texture layers, but requesting index {2}", view_descriptor.identifier().name, num_texture_array_layers, max_view_layer)};
        if (!is_view_format_compatible(gl_texture_format, texture_data_type_to_gl_format(view_descriptor.format.data_type))) return {status_type::INVALID, std::format("Texture view '{0}' is not compatible with source - texture data types are not compatible", view_descriptor.identifier().name)};
        if (!is_view_target_compatible(gl_texture_target, texture_type_to_gl_target(view_descriptor.format.shape, view_descriptor.format.msaa != texture_msaa_level::NONE, is_array))) return {status_type::INVALID, std::format("Texture view '{0}' is not compatible with source - texture shapes are not compatible", view_descriptor.identifier().name)};
        return status_type::SUCCESS;
    }

    status texture_state::set_sampling_config(const texture_sampling_conifg& config) const
    {
        glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, gl_filter_mode_from_modes(config.upscale_filter, config.mipmap_filter));
        glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, gl_filter_mode_from_modes(config.downscale_filter, config.mipmap_filter));

        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, gl_wrapping_mode(config.wrapping_mode_x));
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, gl_wrapping_mode(config.wrapping_mode_y));
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_R, gl_wrapping_mode(config.wrapping_mode_z));

        set_texture_border_color(texture_id, config.border_color);

        if (GLAD_GL_VERSION_4_6 || GLAD_GL_ARB_texture_filter_anisotropic || GLAD_GL_EXT_texture_filter_anisotropic)
        {
            //Anisotropy is not *strictly* core, but it's supported practically universally
            glTextureParameterf(texture_id, GL_TEXTURE_MAX_ANISOTROPY, static_cast<float>(config.anisotropy_level));
        }

        glTextureParameteri(texture_id, GL_TEXTURE_BASE_LEVEL, config.mipmap_min_level);
        glTextureParameteri(texture_id, GL_TEXTURE_MAX_LEVEL, config.mipmap_max_level);
        glTextureParameterf(texture_id, GL_TEXTURE_LOD_BIAS, config.mipmap_bias);

        glTextureParameteri(texture_id, GL_TEXTURE_SWIZZLE_R, gl_swizzle_mode(config.swizzling.swizzle_red));
        glTextureParameteri(texture_id, GL_TEXTURE_SWIZZLE_G, gl_swizzle_mode(config.swizzling.swizzle_green));
        glTextureParameteri(texture_id, GL_TEXTURE_SWIZZLE_B, gl_swizzle_mode(config.swizzling.swizzle_blue));
        glTextureParameteri(texture_id, GL_TEXTURE_SWIZZLE_A, gl_swizzle_mode(config.swizzling.swizzle_alpha));

        return status_type::SUCCESS;
    }
}
