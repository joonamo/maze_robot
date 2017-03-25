import urllib.parse
import http.server
import socketserver
import json
import traceback
import sys
import serial
import re
import os
import getopt

PORT = 8000
opts, args = getopt.getopt(sys.argv[1:], "p:", ["cvd="])
for o, a in opts:
    if o == "-p":
        PORT = int(a)
    elif o == "--cvd":
        os.chdir(a)

last_request = None
ser = serial.Serial('/dev/ttyAMA0', baudrate = 115200, timeout=0.1)
status_pattern = re.compile("MANUAL: ([-+]?\d+), L ([-+]?\d+), F ([-+]?\d+), R ([-+]?\d+), dir ([-+]?\d+), speed ([-+]?\d+), left_mapped ([-+]?\d+), right_mapped ([-+]?\d+)")

class RequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        print("Request!")
        print(self.path)

        if self.path == "/status":
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
            status_table = {"raw_status": str(r)}

            try:
                s = re.search(status_pattern, r).groups()
                status_table["manual"] = int(s[0])
                status_table["dist_l"] = int(s[1])
                status_table["dist_f"] = int(s[2])
                status_table["dist_r"] = int(s[3])
                status_table["dir"] = int(s[4])
                status_table["speed"] = int(s[5])
                status_table["left_mapped"] = int(s[6])
                status_table["right_mapped"] = int(s[7])
            except Exception as e:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                traceback.print_exception(exc_type, exc_value, exc_traceback)

            response["status"] = status_table

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', 'http://127.0.0.1:8000')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode("UTF-8"))

        elif self.path == "/toggle_manual":
            ser.write(b"a")

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', 'http://127.0.0.1:8000')
            self.end_headers()
            self.wfile.write("{}".encode("UTF-8"))

        else:
            http.server.SimpleHTTPRequestHandler.do_GET(self)


socketserver.TCPServer.allow_reuse_address = True
httpd = socketserver.TCPServer(("", PORT), RequestHandler)
print("serving at port", PORT)
httpd.serve_forever()