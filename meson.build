project('VulkanFX', ['c', 'cpp'], version: '0.4.0', default_options: ['c_std=c11', 'cpp_std=c++23'])

cpp = meson.get_compiler('cpp')
add_project_arguments('-fvisibility-inlines-hidden',  language: ['cpp'])

vulkanfx_include_path = ''

lib_dir = get_option('libdir')
ld_lib_dir_vulkanfx = ''

if get_option('append_libdir_vulkanfx')
    lib_dir = join_paths(lib_dir, 'vulkanfx')
    ld_lib_dir_vulkanfx = get_option('prefix') + '/$LIB/vulkanfx/'
endif

if get_option('with_so')
    subdir('src')
endif

if get_option('with_json')
    subdir('config')
endif
