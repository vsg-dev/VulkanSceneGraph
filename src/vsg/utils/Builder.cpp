
#include <vsg/utils/Builder.h>

#include "shaders/assimp_vert.cpp"
#include "shaders/assimp_flat_shaded_frag.cpp"
#include "shaders/assimp_pbr_frag.cpp"
#include "shaders/assimp_phong_frag.cpp"

using namespace vsg;

#define FLOAT_COLORS 1

void Builder::setup(ref_ptr<Window> window, ViewportState* viewport, uint32_t maxNumTextures)
{
    auto device = window->getOrCreateDevice();

    _compile = CompileTraversal::create(window, viewport);

    // for now just allocated enough room for s
    uint32_t maxSets = maxNumTextures;
    DescriptorPoolSizes descriptorPoolSizes{
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxNumTextures}};

    _compile->context.descriptorPool = DescriptorPool::create(device, maxSets, descriptorPoolSizes);

    _allocatedTextureCount = 0;
    _maxNumTextures = maxNumTextures;
}

ref_ptr<BindDescriptorSets> Builder::_createDescriptorSet(const StateInfo& stateInfo)
{
    StateSettings& stateSettings = _getStateSettings(stateInfo);

    auto textureData = stateInfo.image;

    auto& bindDescriptorSets = stateSettings.textureDescriptorSets[textureData];
    if (bindDescriptorSets) return bindDescriptorSets;

    // create texture image and associated DescriptorSets and binding
    auto mat = vsg::PhongMaterialValue::create();
    auto material = vsg::DescriptorBuffer::create(mat, 10);

    if (textureData)
    {
        // create texture image and associated DescriptorSets and binding
        auto sampler = Sampler::create();
        auto texture = DescriptorImage::create(sampler, textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        auto descriptorSet = DescriptorSet::create(stateSettings.descriptorSetLayout, Descriptors{texture, material});
        bindDescriptorSets = BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, stateSettings.pipelineLayout, 0, DescriptorSets{descriptorSet});
    }
    else
    {
        auto descriptorSet = DescriptorSet::create(stateSettings.descriptorSetLayout, Descriptors{material});
        bindDescriptorSets = BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, stateSettings.pipelineLayout, 0, DescriptorSets{descriptorSet});
    }

    return bindDescriptorSets;
}

Builder::StateSettings& Builder::_getStateSettings(const StateInfo& stateInfo)
{
    auto& stateSettings = _stateMap[stateInfo];
    if (stateSettings.bindGraphicsPipeline) return stateSettings;

    // load shaders
    auto vertexShader = read_cast<ShaderStage>("shaders/assimp.vert", options);
    if (!vertexShader) vertexShader = assimp_vert(); // fallback to shaders/assimp_vert.cppp

    ref_ptr<ShaderStage> fragmentShader;
    if (stateInfo.lighting)
    {
        fragmentShader = read_cast<ShaderStage>("shaders/assimp_phong.frag", options);
        if (!fragmentShader) fragmentShader = assimp_phong_frag();
    }
    else
    {
        fragmentShader = read_cast<ShaderStage>("shaders/assimp_flat_shaded.frag", options);
        if (!fragmentShader) fragmentShader = assimp_flat_shaded_frag();
    }

    if (!vertexShader || !fragmentShader)
    {
        std::cout << "Could not create shaders." << std::endl;
        return stateSettings;
    }

    auto shaderHints = vsg::ShaderCompileSettings::create();
    std::vector<std::string>& defines = shaderHints->defines;

    vertexShader->module->hints = shaderHints;
    vertexShader->module->code = {};

    fragmentShader->module->hints = shaderHints;
    fragmentShader->module->code = {};

    if (stateInfo.image) defines.push_back("VSG_DIFFUSE_MAP");
    if (stateInfo.instancce_positions_vec3) defines.push_back("VSG_INSTANCE_POSITIONS");

    // set up graphics pipeline
    DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}, // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
        {10, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

    stateSettings.descriptorSetLayout = DescriptorSetLayout::create(descriptorBindings);

    DescriptorSetLayouts descriptorSetLayouts{stateSettings.descriptorSetLayout};

    PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls autoaatically provided by the VSG's DispatchTraversal
    };

    stateSettings.pipelineLayout = PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);

