slangc -g -O0 shaders/triangle.slang -target spirv -profile vs_6_0 -entry vertexMain -o resources/shaders/triangle-slang.vert.spv
slangc -g -O0 shaders/triangle.slang -target spirv -profile ps_6_0 -entry fragmentMain -o resources/shaders/triangle-slang.frag.spv

slangc -g -O0 shaders/composite.slang -target spirv -profile vs_6_0 -entry vertexMain -o resources/shaders/composite-slang.vert.spv
slangc -g -O0 shaders/composite.slang -target spirv -profile ps_6_0 -entry fragmentMain -o resources/shaders/composite-slang.frag.spv