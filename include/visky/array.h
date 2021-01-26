/*************************************************************************************************/
/*  Array API                                                                                    */
/*  Provides a simplistic 1D array object mostly used by the Visual API                          */
/*************************************************************************************************/

#ifndef VKL_ARRAY_HEADER
#define VKL_ARRAY_HEADER

#include "vklite.h"



/*************************************************************************************************/
/*  Typedefs                                                                                     */
/*************************************************************************************************/

typedef struct VklArray VklArray;



/*************************************************************************************************/
/*  Enums                                                                                        */
/*************************************************************************************************/

// Data types.
typedef enum
{
    VKL_DTYPE_NONE,
    VKL_DTYPE_CUSTOM, // used for structured arrays (aka record arrays)
    VKL_DTYPE_STR,    // 64 bits, pointer

    VKL_DTYPE_CHAR, // 8 bits, unsigned int
    VKL_DTYPE_CVEC2,
    VKL_DTYPE_CVEC3,
    VKL_DTYPE_CVEC4,

    VKL_DTYPE_USHORT, // 16 bits, unsigned int
    VKL_DTYPE_USVEC2,
    VKL_DTYPE_USVEC3,
    VKL_DTYPE_USVEC4,

    VKL_DTYPE_SHORT, // 16 bits, signed int
    VKL_DTYPE_SVEC2,
    VKL_DTYPE_SVEC3,
    VKL_DTYPE_SVEC4,

    VKL_DTYPE_UINT, // 32 bits, unsigned int
    VKL_DTYPE_UVEC2,
    VKL_DTYPE_UVEC3,
    VKL_DTYPE_UVEC4,

    VKL_DTYPE_INT, // 32 bits, signed int
    VKL_DTYPE_IVEC2,
    VKL_DTYPE_IVEC3,
    VKL_DTYPE_IVEC4,

    VKL_DTYPE_FLOAT, // 32 bits float
    VKL_DTYPE_VEC2,
    VKL_DTYPE_VEC3,
    VKL_DTYPE_VEC4,

    VKL_DTYPE_DOUBLE, // 64 bits double
    VKL_DTYPE_DVEC2,
    VKL_DTYPE_DVEC3,
    VKL_DTYPE_DVEC4,

    VKL_DTYPE_MAT2, // matrices of floats
    VKL_DTYPE_MAT3,
    VKL_DTYPE_MAT4,
} VklDataType;



// Array copy types.
typedef enum
{
    VKL_ARRAY_COPY_NONE,
    VKL_ARRAY_COPY_REPEAT,
    VKL_ARRAY_COPY_SINGLE,
} VklArrayCopyType;



/*************************************************************************************************/
/*  Structs                                                                                      */
/*************************************************************************************************/

struct VklArray
{
    VklObject obj;
    VklDataType dtype;
    uint32_t components; // number of components, ie 2 for vec2, 3 for dvec3, etc.
    VkDeviceSize item_size;
    uint32_t item_count;
    VkDeviceSize buffer_size;
    void* data;

    // 3D arrays
    uint32_t ndims; // 1, 2, or 3
    uvec3 shape;    // only for 3D arrays
};



/*************************************************************************************************/
/*  Utils                                                                                        */
/*************************************************************************************************/

// Size in bytes of a single element of a given dtype.
static VkDeviceSize _get_dtype_size(VklDataType dtype)
{
    switch (dtype)
    {
    // 8 bits
    case VKL_DTYPE_CHAR:
        return 1;
    case VKL_DTYPE_CVEC2:
        return 1 * 2;
    case VKL_DTYPE_CVEC3:
        return 1 * 3;
    case VKL_DTYPE_CVEC4:
        return 1 * 4;

    // 16 bits
    case VKL_DTYPE_USHORT:
    case VKL_DTYPE_SHORT:
        return 2;
    case VKL_DTYPE_SVEC2:
    case VKL_DTYPE_USVEC2:
        return 2 * 2;
    case VKL_DTYPE_SVEC3:
    case VKL_DTYPE_USVEC3:
        return 2 * 3;
    case VKL_DTYPE_SVEC4:
    case VKL_DTYPE_USVEC4:
        return 2 * 4;

    // 32 bits
    case VKL_DTYPE_FLOAT:
    case VKL_DTYPE_UINT:
    case VKL_DTYPE_INT:
        return 4;

    case VKL_DTYPE_VEC2:
    case VKL_DTYPE_UVEC2:
    case VKL_DTYPE_IVEC2:
        return 4 * 2;

    case VKL_DTYPE_VEC3:
    case VKL_DTYPE_UVEC3:
    case VKL_DTYPE_IVEC3:
        return 4 * 3;

    case VKL_DTYPE_VEC4:
    case VKL_DTYPE_UVEC4:
    case VKL_DTYPE_IVEC4:
        return 4 * 4;

    // 64 bits
    case VKL_DTYPE_DOUBLE:
        return 8;
    case VKL_DTYPE_DVEC2:
        return 8 * 2;
    case VKL_DTYPE_DVEC3:
        return 8 * 3;
    case VKL_DTYPE_DVEC4:
        return 8 * 4;
    case VKL_DTYPE_STR:
        return sizeof(char*);

    case VKL_DTYPE_MAT2:
        return 2 * 2 * 4;
    case VKL_DTYPE_MAT3:
        return 3 * 3 * 4;
    case VKL_DTYPE_MAT4:
        return 4 * 4 * 4;

    default:
        break;
    }

    if (dtype != VKL_DTYPE_NONE)
        log_error("could not find the size of dtype %d", dtype);
    return 0;
}



