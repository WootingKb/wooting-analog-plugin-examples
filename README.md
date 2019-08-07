# Analog SDK Plugin Examples

This repo contains all plugin examples for the Analog SDK.

Plugins designed in Rust just need to target the analog-sdk-common library as a dependency, have a type that implements the `Plugin` trait and use the `declare_plugin!` macro.

For plugins in other languages, they must export the ABI defined in [`plugin.h`](https://github.com/simon-wh/Analog-SDK/blob/master/includes/plugin.h). So C plugins should define the functions in that header.

* `analog-plugin-c`: This is a example plugin written in C which builds and uses [`analog-sdk-common`](https://github.com/simon-wh/Analog-SDK/tree/master/analog-sdk-common) statically using headers generated during the build and defines and exports the functions defined in [`plugin.h`](https://github.com/simon-wh/Analog-SDK/blob/master/includes/plugin.h) which is what is required for the plugin