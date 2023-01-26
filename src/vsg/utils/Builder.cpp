
#include <vsg/io/Logger.h>
#include <vsg/io/read.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/DescriptorBuffer.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/state/ViewDependentState.h>
#include <vsg/state/ViewportState.h>
#include <vsg/state/material.h>
#include <vsg/utils/Builder.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>

using namespace vsg;

void Builder::assignCompileTraversal(ref_ptr<CompileTraversal> ct)
{
    compileTraversal = ct;
}

ref_ptr<StateGroup> Builder::createStateGroup(const StateInfo& stateInfo)
{
    if (!sharedObjects)
    {
        if (options)
            sharedObjects = options->sharedObjects;
        else
            sharedObjects = vsg::SharedObjects::create();
    }
    if (!shaderSet)
    {
        if (stateInfo.lighting)
            shaderSet = createPhongShaderSet(options);
        else
            shaderSet = createFlatShadedShaderSet(options);
        //shaderSet = createPhysicsBasedRenderingShaderSet(options);
    }

    auto graphicsPipelineConfig = vsg::GraphicsPipelineConfigurator::create(shaderSet);

    auto& defines = graphicsPipelineConfig->shaderHints->defines;

    // set up graphics pipeline
    vsg::Descriptors descriptors;

    // set up graphics pipeline
    DescriptorSetLayoutBindings descriptorBindings;
    if (stateInfo.image)
    {
        auto sampler = Sampler::create();
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        if (sharedObjects) sharedObjects->share(sampler);

        graphicsPipelineConfig->assignTexture(descriptors, "diffuseMap", stateInfo.image, sampler);

        if (stateInfo.greyscale) defines.insert("VSG_GREYSACLE_DIFFUSE_MAP");
    }

    if (stateInfo.displacementMap)
    {
        auto sampler = Sampler::create();
        sampler->addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler->addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        if (sharedObjects) sharedObjects->share(sampler);

        graphicsPipelineConfig->assignTexture(descriptors, "displacementMap", stateInfo.displacementMap, sampler);
    }

    if (auto& materialBinding = shaderSet->getUniformBinding("material"))
    {
        ref_ptr<Data> mat = materialBinding.data;
        if (!mat) mat = vsg::PhongMaterialValue::create();
        graphicsPipelineConfig->assignUniform(descriptors, "material", mat);
    }

    if (sharedObjects) sharedObjects->share(descriptors);

    // set up ViewDependentState
    ref_ptr<ViewDescriptorSetLayout> vdsl;
    if (sharedObjects)
        vdsl = sharedObjects->shared_default<ViewDescriptorSetLayout>();
    else
        vdsl = ViewDescriptorSetLayout::create();
    graphicsPipelineConfig->additionalDescriptorSetLayout = vdsl;

    graphicsPipelineConfig->enableArray("vsg_Vertex", VK_VERTEX_INPUT_RATE_VERTEX, 12);
    graphicsPipelineConfig->enableArray("vsg_Normal", VK_VERTEX_INPUT_RATE_VERTEX, 12);
    graphicsPipelineConfig->enableArray("vsg_TexCoord0", VK_VERTEX_INPUT_RATE_VERTEX, 8);

    if (stateInfo.instance_colors_vec4)
    {
        graphicsPipelineConfig->enableArray("vsg_Color", VK_VERTEX_INPUT_RATE_INSTANCE, 16);
    }

    if (stateInfo.billboard)
    {
        graphicsPipelineConfig->enableArray("vsg_position_scaleDistance", VK_VERTEX_INPUT_RATE_INSTANCE, 16);
    }
    else if (stateInfo.instance_positions_vec3)
    {
        graphicsPipelineConfig->enableArray("vsg_position", VK_VERTEX_INPUT_RATE_INSTANCE, 12);
    }

    if (stateInfo.two_sided)
    {
        graphicsPipelineConfig->rasterizationState->cullMode = VK_CULL_MODE_NONE;
        defines.insert("VSG_TWO_SIDED_LIGHTING");
    }

    graphicsPipelineConfig->colorBlendState->attachments = ColorBlendState::ColorBlendAttachments{
        {stateInfo.blending, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_SUBTRACT, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};

    if (stateInfo.wireframe)
    {
        graphicsPipelineConfig->inputAssemblyState->topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    }

    // if required initialize GraphicsPipeline/Layout etc.
    if (sharedObjects)
        sharedObjects->share(graphicsPipelineConfig, [](auto gpc) { gpc->init(); });
    else
        graphicsPipelineConfig->init();

    auto descriptorSet = vsg::DescriptorSet::create(graphicsPipelineConfig->descriptorSetLayout, descriptors);
    if (sharedObjects) sharedObjects->share(descriptorSet);

    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineConfig->layout, 0, descriptorSet);
    if (sharedObjects) sharedObjects->share(bindDescriptorSet);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto stateGroup = vsg::StateGroup::create();
    stateGroup->add(graphicsPipelineConfig->bindGraphicsPipeline);
    stateGroup->add(bindDescriptorSet);

    // assign any custom ArrayState that may be required.
    stateGroup->prototypeArrayState = shaderSet->getSuitableArrayState(graphicsPipelineConfig->shaderHints->defines);

    auto bindViewDescriptorSets = BindViewDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineConfig->layout, 1);
    if (sharedObjects) sharedObjects->share(bindViewDescriptorSets);
    stateGroup->add(bindViewDescriptorSets);

    //if (sharedObjects) vsg::debug_stream([&](auto& fout) { sharedObjects->report(fout); });

    return stateGroup;
}