// Number of components in a given dtype (e.g. 4 for vec4)
static uint32_t _get_components(VklDataType dtype)
{
    switch (dtype)
    {
    case VKL_DTYPE_CHAR:
    case VKL_DTYPE_USHORT:
    case VKL_DTYPE_SHORT:
    case VKL_DTYPE_UINT:
    case VKL_DTYPE_INT:
    case VKL_DTYPE_FLOAT:
    case VKL_DTYPE_DOUBLE:
        return 1;

    case VKL_DTYPE_CVEC2:
    case VKL_DTYPE_USVEC2:
    case VKL_DTYPE_SVEC2:
    case VKL_DTYPE_UVEC2:
    case VKL_DTYPE_IVEC2:
    case VKL_DTYPE_VEC2:
    case VKL_DTYPE_DVEC2:
        return 2;

    case VKL_DTYPE_CVEC3:
    case VKL_DTYPE_USVEC3:
    case VKL_DTYPE_SVEC3:
    case VKL_DTYPE_UVEC3:
    case VKL_DTYPE_IVEC3:
    case VKL_DTYPE_VEC3:
    case VKL_DTYPE_DVEC3:
        return 3;

    case VKL_DTYPE_CVEC4:
    case VKL_DTYPE_USVEC4:
    case VKL_DTYPE_SVEC4:
    case VKL_DTYPE_UVEC4:
    case VKL_DTYPE_IVEC4:
    case VKL_DTYPE_VEC4:
    case VKL_DTYPE_DVEC4:
        return 4;

    default:
        return 0;
        break;
    }
    return 0;
}



/*************************************************************************************************/
/*  Functions                                                                                    */
/*************************************************************************************************/

// Create a new 1D array with a given dtype, number of elements, and item size (used for record
// arrays containing heterogeneous data)
static VklArray _create_array(uint32_t item_count, VklDataType dtype, VkDeviceSize item_size)
{
    VklArray arr;
    memset(&arr, 0, sizeof(VklArray));
    arr.obj.type = VKL_OBJECT_TYPE_ARRAY;
    arr.dtype = dtype;
    arr.components = _get_components(dtype);
    arr.item_size = item_size;
    arr.item_count = item_count;
    arr.buffer_size = item_count * arr.item_size;
    if (item_count > 0)
        arr.data = calloc(item_count, arr.item_size);
    obj_created(&arr.obj);
    return arr;
}



/**
 * Create a new 1D array.
 *
 * @param item_count initial number of elements
 * @param dtype the data type of the array
 * @returns a new array
 */
static VklArray vkl_array(uint32_t item_count, VklDataType dtype)
{
    ASSERT(dtype != VKL_DTYPE_NONE);
    ASSERT(dtype != VKL_DTYPE_CUSTOM);
    return _create_array(item_count, dtype, _get_dtype_size(dtype));
}



/**
 * Create an array with a single dvec3 position.
 *
 * @param pos initial number of elements
 * @returns a new array
 */
static VklArray vkl_array_point(dvec3 pos)
{
    VklArray arr = vkl_array(1, VKL_DTYPE_DVEC3);
    memcpy(arr.data, pos, sizeof(dvec3));
    return arr;
}



/**
 * Create a 1D array from an existing compatible memory buffer.
 *
 * The created array does not allocate memory, it uses the passed buffer instead.
 *
 * !!! warning
 *     Destroying the array will free the passed pointer!
 *
 * @param item_count number of elements in the passed buffer
 * @param dtype the data type of the array
 * @returns the array wrapping the buffer
 */
