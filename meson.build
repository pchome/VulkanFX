project('vkBasalt', ['c', 'cpp'], default_options: ['c_std=c11', 'cpp_std=c++23'])

cpp = meson.get_compiler('cpp')
add_project_arguments('-fPIC', '-fvisibility=hidden', language: ['c', 'cpp'])
add_project_arguments('-fvisibility-inlines-hidden',  language: ['cpp'])

# TODO: figure out how to iterate swapchainMap
#add_project_arguments('-DDISABLE_DEPTH_CAPTURE=1',  language: ['cpp'])

vkBasalt_include_path = ''

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
