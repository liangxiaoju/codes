
class Looper:

    def __init__(self, messageQueue):
        self.queue = messageQueue

    def loop(self):

        while True:
            msg = self.queue.next()
            if msg is None:
                break
            msg.target.dispatchMessage(msg)
            msg.remove()
