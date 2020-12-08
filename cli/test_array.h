#include "../include/visky/array.h"



/*************************************************************************************************/
/*  Macros                                                                                       */
/*************************************************************************************************/



/*************************************************************************************************/
/*  Graphics tests                                                                               */
/*************************************************************************************************/

static int vklite2_array_1(VkyTestContext* context)
{
    uint8_t values[] = {1, 2, 3, 4, 5, 6};

    VklArray arr = vkl_array(6, VKL_DTYPE_CHAR);

    // 1:6
    vkl_array_data(&arr, 0, 6, 6, values);
    AT(memcmp(arr.data, values, sizeof(values)) == 0);

    // Partial copy of data.
    vkl_array_data(&arr, 2, 3, 3, values); // put [1, 2, 3] in arr[2:5]
    AT(memcmp(arr.data, (uint8_t[]){1, 2, 1, 2, 3, 6}, sizeof(values)) == 0);

    vkl_array_destroy(&arr);
    return 0;
}



static int vklite2_array_2(VkyTestContext* context)
{
    float values[] = {1, 2, 3, 4, 5, 6};

    VklArray arr = vkl_array(6, VKL_DTYPE_FLOAT);

    // 1:6
    vkl_array_data(&arr, 0, 6, 6, values);
    AT(memcmp(arr.data, values, sizeof(values)) == 0);

    // Partial copy of data.
    vkl_array_data(&arr, 2, 3, 3, values); // put [1, 2, 3] in arr[2:5]
    AT(memcmp(arr.data, (float[]){1, 2, 1, 2, 3, 6}, sizeof(values)) == 0);

    // Resize with less elements. Should not lose the original data.
    vkl_array_resize(&arr, 3);
    AT(memcmp(arr.data, (float[]){1, 2, 1}, 3 * sizeof(float)) == 0);

    // Resize with more elements.
    vkl_array_resize(&arr, 9);
    // Resizing should repeat the last element.
    AT(memcmp(arr.data, (float[]){1, 2, 1, 2, 3, 6, 6, 6, 6}, 9 * sizeof(float)) == 0);

    // Fill the array with a constant value.
    float value = 12.0f;
    ASSERT(arr.item_count == 9);
    vkl_array_data(&arr, 0, 9, 1, &value);
    for (uint32_t i = 0; i < 9; i++)
    {
        AT(((float*)arr.data)[i] == value);
    }

    vkl_array_destroy(&arr);
    return 0;
}



typedef struct _TestDtype _TestDtype;
struct _TestDtype
{
    uint8_t a;
    float b;
};

static int vklite2_array_3(VkyTestContext* context)
{
    // uint8, float32
    VklArray arr = vkl_array_struct(3, sizeof(_TestDtype));

    _TestDtype value = {1, 2};
    vkl_array_data(&arr, 0, 3, 1, &value);
    for (uint32_t i = 0; i < 3; i++)
    {
        AT(((_TestDtype*)(vkl_array_item(&arr, i)))->a == 1);
        AT(((_TestDtype*)(vkl_array_item(&arr, i)))->b == 2);
    }

    // Copy data to the second column.
    float b = 20.0f;
    vkl_array_column(&arr, offsetof(_TestDtype, b), sizeof(float), 1, 2, 1, &b);

    // Row #0.
    AT(((_TestDtype*)(vkl_array_item(&arr, 0)))->a == 1);
    AT(((_TestDtype*)(vkl_array_item(&arr, 0)))->b == 2);

    // Row #1.
    AT(((_TestDtype*)(vkl_array_item(&arr, 1)))->a == 1);
    AT(((_TestDtype*)(vkl_array_item(&arr, 1)))->b == 20);

    // Row #2.
    AT(((_TestDtype*)(vkl_array_item(&arr, 2)))->a == 1);
    AT(((_TestDtype*)(vkl_array_item(&arr, 2)))->b == 20);

    // Resize.
    vkl_array_resize(&arr, 4);
    // Row #3
    AT(((_TestDtype*)(vkl_array_item(&arr, 3)))->a == 1);
    AT(((_TestDtype*)(vkl_array_item(&arr, 3)))->b == 20);

    vkl_array_destroy(&arr);
    return 0;
}



static int vklite2_array_4(VkyTestContext* context)
{
    uint8_t values[] = {1, 2, 3, 4, 5, 6};

    VklArray arr = vkl_array(3, VKL_DTYPE_CHAR);

    // Check automatic resize when setting larger data array.
    AT(arr.item_count == 3);
    vkl_array_data(&arr, 0, 6, 6, values);
    AT(arr.item_count == 6);
    AT(memcmp(arr.data, values, sizeof(values)) == 0);

    vkl_array_destroy(&arr);
    return 0;
}



typedef struct _mvp _mvp;
struct _mvp
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

static int vklite2_array_mvp(VkyTestContext* context)
{
    VklArray arr = vkl_array_struct(1, sizeof(_mvp));

    _mvp id = {0};
    glm_mat4_identity(id.model);
    glm_mat4_identity(id.view);
    glm_mat4_identity(id.proj);

    vkl_array_column(&arr, offsetof(_mvp, model), sizeof(mat4), 0, 1, 1, id.model);
    vkl_array_column(&arr, offsetof(_mvp, view), sizeof(mat4), 0, 1, 1, id.view);
    vkl_array_column(&arr, offsetof(_mvp, proj), sizeof(mat4), 0, 1, 1, id.proj);

    _mvp* mvp = vkl_array_item(&arr, 0);
    for (uint32_t i = 0; i < 4; i++)
    {
        for (uint32_t j = 0; j < 4; j++)
        {
            AT(mvp->model[i][j] == (i == j ? 1 : 0));
            AT(mvp->view[i][j] == (i == j ? 1 : 0));
            AT(mvp->proj[i][j] == (i == j ? 1 : 0));
        }
    }

    vkl_array_destroy(&arr);
    return 0;
}



static int vklite2_array_3D(VkyTestContext* context)
{
    VklArray arr = vkl_array_3D(2, 2, 3, 1, VKL_DTYPE_CHAR);

    uint8_t value = 12;
    vkl_array_data(&arr, 0, 6, 1, &value);

    void* data = arr.data;
    VkDeviceSize size = arr.buffer_size;
    uint8_t* item = vkl_array_item(&arr, 5);
    AT(*item == value);

    // Reshaping deletes the data.
    vkl_array_reshape(&arr, 3, 2, 1);
    item = vkl_array_item(&arr, 5);
    AT(*item == 0);
    // If the total size is the same, the data pointer doesn't change.
    AT(arr.data == data);
    AT(arr.buffer_size == size);

    // Reshaping deletes the data.
    vkl_array_reshape(&arr, 4, 3, 1);
    item = vkl_array_item(&arr, 5);
    AT(*item == 0);
    // If the total size is different, the data buffer is reallocated.
    AT(arr.buffer_size != size);

    vkl_array_destroy(&arr);
    return 0;
}