import serial.tools.list_ports
import serial
import threading
import asyncio
import websockets
from multiprocessing import Value

last_dir = Value("d", 0.0)
dir_changed = threading.Event()

ports = serial.tools.list_ports.comports()
for idx, port in enumerate(ports):
    print ("[%d] %s" % (idx, port.device))
ser = serial.Serial(ports[int(input("Select port: "))].device, 115200)

def read_ser():
    while True:
        last_dir.value = float(ser.readline())
        print("dir changed: %f" % last_dir.value)
        dir_changed.set()

async def producer():
    dir_changed.wait()
    dir_changed.clear();
    return str(last_dir.value);

async def server(websocket, path):
    while True:
        websocket.send(last_dir)
        message = await producer()
        await websocket.send(message)



t = threading.Thread(target=read_ser, daemon=True)
t.start()

start_server = websockets.serve(server, '0.0.0.0', 3939)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()