import json


def get_config_default(filename):
    try:
        with open(filename) as fd:
            return json.load(fd)

    except OSError:
        with open(filename, "w") as fd:
            config = {
                "ssid": "default network",
                "password": "default password",
            }
            print(f"Writing config: {config}")
            json.dump(config, fd)
            return config
    
def set_config(filename, config):
    try:
        with open(filename, "w") as fd:
            json.dump(config, fd)
    except OSError:
        print("Failed to write config")
        
if __name__ == "__main__":
    try:
        config = get_config_default("./config.json")
    except OSError:
        print("Failed to load config")
