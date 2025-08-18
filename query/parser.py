def parse(command):
    parts = command.strip().split()
    if not parts:
        return None, []
    
    cmd = parts[0].upper()
    args = parts[1: ]
    return cmd, args