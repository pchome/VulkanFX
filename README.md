# VulkanFX (fork of vkBasalt with some spices)
vkBasalt is a Vulkan post processing layer to enhance the visual graphics of games.

Currently, the build in effects are:
- Contrast Adaptive Sharpening
- Denoised Luma Sharpening
- Fast Approximate Anti-Aliasing
- Enhanced Subpixel Morphological Anti-Aliasing
- 3D color LookUp Table

It is also possible to use ReShade FX shaders.

## Fork difference
So far:
- unbundled `stb` headers, use system package
- unbundeled `ReShade FX`, bumped to most recent version, use system package
- migrated to `vkroots`, use system package
- split in 3 parts, removed `depthCapture` option from config, config initialized only once
- `VULKANFX_CONFIG` environment variable


## Disclaimer
Expect it to have bugs. Use it at your own risk.

## Building from Source

### Dependencies
Before building, you will need:
- GCC >= 14
- x11-libs/libX11
- dev-util/glslang
- dev-util/spirv-headers
- dev-util/vulkan-headers
- dev-libs/stb
- media-libs/vkroots
- dev-util/vma ([Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator))
- \>=dev-util/reshade-fx-6.4.1 ([Parser/Codegen for the ReShade FX shader language](https://github.com/crosire/reshade/tree/main/source))
- (optional) dev-util/reshade-fxc ([ReShade FX shader compiler](https://github.com/crosire/reshade/tree/main/tools))<br>
  In my gentoo overlay it also contain patch to generate config from ReShade FX shader.<br>
  <details>
    <summary>e.g. `$ reshade-fx2conf Daltonize.fx`</summary>

    ```
    # Daltonize.fx

    # type: combo
    # items: Protanopia
    #        Deuteranopia
    #        Tritanopia
    #
    #Type = 0

    # Directives
    #RESHADE_DEPTH_INPUT_X_OFFSET=0
    #RESHADE_DEPTH_INPUT_Y_PIXEL_OFFSET=0
    #RESHADE_DEPTH_INPUT_X_PIXEL_OFFSET=0
    #RESHADE_DEPTH_INPUT_Y_OFFSET=0
    #RESHADE_DEPTH_INPUT_Y_SCALE=1
    #RESHADE_DEPTH_INPUT_X_SCALE=1
    #RESHADE_DEPTH_LINEARIZATION_FAR_PLANE=1000.0
    #RESHADE_DEPTH_INPUT_IS_LOGARITHMIC=0
    #RESHADE_DEPTH_INPUT_IS_MIRRORED=0
    #RESHADE_DEPTH_INPUT_IS_REVERSED=1
    #RESHADE_DEPTH_MULTIPLIER=1
    #RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN=0
    ```

  </details>


### Building

#### Gentoo
Prepare overlay
```sh
emerge app-eselect/eselect-repository -vp
eselect repository add vulkanfx git https://github.com/pchome/VulkanFX-gentoo-overlay.git
emaint sync -r vulkanfx
```

##### 64bit
```sh
emerge media-gfx/VulkanFX -q
```

##### 32bit
nano /etc/portage/package.use/multilib
```vim
media-gfx/VulkanFX abi_x86_32
dev-util/reshade-fx abi_x86_32
dev-util/reshade-fxc abi_x86_32
# on/off hotkey
x11-libs/libX11 abi_x86_32
# required by x11-libs/libX11-1.8.11::gentoo
# required by media-gfx/VulkanFX-9999::vulkanfx[xlib]
# required by media-gfx/VulkanFX (argument)
>=x11-libs/libxcb-1.17.0 abi_x86_32
# required by x11-libs/libxcb-1.17.0::gentoo
# required by x11-libs/libX11-1.8.11::gentoo
# required by media-gfx/VulkanFX-9999::vulkanfx[xlib]
# required by media-gfx/VulkanFX (argument)
>=x11-libs/libXau-1.0.12 abi_x86_32
# required by x11-libs/libxcb-1.17.0::gentoo
# required by x11-libs/libX11-1.8.11::gentoo
# required by media-gfx/VulkanFX-9999::vulkanfx[xlib]
# required by media-gfx/VulkanFX (argument)
>=x11-libs/libXdmcp-1.1.5 abi_x86_32
```
```sh
emerge media-gfx/VulkanFX -q
```

#### non-Gentoo
You can try to follow [pchome/gentoo-dev-playground](https://github.com/pchome/gentoo-dev-playground) instructions and then return to gentoo route.

## Usage
Enable the layer with the environment variable.

### Standard
Implicit vulkan layers will be loaded automatically when corresponding environment variable was provided. Use one of:
* `ENABLE_VULKANFX=1` - default: depth capture disabled
* `ENABLE_VULKANFX=depth` - depth: depth capture enabled
* `ENABLE_VULKANFX=simple` - simple: depth capture and ReShade FX disabled

### Testing
#### System
##### Get some ReShade FX shaders
`$ git clone https://github.com/crosire/reshade-shaders /tmp/fxs`
##### Launch an vulkan application
`$ ENABLE_VULKANFX=1 VULKANFX_CONFIG='effects=fxaa:cas:dlt;casSharpness=1.0;dlt=/tmp/fxs/Shaders/Daltonize.fx;reshadeTexturePath=/tmp/fxs/Textures;reshadeIncludePath=/tmp/fxs/Shaders' vkgears`
##### Expected output
```js
VulkanFX info:  no good config file
VulkanFX info:  config string: effects=fxaa:cas:dlt;casSharpness=1.0;dlt=/tmp/fxs/Shaders/Daltonize.fx;reshadeTexturePath=/tmp/fxs/Textures;reshadeIncludePath=/tmp/fxs/Shaders
VulkanFX info:  effects = fxaa:cas:dlt
VulkanFX info:  casSharpness = 1.0
VulkanFX info:  dlt = /tmp/fxs/Shaders/Daltonize.fx
VulkanFX info:  reshadeTexturePath = /tmp/fxs/Textures
VulkanFX info:  reshadeIncludePath = /tmp/fxs/Shaders
304 frames in 5.0 seconds = 60.779 FPS
301 frames in 5.0 seconds = 60.002 FPS
300 frames in 5.0 seconds = 59.999 FPS
...
```
#### Local
If built in e.g. gentoo environment and copied to different os / location
```sh
#!/bin/sh

export VK_IMPLICIT_LAYER_PATH="share/vulkan/implicit_layer.d"
export LD_LIBRARY_PATH="lib64:lib:${LD_LIBRARY_PATH}"
export ENABLE_VULKANFX=1

export VULKANFX_CONFIG="effects=fxaa:cas"

#export VULKANFX_LOG_LEVEL=debug
#export VK_LOADER_DEBUG=layer

vkgears
```
where curren directory contain `lib  lib64  run.sh  share`

## Configure

Settings like the CAS sharpening strength can be changed in the config file.
The config file will be searched for in the following locations:
* a file set with the environment variable`VULKANFX_CONFIG_FILE=/path/to/VulkanFX.conf`
* `VulkanFX.conf` in the working directory of the game
* `$XDG_CONFIG_HOME/VulkanFX/VulkanFX.conf` or `~/.config/VulkanFX/VulkanFX.conf` if `XDG_CONFIG_HOME` is not set
* `$XDG_DATA_HOME/VulkanFX/VulkanFX.conf` or `~/.local/share/VulkanFX/VulkanFX.conf` if `XDG_DATA_HOME` is not set
* `/etc/VulkanFX.conf`
* `/etc/VulkanFX/VulkanFX.conf`
* `/usr/share/VulkanFX/VulkanFX.conf`

If you want to make changes for one game only, you can create a file named `VulkanFX.conf` in the working directory of the game and change the values there.

#### Quick override
To override some of the default config options the `VULKANFX_CONFIG` env var can be used, e.g. `VULKANFX_CONFIG='effects=fxaa:cas;casSharpness=1.0'`.
The separator is `;`

#### ReShade FX shaders

To run reshade fx shaders e.g. shaders from the [reshade repo](https://github.com/crosire/reshade-shaders), you have to set `reshadeTexturePath` and `reshadeIncludePath` to the matching dirctories from the repo. To then use a specific shader you need to set a custom effect name to the shader path and then add that effect name to `effects` like every other effect.

```ini
effects = colorfulness:denoise

colorfulness = /home/user/reshade-shaders/Shaders/Colourfulness.fx
denoise = /home/user/reshade-shaders/Shaders/Denoise.fx
reshadeTexturePath = /home/user/reshade-shaders/Textures
reshadeIncludePath = /home/user/reshade-shaders/Shaders
```

#### Ingame Input

The [HOME key](https://en.wikipedia.org/wiki/Home_key) can be used to disable and re-enable the applied effects, the key can also be changed in the config file. This is based on X11 so it won't work on pure wayland. It **should** however at least not crash without X11.


#### Debug Output

The amount of debug output can be set with the `VULKANFX_LOG_LEVEL` env var, e.g. `VULKANFX_LOG_LEVEL=debug`. Possible values are: `trace, debug, info, warn, error, none`.

By default the logger outputs to stderr, a file as output location can be set with the `VULKANFX_LOG_FILE` env var, e.g. `VULKANFX_LOG_FILE="VulkanFX.log"`.


## FAQ

#### Why is it called vkBasalt?
It's a joke: vulkan post processing &#8594; after vulcan &#8594; basalt
#### Does vkBasalt work with dxvk and vkd3d?
Yes.
#### Will vkBasalt get me banned?
Maybe. To my knowledge this hasn't happened yet but don't blame me if your frog dies.
#### Will there be a openGl version?
No. I don't know anything about openGl and I don't want to either. Also openGl has no layer system like vulkan.
#### Will there be a GUI in the future?
Maybe, but not soon.
#### So is vkBasalt just a reshade port for linux?
Not really, most of the code was written from scratch. vkBasalt directly uses reshade source code for the shader compiler (thanks [@crosire](https://github.com/crosire)), but that's about it.
#### Does every reshade shader work?
No. Shaders that need multiple techniques do not work, there might still be problems with stencil and blending and depth buffer access isn't ready yet.
#### You said that "depth buffer access isn't ready yet", what does this mean?
There is a wip version that you can enable with `depthCapture = on`. It will lead to many problems especially on non nvidia hardware. Also the selected depth buffer isn't always the one you would want.
#### Is there a way to change settings for reshade shaders?
There is some support for it [#46](https://github.com/DadSchoorse/vkBasalt/pull/46). One easy way so to simply edit the shader file.
