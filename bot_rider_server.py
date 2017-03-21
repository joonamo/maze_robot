import urllib.parse
import http.server
import socketserver
import json

import serial

PORT = 8000

last_request = None
ser = serial.Serial('/dev/ttyAMA0', baudrate = 115200, timeout=0.1)

class RequestHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        print("Request!")
        print(self.path)
        queries = urllib.parse.parse_qs(urllib.parse.urlparse(self.path).query)

        response = {}

        if "speed" in queries:
            print("speed: %s" % str(queries["speed"][0]))
            ser.write((u"s%ds" % int(queries["speed"][0])).encode("UTF-8"))
            response["speed"] = queries["speed"]
        if "dir" in queries:
            print("dir: %s" % str(queries["dir"][0]))
            ser.write((u"d%dd" % int(queries["dir"][0])).encode("UTF-8"))
            response["dir"] = queries["dir"]

        ser.reset_input_buffer()
        ser.write(b"q")
        r = str(ser.readline())
        print("Status: %s" % (r,))
        response["status"] = r

        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(response).encode("UTF-8"))
        return 



httpd = socketserver.TCPServer(("", PORT), RequestHandler)
print("serving at port", PORT)
httpd.serve_forever()