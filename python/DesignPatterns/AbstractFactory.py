#!/usr/bin/env python

class AbstractFactory:

    def CreateProductA(self):
        pass

    def CreateProductB(self):
        pass

class ConcreteFactory1(AbstractFactory):

    def CreateProductA(self):
        return ProductA1()

    def CreateProductB(self):
        return ProductB1()

class ConcreteFactory2(AbstractFactory):

    def CreateProductA(self):
        return ProductA2()

    def CreateProductB(self):
        return ProductB2()

class AbstractProductA:
    pass

class ProductA1(AbstractProductA):
    pass

class ProductA2(AbstractProductA):
    pass

class AbstractProductB:
    pass

class ProductB1(AbstractProductB):
    pass

class ProductB2(AbstractProductB):
    pass

class Client:

    def __init__(self):
        #factory = ConcreteFactory1()
        factory = ConcreteFactory2()
        self.productA = factory.CreateProductA()
        self.productB = factory.CreateProductB()

    def Go(self):
        print(self.productA)
        print(self.productB)

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()
