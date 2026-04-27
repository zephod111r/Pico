import ujson as json
import ubinascii
import utime

SECRET_KEY = b'your_secret_key'

def generate_token(user_id):
    payload = {
        'user_id': user_id,
        'exp': utime.time() + 3600  # 1 hour expiration
    }
    token = json.dumps(payload)
    return ubinascii.b2a_base64(token.encode()).decode().strip()

def verify_token(token):
    try:
        payload = json.loads(ubinascii.a2b_base64(token).decode())
        if payload['exp'] < utime.time():
            return None
        return payload['user_id']
    except Exception:
        return None
    
if __name__ == "__main__":
    try:
        token = generate_token("default user")
        print(f"Token = {token}")
        user = verify_token(token)
        print(f"User = {user}")
        
    except Exception as e:
        print(f"Error: {e}")