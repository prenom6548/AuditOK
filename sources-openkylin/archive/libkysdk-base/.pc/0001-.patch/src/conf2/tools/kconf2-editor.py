#!/usr/bin/python3
import os
import sys
import yaml
import logging
sys.path.append('/home/kylin/disk/kylinos-src/libkysdk-base/src/conf2/service')
import conf2Utils
import tkinter as tk
import tkinter.ttk as ttk
import tkinter.messagebox as msgbox
import tkinter.filedialog as filedialog
import dbus
from gi.repository import GLib

target_view = {}

def on_tree_right_click(event):
    global right_select_item
    item = tree.identify_row(event.y)
    if item and tree.parent(item) == "":
        right_select_item = item
        download_menu.post(event.x_root, event.y_root)

def close_menu(event):
    download_menu.unpost()

def on_search():
    pack_list = root.pack_slaves()
    if view_frame in pack_list:
        view_frame.pack_forget()
        search_frame.pack(side='bottom', fill='both', expand=True)
        search_tree.delete(*search_tree.get_children())
        search_entry.config(state='normal')
        # search_entry.event_generate("<KeyRelease>")
    else:
        search_frame.pack_forget()
        view_frame.pack(side='bottom', fill='both', expand=True)
        items = tree.selection()
        if items:
            text = tree.item(items[0], 'tags')[0]
            search_entry.delete(0, 'end')
            search_entry.insert(0, text)
        else:
            search_entry.delete(0, 'end')
        search_entry.config(state='readonly')

def on_search_entry_change(event):
    new_text = event.widget.get()
    search_tree.delete(*search_tree.get_children())
    if new_text == '':
        return
    for key in search_list.keys():
        name_list = key.split('/')
        if new_text in name_list[-1] or key.startswith(new_text):
            search_tree.insert('', 'end', text= key)

def on_download():
    global target_view
    path = filedialog.askdirectory(mustexist=True, title=' Choose a folder')
    if path:
        success = converter.editor_save(target_view, path)
        if success:
            msgbox.showinfo(message='Save successed')
        else:
            msgbox.showinfo(message='Save failed')

def on_download_one():
    global target_view
    key = tree.item(right_select_item, 'text')
    path = filedialog.askdirectory(mustexist=True, title=' Choose a folder')
    if path:
        success = converter.editor_save({key:target_view[key]}, path)
        if success:
            msgbox.showinfo(message='Save successed')
        else:
            msgbox.showinfo(message='Save failed')

def on_import_file():
    pass

def on_set():
    try:
        if combobox.get() != 'custom':
            msgbox.showinfo(message='Not custom veiw')

        user = destination_file.split('/')[2]

        key = id = value = ''
        for widget in inner_frame.winfo_children():
            if isinstance(widget, tk.Text):
                if 'key' in widget.winfo_name:
                    key = widget.get()
                if 'id' in widget.winfo_name:
                    id = widget.get()
                if 'custom' in widget.winfo_name:
                    value = widget.get()

        path  = search_entry.get()
        version = path.split('/')[1]
        print(f'{user} {id} {version} {key} {value}')

        bus = dbus.SystemBus()
        obj = bus.get_object('com.kylin.kysdk.conf2', '/com/kylin/kysdk/conf2')
        interface = dbus.Interface(obj, 'com.kylin.kysdk.conf2')
        # success = interface.set(user, id, version, key, value)
        # if success:
        #     global target_view
        #     target_view = converter._db2dict(f'{destination_file}/.config/kylin-config/user.db')
        #     msgbox.showinfo(message='Set successed')
        # else:
        #     msgbox.showinfo(message='Set failed')
    except Exception as e:
        print(e)

def on_search_tree_select(event):
    items = search_tree.selection()
    text = search_tree.item(items[0], 'text')
    tree.selection_set(search_list[text])
    tree.see(search_list[text])
    search_frame.pack_forget()
    view_frame.pack(side='bottom', fill='both', expand=True)

