glslc shaders/shader.vert -o cmake-build-debug/shaders/shader.vert.spv
glslc shaders/shader.frag -o cmake-build-debug/shaders/shader.frag.spv

glslc shaders/shader.vert -o build/shaders/shader.vert.spv
glslc shaders/shader.frag -o build/shaders/shader.frag.spv

glslc shaders/shader.vert -o shaders/shader.vert.spv
glslc shaders/shader.frag -o shaders/shader.frag.spv