glslc --target-env=vulkan1.3 -o shaders/triangle.vert.spv shaders/triangle.vert
glslc --target-env=vulkan1.3 -o shaders/triangle.frag.spv shaders/triangle.frag

glslc --target-env=vulkan1.3 -o shaders/composite.vert.spv shaders/composite.vert
glslc --target-env=vulkan1.3 -o shaders/composite.frag.spv shaders/composite.frag