#!/usr/bin/python2.7

import web.wsgiserver

def my_wsgi_app(env, start_response):
    status = "200 OK"

    response_headers = [("Content-type", "text/plain")]
    start_response(status, response_headers)

    for e in env.iteritems():
        print e

    return ["wsgiserver test"]

server = web.wsgiserver.CherryPyWSGIServer(("127.0.0.1", 60000), my_wsgi_app)
server.start()
