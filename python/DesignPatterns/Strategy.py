#!/usr/bin/env python

class Context:

    def __init__(self, data, strategy):

        self.__data = data

        if (isinstance(strategy, Strategy)):
            self.strategy = strategy
        else:
            # default strategy
            self.strategy = ConcreteStrategyA

    def ContextInterface(self):
        print("Context: ContextInterface")
        print(self.__data)

    def Go(self):
        self.strategy.AlgorithmInterface(self.ContextInterface)

class Strategy:

    def AlgorithmInterface(self, contextInterface):
        pass

class ConcreteStrategyA(Strategy):

    def __init__(self):
        self.name = "ConcreteStrategyA"

    def AlgorithmInterface(self, contextInterface):
        print("%s: AlgorithmInterface" % self.name)
        contextInterface()

class ConcreteStrategyB(Strategy):

    def __init__(self):
        self.name = "ConcreteStrategyB"

    def AlgorithmInterface(self, contextInterface):
        print("%s: AlgorithmInterface" % self.name)
        contextInterface()

class ConcreteStrategyC(Strategy):

    def __init__(self):
        self.name = "ConcreteStrategyC"

    def AlgorithmInterface(self, contextInterface):
        print("%s: AlgorithmInterface" % self.name)
        contextInterface()

class Client:

    def __init__(self):
        self.context1 = Context("abc", ConcreteStrategyB())
        self.context2 = Context("123", ConcreteStrategyC())

    def Go(self):
        self.context1.Go()
        self.context2.Go()

def main():
    client = Client()
    client.Go()

if __name__ == "__main__":
    main()

