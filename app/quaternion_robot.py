import math

def process_quaternion(robot, q):
    """
    Converts a quaternion into motor commands for an omnidirectional robot.
    q = (w, x, y, z)
    - x controls West-East movement (-x = East, +x = West)
    - z controls North-South movement (-z = South, +z = North)
    - y is the rotation angle in radians (affects spinning)
    - base_speed is the speed scalar (default 100)
    """
    max_speed = 100

    w, x, y, z = q

    # Normalize quaternion
    norm = math.sqrt(w**2 + x**2 + y**2 + z**2)
    w, x, y, z = w / norm, x / norm, y / norm, z / norm

    # Forward/backward motion: X (West-East) and Z (North-South)
    move_x = x  # Left/Right (West-East)
    move_z = z  # Forward/Backward (North-South)
    
    # Rotation speed: y (rotation in radians)
    spin = y

    # Compute individual motor speeds
    motor_1_speed = max(min(-move_x + move_z - spin, 1.0),-1.0) * max_speed  # Front-left
    motor_2_speed = max(min(move_x + move_z - spin, 1.0),-1.0) * max_speed   # Front-right
    motor_3_speed = max(min(move_x - move_z - spin, 1.0),-1.0) * max_speed   # Back-right
    motor_4_speed = max(min(-move_x - move_z - spin, 1.0),-1.0) * max_speed  # Back-left

    # Convert speeds to directions
    def set_motor(motor, speed):
        if speed > 0:
            robot.motorOn(motor, "f", int(abs(speed)))
        elif speed < 0:
            robot.motorOn(motor, "r", int(abs(speed)))
        else:
            robot.motorOn(motor, "f", 0)  # Stop motor

    # Apply motor speeds
    set_motor(1, motor_1_speed)
    set_motor(2, motor_2_speed)
    set_motor(3, motor_3_speed)
    set_motor(4, motor_4_speed)

    print(f"Motor Speeds: {motor_1_speed}, {motor_2_speed}, {motor_3_speed}, {motor_4_speed}")

def quaternion_to_movement(robot, q):
    """
    Process quaternion JSON input and control motors.
    Expected format: { "w": float, "x": float, "y": float, "z": float }
    """
    w = q.get("w", 1)
    x = q.get("x", 0)
    y = q.get("y", 0)
    z = q.get("z", 0)

    # Use the existing quaternion-based motor control logic
    quaternion = (w, x, y, z)
    
    print(f"Quaternion: {quaternion}")
    process_quaternion(robot, quaternion)