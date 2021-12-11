# Async interfaces(fileio)

Functions of event loop that were implemented:
>-  **_set_timeout_**(callable $callback, int $timout): int
>-  **_cleartimeout_**(int $timerId): bool
>-  **_set_tnterval_**(callable $callback, int $interval): int
>-  **_clear_interval_**(int $timerId): bool
>-  **_file_get_contents_async_**(string $filename, callable $cb, bool $use_include_path = false, int $offset = 0, int $maxlen = null): bool
>-  **_file_put_contents_async_**(string $filename, string $data, callable $cb, bool $use_include_path = false, int $flags = 0): bool 
