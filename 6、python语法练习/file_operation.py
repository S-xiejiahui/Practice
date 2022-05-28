"""
from ast import Num
import os
file = open(os.path.dirname(__file__) + "\\1.txt", encoding='utf-8')
text = file.read()
print(text)
file.close()
"""
# python 调用 exe 文件
# main = r"D:\1.txt"
# rv = os.system(main + '123456')

from tkinter import *

root = Tk()
root.title("pclint V3.0")
root.geometry('300x300')                    # 窗口大小（乘号是 x）
root.resizable(width=False, height=False)   # 窗口大小是否可变

l1 = Label(root, text="检测产品的名字:", width=15, height=2) # 设置显示的属性
l1.pack()                                                   # 显示的位置(上下左右)
v = IntVar()
v.set(3)
product_choise = [(1, "ax2pro_nor"),(2, "rx27pro"), (3, "auto")]
for num, lang in product_choise:
    b = Radiobutton(root, text=lang, variable=v, value=num)
    b.pack(anchor=W)

l2 = Label(root, text="输出格式(xml、txt):")
l2.pack()
sheet_text = StringVar()
sheet = Entry(root, textvariable = sheet_text)
sheet_text.set(" ")
sheet.pack()


l3 = Label(root, text="检测目录名(空：默认路径):")
l3.pack()
loop_text = StringVar()
loop = Entry(root, textvariable = loop_text)
loop_text.set(" ")
loop.pack()


def on_click():
    s = sheet_text.get()
    l = loop_text.get()
    string = str("product_name: output_type:%s path:%s" %(s, l))
    print("product_name: output_type:%s path:%s" %( s, l))
    #messagebox.showinfo(title='aaa', message = string)
    
Button(root, text="确认", command = on_click).pack()
#Button(root, text="确认", command = on_click).grid(row=3, column=0, sticky=W, pady=5)
#Button(root, text="退出", command = on_click).grid(row=3, column=1, sticky=E, pady=5)

root.mainloop()
# pclint 版本号
# 产品型号：ax2pro_nor  rx27pro  auto
# 输出格式：xml  txt
# 运行结果：ERROR WARNING