
class Message:

    def __init__(self):
        self.what = None
        self.arg1 = None
        self.arg2 = None
        self.obj = None
        self.target = None

    @staticmethod
    def obtain():
        return Message()

    def remove(self):
        del self