static VklArray vkl_array_wrap(uint32_t item_count, VklDataType dtype, void* data)
{
    VklArray arr = vkl_array(0, dtype); // do not allocate underlying buffer
    // Manual setting of struct fields with the passed buffer
    arr.item_count = item_count;
    arr.buffer_size = item_count * arr.item_size;
    arr.data = data;
    return arr;
}



/**
 * Create a 1D record array with heterogeneous data type.
 *
 * @param item_count number of elements
 * @param item_size size, in bytes, of each item
 * @returns the array
 */
static VklArray vkl_array_struct(uint32_t item_count, VkDeviceSize item_size)
{
    ASSERT(item_size > 0);
    return _create_array(item_count, VKL_DTYPE_CUSTOM, item_size);
}



/**
 * Create a 3D array holding a texture.
 *
 * @param ndims number of dimensions (1, 2, 3)
 * @param width number of elements along the 1st dimension
 * @param height number of elements along the 2nd dimension
 * @param depth number of elements along the 3rd dimension
 * @param item_size size of each item in bytes
 * @returns the array
 */
static VklArray vkl_array_3D(
    uint32_t ndims, uint32_t width, uint32_t height, uint32_t depth, VkDeviceSize item_size)
{
    ASSERT(ndims > 0);
    ASSERT(ndims <= 3);

    if (ndims == 1)
        ASSERT(height <= 1 && depth <= 1);
    if (ndims == 2)
        ASSERT(depth <= 1);

    uint32_t item_count = width * height * depth;

    VklArray arr = _create_array(item_count, VKL_DTYPE_CUSTOM, item_size);
    arr.ndims = ndims;
    arr.shape[0] = width;
    arr.shape[1] = height;
    arr.shape[2] = depth;
    return arr;
}



// Fill the remaining of an array with the last non-empty value.
static void
_repeat_last(uint32_t old_item_count, VkDeviceSize item_size, void* data, uint32_t item_count)
{
    // Repeat the last item of an array.
    VkDeviceSize old_size = old_item_count * item_size;
    int64_t dst_offset = (int64_t)data + (int64_t)old_size;
    int64_t src_offset = (int64_t)data + (int64_t)old_size - (int64_t)item_size;
    ASSERT(item_count > old_item_count);
    uint32_t repeat_count = item_count - old_item_count;
    for (uint32_t i = 0; i < repeat_count; i++)
    {
        memcpy((void*)dst_offset, (void*)src_offset, item_size);
        dst_offset += (int64_t)item_size;
    }
}



/**
 * Resize an existing array.
 *
 * * If the new size is equal to the old size, do nothing.
 * * If the new size is smaller than the old size, change the size attribute but do not reallocate
 * * If the new size is larger than the old size, reallocate memory and copy over the old values
 *
 * @param array the array to resize
 * @param item_count the new number of items
 */
static void vkl_array_resize(VklArray* array, uint32_t item_count)
{
    ASSERT(array != NULL);
    ASSERT(item_count > 0);
    ASSERT(array->item_size > 0);

    uint32_t old_item_count = array->item_count;

    // Do nothing if the size is the same.
    if (item_count == old_item_count)
        return;

    // If the array was not allocated, allocate it with the specified size.
    if (array->data == NULL)
    {
        array->data = calloc(item_count, array->item_size);
        array->item_count = item_count;

        // NOTE: using next_pow2() below causes a crash in scene_axes test
        array->buffer_size = item_count * array->item_size;
        // array->buffer_size = next_pow2(item_count * array->item_size);

        log_trace(
            "allocate array to contain %d elements (%s)", item_count,
            pretty_size(array->buffer_size));
        return;
    }

    // Here, the array was already allocated, and the requested size is different.
    VkDeviceSize old_size = array->buffer_size;
    VkDeviceSize new_size = item_count * array->item_size;
    ASSERT(array->data != NULL);

    // Only reallocate if the existing buffer is not large enough for the new item_count.
    if (new_size > old_size)
    {
        uint32_t new_item_count = 2 * old_item_count;
        while (new_item_count < item_count)
            new_item_count *= 2;
        ASSERT(new_item_count >= item_count);
        new_size = new_item_count * array->item_size;
        log_debug(
            "resize array from %d to %d items of size %d", old_item_count, new_item_count,
            array->item_size);
        REALLOC(array->data, new_size);
        // Repeat the last element when resizing.
        _repeat_last(old_size / array->item_size, array->item_size, array->data, new_item_count);
        array->buffer_size = new_size;
    }
    array->item_count = item_count;
}



/**
 * Reset to 0 the contents of an existing array.
 *
 * @param array the array to clear
 */
