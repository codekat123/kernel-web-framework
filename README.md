# kernel-web-framework

> A backend web framework built from scratch in C++.

---

## What Is This?

kernel-web-framework is a backend web framework written in C++ from the ground up — no dependencies, no abstractions borrowed from existing frameworks.

Every layer is implemented manually:

- Raw TCP server using POSIX sockets
- HTTP/1.1 request parser
- Request/response abstraction
- Router with exact route matching
- Composable middleware pipeline with `next()` pattern
- Thread pool for concurrent connections
- Graceful shutdown via POSIX signal handling
- Static file serving with MIME type detection and path traversal protection
- SQLite database integration with RAII wrappers, transactions, and connection pooling
- JWT-based authentication _(planned)_
- epoll-based async event loop _(planned)_

---

## Roadmap

### v0.1 — Foundation ✅ Complete

| Phase   | Description                                        | Status  |
| ------- | -------------------------------------------------- | ------- |
| Phase 0 | Toolchain setup, TCP echo server, POSIX socket API | ✅ Done |
| Phase 1 | HTTP/1.1 request parser                            | ✅ Done |
| Phase 2 | HttpRequest / HttpResponse abstractions            | ✅ Done |
| Phase 3 | Router with exact route matching                   | ✅ Done |

### v0.2 — Framework ✅ Complete

| Phase   | Description                               | Status  |
| ------- | ----------------------------------------- | ------- |
| Phase 4 | Middleware pipeline with `next()` pattern | ✅ Done |
| Phase 5 | Static file serving, MIME types           | ✅ Done |
| Phase 6 | Thread pool, mutex, graceful shutdown     | ✅ Done |

### v0.3 — Data & Auth 🔄 In Progress

| Phase   | Description                                                | Status     |
| ------- | ---------------------------------------------------------- | ---------- |
| Phase 7 | SQLite integration, parameterized queries, connection pool | ✅ Done    |
| Phase 8 | Password hashing, JWT generation and verification          | 🔲 Planned |

### v0.4 — Scale

| Phase    | Description                                        | Status     |
| -------- | -------------------------------------------------- | ---------- |
| Phase 9  | epoll-based async event loop, non-blocking sockets | 🔲 Planned |
| Phase 10 | HTTP keep-alive, rate limiting, benchmarking       | 🔲 Planned |

---

## Architecture

```
[Client]
    ↓
[TCP Socket Layer]        ← POSIX socket fd
    ↓
[HTTP Parser]             ← bytes → HttpRequest
    ↓
[Middleware Pipeline]     ← ordered chain, next() pattern
    ↓
[Router]                  ← method + path → handler
    ↓
[Handler]                 ← business logic
    ↓
[HttpResponse]            ← status + headers + body
    ↓
[Serializer]              ← response → bytes
    ↓
[TCP Send]                ← back to client
```

---

## What's Built So Far

### TCP Server

Raw POSIX socket server that accepts connections, reads incoming bytes, and sends responses back. Uses `select()` with a 1-second timeout on the accept loop so the server can respond to shutdown signals without blocking forever.

### HTTP Parser

Parses raw HTTP/1.1 request bytes into a structured `HttpRequest` object with method, path, version, and headers.

### Router

Maps URL paths to handler functions. Returns a 404 response automatically for unregistered routes.

```cpp
router.addRoute("/", homeHandler);
router.addRoute("/hello", helloHandler);
```

### Middleware Pipeline

Composable middleware chain where each middleware receives the request, response, and a `next()` function to pass control forward.

```cpp
server.use([](HttpRequest& req, HttpResponse& res, std::function<void()> next) {
    std::cout << "Request: " << req.path << "\n";
    next();
});
```

The router sits at the end of the pipeline as the final middleware — wired once in the `TcpServer` constructor.

### Thread Pool

Fixed-size pool of worker threads that handle client connections concurrently. Workers sleep on a condition variable and wake when a task is enqueued. The queue is protected by a mutex. The destructor drains all remaining tasks before joining threads — no requests are dropped on shutdown.

```cpp
thread_pool.enqueue([this, client_socket]() {
    handleClient(client_socket);
});
```

### Graceful Shutdown

POSIX signal handling (`SIGINT`, `SIGTERM`) sets an `atomic<bool>` flag and closes the server socket. The accept loop checks the flag on every iteration via `select()` timeout, exits cleanly, and waits for in-flight requests to finish before the process terminates.

```
Ctrl+C
  → running = false, server_fd closed
  → select() returns, loop exits
  → ThreadPool destructor drains queue
  → worker threads joined
  → process exits cleanly
```

### Static File Serving

Serves files from the `public/` directory. Automatically detects the correct `Content-Type` header based on file extension. Includes path traversal protection — requests containing `..` that escape the document root are rejected with `403 Forbidden`.

```
GET /index.html  →  serves public/index.html with Content-Type: text/html
GET /style.css   →  serves public/style.css  with Content-Type: text/css
GET /../../../../etc/passwd  →  403 Forbidden
```

Supported MIME types: `text/html`, `text/css`, `application/javascript`, `application/json`, `image/png`, `image/jpeg`, `image/x-icon`, `text/plain`.

Static file serving is implemented as middleware, so it runs before the router. If no matching file is found, the request falls through to the router normally.

### Database Layer

Full SQLite integration built from scratch using RAII ownership at every level.

**`Database`** — owns a `sqlite3*` connection. Opens on construction, closes on destruction. Non-copyable. Exposes `execute()` for statements with no results and `prepare()` for queries that return rows.

