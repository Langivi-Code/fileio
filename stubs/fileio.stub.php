<?php

/** @generate-function-entries */

function use_promise(): bool {}

function set_timeout(callable $callback, int $timout): int {}
function clear_timeout(int $timerId): bool {}

function set_interval(callable $callback, int $interval): int {}
function clear_interval(int $timerId): bool {}

function file_get_contents_async(string $filename, callable $cb, bool $use_include_path = false, int $offset = 0, int $maxlen = null): bool {}
function file_put_contents_async(string $filename, string $data, callable $cb, bool $use_include_path = false, int $flags = 0): bool {}
function server(int $port, string $host):void{}

class Server{
public function __construct(int $port, string $host = "0.0.0.0", callable|null $callback){}
public function on_data(callable $callback){}
public function on_error(callable $callback){}
public function on_disconnect(callable $callback){}
public function write(string $data){}
public function setReadBufferSize(long $size):void{}
}

class HttpServer {
    public function __construct(int $port, string $host = "0.0.0.0", array $options = [], callable|null $onConnect = null){}
    public function on_connect(callable $callback){}
    public function on_request(callable $callback){}
    public function on_disconnect(callable $callback){}
    public function on_error(callable $callback){}

    public function setPublicPath(string $path){}
}
class HttpRequest {
    public string $method;
    public string $HttpVersion;
    public string $uri;
    public string $querystring;
    public array $headers;
    public array $query;
}
class HttpResponse {
    public int $statusCode;
    public string $body;
    public function setStatusCode(int $code){}
    public function setHeader(string $headerName, string $value):HttpResponse{}
    public function write(string $data){}
    public function end(string $data){}

}
