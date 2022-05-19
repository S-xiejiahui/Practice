#include "cJSON.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief 递归获取
 * 
 * @param obj 
 * @param result 
 */
void recursion_topo_get(cJSON *obj, cJSON *result)
{
    cJSON *obj_mac = cJSON_GetObjectItem(obj, "mac");
    if (NULL != obj_mac)
    {
        cJSON *tmp = cJSON_CreateObject();
        cJSON_AddStringToObject(tmp, obj_mac->string, obj_mac->valuestring);
        cJSON_AddItemToArray(result, tmp);
    }

    cJSON *childNode = cJSON_GetObjectItem(obj, "childNode");
    if (NULL != childNode)
    {
        int i = 0;
        int num = cJSON_GetArraySize(childNode);
        for (i = 0; i < num; i++)
        {
            cJSON *childNode_ptr = cJSON_GetArrayItem(childNode, i);
            if (NULL != childNode_ptr)
            {
                recursion_topo_get(childNode_ptr, result);
            }
        }
    }
    return;
}

int main(int argc, char const *argv[])
{

    char topo_buf[4096] = {0};
    FILE *fp = fopen("topo.txt", "r");
    if (NULL == fp)
    {
        return -1;
    }

    fgets(topo_buf, 4096, fp);

    cJSON *obj = cJSON_Parse(topo_buf);
    cJSON *mac_result = cJSON_CreateArray();

    recursion_topo_get(obj, mac_result);

    int i = 0, arr_size = 0;
    arr_size = cJSON_GetArraySize(mac_result);

    for (i = 0; i < arr_size; i++)
    {
        cJSON *arr_item = cJSON_GetArrayItem(mac_result, i);
        printf("%s\n", cJSON_Print(arr_item));
    }

    cJSON_Delete(mac_result);
    cJSON_Delete(obj);
    fclose(fp);
    return 0;
}
