import os
import socket
import threading
import tkinter as tk
from tkinter import *
from tkinter import simpledialog, scrolledtext
from collections import defaultdict
from time import sleep
import sys

HOST = '172.21.30.132'
PORT = 1234

class Client:
  def __init__(self, host, port):
    msg = tk.Tk()
    msg.withdraw()
    self.nickname = simpledialog.askstring("Nickname", "Please enter your nickname", parent=msg)
    self.current_reciver = None
    self.friends_set = set()
    self.incomming_messeges = list()
    self.messeges = defaultdict(lambda: list())
    # self.host = simpledialog.askstring("Server address", "Please enter server address", parent=msg)

    try:
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((host, port))
      message = "1;" + str(self.nickname) + ";\n"
      self.sock.sendall(message.encode("utf-8"))
    except:
      print("Some issue with connection. Your server not responding")
      exit(-1)

    self.gui_done = False
    self.error = False
    self.closing = False
    self.running = True
    # gui_thread = threading.Thread(target=self.gui_loop)
    # gui_thread.start()
    recive_thread = threading.Thread(target=self.recive)
    recive_thread.start()
    if self.error == True:
      exit(-1)
    self.gui_loop()

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
    self.win.iconbitmap("./client/static/mail.ico")
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
    j=0
    for i in self.friends_set:
      self.users_area.insert(j, i)
      j+=1
    self.users_area.config(width=20, height=20)
    self.users_area.pack(side=TOP, padx=5, pady=5)

    self.add_user = tk.Text(self.framenav)
    self.add_user.config(width=15, height=2)
    self.add_user.pack(side=TOP, padx=5, pady=5)

    self.upadate_user_btn = tk.Button(self.framenav, text="Update Friends", command=self.update_friends)
    self.upadate_user_btn.config(font=("Arial", 12), width=15, height=1)
    self.upadate_user_btn.pack(side=BOTTOM, padx=1, pady=5)

    self.add_user_btn = tk.Button(self.framenav, text="Add Friend", command=self.add_friend)
    self.add_user_btn.config(font=("Arial", 12), width=15, height=1)
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

  # The on_closing method is called when the user attempts to close the window. It sets the running attribute to false,
  # sends a message "9;0;0;" to the server, and then closes the socket, destroys the window, and exits the application.
  def on_closing(self):
    self.running = False
    if self.closing:
      message = "9;;\n"
      self.sock.sendall(message.encode("utf-8"))
    sleep(1)
    self.sock.close()
    self.win.destroy()
    exit(0)

  # The add_friend method is called when the user attempts to add a friend. It takes the input from the "add_user" text field,
  # removes any newline or whitespace characters, and checks to ensure that the input is not empty.
  # If it is not, it adds the new friend to the "friends_set" set and sends a message "4;{newFriend};" to the server.
  def add_friend(self):
    i = len(self.friends_set) + 1
    newFriend = self.add_user.get('1.0', 'end')
    self.add_user.delete('1.0', 'end')
    newFriend = newFriend.replace('\n', '')
    newFriend = newFriend.replace(' ', '')
    if len(newFriend) > 0:
      if newFriend not in self.friends_set:
        self.users_area.insert(i, newFriend)
        message = '4;' + newFriend + ';' + "\n"
        self.sock.sendall(message.encode("utf-8"))

  # he write method is called when the user clicks the "Send" button in the chat interface.
  # It checks to ensure that the user has selected a recipient before allowing them to send a message.
  def write(self):
    if self.current_reciver == None:
      self.add_message("It's no possible to write message to nobody")
    else:
      msg = str(self.input_msg.get('1.0', 'end').replace('\n', ''))
      message = '2;' + str(self.current_reciver) + ';' + msg + "\n"
      self.sock.sendall(message.encode("utf-8"))
      self.messeges[self.current_reciver].append(self.nickname + ": " + msg)
      self.input_msg.delete('1.0', 'end')
      self.add_message(self.nickname + ": " + msg)

  # The writeOnEnter method is similar to the write method, but is called when the user presses
  # the enter key in the "input_msg" text field.
  def writeOnEnter(self, arg):
    if self.current_reciver == None:
      self.add_message("It's no possible to write message to nobody")
    else:
      msg = str(self.input_msg.get('1.0', 'end').replace('\n', ''))
      message = '2;' + str(self.current_reciver) + ';' + msg + "\n"
      self.sock.sendall(message.encode("utf-8"))
      self.messeges[self.current_reciver].append(self.nickname + ": " + msg)
      self.input_msg.delete('1.0', 'end')
      self.add_message(self.nickname + ": " + msg)

  # The print_message method is called when the user selects a different chat recipient.
  # It clears the chat history display and then repopulates it with the chat history of the newly selected recipient.
  def print_message(self):
    self.text_area.config(state="normal")
    self.text_area.delete("1.0", tk.END)
    self.text_area.config(state='disabled')

    for i in self.messeges[self.current_reciver]:
      self.text_area.config(state="normal")
      self.text_area.insert('end', i + '\n')
      self.text_area.yview('end')
      self.text_area.config(state='disabled')
    self.text_area.config(state="normal")
    self.text_area.insert('end', '\n')
    self.text_area.yview('end')
    self.text_area.config(state='disabled')

  # The add_message method is called to add a new message to the chat history display.
  def add_message(self, mess):
    self.text_area.config(state="normal")
    self.text_area.insert('end', mess + '\n')
    self.text_area.yview('end')
    self.text_area.config(state='disabled')

  # The recive_message method is called to parse incoming messages from the server and return them as a list of tuples.
  def recive_message(self, message):
    single_messages = message.split('\n')
    data = list()
    for message_string in single_messages:
      data.append(tuple(message_string.split(';')))
    return data

  # The rosponse_handler method is called to handle the different types of messages sent by the server.
  # It takes the first element of the tuple and uses it to determine the type of message, and then performs the appropriate action.
  def rosponse_handler(self, message):
    if message[0] == '1':
      print("Set connection with server")
      self.update_friends()
    elif message[0] == '2':
      self.messeges[message[1]].append("\t" + message[1] + ": " + message[2])
      if message[1] == self.current_reciver:
        self.add_message("\t" + message[1] + ": " + message[2])
    elif message[0] == '4':
      self.friends_set.clear()
      for i in message[2].split(':'):
        self.friends_set.add(i)
      self.users_area.delete(0,tk.END)
      i=0
      if len(self.friends_set)>0:
        for j in self.friends_set:
          self.users_area.insert(i,j)
          i+=1
    elif message[0] == '9' and message[1] == 'SYS':
      print(message[2])
      os._exit(-1)



  # The recive method is called in a loop to continuously listen for incoming messages from the server.
  def recive(self):
    while self.running:
      try:
        buffer = ""
        
        buffer+=self.sock.recv(1).decode("utf-8")
        while buffer[-1] != "\n":
          buffer+=self.sock.recv(1).decode("utf-8")

        print(buffer + "\n")
        
        print("DONE IT")


        if buffer != "":
          for element in self.recive_message(buffer):
            print(element)
            self.rosponse_handler(element)
      except:
        pass
    else:
      return

  # The update_friends method is called to request an update of the friends list from the server.
  def update_friends(self):
    message = "4;;\n"
    self.sock.sendall(message.encode("utf-8"))

if __name__ == "__main__":
  client = Client(HOST, PORT)
