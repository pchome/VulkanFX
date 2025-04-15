subdir('shader')

reshade_dep = dependency('ReShadeFX')
vkroots_dep = dependency('vkroots')
stb_dep = dependency('stb')

# TODO: split depth, reshade and simple sources
vulkanfx_src = [
    'basalt.cpp',
    'buffer.cpp',
    'command_buffer.cpp',
    'config.cpp',
    'descriptor_set.cpp',

    'effect.cpp',
    'effect_simple.cpp',
    'effect_transfer.cpp',

    'fake_swapchain.cpp',
    'format.cpp',
    'framebuffer.cpp',
    'graphics_pipeline.cpp',
    'image.cpp',
    'image_view.cpp',
    'keyboard_input.cpp',
    'logger.cpp',
    'logical_swapchain.cpp',
    'lut_cube.cpp',
    'memory.cpp',
    'renderpass.cpp',

    'sampler.cpp',
    'shader.cpp',
    'stb_image.c',
    'stb_image_resize.c',
    'util.cpp',
]

simple_effects_src = files([
    'effect_cas.cpp',
    'effect_deband.cpp',
    'effect_dls.cpp',
    'effect_fxaa.cpp',
    'effect_lut.cpp',
    'effect_smaa.cpp',
])

reshadefx_src = [
    'effect_reshade.cpp',
    'reshade_uniforms.cpp',
]

x11_dep = dependency('x11')

conf_paths = configuration_data()
conf_paths.set_quoted('SYSCONFDIR', join_paths(get_option('prefix'), get_option('sysconfdir')))
conf_paths.set_quoted('DATADIR', join_paths(get_option('prefix'), get_option('datadir')))

configure_file(output: 'config_paths.hpp',
               configuration : conf_paths)

# X11/X.h library contains many definitions (e.g. None)
# which conflicts with the rest codebase with with Meson --unity=on option
keyboard_input_x11_lib = static_library('keyboard_input_x11', 'keyboard_input_x11.cpp',
    gnu_symbol_visibility: 'hidden',
    install : false)

simple_effects_lib = static_library('simple_effects', simple_effects_src, shader_include,
    gnu_symbol_visibility: 'hidden',
    install : false)

shared_library('VkLayer_VulkanFX_default', vulkanfx_src, reshadefx_src,
    cpp_args: ['-DDISABLE_DEPTH_CAPTURE=1'],
    link_with: [keyboard_input_x11_lib, simple_effects_lib],
    include_directories : vulkanfx_include_path,
    dependencies : [x11_dep, reshade_dep, vkroots_dep, stb_dep],
    gnu_symbol_visibility: 'hidden',
    install : true,
    install_dir : lib_dir)

shared_library('VkLayer_VulkanFX_depth', vulkanfx_src, reshadefx_src,
    link_with: [keyboard_input_x11_lib, simple_effects_lib],
    include_directories : vulkanfx_include_path,
    dependencies : [x11_dep, reshade_dep, vkroots_dep, stb_dep],
    gnu_symbol_visibility: 'hidden',
    install : true,
    install_dir : lib_dir)

shared_library('VkLayer_VulkanFX_simple', vulkanfx_src,
    cpp_args: ['-DDISABLE_RESHADEFX=1', '-DDISABLE_DEPTH_CAPTURE=1'],
    link_with: [keyboard_input_x11_lib, simple_effects_lib],
    include_directories : vulkanfx_include_path,
    dependencies : [x11_dep, vkroots_dep, stb_dep],
    gnu_symbol_visibility: 'hidden',
    install : true,
    install_dir : lib_dir)
