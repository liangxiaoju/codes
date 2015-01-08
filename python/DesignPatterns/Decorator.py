#!/usr/bin/env python

class Component(object):

    def Operation(self):
        pass

class ConcreteComponent(Component):

    def __init__(self, data):
        self.data = data

    def Operation(self):
        print("ConcreteComponent:Operation " + self.data)

class Decorator(Component):

    def __init__(self, component):
        self.component = component

    def Operation(self):
        print("Decorator:Operation")
        self.component.Operation()

class ConcreteDecoratorA(Decorator):

    def __init__(self, component):
        super(ConcreteDecoratorA, self).__init__(component)
        self.addedState = False

    def Operation(self):
        print("ConcreteDecoratorA:Operation")
        super(ConcreteDecoratorA, self).Operation()
        self.addedState = True

class ConcreteDecoratorB(Decorator):

    def __init__(self, component):
        super(ConcreteDecoratorB, self).__init__(component)

    def Operation(self):
        print("ConcreteDecoratorB:Operation")
        super(ConcreteDecoratorB, self).Operation()
        self.AddedBehavior()

    def AddedBehavior(self):
        print("ConcreteDecoratorB:AddedBehavior " + repr(self.component))

class Client:

    def __init__(self):
        component = ConcreteComponent("abc")
        # use ConcreteDecoratorB to decorate component
        component = ConcreteDecoratorB(component)
        # use ConcreteDecoratorA to decorate component
        component = ConcreteDecoratorA(component)

        self.component = component

    def Go(self):
        self.component.Operation()

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()

