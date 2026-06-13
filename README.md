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
- JWT-based authentication with HMAC-SHA256 and PBKDF2 password hashing
- epoll-based async event loop with non-blocking sockets
- HTTP keep-alive with idle connection timeout sweep

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

### v0.3 — Data & Auth ✅ Complete

| Phase   | Description                                                | Status  |
| ------- | ---------------------------------------------------------- | ------- |
| Phase 7 | SQLite integration, parameterized queries, connection pool | ✅ Done |
| Phase 8 | Password hashing, JWT generation and verification          | ✅ Done |

### v0.4 — Scale ✅ Complete

| Phase    | Description                                        | Status  |
| -------- | -------------------------------------------------- | ------- |
| Phase 9  | epoll-based async event loop, non-blocking sockets | ✅ Done |
| Phase 10 | HTTP keep-alive with idle timeout sweep            | ✅ Done |

---

## Architecture

```[Client]
    ↓
[TCP Socket Layer]        ← POSIX socket fd
    ↓
[epoll Event Loop]        ← non-blocking I/O, idle timeout sweep
    ↓
[HTTP Parser]             ← bytes → HttpRequest
    ↓
[Middleware Pipeline]     ← ordered chain, next() pattern
    ↓
[JWT Middleware]          ← verifies Bearer token, 401 if invalid
    ↓
[Router]                  ← method + path → handler
    ↓
[Handler]                 ← business logic
    ↓
[HttpResponse]            ← status + headers + body
    ↓
[Serializer]              ← response → bytes
    ↓
[TCP Send]                ← back to client (connection reused if keep-alive)
```

---

## What's Built

### TCP Server

Raw POSIX socket server that accepts connections, reads incoming bytes, and sends responses back. The server socket is set to non-blocking mode and registered with epoll — new connections are accepted in a tight loop until `EAGAIN` signals no more are pending.

### HTTP Parser

Parses raw HTTP/1.1 request bytes into a structured `HttpRequest` object with method, path, version, headers, and body. Body is read based on the `Content-Length` header.

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

POSIX signal handling (`SIGINT`, `SIGTERM`) stops the epoll event loop and closes the server socket. The thread pool destructor drains all remaining tasks and joins worker threads before the process exits.

```
Ctrl+C
  → loop_.stop(), server_fd closed
  → epoll loop exits
  → ThreadPool destructor drains queue
  → worker threads joined
  → process exits cleanly
```

### epoll Event Loop

Non-blocking I/O using Linux's `epoll`. The server fd and all client fds are registered with `epoll_ctl`. `epoll_wait` wakes up with a 1-second timeout to run the idle sweep even when no events are pending. Each readable fd dispatches to its registered handler.

```
epoll_wait (1s timeout)
  → server fd readable → acceptClients()
  → client fd readable → removeFd, enqueue handleClient
  → timeout          → sweepIdleConnections()
```

Tracks which fds are already registered via an `unordered_set` to correctly use `EPOLL_CTL_MOD` vs `EPOLL_CTL_ADD` on re-registration.

### HTTP Keep-Alive

HTTP/1.1 connections are kept alive by default. After sending a response, the client fd is re-registered with epoll to wait for the next request on the same connection. Connections are only closed if the client explicitly sends `Connection: close`.

An idle timeout sweep runs every epoll cycle. Each fd's last-activity timestamp is tracked in an `unordered_map`. Any client fd idle for more than 30 seconds is closed automatically. The server fd is excluded from the sweep.

```
Connection: keep-alive  →  fd re-registered with epoll, reused for next request
Connection: close       →  fd closed after response
idle > 30s              →  fd closed by sweep
```

### Static File Serving

Serves files from the `public/` directory. Automatically detects the correct `Content-Type` header based on file extension. Includes path traversal protection — requests that escape the document root are rejected with `403 Forbidden`.