void Builder::transform(const mat4& matrix, ref_ptr<vec3Array> vertices, ref_ptr<vec3Array> normals)
{
    for (auto& v : *vertices)
    {
        v = matrix * v;
    }

    if (normals)
    {
        mat4 normal_matrix = inverse(matrix);
        for (auto& n : *normals)
        {
            vec4 nv = vec4(n.x, n.y, n.z, 0.0) * normal_matrix;
            n = normalize(vec3(nv.x, nv.y, nv.z));
        }
    }
}

vsg::ref_ptr<vsg::Data> Builder::instancePositions(const GeometryInfo& info, uint32_t& instanceCount)
{
    instanceCount = 1;

    if (info.positions && info.positions->dataAvailable())
    {
        instanceCount = static_cast<uint32_t>(info.positions->valueCount());
        return info.positions;
    }
    return {};
}

vsg::ref_ptr<vsg::Data> Builder::instanceColors(const GeometryInfo& info, uint32_t instanceCount)
{
    if (info.colors && (info.colors->valueCount() == instanceCount))
        return info.colors;
    else
        return vec4Array::create(instanceCount, info.color);
}

vec3 Builder::y_texcoord(const StateInfo& info) const
{
    if ((info.image && info.image->properties.origin == Origin::TOP_LEFT) ||
        (info.displacementMap && info.displacementMap->properties.origin == Origin::TOP_LEFT))
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
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

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
        vec3 n0 = normalize(v000 - v111);
        vec3 n1 = normalize(v100 - v011);
        vec3 n2 = normalize(v110 - v001);
        vec3 n3 = normalize(v010 - v101);
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
             4, 5, 5, 6, 6, 7, 7, 4});
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

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
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

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
        float alpha = (r)*2.0f * PIf;
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
                float beta = ((float(r) / float(num_rows - 1)) - 1.0f) * PIf * 0.5f;
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
                    float alpha = (float(c) / float(num_columns - 1)) * 2.0f * PIf;
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
                float beta = ((float(r) / float(num_rows - 1))) * PIf * 0.5f;
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
                    float alpha = (float(c) / float(num_columns - 1)) * 2.0f * PIf;
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

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
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto bottom = info.position - dz;
    auto top = info.position + dz;

    ref_ptr<vec3Array> vertices;
    ref_ptr<vec3Array> normals;
    ref_ptr<vec2Array> texcoords;
    ref_ptr<ushortArray> indices;

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

    if (stateInfo.wireframe)
    {
        unsigned int num_columns = 20;

        unsigned int num_vertices = 1 + num_columns;
        unsigned int num_indices = num_columns * 4;

        vertices = vec3Array::create(num_vertices);
        normals = vec3Array::create(num_vertices);
        texcoords = vec2Array::create(num_vertices);
        indices = ushortArray::create(num_indices);

        vertices->set(0, top);
        normals->set(0, normalize(info.dz));
        texcoords->set(0, vec2(0.0, 0.0));

        for (unsigned int c = 0; c < num_columns; ++c)
        {
            unsigned int vi = 1 + c;
            float r = float(c) / float(num_columns);
            alpha = (r)*2.0f * PIf;
            v = edge(alpha);
            n = normal(alpha);

            vertices->set(vi, bottom + v);
            normals->set(vi, n);
            texcoords->set(vi, vec2(r, t_origin));
        }

        unsigned int i = 0;
        indices->set(i++, 0);
        indices->set(i++, num_columns);
        indices->set(i++, num_columns);
        indices->set(i++, 1);
        for (unsigned int c = 1; c < num_columns; ++c)
        {
            unsigned lower = 1 + c;
            indices->set(i++, 0);
            indices->set(i++, lower - 1);
            indices->set(i++, lower - 1);
            indices->set(i++, lower);
        }
    }
    else
    {
        bool withEnds = true;

        unsigned int num_columns = 20;
        unsigned int num_vertices = num_columns * 2;
        unsigned int num_indices = (num_columns - 1) * 3;

        if (withEnds)
        {
            num_vertices += num_columns;
            num_indices += (num_columns - 2) * 3;
        }

        vertices = vec3Array::create(num_vertices);
        normals = vec3Array::create(num_vertices);
        texcoords = vec2Array::create(num_vertices);
        indices = ushortArray::create(num_indices);

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
            alpha = (r)*2.0f * PIf;
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
                alpha = (r)*2.0f * PIf;
                v = edge(alpha);

                unsigned int vi = bottom_i + c;
                vertices->set(vi, bottom + v);
                normals->set(vi, bottom_n);
                texcoords->set(vi, vec2(r, t_origin));
            }

            for (unsigned int c = 0; c < num_columns - 2; ++c)
            {
                indices->set(i++, bottom_i + c);
                indices->set(i++, bottom_i + num_columns - 1);
                indices->set(i++, bottom_i + c + 1);
            }
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

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
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto bottom = info.position - dz;
    auto top = info.position + dz;

    ref_ptr<vec3Array> vertices;
    ref_ptr<vec3Array> normals;
    ref_ptr<vec2Array> texcoords;
    ref_ptr<ushortArray> indices;

    if (stateInfo.wireframe)
    {
        unsigned int num_columns = 20;

        unsigned int num_vertices = num_columns * 2;
        unsigned int num_indices = num_columns * 6;

        vertices = vec3Array::create(num_vertices);
        normals = vec3Array::create(num_vertices);
        texcoords = vec2Array::create(num_vertices);
        indices = ushortArray::create(num_indices);

        vec3 v = dy;
        vec3 n = normalize(dy);
        vertices->set(0, bottom + v);
        normals->set(0, n);
        texcoords->set(0, vec2(0.0, t_origin));

        vertices->set(1, top + v);
        normals->set(1, n);
        texcoords->set(1, vec2(0.0, t_top));

        for (unsigned int c = 1; c < num_columns; ++c)
        {
            unsigned int vi = c * 2;
            float r = float(c) / float(num_columns - 1);
            float alpha = (r)*2.0f * PIf;
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
        unsigned int lower = (num_columns - 1) * 2;
        unsigned int upper = lower + 1;

        indices->set(i++, 0);
        indices->set(i++, lower);

        indices->set(i++, lower);
        indices->set(i++, upper);

        indices->set(i++, upper);
        indices->set(i++, 1);

        for (unsigned int c = 0; c < num_columns - 1; ++c)
        {
            lower = c * 2;
            upper = lower + 1;

            indices->set(i++, lower + 2);
            indices->set(i++, lower);

            indices->set(i++, lower);
            indices->set(i++, upper);

            indices->set(i++, upper);
            indices->set(i++, upper + 2);
        }
    }
    else
    {
        bool withEnds = true;

        unsigned int num_columns = 20;
        unsigned int num_vertices = num_columns * 2;
        unsigned int num_indices = (num_columns - 1) * 6;

        if (withEnds)
        {
            num_vertices += num_columns * 2;
            num_indices += (num_columns - 2) * 6;
        }

        vertices = vec3Array::create(num_vertices);
        normals = vec3Array::create(num_vertices);
        texcoords = vec2Array::create(num_vertices);
        indices = ushortArray::create(num_indices);

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
            float alpha = (r)*2.0f * PIf;
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
                float alpha = (r)*2.0f * PIf;
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createDisk(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _cylinders[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

    auto dx = info.dx * 0.5f;
    auto dy = info.dy * 0.5f;
    auto dz = info.dz * 0.5f;

    auto center = info.position;
    auto n = normalize(dz);

    unsigned int num_vertices = 20;

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    ref_ptr<ushortArray> indices;

    vertices->set(0, center + dy);
    normals->set(0, n);
    texcoords->set(0, vec2(0.5f, t_top));

    for (unsigned int c = 1; c < num_vertices; ++c)
    {
        float r = float(c) / float(num_vertices - 1);
        float alpha = (r)*2.0f * PIf;
        float sn = sinf(alpha);
        float cs = cosf(alpha);
        vec3 v = dy * cs - dx * sn;
        vertices->set(c, center + v);
        normals->set(c, n);
        texcoords->set(c, vec2((1.0f - sn) * 0.5f, t_origin + t_scale * (cs + 1.0f) * 0.5f));
    }

    if (stateInfo.wireframe)
    {
        unsigned int num_indices = (num_vertices)*2;
        indices = ushortArray::create(num_indices);

        unsigned int i = 0;

        indices->set(i++, num_vertices - 1);
        indices->set(i++, 0);
        for (unsigned vi = 0; vi < num_vertices - 1; ++vi)
        {
            indices->set(i++, vi);
            indices->set(i++, vi + 1);
        }
    }
    else
    {
        unsigned int num_indices = (num_vertices - 2) * 3;
        indices = ushortArray::create(num_indices);

        unsigned int i = 0;
        for (unsigned vi = 1; vi < num_vertices - 2; ++vi)
        {
            indices->set(i++, 0);
            indices->set(i++, vi);
            indices->set(i++, vi + 1);
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

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
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);

    auto scenegraph = createStateGroup(stateInfo);

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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

    return scenegraph;
}

ref_ptr<Node> Builder::createSphere(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _spheres[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

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
        float beta = ((float(r) / float(num_rows - 1)) - 0.5f) * PIf;
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
            float alpha = (float(c) / float(num_columns - 1)) * 2.0f * PIf;
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}

ref_ptr<Node> Builder::createHeightField(const GeometryInfo& info, const StateInfo& stateInfo)
{
    auto& subgraph = _heightfields[info];
    if (subgraph)
    {
        return subgraph;
    }

    uint32_t instanceCount = 1;
    auto positions = instancePositions(info, instanceCount);
    auto colors = instanceColors(info, instanceCount);
    auto [t_origin, t_scale, t_top] = y_texcoord(stateInfo).value;

    // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
    auto scenegraph = createStateGroup(stateInfo);

    auto dx = info.dx;
    auto dy = info.dy;
    auto dz = info.dz;
    auto origin = info.position - (dx + dy) * 0.5f;

    unsigned int num_columns = 2;
    unsigned int num_rows = 2;

    if (stateInfo.displacementMap)
    {
        num_columns = stateInfo.displacementMap->width();
        num_rows = stateInfo.displacementMap->height();

        if (num_columns < 2) num_columns = 2;
        if (num_rows < 2) num_rows = 2;
    }

    unsigned int num_vertices = num_columns * num_rows;
    unsigned int num_indices = (stateInfo.wireframe) ? (4 * num_columns * num_rows - 2 * (num_columns + num_rows)) : (num_rows - 1) * (num_rows - 1) * 6;

    auto normal = normalize(dz);

    auto vertices = vec3Array::create(num_vertices);
    auto normals = vec3Array::create(num_vertices);
    auto texcoords = vec2Array::create(num_vertices);
    auto indices = ushortArray::create(num_indices);

    for (unsigned int r = 0; r < num_rows; ++r)
    {
        //float ty = t_origin + t_scale * float(r) / float(num_rows - 1);
        float ty = float(r) / float(num_rows - 1);
        unsigned int left_i = r * num_columns;

        for (unsigned int c = 0; c < num_columns; ++c)
        {
            unsigned int vi = left_i + c;
            float tx = float(c) / float(num_columns - 1);
            ;
            vertices->set(vi, origin + dx * tx + dy * ty);
            normals->set(vi, normal);
            texcoords->set(vi, vec2(float(c) / float(num_columns - 1), t_origin + t_scale * ty));
        }
    }

    if (stateInfo.wireframe)
    {
        unsigned int i = 0;
        for (unsigned int r = 0; r < num_rows; ++r)
        {
            for (unsigned int c = 0; c < num_columns - 1; ++c)
            {
                unsigned vi = num_columns * r + c;

                indices->set(i++, vi);
                indices->set(i++, vi + 1);
            }
        }

        for (unsigned int c = 0; c < num_columns; ++c)
        {
            for (unsigned int r = 0; r < num_rows - 1; ++r)
            {
                unsigned vi = num_columns * r + c;
                indices->set(i++, vi);
                indices->set(i++, vi + num_columns);
            }
        }
    }
    else
    {
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
    if (texcoords) arrays.push_back(texcoords);
    if (colors) arrays.push_back(colors);
    if (positions) arrays.push_back(positions);
    vid->assignArrays(arrays);

    vid->assignIndices(indices);
    vid->indexCount = static_cast<uint32_t>(indices->size());
    vid->instanceCount = instanceCount;

    scenegraph->addChild(vid);

    if (compileTraversal) compileTraversal->compile(scenegraph);

    subgraph = scenegraph;
    return subgraph;
}
