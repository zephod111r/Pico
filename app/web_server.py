import json
import socket
import rp2

from app.http_request import HttpRequest

class Route:
    """Defines a simple route with method, path, and handler function."""
    def __init__(self, method, path, handler):
        self.method = method
        self.path = path
        self.handler = handler

class WebServer:
    """Handles incoming HTTP requests and routes them accordingly."""
    def __init__(self, host="0.0.0.0", port=80):
        self.host = host
        self.port = port
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.bind((host, port))
        self.server.listen(5)
        self.routes = []
        self.running = True

    def add_route(self, method, path, handler):
        """Registers a new route."""
        self.routes.append(Route(method, path, handler))

    def serve(self):
        """Starts the server and handles incoming requests."""
        print(f"Server {self.host} listening on port {self.port}...")
        while self.running:
            client = None
            try:
                
                if rp2.bootsel_button() == 1:
                    self.close()
                    continue
                
                client, addr = self.server.accept()
                request_headers, request_body = WebServer.read_request(client)
                
                #print(f"Request headers: {request_headers}")
                
                if not request_headers:
                    client.close()
                    continue

                #print(f"Body: {request_body}")
                if not request_body:
                    request_body = ""
                    
                request_data = request_headers + "\r\n\r\n" + request_body  # Reconstruct full request
                try:
                    request = HttpRequest(request_data)
                except ValueError as e:
                    print(f"Error processing request: {e}")
                    client.close()
                    continue

                # Match route
                response = self.handle_request(request)

                client.send(response.encode())

            except OSError as e:
                print("Socket error:", e)
            except Exception as e:
                print("Unexpected error:", e)
            finally:
                if client:
                    client.close()

    def handle_request(self, request):
        """Processes the request and returns an appropriate response."""
        if not request.method or not request.path:
            return self.http_response(400, {"error": "Bad Request - Invalid method or path"})

        # Match a registered route
        for route in self.routes:
            if request.method == route.method and request.path == route.path:
                try:
                    return route.handler(request)
                except Exception as e:
                    print(f"Error in handler for {request.method} {request.path}: {e}")
                    return self.http_response(500, {"error": "Internal Server Error"})

        print(f"No matching route found for {request.method} {request.path}")
        return self.http_response(404, {"error": "Not Found"})

    def close(self):
        self.running = False
        self.server.close()

    @staticmethod
    def read_request(client):
        # Reads the full HTTP request, handling large bodies properly."""
        request_data = b""
        
        # Read headers first
        while b"\r\n\r\n" not in request_data:
            chunk = client.recv(1024)  # Read in 1KB chunks
            if not chunk:
                break  # Connection closed
            request_data += chunk

        # Decode headers
        try:
            header_data = request_data.decode("utf-8")
            headers, body = header_data.split("\r\n\r\n", 1)
        except ValueError:
            return None, None  # Invalid request format

        # Find Content-Length if present
        content_length = 0
        for line in headers.split("\r\n"):
            if line.lower().startswith("content-length:"):
                content_length = int(line.split(":")[1].strip())

        # Read remaining body if needed
        while len(body) < content_length:
            chunk = client.recv(1024)
            if not chunk:
                break
            body += chunk.decode("utf-8")

        return headers, body

    def http_response(status, body):
        """Generates an HTTP response."""
        response_body = json.dumps(body)
        return f"HTTP/1.1 {status} {WebServer.status_message(status)}\r\nContent-Type: application/json\r\nContent-Length: {len(response_body)}\r\n\r\n{response_body}"

    @staticmethod
    def status_message(status):
        """Maps HTTP status codes to messages."""
        return {
            200: "OK",
            400: "Bad Request",
            404: "Not Found",
            500: "Internal Server Error"
        }.get(status, "Unknown")
