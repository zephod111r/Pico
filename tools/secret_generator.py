import urandom
import ubinascii

def generate_secret(length=32):
    return ubinascii.hexlify(bytes([urandom.getrandbits(8) for _ in range(length)])).decode()

if __name__ == "__main__":
    # Generate a 32-byte secret
    secret = generate_secret()
    print(secret)
