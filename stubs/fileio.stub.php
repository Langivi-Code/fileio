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
public function __construct(int $port, string $host, callable $callback){}
public function on_data(callable $callback){}
public function on_error(callable $callback){}
public function on_disconnect(callable $callback){}
public function write(string $data){}
}