#if FLOAT_COLORS
    uint32_t colorSize = sizeof(vec4);
    VkFormat colorFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
#else
    uint32_t colorSize = sizeof(ubvec4);
    VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
#endif

    VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX},  // vertex data
        VkVertexInputBindingDescription{1, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX},  // normal data
        VkVertexInputBindingDescription{2, colorSize, VK_VERTEX_INPUT_RATE_INSTANCE},  // color
        VkVertexInputBindingDescription{3, sizeof(vec2), VK_VERTEX_INPUT_RATE_VERTEX},  // tex coord data
        VkVertexInputBindingDescription{4, sizeof(vec3), VK_VERTEX_INPUT_RATE_INSTANCE} // instance coord
    };

    VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // normal data
        VkVertexInputAttributeDescription{2, 2, colorFormat, 0},    // color data
        VkVertexInputAttributeDescription{3, 3, VK_FORMAT_R32G32_SFLOAT, 0},    // tex coord data
        VkVertexInputAttributeDescription{4, 4, VK_FORMAT_R32G32B32_SFLOAT, 0}  // instance coord
    };

    auto rasterState = vsg::RasterizationState::create();
    rasterState->cullMode = stateInfo.doubleSided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

    auto colorBlendState = vsg::ColorBlendState::create();
    colorBlendState->attachments = vsg::ColorBlendState::ColorBlendAttachments{
        {stateInfo.blending, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_SUBTRACT, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};

    auto inputAssemblyState = InputAssemblyState::create();

    if (stateInfo.wireframe)
    {
        inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    }

    std::cout<<"stateInfo.wireframe = "<<stateInfo.wireframe<<std::endl;

    GraphicsPipelineStates pipelineStates{
        VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        inputAssemblyState,
        rasterState,
        MultisampleState::create(),
        colorBlendState,
        DepthStencilState::create()};

    auto graphicsPipeline = GraphicsPipeline::create(stateSettings.pipelineLayout, ShaderStages{vertexShader, fragmentShader}, pipelineStates);

    stateSettings.bindGraphicsPipeline = BindGraphicsPipeline::create(graphicsPipeline);

    return stateSettings;
}

void Builder::_assign(StateGroup& stateGroup, const StateInfo& stateInfo)
{
    auto& stateSettings = _getStateSettings(stateInfo);
    stateGroup.add(stateSettings.bindGraphicsPipeline);
    stateGroup.add(_createDescriptorSet(stateInfo));
}


void Builder::compile(ref_ptr<Node> subgraph)
{
    if (verbose) std::cout << "Builder::compile(" << subgraph << ") _compile = " << _compile << std::endl;

    if (_compile)
    {
        subgraph->accept(*_compile);
        _compile->context.record();
        _compile->context.waitForCompletion();
    }
}

void Builder::transform(const mat4& matrix, ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals)
{
    for(auto& v : *vertices)
    {
        v = matrix * v;
    }

    if (normals)
    {
        mat4 normal_matrix = vsg::inverse(matrix);
        for(auto& n : *normals)
        {
            vsg::vec4 nv = vsg::vec4(n.x, n.y, n.z, 0.0) * normal_matrix;
            n = normalize(vsg::vec3(nv.x, nv.y, nv.z));
        }
    }
}

vec3 Builder::y_texcoord(const StateInfo& info) const
{
    if (info.image && info.image->getLayout().origin == Origin::TOP_LEFT)
    {
        return {1.0f, -1.0f, 0.0f};
    }
    else
    {
        return {0.0f, 1.0f, 1.0f};
    }
}