static void vkl_array_clear(VklArray* array)
{
    ASSERT(array != NULL);
    memset(array->data, 0, array->buffer_size);
}



/**
 * Reshape a 3D array and *delete all the data in it*.
 *
 * !!! warning
 *     The contents of the array will be cleared. Copying the existing data would require more work
 *     and is not necessary at the moment.
 *
 * @param array the array to reshape and clear
 * @param width number of elements along the 1st dimension
 * @param height number of elements along the 2nd dimension
 * @param depth number of elements along the 3rd dimension
 */
static void vkl_array_reshape(VklArray* array, uint32_t width, uint32_t height, uint32_t depth)
{
    ASSERT(array != NULL);
    ASSERT(width > 0);
    ASSERT(height > 0);
    ASSERT(depth > 0);
    uint32_t item_count = width * height * depth;

    // If the shape is the same, do nothing.
    if (width == array->shape[0] && height == array->shape[1] && depth == array->shape[2])
        return;

    // Resize the underlying buffer.
    vkl_array_resize(array, item_count);

    // HACK: reset to 0 the array instead of having to deal with reshaping.
    log_trace("clearing the 3D array while reshaping it to %dx%dx%d", width, height, depth);
    vkl_array_clear(array);

    array->shape[0] = width;
    array->shape[1] = height;
    array->shape[2] = depth;
}



/**
 * Copy data into an array.
 *
 * * There will be `item_count` values copied between `first_item` and `first_item + item_count` in
 *   the array.
 * * There are `data_item_count` values in the passed buffer.
 * * If `item_count > data_item_count`, the last value of `data` will be repeated until the last
 * value.
 *
 * Example:
 *
 * === "C"
 *     ```c
 *     // Create an array of 10 double numbers, initialize all elements with 1.23.
 *     VklArray arr = vkl_array(10, VKL_DTYPE_DOUBLE);
 *     double item = 1.23;
 *     vkl_array_data(&arr, 0, 10, 1, &item);
 *     ```
 *
 * @param array the array
 * @param first_item first element in the array to be overwritten
 * @param item_count number of items to write
 * @param data_item_count number of elements in `data`
 * @param data the buffer containing the data to copy
 */
static void vkl_array_data(
    VklArray* array, uint32_t first_item, uint32_t item_count, //
    uint32_t data_item_count, const void* data)
{
    ASSERT(array != NULL);
    ASSERT(data_item_count > 0);
    ASSERT(array->data != NULL);
    if (data == NULL)
    {
        log_debug("skipping vkl_array_data() with NULL data");
        return;
    }
    ASSERT(item_count > 0);

    // Resize if necessary.
    if (first_item + item_count > array->item_count)
    {
        vkl_array_resize(array, first_item + item_count);
    }
    ASSERT(first_item + item_count <= array->item_count);
    ASSERT(array->item_size > 0);
    ASSERT(array->item_count > 0);

    VkDeviceSize item_size = array->item_size;
    ASSERT(item_size > 0);

    void* dst = array->data;
    // Allocate the array if needed.
    if (dst == NULL)
        dst = array->data = calloc(first_item + array->item_count, array->item_size);
    ASSERT(dst != NULL);
    const void* src = data;
    ASSERT(src != NULL);

    VkDeviceSize copy_size = MIN(item_count, data_item_count) * item_size;
    ASSERT(copy_size > 0);
    log_trace(
        "copy %d elements (%d bytes) into array[%d:%d]", //
        data_item_count, copy_size, first_item, first_item + item_count);
    ASSERT(array->buffer_size >= (first_item + item_count) * item_size);
    memcpy((void*)((int64_t)dst + (int64_t)(first_item * item_size)), src, copy_size);

    // If the source data array is smaller than the destination array, repeat the last value.
    if (data_item_count < item_count)
    {
        _repeat_last(
            data_item_count, array->item_size,
            (void*)((int64_t)array->data + (int64_t)(first_item * item_size)), item_count);
    }
}



/**
 * Retrieve a single element from an array.
 *
 * @param array the array
 * @param idx the index of the element to retrieve
 * @returns a pointer to the requested element
 */
static inline void* vkl_array_item(VklArray* array, uint32_t idx)
{
    ASSERT(array != NULL);
    idx = CLIP(idx, 0, array->item_count - 1);
    return (void*)((int64_t)array->data + (int64_t)(idx * array->item_size));
}



