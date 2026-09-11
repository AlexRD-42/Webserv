"""Shared fixture helpers. All normal responses use CRLF and an exact byte length."""
import hashlib
import json
import os
import sys
from urllib.parse import parse_qs

OUT = sys.stdout.buffer


def query():
    return parse_qs(os.environ.get("QUERY_STRING", ""), keep_blank_values=True)


def read_body():
    if os.environ.get("REQUEST_METHOD", "GET") == "GET":
        return b""
    length = os.environ.get("CONTENT_LENGTH", "")
    return sys.stdin.buffer.read(int(length)) if length else sys.stdin.buffer.read()


def start(content_type="text/plain", length=None, status="200 OK", extra=(), newline=b"\r\n"):
    headers = []
    if status != "200 OK":
        headers.append("Status: " + status)
    headers.append("Content-Type: " + content_type)
    if length is not None:
        headers.append("Content-Length: " + str(length))
    headers.extend(name + ": " + value for name, value in extra)
    OUT.write(newline.join(item.encode("ascii") for item in headers) + newline + newline)


def emit(body, content_type="text/plain", status="200 OK", extra=(), newline=b"\r\n"):
    if isinstance(body, str):
        body = body.encode("utf-8")
    start(content_type, len(body), status, extra, newline)
    OUT.write(body)
    OUT.flush()


def emit_json(value, status="200 OK", extra=()):
    emit(json.dumps(value, sort_keys=True, ensure_ascii=False) + "\n", "application/json", status, extra)


def body_info(body):
    return {"fixture": "webserv-siege:echo", "method": os.environ.get("REQUEST_METHOD", ""),
            "bytes": len(body), "sha256": hashlib.sha256(body).hexdigest(),
            "content_type": os.environ.get("CONTENT_TYPE", ""),
            "declared_length": os.environ.get("CONTENT_LENGTH", ""),
            "query": query()}
