import uhashlib
import ubinascii
import utime

def hmac_sha256(key, message):
    block_size = 64  # Block size for SHA-256
    if len(key) > block_size:
        key = uhashlib.sha256(key).digest()
    key = key + b'\x00' * (block_size - len(key))

    o_key_pad = bytes((x ^ 0x5c) for x in key)
    i_key_pad = bytes((x ^ 0x36) for x in key)

    inner_hash = uhashlib.sha256(i_key_pad + message).digest()
    return uhashlib.sha256(o_key_pad + inner_hash).digest()

def generate_pop_token(user_id, secret):
    message = f'{user_id}:{utime.time()}'.encode()
    signature = ubinascii.hexlify(hmac_sha256(secret.encode(), message)).decode()
    return f'{message.decode()}:{signature}'

def verify_pop_token(token, secret):
    try:
        message, signature = token.rsplit(':', 1)
        expected_signature = ubinascii.hexlify(hmac_sha256(secret.encode(), message.encode())).decode()
        return ubinascii.hexlify(signature.encode()).decode() == expected_signature
    except ValueError:
        return False
    
if __name__ == "__main__":
    try:
        token = generate_pop_token("default user", "5b90e5c8a8cf79238042628e325e7ea0e8915294be47066a53b10015bda30562")
        print(f"Token = {token}")
        valid = verify_pop_token(token, "5b90e5c8a8cf79238042628e325e7ea0e8915294be47066a53b10015bda30562")
        print(f"User approved = {valid}")
        
    except Exception as e:
        print(f"Error: {e}")