**`Statement`** — owns a `sqlite3_stmt*`. RAII wrapper around `sqlite3_finalize()`. Non-copyable, movable. Supports typed `bind()` overloads for `int` and `std::string`, `step()` to advance the cursor, `currentRow()` to read one row, and `fetchAll()` to collect all rows into a `ResultSet`.

```cpp
using Row = std::unordered_map<std::string, std::string>;
using ResultSet = std::vector<Row>;
```

**`Transaction`** — holds a `Database&` and executes `BEGIN TRANSACTION` on construction. `commit()` commits, `rollback()` rolls back. The destructor automatically rolls back if neither was called — so exceptions never leave the database in a partial state.

```cpp
Transaction tx(db);
db.execute("INSERT INTO users (name) VALUES ('Ahmed');");
tx.commit();
// if an exception fires before commit(), destructor rolls back automatically
```

**`ConnectionPool`** — creates N `Database` connections at startup and manages concurrent access. `acquire()` blocks the calling thread until a connection is available, then returns a `ConnectionGuard`. `release()` returns the connection and wakes one waiting thread via `condition_variable`.

**`ConnectionGuard`** — RAII wrapper returned by `acquire()`. Holds a borrowed `Database*` and a reference to the pool. Calls `pool.release()` automatically in its destructor — the caller never manages this manually.

```cpp
ConnectionPool pool("app.db", 4);

{
    ConnectionGuard guard = pool.acquire();
    Database& db = guard.get();
    db.execute("INSERT INTO logs (msg) VALUES ('hit');");
} // release() called here automatically
```

The pool is safe for concurrent use — `acquire()` and `release()` are both protected by a `mutex`, and threads that find no available connection sleep on a `condition_variable` rather than busy-waiting.

---

## Folder Structure

```
kernel-web-framework/
├── CMakeLists.txt
├── README.md
├── public/                        ← static files served here
├── include/
│   ├── server/
│   │   └── TcpServer.hpp
│   ├── http/
│   │   ├── HttpRequest.hpp
│   │   ├── HttpResponse.hpp
│   │   └── HttpParser.hpp
│   ├── router/
│   │   └── Router.hpp
│   ├── middleware/
│   │   └── MiddlewarePipeline.hpp
│   ├── static/
│   │   └── StaticFileHandler.hpp
│   ├── threading/
│   │   └── ThreadPool.hpp
│   └── database/
│       ├── Database.hpp
│       ├── Statement.hpp
│       ├── Transaction.hpp
│       ├── ConnectionPool.hpp
│       └── ConnectionGuard.hpp
├── src/
│   ├── main.cpp
│   ├── server/
│   │   └── TcpServer.cpp
│   ├── http/
│   │   ├── HttpRequest.cpp
│   │   ├── HttpResponse.cpp
│   │   └── HttpParser.cpp
│   ├── router/
│   │   └── Router.cpp
│   ├── middleware/
│   │   └── MiddlewarePipeline.cpp
│   ├── static/
│   │   └── StaticFileHandler.cpp
│   ├── threading/
│   │   └── ThreadPool.cpp
│   └── database/
│       ├── Database.cpp
│       ├── Statement.cpp
│       ├── Transaction.cpp
│       ├── ConnectionPool.cpp
│       └── ConnectionGuard.cpp
```

---

## Build

**Requirements**

- Linux (any distro) — or macOS
- GCC 11+ or Clang 14+
- CMake 3.16+
- libsqlite3-dev

**Install dependencies**

```bash
# Debian / Ubuntu
sudo apt install -y build-essential cmake libsqlite3-dev

# Arch Linux
sudo pacman -S base-devel cmake sqlite
```

> **Not on Linux?** Here are your options:
>
> - **Docker (easiest)** — works on any OS, no setup needed:
>   ```bash
>   docker run -it --rm -v $(pwd):/app -w /app gcc:latest bash
>   ```
>   Then build normally from inside the container
> - **Windows** — use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) (Windows Subsystem for Linux)
> - **macOS** — works natively with Clang via Xcode Command Line Tools: `xcode-select --install`
> - **Any OS** — spin up a Linux VM with [VirtualBox](https://www.virtualbox.org/) or a cloud VM (AWS, DigitalOcean, etc.)

**Build**

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

**Run**

Always run from the project root so the `public/` folder is reachable:

```bash
cd ~/projects/kernel-web-framework
./build/server
# Server listening on port 8080...
```

**Test static files**

```bash
curl http://localhost:8080/index.html
```

**Test concurrent requests**

```bash
for i in $(seq 1 100); do
    curl http://localhost:8080/ &
done
wait
```

**Graceful shutdown**

```bash
# Press Ctrl+C while the server is running
# The server will finish in-flight requests, then exit cleanly
```

---

## Version History

| Version | Status         | Description                                                 |
| ------- | -------------- | ----------------------------------------------------------- |
| v0.1    | ✅ Complete    | TCP server + HTTP parser + router                           |
| v0.2    | ✅ Complete    | Middleware + thread pool + graceful shutdown + static files |
| v0.3    | 🔄 In progress | Database layer + connection pool                            |
| v0.4    | 🔲 Planned     | epoll event loop + performance                              |
| v0.5    | 🔲 Future      | TLS + HTTP/2                                                |

---

## Author

**Ahmed Gaber** — Backend Developer (Python | Django | C++)
