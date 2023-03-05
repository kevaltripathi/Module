import wsgiref.util
import json
import MySQLdb


def db_commit(column, value):
    try:
        connection = MySQLdb.connect('localhost', 'user', 'password', 'database')
        with connection:
            cursor = connection.cursor()
            sql = "UPDATE `Data` SET `{0}` = '{1}' WHERE `id` = 1".format(column, value)
            cursor.execute(sql)
            cursor.close()
            connection.commit()
        return "200 OK", "Success"
    except Exception as e:
        return "500 Internal Server Error", e


def db_get(column):
    try:
        connection = MySQLdb.connect('localhost', 'user', 'password', 'database')
        with connection:
            cursor = connection.cursor()
            sql = "SELECT `{0}` FROM `Data` WHERE `id` = 1".format(column)
            cursor.execute(sql)
            result = cursor.fetchone()
            cursor.close()
            return result[0]
    except Exception as e:
        return e


def webserver(environ, start_response):
    uri = wsgiref.util.request_uri(environ)
    path = uri.split("/")[3]

    status = ""
    response = ""

    if environ['REQUEST_METHOD'] == 'GET':
        if path.lower() == "matrix":
            status = "200 OK"
            response = db_get("matrix")
        elif path.lower() == "title":
            status = "200 OK"
            response = db_get("title")
        elif path.lower() == "message":
            status = "200 OK"
            response = db_get("message")
        else:
            status = "404 Not Found"
            response = "Page Not Found"

    elif environ['REQUEST_METHOD'] == 'POST':
        try:
            size = int(environ['CONTENT_LENGTH'])
            body = str(environ['wsgi.input'].read(size))

            if path.lower() == "matrix":
                status, response = db_commit("matrix", json.dumps(body.strip(" []\n").split(',', 7)))
            elif path.lower() == "title":
                status, response = db_commit("title", body.strip("\n"))
            elif path.lower() == "message":
                message = {"lineQty": "", "lines": [], "weight": []}
                split = body.strip(" []\n").split(',')
                message["lineQty"] = split[0]
                lines = []
                weight = []
                for x in range(int(split[0])):
                    lines.append(split[x + 1])
                    weight.append(split[x + int(split[0]) + 1])
                message["lines"] = lines
                message["weight"] = weight
                status, response = db_commit("message", json.dumps(message))
            else:
                status = "404 Not Found"
                response = "Page Not Found"
        except Exception as e:
            status = "500 Internal Server Error"
            response = e

    headers = [('Content-type', 'text/plain')]
    start_response(status, headers)
    return [str(response)]
