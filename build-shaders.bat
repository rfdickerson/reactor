glslc -g --target-env=vulkan1.3 -o resources/shaders/triangle.vert.spv shaders/triangle.vert
glslc -g --target-env=vulkan1.3 -o resources/shaders/triangle.frag.spv shaders/triangle.frag

glslc -g --target-env=vulkan1.3 -o resources/shaders/composite.vert.spv shaders/composite.vert
glslc -g --target-env=vulkan1.3 -o resources/shaders/composite.frag.spv shaders/composite.frag