ref_ptr<Node> Builder::createBox(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _boxes[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx;
    auto dy = info.dy;
    auto dz = info.dz;
    auto origin = info.position - dx * 0.5f - dy * 0.5f - dz * 0.5f;
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    vec3 v000(origin);
    vec3 v100(origin + dx);
    vec3 v110(origin + dx + dy);
    vec3 v010(origin + dy);
    vec3 v001(origin + dz);
    vec3 v101(origin + dx + dz);
    vec3 v111(origin + dx + dy + dz);
    vec3 v011(origin + dy + dz);

    vec2 t00(0.0f, t_origin);
    vec2 t01(0.0f, t_top);
    vec2 t10(1.0f, t_origin);
    vec2 t11(1.0f, t_top);

    ref_ptr<vec3Array> vertices;
    ref_ptr<vec3Array> normals;
    ref_ptr<vec2Array> texcoords;
    ref_ptr<ushortArray> indices;

    if (stateInfo.wireframe)
    {
        vec3 n0 = normalize(v000-v111);
        vec3 n1 = normalize(v100-v011);
        vec3 n2 = normalize(v110-v001);
        vec3 n3 = normalize(v010-v101);
        vec3 n4 = -n2;
        vec3 n5 = -n3;
        vec3 n6 = -n0;
        vec3 n7 = -n1;

        // set up vertex and index arrays
        vertices = vec3Array::create(
            {v000, v100, v110, v010,
             v001, v101, v111, v011});

        normals = vec3Array::create(
            {n0, n1, n2, n3,
             n4, n5, n6, n7});

        texcoords = vec2Array::create(
            {t00, t10, t11, t01,
             t00, t10, t11, t01});

        indices = ushortArray::create(
            {0, 1, 1, 2, 2, 3, 3, 0,
             0, 4, 1, 5, 2, 6, 3, 7,
             4, 5, 5, 6, 6, 7, 7, 4
            });
    }
    else
    {
        vec3 n0 = normalize(cross(dx, dz));
        vec3 n1 = normalize(cross(dy, dz));
        vec3 n2 = -n0;
        vec3 n3 = -n1;
        vec3 n4 = normalize(cross(dy, dx));
        vec3 n5 = -n4;

        // set up vertex and index arrays
        vertices = vec3Array::create(
            {v000, v100, v101, v001,   // front
            v100, v110, v111, v101,   // right
            v110, v010, v011, v111,   // far
            v010, v000, v001, v011,   // left
            v010, v110, v100, v000,   // bottom
            v001, v101, v111, v011}); // top

        normals = vec3Array::create(
            {n0, n0, n0, n0,
            n1, n1, n1, n1,
            n2, n2, n2, n2,
            n3, n3, n3, n3,
            n4, n4, n4, n4,
            n5, n5, n5, n5});

        texcoords = vec2Array::create(
            {t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01,
            t00, t10, t11, t01});

        indices = ushortArray::create(
            {0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23});
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;

    std::cout<<"vid->instanceCount = "<<vid->instanceCount<<std::endl;

    scenegraph->addChild(vid);

    compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createCapsule(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _capsules[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto bottom = info.position - dz;
    auto top = info.position + dz;

    bool withEnds = true;

    unsigned int num_columns = 20;
    unsigned int num_rows = 6;

    unsigned int num_vertices = num_columns * 2;
    unsigned int num_indices = (num_columns - 1) * 6;

    if (withEnds)
    {
        num_vertices += num_columns * num_rows * 2;
        num_indices += (num_columns - 1) * (num_rows - 1) * 6 * 2;
    }

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    auto indices = ushortArray::create(num_indices);

    vec3 v = dy;
    vec3 n = normalize(dy);
    vertices->set(0, bottom + v);
    normals->set(0, n);
    texcoords->set(0, vec2(0.0, t_origin));
    vertices->set(num_columns * 2 - 2, bottom + v);
    normals->set(num_columns * 2 - 2, n);
    texcoords->set(num_columns * 2 - 2, vec2(1.0, t_origin));

    vertices->set(1, top + v);
    normals->set(1, n);
    texcoords->set(1, vec2(0.0, t_top));
    vertices->set(num_columns * 2 - 1, top + v);
    normals->set(num_columns * 2 - 1, n);
    texcoords->set(num_columns * 2 - 1, vec2(1.0, t_top));

    for (unsigned int c = 1; c < num_columns - 1; ++c)
    {
        unsigned int vi = c * 2;
        float r = float(c) / float(num_columns - 1);
        float alpha = (r)*2.0 * PI;
        v = dx * (-sinf(alpha)) + dy * (cosf(alpha));
        n = normalize(v);

        vertices->set(vi, bottom + v);
        normals->set(vi, n);
        texcoords->set(vi, vec2(r, t_origin));

        vertices->set(vi + 1, top + v);
        normals->set(vi + 1, n);
        texcoords->set(vi + 1, vec2(r, t_top));
    }

    unsigned int i = 0;
    for (unsigned int c = 0; c < num_columns - 1; ++c)
    {
        unsigned lower = c * 2;
        unsigned upper = lower + 1;

        indices->set(i++, lower);
        indices->set(i++, lower + 2);
        indices->set(i++, upper);

        indices->set(i++, upper);
        indices->set(i++, lower + 2);
        indices->set(i++, upper + 2);
    }

    if (withEnds)
    {
        unsigned int base_vi = num_columns * 2;

        // bottom
        {
            for (unsigned int r = 0; r < num_rows; ++r)
            {
                float beta = ((float(r) / float(num_rows - 1)) - 1.0f) * PI * 0.5f;
                float ty = t_origin + t_scale * float(r) / float(num_rows - 1);
                float cos_beta = cosf(beta);
                vec3 dz_sin_beta = dz * sinf(beta);

                v = dy * cos_beta + dz_sin_beta;
                n = normalize(v);

                unsigned int left_i = base_vi + r * num_columns;
                vertices->set(left_i, bottom + v);
                normals->set(left_i, n);
                texcoords->set(left_i, vec2(0.0f, ty));

                unsigned int right_i = left_i + num_columns - 1;
                vertices->set(right_i, bottom + v);
                normals->set(right_i, n);
                texcoords->set(right_i, vec2(1.0f, ty));

                for (unsigned int c = 1; c < num_columns - 1; ++c)
                {
                    unsigned int vi = left_i + c;
                    float alpha = (float(c) / float(num_columns - 1)) * 2.0 * PI;
                    v = dx * (-sinf(alpha) * cos_beta) + dy * (cosf(alpha) * cos_beta) + dz_sin_beta;
                    n = normalize(v);
                    vertices->set(vi, bottom + v);
                    normals->set(vi, n);
                    texcoords->set(vi, vec2(float(c) / float(num_columns - 1), ty));
                }
            }

            for (unsigned int r = 0; r < num_rows - 1; ++r)
            {
                for (unsigned int c = 0; c < num_columns - 1; ++c)
                {
                    unsigned lower = base_vi + num_columns * r + c;
                    unsigned upper = lower + num_columns;

                    indices->set(i++, lower);
                    indices->set(i++, lower + 1);
                    indices->set(i++, upper);

                    indices->set(i++, upper);
                    indices->set(i++, lower + 1);
                    indices->set(i++, upper + 1);
                }
            }

            base_vi += num_columns * num_rows;
        }

        // top
        {
            for (unsigned int r = 0; r < num_rows; ++r)
            {
                float beta = ((float(r) / float(num_rows - 1))) * PI * 0.5f;
                float ty = t_origin + t_scale * float(r) / float(num_rows - 1);
                float cos_beta = cosf(beta);
                vec3 dz_sin_beta = dz * sinf(beta);

                v = dy * cos_beta + dz_sin_beta;
                n = normalize(v);

                unsigned int left_i = base_vi + r * num_columns;
                vertices->set(left_i, top + v);
                normals->set(left_i, n);
                texcoords->set(left_i, vec2(0.0f, ty));

                unsigned int right_i = left_i + num_columns - 1;
                vertices->set(right_i, top + v);
                normals->set(right_i, n);
                texcoords->set(right_i, vec2(1.0f, ty));

                for (unsigned int c = 1; c < num_columns - 1; ++c)
                {
                    unsigned int vi = left_i + c;
                    float alpha = (float(c) / float(num_columns - 1)) * 2.0 * PI;
                    v = dx * (-sinf(alpha) * cos_beta) + dy * (cosf(alpha) * cos_beta) + dz_sin_beta;
                    n = normalize(v);
                    vertices->set(vi, top + v);
                    normals->set(vi, n);
                    texcoords->set(vi, vec2(float(c) / float(num_columns - 1), ty));
                }
            }

            for (unsigned int r = 0; r < num_rows - 1; ++r)
            {
                for (unsigned int c = 0; c < num_columns - 1; ++c)
                {
                    unsigned lower = base_vi + num_columns * r + c;
                    unsigned upper = lower + num_columns;

                    indices->set(i++, lower);
                    indices->set(i++, lower + 1);
                    indices->set(i++, upper);

                    indices->set(i++, upper);
                    indices->set(i++, lower + 1);
                    indices->set(i++, upper + 1);
                }
            }

            base_vi += num_columns * num_rows;
        }
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createCone(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _cones[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto bottom = info.position - dz;
    auto top = info.position + dz;

    bool withEnds = true;

    unsigned int num_columns = 20;
    unsigned int num_vertices = num_columns * 2;
    unsigned int num_indices = (num_columns - 1) * 3;

    if (withEnds)
    {
        num_vertices += num_columns;
        num_indices += (num_columns - 2) * 3;
    }

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    auto indices = ushortArray::create(num_indices);

    auto edge = [&](float alpha) -> vec3 {
        return dy * (cosf(alpha)) - dx * (sinf(alpha));
    };

    auto normal = [&](float alpha) -> vec3 {
        float delta = 0.001f;
        vec3 before = edge(alpha - delta);
        vec3 mid = edge(alpha);
        vec3 after = edge(alpha + delta);
        return normalize(cross(after - before, dz - mid));
    };

    float alpha = 0.0f;
    vec3 v = edge(alpha);
    vec3 n = normal(alpha);

    vertices->set(0, bottom + v);
    normals->set(0, n);
    texcoords->set(0, vec2(0.0, t_origin));
    vertices->set(num_columns * 2 - 2, bottom + v);
    normals->set(num_columns * 2 - 2, n);
    texcoords->set(num_columns * 2 - 2, vec2(1.0, t_origin));

    vertices->set(1, top);
    normals->set(1, n);
    texcoords->set(1, vec2(0.0, t_top));
    vertices->set(num_columns * 2 - 1, top);
    normals->set(num_columns * 2 - 1, n);
    texcoords->set(num_columns * 2 - 1, vec2(1.0, t_top));

    for (unsigned int c = 1; c < num_columns - 1; ++c)
    {
        unsigned int vi = c * 2;
        float r = float(c) / float(num_columns - 1);
        alpha = (r)*2.0 * PI;
        v = edge(alpha);
        n = normal(alpha);

        vertices->set(vi, bottom + v);
        normals->set(vi, n);
        texcoords->set(vi, vec2(r, t_origin));

        vertices->set(vi + 1, top);
        normals->set(vi + 1, n);
        texcoords->set(vi + 1, vec2(r, t_top));
    }

    unsigned int i = 0;
    for (unsigned int c = 0; c < num_columns - 1; ++c)
    {
        unsigned lower = c * 2;
        unsigned upper = lower + 1;

        indices->set(i++, lower);
        indices->set(i++, lower + 2);
        indices->set(i++, upper);
    }

    if (withEnds)
    {
        unsigned int bottom_i = num_columns * 2;
        v = edge(0.0f);
        vec3 bottom_n = normalize(-dz);

        vertices->set(bottom_i, bottom + v);
        normals->set(bottom_i, bottom_n);
        texcoords->set(bottom_i, vec2(0.0, t_origin));
        vertices->set(bottom_i + num_columns - 1, bottom + v);
        normals->set(bottom_i + num_columns - 1, bottom_n);
        texcoords->set(bottom_i + num_columns - 1, vec2(1.0, t_origin));

        for (unsigned int c = 1; c < num_columns - 1; ++c)
        {
            float r = float(c) / float(num_columns - 1);
            alpha = (r)*2.0 * PI;
            v = edge(alpha);

            unsigned int vi = bottom_i + c;
            vertices->set(vi, bottom + v);
            normals->set(vi, bottom_n);
            texcoords->set(vi, vec2(r, t_origin));
        }

        for (unsigned int c = 0; c < num_columns - 2; ++c)
        {
            indices->set(i++, bottom_i + c);
            indices->set(i++, bottom_i + c + 1);
            indices->set(i++, bottom_i + num_columns - 1);
        }
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;


    scenegraph->addChild(vid);

    compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createCylinder(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _cylinders[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto bottom = info.position - dz;
    auto top = info.position + dz;

    bool withEnds = true;

    unsigned int num_columns = 20;
    unsigned int num_vertices = num_columns * 2;
    unsigned int num_indices = (num_columns - 1) * 6;

    if (withEnds)
    {
        num_vertices += num_columns * 2;
        num_indices += (num_columns - 2) * 6;
    }

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    auto indices = ushortArray::create(num_indices);

    vec3 v = dy;
    vec3 n = normalize(dy);
    vertices->set(0, bottom + v);
    normals->set(0, n);
    texcoords->set(0, vec2(0.0, t_origin));
    vertices->set(num_columns * 2 - 2, bottom + v);
    normals->set(num_columns * 2 - 2, n);
    texcoords->set(num_columns * 2 - 2, vec2(1.0, t_origin));

    vertices->set(1, top + v);
    normals->set(1, n);
    texcoords->set(1, vec2(0.0, t_top));
    vertices->set(num_columns * 2 - 1, top + v);
    normals->set(num_columns * 2 - 1, n);
    texcoords->set(num_columns * 2 - 1, vec2(1.0, t_top));

    for (unsigned int c = 1; c < num_columns - 1; ++c)
    {
        unsigned int vi = c * 2;
        float r = float(c) / float(num_columns - 1);
        float alpha = (r)*2.0 * PI;
        v = dx * (-sinf(alpha)) + dy * (cosf(alpha));
        n = normalize(v);

        vertices->set(vi, bottom + v);
        normals->set(vi, n);
        texcoords->set(vi, vec2(r, t_origin));

        vertices->set(vi + 1, top + v);
        normals->set(vi + 1, n);
        texcoords->set(vi + 1, vec2(r, t_top));
    }

    unsigned int i = 0;
    for (unsigned int c = 0; c < num_columns - 1; ++c)
    {
        unsigned lower = c * 2;
        unsigned upper = lower + 1;

        indices->set(i++, lower);
        indices->set(i++, lower + 2);
        indices->set(i++, upper);

        indices->set(i++, upper);
        indices->set(i++, lower + 2);
        indices->set(i++, upper + 2);
    }

    if (withEnds)
    {
        v = dy;

        unsigned int bottom_i = num_columns * 2;
        unsigned int top_i = bottom_i + num_columns;
        vec3 top_n = normalize(dz);
        vec3 bottom_n = -top_n;

        vertices->set(bottom_i, bottom + v);
        normals->set(bottom_i, bottom_n);
        texcoords->set(bottom_i, vec2(0.0, t_origin));
        vertices->set(bottom_i + num_columns - 1, bottom + v);
        normals->set(bottom_i + num_columns - 1, bottom_n);
        texcoords->set(bottom_i + num_columns - 1, vec2(1.0, t_origin));

        vertices->set(top_i, top + v);
        normals->set(top_i, top_n);
        texcoords->set(top_i, vec2(0.0, t_top));
        vertices->set(top_i + num_columns - 1, top + v);
        normals->set(top_i + num_columns - 1, top_n);
        texcoords->set(top_i + num_columns - 1, vec2(0.0, t_top));

        for (unsigned int c = 1; c < num_columns - 1; ++c)
        {
            float r = float(c) / float(num_columns - 1);
            float alpha = (r)*2.0 * PI;
            v = dx * (-sinf(alpha)) + dy * (cosf(alpha));
            n = normalize(v);

            unsigned int vi = bottom_i + c;
            vertices->set(vi, bottom + v);
            normals->set(vi, bottom_n);
            texcoords->set(vi, vec2(r, t_origin));

            vi = top_i + c;
            vertices->set(vi, top + v);
            normals->set(vi, top_n);
            texcoords->set(vi, vec2(r, t_top));
        }

        for (unsigned int c = 0; c < num_columns - 2; ++c)
        {
            indices->set(i++, bottom_i + c);
            indices->set(i++, bottom_i + num_columns - 1);
            indices->set(i++, bottom_i + c + 1);
        }

        for (unsigned int c = 0; c < num_columns - 2; ++c)
        {
            indices->set(i++, top_i + c);
            indices->set(i++, top_i + c + 1);
            indices->set(i++, top_i + num_columns - 1);
        }
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createQuad(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _boxes[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx;
    auto dy = info.dy;
    auto origin = info.position - dx * 0.5f - dy * 0.5f;
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;
    auto normal = normalize(cross(dx, dy));

    // set up vertex and index arrays
    auto vertices = vec3Array::create(
        {origin,
         origin + dx,
         origin + dx + dy,
         origin + dy}); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_INSTANCE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    auto normals = vec3Array::create(
        {normal,
         normal,
         normal,
         normal}); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    auto texcoords = vec2Array::create(
        {{0.0f, t_origin},
         {1.0f, t_origin},
         {1.0f, t_top},
         {0.0f, t_top}}); // VK_FORMAT_R32G32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE


    ref_ptr<ushortArray> indices;

    if (stateInfo.wireframe)
    {
        indices = ushortArray::create(
            {0, 1, 1, 2, 2, 3, 3, 0});
    }
    else
    {
        indices = ushortArray::create(
            {0, 1, 2,
             2, 3, 0});
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    compile(scenegraph);

    return scenegraph;
}

ref_ptr<Node> Builder::createSphere(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _spheres[info];
    if (subgraph)
    {
        return subgraph;
    }

    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    uint32_t instanceCount = 1;
    auto positions = info.positions;
    if (positions)
    {
        if (positions->size()>=1) instanceCount = positions->size();
        else positions = {};
    }

    auto colors = info.colors;
    if (colors && colors->valueCount() != instanceCount) colors = {};
    if (!colors) colors = vec4Array::create(instanceCount, info.color);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = StateGroup::create();
    _assign(*scenegraph, stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;
    auto origin = info.position;

    unsigned int num_columns = 20;
    unsigned int num_rows = 10;
    unsigned int num_vertices = num_columns * num_rows;
    unsigned int num_indices = (num_columns - 1) * (num_rows - 1) * 6;

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    auto indices = ushortArray::create(num_indices);

    for (unsigned int r = 0; r < num_rows; ++r)
    {
        float beta = ((float(r) / float(num_rows - 1)) - 0.5) * PI;
        float ty = t_origin + t_scale * float(r) / float(num_rows - 1);
        float cos_beta = cosf(beta);
        vec3 dz_sin_beta = dz * sinf(beta);

        vec3 v = dy * cos_beta + dz_sin_beta;
        vec3 n = normalize(v);

        unsigned int left_i = r * num_columns;
        vertices->set(left_i, v + origin);
        normals->set(left_i, n);
        texcoords->set(left_i, vec2(0.0f, ty));

        unsigned int right_i = left_i + num_columns - 1;
        vertices->set(right_i, v + origin);
        normals->set(right_i, n);
        texcoords->set(right_i, vec2(1.0f, ty));

        for (unsigned int c = 1; c < num_columns - 1; ++c)
        {
            unsigned int vi = left_i + c;
            float alpha = (float(c) / float(num_columns - 1)) * 2.0 * PI;
            v = dx * (-sinf(alpha) * cos_beta) + dy * (cosf(alpha) * cos_beta) + dz_sin_beta;
            n = normalize(v);
            vertices->set(vi, origin + v);
            normals->set(vi, n);
            texcoords->set(vi, vec2(float(c) / float(num_columns - 1), ty));
        }
    }

    unsigned int i = 0;
    for (unsigned int r = 0; r < num_rows - 1; ++r)
    {
        for (unsigned int c = 0; c < num_columns - 1; ++c)
        {
            unsigned lower = num_columns * r + c;
            unsigned upper = lower + num_columns;

            indices->set(i++, lower);
            indices->set(i++, lower + 1);
            indices->set(i++, upper);

            indices->set(i++, upper);
            indices->set(i++, lower + 1);
            indices->set(i++, upper + 1);
        }
    }

    if (info.transform != identity)
    {
        transform(info.transform, vertices, normals);
    }

    // setup geometry
    auto vid = VertexIndexDraw::create();

    DataList arrays;
    arrays.push_back(vertices);
    if (normals) arrays.push_back(normals);
    if (colors) arrays.push_back(colors);
    if (texcoords) arrays.push_back(texcoords);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = indices->size();
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}
