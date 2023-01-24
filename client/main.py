import socket
import threading
import tkinter as tk
from tkinter import *
from tkinter import simpledialog, scrolledtext

HOST = '192.168.55.105'
PORT = 1234


class Client:
  def __init__(self, host, port):
    msg = tk.Tk()
    msg.withdraw()
    self.nickname = simpledialog.askstring("Nickname", "Please enter your nickname", parent=msg)
    self.current_reciver = None
    self.friends_array = ['Jan', 'Dzban', "Wan", 'Pan', 'Chłam']
    # self.host = simpledialog.askstring("Server address", "Please enter server address", parent=msg)
    try:
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((host, port))
    except:
      print("Some issue with connection")
    self.gui_done = False

    self.running = True
    gui_thread = threading.Thread(target=self.gui_loop)
    recive_thread = threading.Thread(target=self.recive)

    gui_thread.start()
    recive_thread.start()



  def gui_loop(self):
    self.win = tk.Tk()
    self.win.iconbitmap("static/mail.ico")
    self.win.title('BSD Socket based communicator')
    self.win.configure()

    self.frametop = Frame(self.win)
    self.framebottom = Frame(self.win)
    self.framenav = Frame(self.framebottom)
    self.frameright = Frame(self.framebottom)
    self.frameright_msg = Frame(self.frameright)
    self.frameright_input = Frame(self.frameright)

    # main label
    self.win_label = tk.Label(self.frametop, text="Name: {}".format(self.nickname))
    self.win_label.config(font=("Arial", 12))
    self.win_label.pack(side=RIGHT, padx=10, pady=10)
    self.frametop.pack(side=TOP, fill=BOTH, expand=1, padx=10, pady=10)

    def callback(event):
      selection = event.widget.curselection()
      if selection:
        index = selection[0]
        data = event.widget.get(index)
        print("You selected ", data)
        self.current_reciver = data


    # nav pane
    self.users_area = Listbox(self.framenav, selectbackground="lightblue")
    self.users_area.bind("<<ListboxSelect>>", callback)
    for i in range(len(self.friends_array)):
      self.users_area.insert(i, (self.friends_array[i]))

    self.users_area.pack(side=LEFT, fill=BOTH, expand=1, padx=5, pady=5)

    self.framenav.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)

    # chat pane

    self.text_area = scrolledtext.ScrolledText(self.frameright)
    self.text_area.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)
    self.text_area.config(state='disable')

    self.frameright.pack(side=TOP, fill=BOTH, expand=1, padx=5, pady=5)
    # input message pane
    self.sending_frame = tk.Frame(self.frameright)

    self.input_msg = tk.Text(self.sending_frame, height=2)
    self.input_msg.pack(side=LEFT, fill=BOTH, expand=1, padx=5, pady=5)
    self.win.bind('<Return>', self.write)
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
    self.win.protocol("WM_DELETE_WINDOW", self.stop)
    self.win.mainloop()

  def stop(self):
    self.running = False
    self.win.destroy()
    message = "wyjeb mnie"
    self.sock.send(message.encode("utf-8"))
    self.sock.close()
    print("User exit system")
    exit(0)



  def test(self):
    print("wyslano od {}".format(self.nickname))

  def write(self):
    # message = str(self.nickname)+'\n'+str(self.current_reciver)+'\n'+str(self.input_msg.get('1.0', 'end'))
    message = str(self.nickname)+':'+str(self.current_reciver)+':'+str(self.input_msg.get('1.0', 'end'))
    self.sock.send(message.encode("utf-8"))
    print(message)
    self.input_msg.delete('1.0', 'end')

  def recive(self):
    while self.running:
      try:
        message = self.sock.recv(512).decode('utf-8')
        if self.gui_done:
          self.text_area.config(state="normal")
          self.text_area.insert('end', message)
          self.text_area.yview('end')
          self.text_area.config(state='disabled')
      except ConnectionAbortedError:
        break
      except:
        print("error")
        self.sock.close()
        break


client = Client(HOST, PORT)
