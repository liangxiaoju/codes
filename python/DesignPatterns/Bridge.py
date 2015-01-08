#!/usr/bin/env python

class Abstraction:

    def Operation(self):
        pass

class RefinedAbstraction(Abstraction):

    def __init__(self, implementor):
        self.implementor = implementor

    def Operation(self):
        self.implementor.OperationImp()

class Implementor:

    def OperationImp(self):
        pass

class ConcreteImplementorA(Implementor):

    def OperationImp(self):
        print(repr(self.OperationImp))

class ConcreteImplementorB(Implementor):

    def OperationImp(self):
        print(repr(self.OperationImp))

class Client:

    def __init__(self):
        implementor = ConcreteImplementorA()
        #implementor = ConcreteImplementorB()
        self.abstraction = RefinedAbstraction(implementor)

    def Go(self):
        self.abstraction.Operation()

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()
