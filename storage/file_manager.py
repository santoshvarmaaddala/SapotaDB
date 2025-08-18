import os

class FileManager:
    def __init__(self, filename="data/sapotadb.log"):
        self.filename = filename
        os.makedirs(os.path.dirname(filename), exist_ok=True)

    def append(self, command):
        with open(self.filename, "a") as f:
            f.write(command + "\n")

    def load(self):
        if not os.path.exists(self.filename):
            return []

        with open(self.filename, "r") as f:
            return [line.strip() for line in f.readlines()]
        
