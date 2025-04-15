data_dir = get_option('datadir')

vulkan_layer_dir = join_paths(data_dir, 'vulkan', 'implicit_layer.d')

configure_file(
    input : 'VulkanFX.json.in',
    output : 'VulkanFX.json',
    configuration : {'ld_lib_dir_vulkanfx' : ld_lib_dir_vulkanfx},
    install : true,
    install_dir : vulkan_layer_dir,
)
