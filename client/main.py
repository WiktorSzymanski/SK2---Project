import socket
import threading
import tkinter as tk
from tkinter import *
from tkinter import simpledialog, scrolledtext, messagebox
from collections import defaultdict

HOST = '192.168.55.106'
PORT = 1234


class Client:
  def __init__(self, host, port):
    msg = tk.Tk()
    msg.withdraw()
    self.nickname = simpledialog.askstring("Nickname", "Please enter your nickname", parent=msg)
    self.current_reciver = None
    self.friends_set= set()
    self.incomming_messeges = list()
    self.messeges = defaultdict(lambda: list())
    # self.host = simpledialog.askstring("Server address", "Please enter server address", parent=msg)

    try:
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((host, port))
      message = "1;" + str(self.nickname) + ";0;"
      self.sock.send(message.encode("utf-8"))
    except:
      print("Some issue with connection")

    self.gui_done = False

    self.closing = False
    self.running = True
    gui_thread = threading.Thread(target=self.gui_loop)
    recive_thread = threading.Thread(target=self.recive)

    gui_thread.start()
    recive_thread.start()

  def callback(self, event):
      selection = event.widget.curselection()
      if selection:
        index = selection[0]
        data = event.widget.get(index)
        # print("You selected ", data.replace('\n',''))
        self.current_reciver = data
        self.print_message()

  def gui_loop(self):
    self.win = tk.Tk()
    self.win.iconbitmap("static/mail.ico")
    self.win.title('BSD Socket based communicator')
    self.win.configure()

    self.frametop = Frame(self.win)
    self.framebottom = Frame(self.win)
    self.framenav = Frame(self.framebottom, width=100, height=250)
    self.frameright = Frame(self.framebottom)
    self.frameright_msg = Frame(self.frameright)
    self.frameright_input = Frame(self.frameright)

    # main label
    self.win_label = tk.Label(self.frametop, text="Name: {}".format(self.nickname))
    self.win_label.config(font=("Arial", 12))
    self.win_label.pack(side=RIGHT, padx=10, pady=10)
    self.frametop.pack(side=TOP, fill=BOTH, expand=1, padx=10, pady=10)



    # nav pane

    self.users_area = Listbox(self.framenav, selectbackground="lightblue")
    self.users_area.bind("<<ListboxSelect>>", self.callback)
    for i in range(len(self.friends_set)):
      self.users_area.insert(i, (self.friends_set[i]))
    self.users_area.config( width = 20, height = 20)
    self.users_area.pack(side=TOP, padx=5, pady=5)


    self.add_user = tk.Text(self.framenav)
    self.add_user.config( width = 15, height = 2)
    self.add_user.pack(side=TOP, padx=5, pady=5)

    self.upadate_user_btn = tk.Button(self.framenav, text="Update Friends", command=self.update_friends)
    self.upadate_user_btn.config(font=("Arial", 12), width=15, height=1)
    self.upadate_user_btn.pack(side=BOTTOM, padx=1, pady=5)

    self.add_user_btn = tk.Button(self.framenav, text="Add Friend", command=self.add_friend)
    self.add_user_btn.config(font=("Arial", 12),width = 15, height = 1 )
    self.add_user_btn.pack(side=BOTTOM, padx=1, pady=5)

    self.framenav.pack(side=TOP, padx=5, pady=5)

    # Create chat pane

    self.text_area = scrolledtext.ScrolledText(self.frameright)
    self.text_area.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)
    self.text_area.config(state='disable')

    self.frameright.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)
    # input message pane
    self.sending_frame = tk.Frame(self.frameright)

    self.input_msg = tk.Text(self.sending_frame, height=2)
    self.input_msg.pack(side=LEFT, fill=BOTH, expand=1, padx=5, pady=5)
    self.win.bind('<Return>', self.writeOnEnter)
    self.input_btn = tk.Button(self.sending_frame, text=">", command=self.write)
    self.input_btn.config(font=("Arial", 16))
    self.input_btn.pack(side=RIGHT, fill=BOTH, expand=1, padx=5, pady=5)

    self.sending_frame.pack(side=TOP, fill=BOTH, expand=1)

    self.frametop.pack(side=TOP, fill=BOTH, expand=1)
    self.framebottom.pack(side=BOTTOM, fill=BOTH, expand=1)
    self.framenav.pack(side=LEFT, fill=BOTH, expand=1)
    self.frameright.pack(side=RIGHT, fill=BOTH, expand=1)
    self.frameright_msg.pack(side=TOP, fill=BOTH, expand=1)
    self.frameright_input.pack(side=BOTTOM, fill=BOTH, expand=1)



    self.gui_done = True
    self.win.protocol("WM_DELETE_WINDOW", self.on_closing)
    if not self.closing:
      self.closing = True

    self.win.mainloop()

  def on_closing(self):
    self.running = False
    if self.closing:
      message = "9;0;0;"
      self.sock.send(message.encode("utf-8"))
    self.sock.close()
    self.win.destroy()
    exit(0)

  def add_friend(self):
    i = len(self.friends_set) +1
    newFriend = self.add_user.get('1.0', 'end')
    self.add_user.delete('1.0', 'end')

    newFriend = newFriend.replace('\n','')
    newFriend = newFriend.replace(' ','')
    # print(newFriend)
    if len(newFriend) > 0:
      if newFriend not in self.friends_set:
        self.users_area.insert(i, newFriend)
        message = '4;' + newFriend + ';'+ ";"
        self.sock.send(message.encode("utf-8"))
      self.friends_set.add(newFriend)

  def write(self):
    msg =  str(self.input_msg.get('1.0', 'end').replace('\n', ''))
    message = '2;' + str(self.current_reciver) + ';' + msg + ";"
    self.sock.send(message.encode("utf-8"))
    self.messeges[self.current_reciver].append(self.nickname + ": " + msg)
    self.input_msg.delete('1.0', 'end')
    self.add_message(self.nickname + ": " + msg)

  def writeOnEnter(self, arg):
    msg = str(self.input_msg.get('1.0', 'end').replace('\n', ''))
    message = '2;' + str(self.current_reciver) + ';' + msg + ";"
    self.sock.send(message.encode("utf-8"))
    self.messeges[self.current_reciver].append(self.nickname + ": " + msg)
    self.input_msg.delete('1.0', 'end')
    self.add_message(self.nickname + ": " + msg)


  def print_message(self):
    self.text_area.config(state="normal")
    self.text_area.delete("1.0", tk.END)
    self.text_area.config(state='disabled')

    for i in self.messeges[self.current_reciver]:
      self.text_area.config(state="normal")
      self.text_area.insert('end', "\n" + i)
      self.text_area.yview('end')
      self.text_area.config(state='disabled')

  def add_message(self, mess):
    self.text_area.config(state="normal")
    self.text_area.insert('end', "\n" + mess)
    self.text_area.yview('end')
    self.text_area.config(state='disabled')


  def recive(self):
    while self.running:
      try:
        message = self.sock.recv(512).decode('utf-8')

        if message:
          print("Dostalem cos z servera: " + message)
          mes_arr = message.split(';')
          if mes_arr[0] == '1':
            print("Set connection with server")
          elif mes_arr[0] == '2':
            self.messeges[mes_arr[1]].append(mes_arr[1] + ": " + mes_arr[2])
            if mes_arr[1] == self.current_reciver:
              self.add_message(mes_arr[1] + ": " + mes_arr[2])
          elif mes_arr[0] == '3':
            self.friends_set.clear()
            for i in mes_arr[2].split(':'):
              self.friends_set.add(i)

      except:
        pass
    else:
      return
  def update_friends(self):
    message = "3;.;.;"
    self.sock.send(message.encode("utf-8"))
    print(message)


if __name__ == "__main__":
  client = Client(HOST, PORT)