```
GET /index.html              →  serves public/index.html with Content-Type: text/html
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

### Auth Layer

Password hashing and JWT-based authentication built from scratch using OpenSSL's low-level C API — no auth libraries.

**`PasswordHasher`** — hashes passwords using PBKDF2-HMAC-SHA256 with a random 16-byte salt and 10,000 iterations. Stores the result as `salt$hash` in hex. `verify()` recovers the salt, re-runs PBKDF2, and compares — the raw password is never stored anywhere.

```cpp
std::string stored = PasswordHasher::hash("mypassword");
bool valid   = PasswordHasher::verify("mypassword", stored); // true
bool invalid = PasswordHasher::verify("wrong", stored);      // false
```

**`Base64`** — manual Base64url encode/decode. Operates on raw bytes, extracts 6-bit groups via bitwise arithmetic, maps them to the 64-character alphabet. Applies Base64url substitutions (`+` → `-`, `/` → `_`, no `=` padding) so output is safe in HTTP headers and URLs.

**`Hmac`** — computes HMAC-SHA256 via OpenSSL's `HMAC()` and returns the result Base64url-encoded. Used by `JwtService` to sign and verify tokens.

**`JwtService`** — issues and verifies JWTs manually. A token is three Base64url-encoded parts joined by dots: `header.payload.signature`. The signature is `HMAC-SHA256(header.payload, secret)` — impossible to forge without the secret key. Tokens carry a `sub` claim (user id) and an `exp` claim (Unix timestamp), and expire after 1 hour.

```cpp
std::string token   = JwtService::issue("42", secret);
std::string user_id = JwtService::verify(token, secret); // "42"
// throws std::runtime_error if signature is invalid or token is expired
```

**`JwtMiddleware`** — wraps `JwtService` as a middleware. Reads the `Authorization: Bearer <token>` header, verifies the token, and either calls `next()` or short-circuits with `401 Unauthorized`. Applied per-route by adding it to the pipeline before the router.

```cpp
server.use(JwtMiddleware::create(secret));
```

---

## Folder Structure

```
kernel-web-framework/
├── CMakeLists.txt
├── README.md
├── public/                        ← static files served here
├── include/
│   ├── server/
│   │   ├── TcpServer.hpp
│   │   └── EventLoop.hpp
│   ├── http/
│   │   ├── HttpRequest.hpp
│   │   ├── HttpResponse.hpp
│   │   └── HttpParser.hpp
│   ├── router/
│   │   ├── Router.hpp
│   │   ├── HomeRoutes.hpp
│   │   └── AuthRoutes.hpp
│   ├── middleware/
│   │   ├── Middleware.hpp
│   │   ├── MiddlewarePipeline.hpp
│   │   ├── Logger.hpp
│   │   ├── RequestTimer.hpp
│   │   └── JwtMiddleware.hpp
│   ├── static/
│   │   └── StaticFileHandler.hpp
│   ├── threading/
│   │   └── ThreadPool.hpp
│   ├── database/
│   │   ├── Database.hpp
│   │   ├── Statement.hpp
│   │   ├── Transaction.hpp
│   │   ├── ConnectionPool.hpp
│   │   └── ConnectionGuard.hpp
│   ├── auth/
│   │   ├── PasswordHasher.hpp
│   │   └── JwtService.hpp
│   └── utils/
│       ├── Base64.hpp
│       ├── Hmac.hpp
│       └── JsonParser.hpp
├── src/
│   ├── main.cpp
│   ├── server/
│   │   ├── TcpServer.cpp
│   │   ├── EventLoop.cpp
│   │   ├── EpollEventLoop.cpp
│   │   └── ClientHandler.cpp
│   ├── http/
│   │   ├── HttpRequest.cpp
│   │   ├── HttpResponse.cpp
│   │   └── HttpParser.cpp
│   ├── router/
│   │   ├── Router.cpp
│   │   ├── HomeRoutes.cpp
│   │   └── AuthRoutes.cpp
│   ├── middleware/
│   │   ├── MiddlewarePipeline.cpp
│   │   ├── Logger.cpp
│   │   ├── RequestTimer.cpp
│   │   └── JwtMiddleware.cpp
│   ├── static/
│   │   └── StaticFileHandler.cpp
│   ├── threading/
│   │   └── ThreadPool.cpp
│   ├── database/
│   │   ├── Database.cpp
│   │   ├── Statement.cpp
│   │   ├── Transaction.cpp
│   │   ├── ConnectionPool.cpp
│   │   └── ConnectionGuard.cpp
│   ├── auth/
│   │   ├── PasswordHasher.cpp
│   │   └── JwtService.cpp
│   └── utils/
│       ├── Base64.cpp
│       ├── Hmac.cpp
│       └── JsonParser.cpp
```

---

## Build

**Requirements**

- Linux (any distro) — or macOS
- GCC 11+ or Clang 14+
- CMake 3.16+
- libsqlite3-dev
- libssl-dev

**Install dependencies**

```bash
# Debian / Ubuntu
sudo apt install -y build-essential cmake libsqlite3-dev libssl-dev

# Arch Linux
sudo pacman -S base-devel cmake sqlite openssl
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

**Test auth**

```bash
# register a user
curl -X POST http://localhost:8080/register \
  -H "Content-Type: application/json" \
  -d '{"username":"ahmed","password":"mypassword"}'

# get a token
TOKEN=$(curl -s -X POST http://localhost:8080/login \
  -H "Content-Type: application/json" \
  -d '{"username":"ahmed","password":"mypassword"}' | jq -r .token)

# use it on a protected route
curl http://localhost:8080/protected \
  -H "Authorization: Bearer $TOKEN"
```

**Test keep-alive**

```bash
# two requests reusing the same connection
curl -v --keepalive-time 10 \
  http://localhost:8080/ \
  http://localhost:8080/hello
# look for: "Reusing existing connection" in curl output
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

| Version | Status      | Description                                                    |
| ------- | ----------- | -------------------------------------------------------------- |
| v0.1    | ✅ Complete | TCP server + HTTP parser + router                              |
| v0.2    | ✅ Complete | Middleware + thread pool + graceful shutdown + static files    |
| v0.3    | ✅ Complete | Database layer + connection pool + password hashing + JWT auth |
| v0.4    | ✅ Complete | epoll event loop + HTTP keep-alive + idle timeout sweep        |
| v0.5    | 🔲 Future   | TLS + HTTP/2                                                   |

---

## Author

**Ahmed Gaber** — Backend Developer (Python | Django | C++)
