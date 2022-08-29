import wsgiref.util

Light = True
Colour = [255, 255, 255]
Message = ""


def webserver(environ, start_response):
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
            body = str(environ['wsgi.input'].read(size), "utf-8")

            if path.lower() == "light":
                Light = body.strip().lower() == "true"
                status = "200 OK"
                message = "Success"
            elif path.lower() == "colour":
                Colour = list(map(int, body.strip().split(',', 2)))
                status = "200 OK"
                message = "Success"
            elif path.lower() == "message":
                Message = body
                status = "200 OK"
                message = "Success"
            else:
                status = "404 Not Found"
                message = "Page Not Found"
        except Exception as e:
            status = "500 Internal Server Error"
            message = e

    headers = [('Content-type', 'text/plain')]
    start_response(status, headers)
    return [str(message)]
