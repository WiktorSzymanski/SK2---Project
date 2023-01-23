import socket
import threading
import tkinter as tk
from tkinter import *
from tkinter import simpledialog, scrolledtext

HOST = '192.168.055.108'
PORT = 1234


class Client:
  def __init__(self, host, port):
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.sock.connect((host, port))
    msg = tk.Tk()
    msg.withdraw()
    self.nickname = simpledialog.askstring("Nickname", "Please enter your nickname", parent=msg)

    self.gui_done = False

    self.running = True

    gui_thread = threading.Thread(target=self.gui_loop())
    # recive_thread = threading.Thread(targer = recive)

    gui_thread.start()
    # recive_thread.start()

  def gui_loop(self):
    self.win = tk.Tk()
    self.win.iconbitmap("./static/mail.ico")
    self.win.title('BSD Socket based communicator')
    self.win.configure()

    self.frametop = Frame(self.win)
    self.framebottom = Frame(self.win)
    self.framenav = Frame(self.framebottom)
    self.frameright = Frame(self.framebottom)
    self.frameright_msg = Frame(self.frameright)
    self.frameright_input = Frame(self.frameright)


    #main label
    self.win_label = tk.Label(self.frametop, text="Name: {}".format(self.nickname))
    self.win_label.config(font=("Arial", 12))
    self.win_label.pack(side = RIGHT, padx=10, pady=10)
    self.frametop.pack(side=TOP, fill=BOTH, expand=1, padx=10, pady=10)

    #nav pane
    self.users_area = Listbox(self.framenav,selectbackground="lightblue")
    for i in range(10):
      self.users_area.insert(i,"User {}".format(str(i)))

    self.users_area.pack(side=LEFT, fill=BOTH, expand=1, padx=5, pady=5)

    self.framenav.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)

    #chat pane
    self.text_area = scrolledtext.ScrolledText(self.frameright)
    self.text_area.pack(side=TOP, fill=BOTH, expand=1,padx=5, pady=5)
    self.text_area.config(state='disable')

    self.frameright.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)
    #input message pane
    self.sending_frame = tk.Frame(self.frameright)

    self.input_msg = tk.Text(self.sending_frame, height=2)
    self.input_msg.pack(side=LEFT, fill=BOTH, expand=1, padx=5, pady=5)

    self.input_btn = tk.Button(self.sending_frame, text=">", command=self.test)
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
    self.win.protocol("WM_DELETE_WINDOW", self.stop)
    self.win.mainloop()

  def stop(self):
    self.running = False
    self.win.destroy()
    # self.sock.close()
    print("User exit system")
    exit(0)

  # def recive(self):
  #   pass
  #
  # def send(self):
  #   pass
  def test(self):
    print("wyslano od {}".format(self.nickname))
    self.input_msg.delete('1.0', 'end')


client = Client(HOST, PORT)