def on_tree_select(event):
    selected_item = tree.selection()
    global target_view
    if not selected_item:
        return
    item = selected_item[0]
    tag = tree.item(item, 'tags')[0]

    search_entry.config(state='normal')
    search_entry.delete(0, 'end')
    search_entry.insert(0, tag)
    search_entry.config(state='readonly')

    if tree.parent(item) == '':
        return
    
    group_list = tag.split('/')
    tmp = target_view
    for group in group_list:
        tmp = tmp[group]
    for widget in inner_frame.winfo_children():
        widget.destroy()

    text = tree.item(item, 'text')
    
    name_label = tk.Label(inner_frame, text=f'名称', anchor='w')
    name_label.pack(side='top', fill='x')

    name_text = tk.Text(inner_frame, height=1, width=100, wrap='word', name='key')
    name_text.insert('1.0', text)
    name_text.config(state='disabled')
    name_text.pack(side='top', fill='x', expand=True)
    
    type = tmp['_type'] if '_type' in tmp else 'group'

    if len(group_list) > 1:
        group_list.pop(1)
    if type != 'group':
        group_list.pop(-1)

    id_label = tk.Label(inner_frame, text=f'路径', anchor='w')
    id_label.pack(side='top', fill='x')

    id_text = tk.Text(inner_frame, height=1, width=100, wrap='word', name='id')
    id_text.insert('1.0', '.'.join(group_list))
    id_text.config(state='disabled')
    id_text.pack(side='top', fill='x', expand=True)

    type_label = tk.Label(inner_frame, text=f'类型', anchor='w')
    type_label.pack(side='top', fill='x')

    type_text = tk.Text(inner_frame, height=1, width=100, wrap='word', name='type')
    type_text.insert('1.0', type)
    type_text.config(state='disabled')
    type_text.pack(side='top', fill='x', expand=True)

    if tmp.get('_summary') is not None:
        summary_label = tk.Label(inner_frame, text=f'摘要', anchor='w')
        summary_label.pack(side='top', fill='x')

        summary_text = tk.Text(inner_frame, height=3, width=100, wrap='word')
        summary_text.insert('1.0', tmp.get('_summary'))
        summary_text.config(state='disabled')
        summary_text.pack(side='top', fill='x', expand=True)

    if tmp.get('_description') is not None:
        decription_label = tk.Label(inner_frame, text=f'描述', anchor='w')
        decription_label.pack(side='top', fill='x')

        decription_text = tk.Text(inner_frame, height=3, width=100, wrap='word')
        decription_text.insert('1.0', tmp.get('_description'))
        decription_text.config(state='disabled')
        decription_text.pack(side='top', fill='x', expand=True)

    permission = tmp.get('_permission', 'public')
    permission_label = tk.Label(inner_frame, text=f'权限', anchor='w')
    permission_label.pack(side='top', fill='x')

    permission_text = tk.Text(inner_frame, height=1, width=100, wrap='word')
    permission_text.insert('1.0', permission)
    permission_text.config(state='disabled')
    permission_text.pack(side='top', fill='x', expand=True)

    if '_default' in tmp:
        default_value = tmp['_default']
        if isinstance(default_value, bool):
            default_value = 'true' if tmp['_default'] else 'false'

        default_label = tk.Label(inner_frame, text=f'默认值', anchor='w')
        default_label.pack(side='top', fill='x')

        default_text = tk.Text(inner_frame, height=5, width=100, wrap='word', name='default')
        default_text.insert('1.0', tmp['_default'])
        default_text.config(state='disabled')
        default_text.pack(side='top', fill='x', expand=True)

        custom_value = tmp.get('_value')
        if custom_value is None or custom_value == default_value:
            custom_value = '默认值'
        custom_label = tk.Label(inner_frame, text=f'当前值', anchor='w')
        custom_label.pack(side='top', fill='x')

        custom_text = tk.Text(inner_frame, height=5, width=100, wrap='word', name='custom')
        custom_text.insert('1.0', custom_value)
        custom_text.config(state='disabled')
        custom_text.pack(side='top', fill='x', expand=True)
    
    if type != 'group' and permission == 'public' and combobox.get() == 'custom':
        custom_text.config(state='normal')
        set_button = tk.Button(inner_frame, text='Set', command=on_set)
        set_button.pack(side='top')

def on_combobox_select(event):
    selected_item = combobox.get()
    global target_view
    if selected_item == 'custom':
        target_view = converter._db2dict(f'{destination_file}/.config/kylin-config/user.db')
    else:
        dirs = []
        for dir in list(reversed(options)):
            if dir == 'user':
                dirs.append(f'{destination_file}/.config/kylin-config/configs')
            else:
                dirs.append(f'/etc/kylin-config/{dir}')
            if dir == selected_item:
                break
        target_view = converter.load_dirs_config(dirs)

    tree.delete(*tree.get_children())
    for widget in inner_frame.winfo_children():
        widget.destroy()
    search_list.clear()
    populate_tree(tree, '', target_view)
    view_frame.update()

