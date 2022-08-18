import time
import BaseHTTPServer

HOST_NAME = "127.0.0.1"  # !!!REMEMBER TO CHANGE THIS!!!
PORT_NUMBER = 80

Light = True
Colour = [255, 255, 255]
Message = ""


class MyHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    def do_GET(s):
        if s.path.lower() == "/light":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Light Status: %s</p></body></html>" % Light)
        elif s.path.lower() == "/colour":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Current Colour: %s</p></body></html>" % Colour)
        elif s.path.lower() == "/message":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Current Message: %s</p></body></html>" % Message)
        else:
            s.send_response(404)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Page Not Found</p></body></html>")

    def do_POST(s):
        global Light, Colour, Message
        content_len = int(s.headers.getheader('content-length', 0))
        body = s.rfile.read(content_len)
        if s.path.lower() == "/light":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            Light = bool(body.strip() == "True")
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Light Status Set to: %s</p></body></html>" % Light)
        elif s.path.lower() == "/colour":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            Colour = list(body.strip())
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Current Colour set to: %s</p></body></html>" % Colour)
        elif s.path.lower() == "/message":
            s.send_response(200)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            Message = str(body.strip())
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Current Message set to: %s</p></body></html>" % Message)
        else:
            s.send_response(404)
            s.send_header("Content-type", "text/html")
            s.end_headers()
            s.wfile.write("<html><head><title>API</title></head><body>")
            s.wfile.write("<p>Page Not Found</p></body></html>")


if __name__ == '__main__':
    server_class = BaseHTTPServer.HTTPServer
    httpd = server_class((HOST_NAME, PORT_NUMBER), MyHandler)
    print time.asctime(), "Server Starts - %s:%s" % (HOST_NAME, PORT_NUMBER)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print time.asctime(), "Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER)
