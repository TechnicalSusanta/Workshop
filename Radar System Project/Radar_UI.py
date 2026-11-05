
import tkinter as tk
from tkinter import ttk
import serial
import threading
import math
import queue
import time

# ---------------- CONFIGURATION ----------------

COM_PORT = "COM8"       # Change to your Arduino port
BAUD_RATE = 9600
MAX_DISTANCE = 100      # Radar range in cm

data_queue = queue.Queue()
running = True

# ---------------- SERIAL CONNECTION ----------------

try:
    arduino = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)
    connected = True
except Exception as e:
    arduino = None
    connected = False
    print("Serial Error:", e)

def serial_reader():
    while running and arduino:
        try:
            line = arduino.readline().decode(
                "utf-8", errors="ignore"
            ).strip()

            if line:
                values = line.split(",")

                if len(values) == 2:
                    angle = float(values[0])
                    distance = float(values[1])

                    data_queue.put((angle, distance))

        except Exception:
            pass

if connected:
    threading.Thread(
        target=serial_reader,
        daemon=True
    ).start()

# ---------------- GUI ----------------

root = tk.Tk()
root.title("ULTRASONIC RADAR SYSTEM")
root.geometry("1100x750")
root.configure(bg="#07110d")
root.minsize(850, 650)

style = ttk.Style()
style.theme_use("clam")

style.configure(
    "TButton",
    font=("Arial", 11, "bold"),
    padding=8
)

header = tk.Frame(root, bg="#07110d")
header.pack(fill="x", pady=12)

tk.Label(
    header,
    text="ULTRASONIC RADAR SYSTEM",
    font=("Arial", 23, "bold"),
    fg="#39ff14",
    bg="#07110d"
).pack()

status_text = "CONNECTED" if connected else "DISCONNECTED"

status = tk.Label(
    header,
    text="● " + status_text,
    fg="#39ff14" if connected else "red",
    bg="#07110d",
    font=("Arial", 11, "bold")
)
status.pack(pady=5)

# ---------------- RADAR DISPLAY ----------------

canvas = tk.Canvas(
    root,
    bg="#020a08",
    highlightthickness=2,
    highlightbackground="#39ff14"
)
canvas.pack(fill="both", expand=True, padx=20, pady=10)

angle = 90
distance = 0

detections = []

def draw_radar():

    canvas.delete("all")

    w = canvas.winfo_width()
    h = canvas.winfo_height()

    cx = w / 2
    cy = h - 35

    radius = min(w * 0.43, h * 0.85)

    # Radar background

    canvas.create_rectangle(
        0, 0, w, h,
        fill="#020a08",
        outline=""
    )

    # Radar semicircle grid

    for r in range(1, 6):

        rad = radius * r / 5

        canvas.create_arc(
            cx-rad, cy-rad,
            cx+rad, cy+rad,
            start=0,
            extent=180,
            style="arc",
            outline="#39ff14",
            width=2
        )

        canvas.create_text(
            cx + rad - 15,
            cy - 12,
            text=f"{MAX_DISTANCE*r/5:.0f}cm",
            fill="#39ff14",
            font=("Arial", 9)
        )

    # Angle lines

    for a in range(0, 181, 30):

        radian = math.radians(a)

        x = cx + radius * math.cos(radian)
        y = cy - radius * math.sin(radian)

        canvas.create_line(
            cx, cy, x, y,
            fill="#39ff14",
            width=1
        )

        tx = cx + (radius+22) * math.cos(radian)
        ty = cy - (radius+22) * math.sin(radian)

        canvas.create_text(
            tx, ty,
            text=f"{a}°",
            fill="#39ff14",
            font=("Arial", 10)
        )

    # Draw detected objects

    for a, d, timestamp in detections:

        if time.time() - timestamp > 3:
            continue

        if 0 < d <= MAX_DISTANCE:

            radian = math.radians(a)
            obj_r = radius * d / MAX_DISTANCE

            x = cx + obj_r * math.cos(radian)
            y = cy - obj_r * math.sin(radian)

            canvas.create_oval(
                x-5, y-5, x+5, y+5,
                fill="red",
                outline="#ff5555",
                width=2
            )

    # Current scanning line

    radian = math.radians(angle)

    x = cx + radius * math.cos(radian)
    y = cy - radius * math.sin(radian)

    canvas.create_line(
        cx, cy, x, y,
        fill="#ff3333",
        width=4
    )

    # Center point

    canvas.create_oval(
        cx-6, cy-6, cx+6, cy+6,
        fill="#39ff14",
        outline=""
    )

    canvas.create_text(
        cx, 20,
        text="LIVE RADAR SCANNING",
        fill="#39ff14",
        font=("Arial", 13, "bold")
    )

# ---------------- INFORMATION PANEL ----------------

panel = tk.Frame(root, bg="#07110d")
panel.pack(fill="x", padx=20, pady=8)

def create_info(title, value):

    frame = tk.Frame(
        panel,
        bg="#10241a",
        padx=20,
        pady=10
    )

    frame.pack(
        side="left",
        expand=True,
        fill="x",
        padx=5
    )

    tk.Label(
        frame,
        text=title,
        fg="#39ff14",
        bg="#10241a",
        font=("Arial", 10, "bold")
    ).pack()

    label = tk.Label(
        frame,
        text=value,
        fg="white",
        bg="#10241a",
        font=("Arial", 19, "bold")
    )

    label.pack()

    return label

angle_label = create_info("ANGLE", "0°")
distance_label = create_info("DISTANCE", "-- cm")
object_label = create_info("DETECTION", "NO OBJECT")

# ---------------- UPDATE DATA ----------------

def update_radar():

    global angle, distance, detections

    while not data_queue.empty():

        try:
            angle, distance = data_queue.get_nowait()

            angle_label.config(
                text=f"{angle:.0f}°"
            )

            if 0 < distance <= MAX_DISTANCE:

                distance_label.config(
                    text=f"{distance:.1f} cm"
                )

                object_label.config(
                    text="OBJECT FOUND",
                    fg="#ff3333"
                )

                detections.append(
                    (angle, distance, time.time())
                )

            else:

                distance_label.config(
                    text="OUT OF RANGE"
                )

                object_label.config(
                    text="NO OBJECT",
                    fg="white"
                )

        except queue.Empty:
            break

    detections = [
        item for item in detections
        if time.time() - item[2] < 3
    ]

    draw_radar()

    root.after(40, update_radar)

# ---------------- BUTTONS ----------------

buttons = tk.Frame(root, bg="#07110d")
buttons.pack(pady=10)

def clear_radar():
    detections.clear()
    draw_radar()

tk.Button(
    buttons,
    text="CLEAR RADAR",
    command=clear_radar,
    bg="#146b36",
    fg="white",
    font=("Arial", 11, "bold"),
    padx=15
).pack(side="left", padx=10)

def close_app():
    global running
    running = False

    if arduino:
        arduino.close()

    root.destroy()

tk.Button(
    buttons,
    text="EXIT",
    command=close_app,
    bg="#a32626",
    fg="white",
    font=("Arial", 11, "bold"),
    padx=15
).pack(side="left", padx=10)

root.protocol("WM_DELETE_WINDOW", close_app)

update_radar()

root.mainloop()