def populate_tree(tree, node, dictionary):
    global search_list
    tag = tree.item(node, 'tags')
    for key, value in dictionary.items():
        if isinstance(value, dict):
            child_node = tree.insert(node, 'end', text=key)
            if isinstance(tag, str):
                tree.item(child_node, tags=(key))
            else:
                tree.item(child_node, tags=(f'{tag[0]}/{key}'))
                search_list[f'{tag[0]}/{key}'] = child_node
            populate_tree(tree, child_node, value)

root = tk.Tk()
root.title("kconf2-editor")
root.geometry('1024x768')
root.bind("<Button-1>", close_menu)

download_menu = tk.Menu(root, tearoff=0)
download_menu.add_command(label="下载", command=on_download_one)
download_menu.add_command(label="导入到其他目录", command=on_import_file)

search_list = {}

destination_file = os.getenv('HOME')
logger = logging.getLogger(f'{destination_file}/.log/widget.log')
converter = conf2Utils.Converter(logger)

# 读取统一视图
target_view = converter._db2dict(f'{destination_file}/.config/kylin-config/user.db')

# 创建一个Frame来包含按钮，使其一直位于界面上方
button_frame = tk.Frame(root, height=10)
# 创建一个Frame来包含Treeview，占满界面其他空余部分
view_frame = tk.Frame(root)
# 创建一个Frame来显示搜索结果，它与view_frame只会显示一个
search_frame = tk.Frame(root)

# 排列root的三个子组件
button_frame.pack(side="top", fill="x")
view_frame.pack(side='top', fill='both', expand=True)

# 搜索栏
search_entry = tk.Entry(button_frame, state='readonly')
search_entry.bind("<KeyRelease>", on_search_entry_change)

# 搜索按钮
# search_icon = tk.BitmapImage(data=search_icon_data)
search_button = tk.Button(button_frame, text= 's', command=on_search, width=20)

# 下载按钮
# download_icon = tk.PhotoImage(file="download_icon.png")
download_button = tk.Button(button_frame, text= 'd', command=on_download, width=20)

# 视图选择列表
with open('/etc/kylin-config/conf2.yaml', 'r') as file:
    configures = yaml.safe_load(file)
options = configures['dirs']
if os.path.exists(f'{destination_file}/.config/kylin-config/configs'):
    options.append('user')
options.append('custom')
options.reverse()

combobox = ttk.Combobox(button_frame, values=options, width=20, state='readonly')
combobox.set(options[0])
combobox.bind("<<ComboboxSelected>>", on_combobox_select)

# 排列button_frame子组件
search_entry.pack(side="left", fill="both", expand= True)
combobox.pack(side='right')
download_button.pack(side="right", fill="x")
search_button.pack(side="right", fill="x")

# 创建Frame在view_frame的左侧，显示配置树
left_frame = tk.Frame(view_frame)
left_frame.pack(side='left', fill='both')

# 创建Treeview，显示配置树
tree = ttk.Treeview(left_frame)
tree.column('#0', width=200, stretch=True)
tree.heading('#0', text='SubFolders')
tree.bind("<<TreeviewSelect>>", on_tree_select)
tree.bind("<Button-3>", on_tree_right_click)
search_list.clear()
populate_tree(tree, '', target_view)
tree.pack(side='left', fill='both')

# 创建滚动条
tree_scrollbar = ttk.Scrollbar(left_frame, orient="vertical", command=tree.yview)
tree_scrollbar.pack(side="right", fill="y")
tree.configure(yscrollcommand=tree_scrollbar.set)

# 创建Frame在view_frame的右侧，显示配置信息
right_frame = tk.Frame(view_frame)
right_frame.pack(side='right', fill='both', expand=True)

# 创建一个Canvas
canvas = tk.Canvas(right_frame)
canvas.pack(side="left", fill="both", expand=True)

# 将Canvas的窗口设置为Frame
inner_frame = tk.Frame(canvas)
inner_frame.grid(row=0, column=0, sticky="nsew")
canvas_window = canvas.create_window((0, 0), window=inner_frame, anchor="nw")

right_scrollbar = ttk.Scrollbar(right_frame, orient="vertical", command=canvas.yview)
right_scrollbar.pack(side="right", fill="y")
canvas.configure(yscrollcommand=right_scrollbar.set)

# 创建Treeview显示搜索匹配的路径
search_tree = ttk.Treeview(search_frame, show='tree')
search_tree.bind("<<TreeviewSelect>>", on_search_tree_select)
search_tree.pack(side='left', fill='both', expand=True)

search_scrollbar = ttk.Scrollbar(search_frame, orient="vertical", command=search_tree.yview)
search_scrollbar.pack(side="right", fill="y")
search_tree.configure(yscrollcommand=search_scrollbar.set)

root.mainloop()