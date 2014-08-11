#!/usr/bin/env python

import sys

class Person(int):
    def getId(self):
        return self

class Circle:
    def __init__(self, num):
        self._persons = list()
        for i in range(num):
            self._add(Person(i+1))

    def _add(self, person):
        self._persons.append(person)

    def _del(self, person):
        self._persons.remove(person)
        print("Remove person:%d" % person.getId())

    def countOff(self):
        count = 0
        round = 0
        while True:
            l = self._persons[:]

            if len(l) == 1:
                print("round:%d count:%d" % (round, count))
                return l[0].getId()

            round += 1
            print("=====")

            for person in l:
                count += 1
                print("person:%d --> count:%d" % (person.getId(), count))
                if count % 3 == 0:
                    self._del(person)

def main(num):
    circle = Circle(int(num))
    last = circle.countOff()
    print("Last Person is %d" % last)

if __name__ == "__main__":
    main(sys.argv[1])
