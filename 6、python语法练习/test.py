import tkinter

root = tkinter.Tk()

tkinter.Label(root, text="账号：").grid(row=0, column=0)
tkinter.Label(root, text="账号：").grid(row=1, column=0)

tkinter.Entry(root).grid(row=0, column=1, padx=10, pady=5)
tkinter.Entry(root).grid(row=1, column=1, padx=10, pady=5)

tkinter.mainloop()