#!/usr/bin/env python

class Component:

    def Operation(self):
        pass

    def Add(self, component):
        pass

    def Remove(self, component):
        pass

    def GetChild(self, i):
        pass

class Leaf(Component):

    def __init__(self, name):
        self.name = name

    def Operation(self):
        print("Leaf(%s):Operation" % self.name)

class Composite(Component):

    def __init__(self, name):
        self.name = name
        self.__components = []

    def Operation(self):
        print("Composite(%s):Operation" % self.name)
        for component in self.__components:
            component.Operation()

    def Add(self, component):
        if (isinstance(component, Component)):
            self.__components.append(component)

    def Remove(self, component):
        if (isinstance(component, Component)):
            self.__components.remove(component)

    def GetChild(self, i):
        return self.__components[i]

class Client:

    def __init__(self):
        composite_0 = Composite("root")
        leaf_10 = Leaf("10")
        composite_11 = Composite("11")
        leaf_12 = Leaf("12")
        leaf_20 = Leaf("20")
        leaf_21 = Leaf("21")

        composite_0.Add(leaf_10)
        composite_0.Add(composite_11)
        composite_0.Add(leaf_12)
        composite_11.Add(leaf_20)
        composite_11.Add(leaf_21)

        self.component = composite_0

    def Go(self):
        self.component.Operation()
        child = self.component.GetChild(0)
        child.Operation()

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()

