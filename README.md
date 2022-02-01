# Async interfaces(standard_async)

Functions of event loop that were implemented:
>-  **_set_timeout_**(callable $callback, int $timout): int
>-  **_cleartimeout_**(int $timerId): bool
>-  **_set_tnterval_**(callable $callback, int $interval): int
>-  **_clear_interval_**(int $timerId): bool
>-  **_file_get_contents_async_**(string $filename, callable $cb, bool $use_include_path = false, int $offset = 0, int $maxlen = null): bool
>-  **_file_put_contents_async_**(string $filename, string $data, callable $cb, bool $use_include_path = false, int $flags = 0): bool

TCP server interfaces `Server class`:
> class **Server** {<br>
 &nbsp;&nbsp;&nbsp;public function **__construct**(int $port, string $host = "0.0.0.0", callable | null $callback);<br>
 &nbsp;&nbsp;&nbsp;public function **on_data**(callable $callback);<br>
 &nbsp;&nbsp;&nbsp;public function **on_error**(callable $callback);<br>
 &nbsp;&nbsp;&nbsp;public function **on_disconnect**(callable $callback);<br>
 &nbsp;&nbsp;&nbsp;public function **write**(string $data): void;<br>
 &nbsp;&nbsp;&nbsp;public function **setReadBufferSize**(long $size): void<br>
 }`

Constructor functional synonym `server(int $port, string $host):Server`


HTTP server interfaces `HttpServer class`:
> class **HttpServer** {<br>
&nbsp;&nbsp;&nbsp;public function **__construct**(int $port, string $host = "0.0.0.0", array $options = [], callable|null $onConnect = null);<br>
&nbsp;&nbsp;&nbsp;public function **on_connect**(callable $callback): void;<br>
&nbsp;&nbsp;&nbsp;public function **on_request**(callable $callback): void;<br>
&nbsp;&nbsp;&nbsp;public function **on_disconnect**(callable $callback): void;<br>
&nbsp;&nbsp;&nbsp;public function **on_error**(callable $callback): void;<br>
&nbsp;&nbsp;&nbsp;public function **setPublicPath**(string $data): void;<br>
}

 
HTTP request interfaces `HttpRequest class`:
> class **HttpServer** {<br>
&nbsp;&nbsp;&nbsp;public string **$method**;<br>
&nbsp;&nbsp;&nbsp;public string **$HttpVersion**;<br>
&nbsp;&nbsp;&nbsp;public string **$uri**;<br>
&nbsp;&nbsp;&nbsp;public string **$querystring**;<br>
&nbsp;&nbsp;&nbsp;public array **$body**;<br>
&nbsp;&nbsp;&nbsp;public array **$headers**;<br>
&nbsp;&nbsp;&nbsp;public array **$query**;<br>
}

HTTP response interfaces `HttpResponse class`:
> class **HttpResponse** {<br>
&nbsp;&nbsp;&nbsp;public int **$statusCode**;<br>
&nbsp;&nbsp;&nbsp;public string **$body**;<br>
&nbsp;&nbsp;&nbsp;public function **setStatusCode**(int $code);<br>
&nbsp;&nbsp;&nbsp;public function **setHeader**(string $headerName, string $value): HttpResponse;<br>
&nbsp;&nbsp;&nbsp;public function **send**(string $data): void;<br>
}`
