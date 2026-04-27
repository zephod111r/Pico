import network
from time import sleep
from picozero import pico_led
import rp2
import sys

from app import quaternion_robot
from app.web_server import WebServer
from app.config import get_config_default
from drivers.pico_robotics import KitronikPicoRobotics


STATIC_INDEX_PATH = "static/index.html"


def connect(wlan):
    config = get_config_default("./config.json")
    # Connect to WLAN
    wlan.connect(config["ssid"], config["password"])
    while wlan.isconnected() == False:
        if rp2.bootsel_button() == 1:
            sys.exit()
        print("Waiting for connection...")
        pico_led.on()
        sleep(0.5)
        pico_led.off()
        sleep(0.5)
    ip = wlan.ifconfig()[0]
    print(f"Connected on {ip}")
    pico_led.on()
    return ip


def handle_motors_factory(robot):
    def handle_motors(request):
        """Handles /api/control POST requests."""

        if request.method != "POST":
            return WebServer.http_response(405, {"error": "Method Not Allowed"})

        q = request.body_to_json()

        if isinstance(q, list):
            q = q[0]
        print(f"q: {q}")

        if not q:
            return WebServer.http_response(400, {"error": "Invalid JSON"})

        try:
            quaternion_robot.quaternion_to_movement(robot, q)
            return WebServer.http_response(200, {"status": "success"})
        except Exception as e:
            print(f"Exception: {e}")
            return WebServer.http_response(500, {"error": "Internal Server Error"})

    return handle_motors


def handle_index(request):
    """Handles serving the index page."""
    try:
        with open(STATIC_INDEX_PATH) as fd:
            html = fd.read()
            return f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n{html}"
    except Exception:
        return WebServer.http_response(500, {"error": "Could not load index.html"})


def run():
    wlan = network.WLAN(network.STA_IF)
    try:
        wlan.active(True)
        ip = connect(wlan)

        robot = KitronikPicoRobotics()
        server = WebServer(ip, 80)
        server.add_route("POST", "/api/motors", handle_motors_factory(robot))
        server.add_route("GET", "/", handle_index)

        # === Start the server ===
        server.serve()
    except Exception as e:
        print(f"Exception: {e}")
    finally:
        print("Exiting")
        wlan.disconnect()
        while wlan.isconnected():
            print("Disconnecting...")
            pico_led.on()
            sleep(0.5)
            pico_led.off()
            sleep(0.5)
        print("Disconnected")
        pico_led.off()
        sys.exit()
