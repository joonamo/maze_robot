
import sys
import serial
import asyncio
import websockets

ser = serial.Serial("dev/ttyAMA0", 115200)

async def client():
    async with websockets.connect('ws://%s:3939'% sys.argv[1]) as websocket:
        while True:
            message = await websocket.recv()
            print(message)
            ser.write(bytes("b" + message, "utf-8"))

while True:
    try:
        asyncio.get_event_loop().run_until_complete(client())
    except KeyboardInterrupt:
        break
    except:
        pass