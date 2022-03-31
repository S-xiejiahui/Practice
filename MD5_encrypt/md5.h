#ifndef __MD5_H__
#define __MD5_H__

/**
 * @brief： 该函数用于将字符串加密成 MD5 格式
 *
 * @param input_string：   待加密的数据
 * @param md5_string ：    加密后的数据
 * @param md5_string_len： md5_string 数组的长度
 */
void MD5_Encrypt_String(unsigned char *input_string, char *output_md5_string, int md5_string_len);

/**
 * @brief: 函数用于将未加密的字符串与加密后的 md5 数据比较
 *
 * @param normal_string：未加密的字符串
 * @param md5_string：md5 字符串
 * @return int ：与 strncmp 返回值一致，成功：0
 */
int string_compared_with_md5_data(char *normal_string, char *md5_string);

#endif