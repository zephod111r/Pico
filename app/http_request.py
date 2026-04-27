import json

class HttpRequest:
    def __init__(self, request_text):
        self.method = None
        self.path = None
        self.http_version = None
        self.query_params = {}  # Dictionary of parsed query parameters
        self.headers = {}
        self.body = None

        self._parse_request(request_text)

    def _parse_request(self, request_text):
        """Parse the raw HTTP request into structured data."""
        lines = request_text.split("\r\n")
        request_line = lines[0].split(" ")

        if len(request_line) < 3:
            return  # Invalid request

        self.method = request_line[0]
        full_path = request_line[1]
        self.http_version = request_line[2]

        # Split path and query parameters
        if "?" in full_path:
            self.path, query_string = full_path.split("?", 1)
            self.query_params = self._parse_query_string(query_string)
        else:
            self.path = full_path

        # Parse headers
        header_lines = lines[1:]
        body_index = None
        for i, line in enumerate(header_lines):
            if line == "":
                body_index = i + 1  # Body starts after empty line
                break
            key, value = line.split(":", 1)
            self.headers[key.strip()] = value.strip()

        # Extract body if present
        if body_index and body_index < len(lines):
            self.body = "\n".join(lines[body_index:])

    def _parse_query_string(self, query_string):
        """Parse query parameters into a dictionary and convert it to JSON."""
        params = {}
        for param in query_string.split("&"):
            key_value = param.split("=", 1)
            if len(key_value) == 2:
                key, value = key_value
                params[key] = value

        return params

    def query_params_json(self):
        """Return query parameters as a JSON string."""
        return json.dumps(self.query_params, indent=2)

    def body_to_json(self):
        try:
            return json.loads(self.body)
        except Exception as e:
            print(f"Error: {e}")
            return None

    def __str__(self):
        return (
            f"Method: {self.method}, Path: {self.path}, "
            f"Query (JSON): {self.query_params_json()}, Headers: {self.headers}, Body: {self.body}"
        )
