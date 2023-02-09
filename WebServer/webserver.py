import wsgiref.util
import json

Matrix = ["", "", "", "", "", "", "", ""]
Title = ""
Message = {"lineQty": "", "lines": [], "weight": []}


def webserver(environ, start_response):
    global Matrix, Title, Message
    uri = wsgiref.util.request_uri(environ)
    path = uri.split("/")[3]

    status = ""
    message = ""

    if environ['REQUEST_METHOD'] == 'GET':
        if path.lower() == "matrix":
            status = "200 OK"
            message = json.dumps(Matrix)
        elif path.lower() == "title":
            status = "200 OK"
            message = Title
        elif path.lower() == "message":
            status = "200 OK"
            message = json.dumps(Message)
        else:
            status = "404 Not Found"
            message = "Page Not Found"

    elif environ['REQUEST_METHOD'] == 'POST':
        try:
            size = int(environ['CONTENT_LENGTH'])
            body = str(environ['wsgi.input'].read(size))

            if path.lower() == "matrix":
                Matrix = body.strip(" []\n").split(',', 7)
                status = "200 OK"
                message = "Success"
            elif path.lower() == "title":
                Title = body
                status = "200 OK"
                message = "Success"
            elif path.lower() == "message":
                split = body.strip(" []\n").split(',')
                Message["lineQty"] = split[0]
                lines = []
                weight = []
                for x in range(int(split[0])):
                    lines.append(split[x + 1])
                    weight.append(split[x + int(split[0]) + 1])
                Message["lines"] = lines
                Message["weight"] = weight
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
