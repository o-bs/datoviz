#include "../include/visky/visuals2.h"
#include "utils.h"


#define N_FRAMES 10



/*************************************************************************************************/
/*  Macros                                                                                       */
/*************************************************************************************************/

#ifndef RANDN_POS
#define RANDN_POS(x)                                                                              \
    x[0] = .25 * randn();                                                                         \
    x[1] = .25 * randn();                                                                         \
    x[2] = .5;
#endif

#ifndef RAND_COLOR
#define RAND_COLOR(x)                                                                             \
    x[0] = rand_byte();                                                                           \
    x[1] = rand_byte();                                                                           \
    x[2] = rand_byte();                                                                           \
    x[3] = 255;
#endif



/*************************************************************************************************/
/*  Graphics tests                                                                               */
/*************************************************************************************************/

static void _canvas_fill(VklCanvas* canvas, VklPrivateEvent ev)
{
    ASSERT(canvas != NULL);
    ASSERT(ev.user_data != NULL);
    VklVisual* visual = (VklVisual*)ev.user_data;
    VklViewport viewport = vkl_viewport_full(canvas);

    // TODO: choose which of all canvas command buffers need to be filled with the visual
    // For now, update all of them.
    for (uint32_t i = 0; i < ev.u.rf.cmd_count; i++)
    {
        vkl_visual_fill_event(
            visual, ev.u.rf.clear_color, ev.u.rf.cmds[i], ev.u.rf.img_idx, viewport, NULL);
    }
}