// Cast a vector.
static inline void _cast(VklDataType target_dtype, void* dst, VklDataType source_dtype, void* src)
{
    if (source_dtype == VKL_DTYPE_DOUBLE && target_dtype == VKL_DTYPE_FLOAT)
    {
        ((vec3*)dst)[0][0] = ((dvec3*)src)[0][0];
    }
    else if (source_dtype == VKL_DTYPE_DVEC2 && target_dtype == VKL_DTYPE_VEC2)
    {
        ((vec3*)dst)[0][0] = ((dvec3*)src)[0][0];
        ((vec3*)dst)[0][1] = ((dvec3*)src)[0][1];
    }
    else if (source_dtype == VKL_DTYPE_DVEC3 && target_dtype == VKL_DTYPE_VEC3)
    {
        ((vec3*)dst)[0][0] = ((dvec3*)src)[0][0];
        ((vec3*)dst)[0][1] = ((dvec3*)src)[0][1];
        ((vec3*)dst)[0][2] = ((dvec3*)src)[0][2];
    }
    else
        log_error("unknown casting dtypes %d %d", source_dtype, target_dtype);
}



/**
 * Copy data into the column of a record array.
 *
 * This function is used by the default visual baking function, which copies to the vertex buffer
 * (corresponding to a record array with as many fields as GLSL attributes in the vertex shader)
 * the user-specified visual props (data for the individual elements).
 *
 * @param array the array
 * @param offset the offset within the array, in bytes
 * @param col_size stride in the source array, in bytes
 * @param first_item first element in the array to be overwritten
 * @param item_count number of elements to write
 * @param data_item_count number of elements in `data`
 * @param data the buffer containing the data to copy
 * @param source_dtype the source dtype (only used when casting)
 * @param target_dtype the target dtype (only used when casting)
 * @param copy_type the type of copy
 * @param reps the number of repeats for each copied element
 */
static void vkl_array_column(
    VklArray* array, VkDeviceSize offset, VkDeviceSize col_size, //
    uint32_t first_item, uint32_t item_count,                    //
    uint32_t data_item_count, const void* data,                  //
    VklDataType source_dtype, VklDataType target_dtype,          //
    VklArrayCopyType copy_type, uint32_t reps)                   //
{
    ASSERT(array != NULL);
    ASSERT(data_item_count > 0);
    ASSERT(array->data != NULL);
    ASSERT(data != NULL);
    ASSERT(item_count > 0);
    ASSERT(first_item + item_count <= array->item_count);

    VkDeviceSize src_offset = 0;
    VkDeviceSize src_stride = col_size;

    VkDeviceSize dst_offset = offset;
    VkDeviceSize dst_stride = array->item_size;

    void* dst = array->data;
    const void* src = data;

    ASSERT(src != NULL);
    ASSERT(dst != NULL);
    ASSERT(src_stride > 0);
    ASSERT(dst_stride > 0);
    ASSERT(item_count > 0);

    log_trace(
        "copy src offset %d stride %d, dst offset %d stride %d, item size %d count %d", //
        src_offset, src_stride, dst_offset, dst_stride, col_size, item_count);

    int64_t src_byte = (int64_t)src + (int64_t)src_offset;
    int64_t dst_byte = (int64_t)dst + (int64_t)(first_item * dst_stride) + (int64_t)dst_offset;

    uint32_t j = 0; // j: src index
    uint32_t m = 0;
    bool skip = false;
    for (uint32_t i = 0; i < item_count; i++) // i: dst index
    {
        if (reps > 1)
            m = i % reps;
        // Determine whether the current item copy should be skipped.
        skip = copy_type == VKL_ARRAY_COPY_SINGLE && reps > 1 && m > 0;

        // NOTE: this function is not optimized and might benefit from being refactored.

        // Copy the current item, unless we are in SINGLE copy mode
        if (!skip)
        {
            if (source_dtype == target_dtype ||   //
                source_dtype == VKL_DTYPE_NONE || //
                target_dtype == VKL_DTYPE_NONE)   //
                memcpy((void*)dst_byte, (void*)src_byte, col_size);
            else
            {
                _cast(target_dtype, (void*)dst_byte, source_dtype, (void*)src_byte);
            }
        }

        // Advance the source pointer, unless we are in SINGLE copy mode
        skip = reps > 1 && m < reps - 1;
        if (j < data_item_count - 1 && !skip)
        {
            src_byte += (int64_t)src_stride;
            j++;
        }

        dst_byte += (int64_t)dst_stride;
    }
}



/**
 * Destroy an array.
 *
 * This function frees the allocated underlying data buffer.
 *
 * @param array the array to destroy
 */
static void vkl_array_destroy(VklArray* array)
{
    ASSERT(array != NULL);
    if (!is_obj_created(&array->obj))
        return;
    obj_destroyed(&array->obj);
    FREE(array->data) //
}



#endif
