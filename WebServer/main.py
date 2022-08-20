import wsgiref.util

Light = True
Colour = [255, 255, 255]
Message = ""


def webServer(environ, start_response):
    global Light, Colour, Message
    uri = wsgiref.util.request_uri(environ)
    path = uri.split("/")[3]

    status = ""
    message = ""

    if environ['REQUEST_METHOD'] == 'GET':
        if path.lower() == "light":
            status = "200 OK"
            message = Light
        elif path.lower() == "colour":
            status = "200 OK"
            message = Colour
        elif path.lower() == "message":
            status = "200 OK"
            message = Message
        else:
            status = "404 Not Found"
            message = "Page Not Found"

    elif environ['REQUEST_METHOD'] == 'POST':
        try:
            size = int(environ['CONTENT_LENGTH'])
            body = environ['wsgi.input'].read(size)

            if path.lower() == "light":
                Light = bool(body)
                status = "200 OK"
                message = "Success"
            elif path.lower() == "colour":
                Colour = list(body)
                status = "200 OK"
                message = "Success"
            elif path.lower() == "message":
                Message = str(body)
                status = "200 OK"
                message = "Success"
            else:
                status = "404 Not Found"
                message = "Page Not Found"
        except ValueError:
            status = "500 Internal Server Error"
            message = "Failed"

    headers = [('Content-type', 'text/plain')]
    start_response(status, headers)
    return [str(message)]