static void _bindings(VklVisual* visual, uint32_t idx, VklMVP* mvp)
{
    ASSERT(visual != NULL);
    ASSERT(mvp != NULL);
    VklGpu* gpu = visual->canvas->gpu;

    // Create the bindings.
    visual->gbindings[idx] = vkl_bindings(&visual->graphics[idx]->slots, 1);
    VklBindings* bindings = &visual->gbindings[idx];

    // Binding resources.
    visual->buffers[0] =
        vkl_ctx_buffers(gpu->context, VKL_DEFAULT_BUFFER_UNIFORM, 1, sizeof(VklMVP));
    visual->buffers[1] = vkl_ctx_buffers(gpu->context, VKL_DEFAULT_BUFFER_UNIFORM, 1, 16);
    visual->buffers[2] = vkl_ctx_buffers(
        gpu->context, VKL_DEFAULT_BUFFER_UNIFORM, 1, sizeof(VklGraphicsPointsParams));
    visual->textures[0] =
        vkl_ctx_texture(gpu->context, 2, (uvec3){16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM);

    // Upload MVP.
    glm_mat4_identity(mvp->model);
    glm_mat4_identity(mvp->view);
    glm_mat4_identity(mvp->proj);
    vkl_upload_buffers(gpu->context, &visual->buffers[0], 0, sizeof(VklMVP), mvp);

    // Upload params.
    float param = 5.0f;
    VklGraphicsPointsParams params = {.point_size = param};
    vkl_upload_buffers(
        gpu->context, &visual->buffers[2], 0, sizeof(VklGraphicsPointsParams), &params);

    // Bindings
    vkl_bindings_buffer(bindings, 0, &visual->buffers[0]);
    vkl_bindings_buffer(bindings, 1, &visual->buffers[1]);
    vkl_bindings_texture(bindings, 2, visual->textures[0]->image, visual->textures[0]->sampler);
    vkl_bindings_buffer(bindings, 3, &visual->buffers[2]);

    vkl_bindings_update(bindings);
}

static void _copy_array(
    const void* src, VkDeviceSize src_offset, VkDeviceSize src_stride, //
    void* dst, VkDeviceSize dst_offset, VkDeviceSize dst_stride,       //
    VkDeviceSize item_size, uint32_t item_count)
{
    ASSERT(src != NULL);
    ASSERT(dst != NULL);
    ASSERT(src_stride > 0);
    ASSERT(dst_stride > 0);
    ASSERT(item_size > 0);
    ASSERT(item_count > 0);

    log_debug(
        "copy src offset %d stride %d, dst offset %d stride %d, item size %d count %d", src_offset,
        src_stride, dst_offset, dst_stride, item_size, item_count);

    int64_t src_byte = (int64_t)src + (int64_t)src_offset;
    int64_t dst_byte = (int64_t)dst + (int64_t)dst_offset;
    for (uint32_t i = 0; i < item_count; i++)
    {
        memcpy((void*)dst_byte, (void*)src_byte, item_size);
        src_byte += (int64_t)src_stride;
        dst_byte += (int64_t)dst_stride;
    }
}

static void _visual_bake(VklVisual* visual, VklVisualDataEvent ev)
{
    ASSERT(visual != NULL);
    uint32_t item_count = visual->item_count;
    if (visual->item_count_triangulated != 0)
        item_count = visual->item_count_triangulated;
    ASSERT(item_count > 0);
    VkDeviceSize item_size = 0;

    log_debug("bake visual");
    vkl_visual_data_alloc(visual, item_count, 0);

    VklSource* source = NULL;
    void* src = NULL;
    for (uint32_t s = 0; s < visual->source_count; s++)
    {
        source = &visual->sources[s];
        if (source->loc == VKL_PROP_LOC_VERTEX_ATTR)
        {
            // Source data array.
            src = source->u.a.data_original;
            if (source->u.a.data_transformed != NULL)
                src = source->u.a.data_transformed;
            if (source->u.a.data_triangulated != NULL)
                src = source->u.a.data_triangulated;

            // Item size.
            item_size = source->dtype_size;
            ASSERT(item_size > 0);

            // Copy the source data array to the vertex data.
            _copy_array(
                src, 0, item_size,                                        //
                visual->vertex_data, source->offset, visual->vertex_size, //
                item_size, item_count);
        }
    }
}

static int vklite2_visuals_1(VkyTestContext* context)
{
    VklApp* app = vkl_app(VKL_BACKEND_GLFW);
    VklGpu* gpu = vkl_gpu(app, 0);
    VklCanvas* canvas = vkl_canvas(gpu, TEST_WIDTH, TEST_HEIGHT);

    VklVisual visual = vkl_visual(canvas);

    // Props.
    vkl_visual_vertex(&visual, sizeof(VklVertex));
    vkl_visual_prop(
        &visual, VKL_PROP_POS, 0, VKL_DTYPE_VEC3, VKL_PROP_LOC_VERTEX_ATTR, 0, 0,
        offsetof(VklVertex, pos));
    vkl_visual_prop(
        &visual, VKL_PROP_COLOR, 0, VKL_DTYPE_CVEC4, VKL_PROP_LOC_VERTEX_ATTR, 0, 1,
        offsetof(VklVertex, color));

    // Graphics.
    vkl_visual_graphics(&visual, vkl_graphics_builtin(canvas, VKL_GRAPHICS_POINTS, 0));

    // Transform data.
    VklMVP mvp = {0};
    glm_mat4_identity(mvp.model);
    glm_mat4_identity(mvp.view);
    glm_mat4_identity(mvp.proj);
    // HACK: make sure the transfer are sync so that transient pointers like parameters
    // exist by the time they are uploaded to the GPU.
    vkl_transfer_mode(gpu->context, VKL_TRANSFER_MODE_SYNC);
    _bindings(&visual, 0, &mvp);
    vkl_transfer_mode(gpu->context, VKL_TRANSFER_MODE_ASYNC);

    // Vertex data.
    const uint32_t N = 10000;
    vec3* pos = calloc(N, sizeof(vec3));
    cvec4* color = calloc(N, sizeof(cvec4));
    for (uint32_t i = 0; i < N; i++)
    {
        RANDN_POS(pos[i])
        RAND_COLOR(color[i])
    }

    // Set visual data.
    vkl_visual_size(&visual, N, 0);
    vkl_visual_data(&visual, VKL_PROP_POS, 0, pos);
    vkl_visual_data(&visual, VKL_PROP_COLOR, 0, color);

    vkl_visual_bake_callback(&visual, _visual_bake);

    // Update visual data.
    VklViewport viewport = vkl_viewport_full(canvas);
    vkl_visual_data_update(&visual, viewport, (VklDataCoords){0}, NULL);

    vkl_canvas_callback(canvas, VKL_PRIVATE_EVENT_REFILL, 0, _canvas_fill, &visual);

    // Run and end.
    vkl_app_run(app, 0);
    vkl_visual_destroy(&visual);
    FREE(pos);
    FREE(color);
    TEST_END
}
