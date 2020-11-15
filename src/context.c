#include "../include/visky/context.h"
#include <stdlib.h>



/*************************************************************************************************/
/*  Macros                                                                                       */
/*************************************************************************************************/



/*************************************************************************************************/
/*  Context                                                                                      */
/*************************************************************************************************/

VklContext vkl_context(VklGpu* gpu)
{
    log_trace("creating context");
    VklContext context_struct = {0};
    VklContext* context = &context_struct;
    context->gpu = gpu;

    // Allocate memory for buffers, textures, and computes.
    INSTANCES_INIT(
        VklBuffer, context, buffers, max_buffers, VKL_MAX_BUFFERS, VKL_OBJECT_TYPE_IMAGES)

    INSTANCES_INIT(
        VklImages, context, images, max_images, VKL_MAX_TEXTURES, VKL_OBJECT_TYPE_IMAGES)

    INSTANCES_INIT(
        VklSampler, context, samplers, max_samplers, VKL_MAX_TEXTURES, VKL_OBJECT_TYPE_SAMPLER)

    INSTANCES_INIT(
        VklCompute, context, computes, max_computes, VKL_MAX_COMPUTES, VKL_OBJECT_TYPE_COMPUTE)

    return context_struct;
}



void vkl_context_destroy(VklContext* context)
{
    if (context == NULL)
    {
        log_error("skip destruction of null context");
        return;
    }
    log_trace("destroying context");
    ASSERT(context != NULL);
    ASSERT(context->gpu != NULL);


    log_trace("context destroy buffers");
    for (uint32_t i = 0; i < context->max_buffers; i++)
    {
        if (context->buffers[i].obj.status == VKL_OBJECT_STATUS_NONE)
            break;
        vkl_buffer_destroy(&context->buffers[i]);
    }
    INSTANCES_DESTROY(context->buffers)


    log_trace("context destroy sets of images");
    for (uint32_t i = 0; i < context->max_images; i++)
    {
        if (context->images[i].obj.status == VKL_OBJECT_STATUS_NONE)
            break;
        vkl_images_destroy(&context->images[i]);
    }
    INSTANCES_DESTROY(context->images)


    log_trace("context destroy samplers");
    for (uint32_t i = 0; i < context->max_samplers; i++)
    {
        if (context->samplers[i].obj.status == VKL_OBJECT_STATUS_NONE)
            break;
        vkl_sampler_destroy(&context->samplers[i]);
    }
    INSTANCES_DESTROY(context->samplers)


    log_trace("context destroy computes");
    for (uint32_t i = 0; i < context->max_computes; i++)
    {
        if (context->computes[i].obj.status == VKL_OBJECT_STATUS_NONE)
            break;
        vkl_compute_destroy(&context->computes[i]);
    }
    INSTANCES_DESTROY(context->computes)
}



VklBufferRegions vkl_alloc_buffer(VklContext* context, VklBufferType type, VkDeviceSize size) {}



VklCompute* vkl_new_compute(VklContext* context, const char* shader_path) {}



VklTexture* vkl_new_texture(VklContext* context, uint32_t dims, uvec3 size, VkFormat format) {}



void vkl_texture_resize(VklTexture* texture, uvec3 size) {}



void vkl_texture_filter(VklTexture* texture, VklFilterType type, VkFilter filter) {}



void vkl_texture_address_mode(
    VklTexture* texture, VklTextureAxis axis, VkSamplerAddressMode address_mode)
{
}



void vkl_texture_destroy(VklTexture* texture)
{
    if (texture != NULL)
    {
        vkl_images_destroy(texture->image);
        vkl_sampler_destroy(texture->sampler);
    }
}



/*************************************************************************************************/
/*  Default app                                                                                  */
/*************************************************************************************************/

VklApp* vkl_default_app(uint32_t gpu_idx, bool offscreen)
{
    VklApp* app = vkl_app(VKL_BACKEND_GLFW);
    VklGpu* gpu = vkl_gpu(app, gpu_idx);

    // Default queues.
    {
        vkl_gpu_queue(gpu, VKL_QUEUE_TRANSFER, VKL_DEFAULT_QUEUE_DATA);
        vkl_gpu_queue(gpu, VKL_QUEUE_COMPUTE, VKL_DEFAULT_QUEUE_COMPUTE);
        vkl_gpu_queue(gpu, VKL_QUEUE_RENDER, VKL_DEFAULT_QUEUE_RENDER);
    }

    // Create the GPU.
    if (!offscreen)
    {
        // Need a present queue.
        vkl_gpu_queue(gpu, VKL_QUEUE_PRESENT, VKL_DEFAULT_QUEUE_PRESENT);

        // Need a surface to create the GPU, unless offscree
        VklWindow* window = vkl_window(app, VKL_DEFAULT_WIDTH, VKL_DEFAULT_HEIGHT);
        vkl_gpu_create(gpu, window->surface);
    }
    else
    {
        vkl_gpu_create(gpu, 0);
    }

    // Create a context.
    VklContext context_struct = vkl_context(gpu);
    gpu->context = calloc(1, sizeof(VklContext));
    *gpu->context = context_struct;
    VklContext* context = gpu->context;

    // Create a predetermined set of buffers.
    VklBuffer* buffer = NULL;
    for (uint32_t i = 0; i < VKL_DEFAULT_BUFFER_COUNT; i++)
    {
        context->buffers[i] = vkl_buffer(gpu);
        buffer = &context->buffers[i];

        // All buffers may be accessed from these queues.
        vkl_buffer_queue_access(buffer, VKL_DEFAULT_QUEUE_DATA);
        vkl_buffer_queue_access(buffer, VKL_DEFAULT_QUEUE_COMPUTE);
        vkl_buffer_queue_access(buffer, VKL_DEFAULT_QUEUE_RENDER);
    }

    // Staging buffer
    buffer = &context->buffers[VKL_DEFAULT_BUFFER_STAGING];
    vkl_buffer_size(buffer, VKL_DEFAULT_BUFFER_STAGING_SIZE, 0);
    vkl_buffer_usage(buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    vkl_buffer_memory(
        buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkl_buffer_create(buffer);

    VkBufferUsageFlagBits transferable =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    // Vertex buffer
    buffer = &context->buffers[VKL_DEFAULT_BUFFER_VERTEX];
    vkl_buffer_size(buffer, VKL_DEFAULT_BUFFER_VERTEX_SIZE, 0);
    vkl_buffer_usage(buffer, transferable | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    vkl_buffer_memory(buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkl_buffer_create(buffer);

    // Index buffer
    buffer = &context->buffers[VKL_DEFAULT_BUFFER_INDEX];
    vkl_buffer_size(buffer, VKL_DEFAULT_BUFFER_INDEX_SIZE, 0);
    vkl_buffer_usage(buffer, transferable | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    vkl_buffer_memory(buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkl_buffer_create(buffer);

    // Storage buffer
    buffer = &context->buffers[VKL_DEFAULT_BUFFER_STORAGE];
    vkl_buffer_size(buffer, VKL_DEFAULT_BUFFER_STORAGE_SIZE, 0);
    vkl_buffer_usage(buffer, transferable | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    vkl_buffer_memory(buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkl_buffer_create(buffer);

    // Uniform buffer
    buffer = &context->buffers[VKL_DEFAULT_BUFFER_UNIFORM];
    vkl_buffer_size(buffer, VKL_DEFAULT_BUFFER_UNIFORM_SIZE, 0);
    vkl_buffer_usage(buffer, transferable | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vkl_buffer_memory(buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkl_buffer_create(buffer);

    return app;
}