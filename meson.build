project('VulkanFX', ['c', 'cpp'], default_options: ['c_std=c11', 'cpp_std=c++23'])

cpp = meson.get_compiler('cpp')
add_project_arguments('-fvisibility-inlines-hidden',  language: ['cpp'])

vulkanfx_include_path = ''

lib_dir = get_option('libdir')
ld_lib_dir_vkbasalt = ''

if get_option('append_libdir_vkbasalt')
    lib_dir = join_paths(lib_dir, 'vkbasalt')
    ld_lib_dir_vkbasalt = get_option('prefix') + '/$LIB/vkbasalt/'
endif

if get_option('with_so')
    subdir('src')
endif

if get_option('with_json')
    subdir('config')
endif
