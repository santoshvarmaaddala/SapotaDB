from storage.memory_table import MemoryTable
from storage.file_manager import FileManager

class KVEngine:
    def __init__(self):
        self.memory_table = MemoryTable()
        self.file_manager = FileManager()
        self._recover()

    def _recover(self):
        commands = self.file_manager.load()
        for cmd in commands:
            parts = cmd.split()
            if parts[0] == "SET":
                self.memory_table.set(parts[1], " ".join(parts[2:]))
            elif parts[0] == "DELETE":
                self.memory_table.delete(parts[1])

    def set(self, key, value):
        self.memory_table.set(key, value)
        self.file_manager.append(f"SET {key} {value}")

    def get(self, key):
        return self.memory_table.get(key)

    def delete(self, key):
        result = self.memory_table.delete(key)
        if result is not None:
            self.file_manager.append(f"DELETE {key}")
        return result