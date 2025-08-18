from engine.kv_engine import KVEngine
from query.parser import parse

def main():
    print("🌱 Welcome to SapotaDB CLI 🌱")
    print("Commands: SET key value | GET key | DELETE key | EXIT")

    db = KVEngine()

    while True:
        try: 
            command = input("sapota> ").strip()
            if not command:
                continue

            cmd, args = parse(command)

            if cmd == "EXIT":
                print("Thanks for Tasting...")
                break
            elif cmd == "SET" and len(args) >= 2:
                key, value = args[0], " ".join(args[1:])
                db.set(key, value)
                print("Inserted")
            elif cmd == "GET" and len(args) == 1:
                value = db.get(args[0])
                print(value)
            elif cmd == "DELETE" and len(args) == 1:
                result = db.delete(args[0])
                print("Deleted" if result is not None else "Key Not Found")
            else:
                print("Invalid command")
        except KeyboardInterrupt:
            print("\nExiting SapotaDB...")
            break

if __name__ == "__main__":
    main()