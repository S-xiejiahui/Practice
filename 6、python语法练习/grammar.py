arr = ['123', '456', '789']
# 元组
tuple_tmp = ("123", "456", "789")
# 数字、字符串
num_tmp = 1
string_tmp = "456789"
# 集合、字典
set_tmp = (['123', 456, 456])
# 删除元素
del arr[1], num_tmp
# 判断语法 if 和打印 print
if arr[0] == '1234' and type(arr) == list:
    print("count = {}".format(arr.count("123")))
    print("[debug]: {0},{1}".format("yes", "it is"))
else:
    print(arr, "max = %s,"%max(arr), "type = %s"%type(arr))
# 循环，打印列表各元素并把每行结束符替换为 ';' 号
for i in arr:
    print(i, end=";")