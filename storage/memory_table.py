class MemoryTable():

    def __init__(self) -> None:
        self.store = {}

    def set(self, key, value) -> None:
        self.store[key] = value

    def get(self, key):
        return self.store.get(key, None)

    def delete(self, key):
        return self.store.pop(key, None)
