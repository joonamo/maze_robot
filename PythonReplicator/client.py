import traceback
import sys
import serial
import asyncio
import websockets
import time

ser = serial.Serial("/dev/ttyAMA0", 115200)

async def client():
    async with websockets.connect('ws://%s:3939'% sys.argv[1]) as websocket:
        print("Connected!")
        while True:
            message = await websocket.recv()
            print(message)
            ser.write(bytes("d" + message, "utf-8"))

while True:
    try:
        time.sleep(1)
        print("Connecting...")
        asyncio.get_event_loop().run_until_complete(client())
    except KeyboardInterrupt:
        break
    except:
        traceback.print_exc()
