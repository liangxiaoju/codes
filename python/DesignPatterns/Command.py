#!/usr/bin/env python

class Command:

    def Execute(self):
        pass

class ConcreteCommand(Command):

    def __init__(self, receiver):
        self.receiver = receiver
        self.state = None

    def Execute(self):
        self.receiver.Action()

class Receiver:

    def Action(self):
        print(repr(self.Action))

class Invoker:

    def __init__(self, command):
        self.command = command

    def OnClicked(self):
        self.command.Execute()

class Client:

    def __init__(self):
        receiver = Receiver()
        command = ConcreteCommand(receiver)
        self.invoker = Invoker(command)

    def Go(self):
        self.invoker.OnClicked()

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()

