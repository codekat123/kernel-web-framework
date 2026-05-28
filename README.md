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
- Static file serving and templating _(planned)_
- Thread pool for concurrent connections _(planned)_
- SQLite database integration with connection pooling _(planned)_
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

### v0.2 — Framework 🔄 In Progress

| Phase   | Description                                      | Status     |
| ------- | ------------------------------------------------ | ---------- |
| Phase 4 | Middleware pipeline with `next()` pattern        | ✅ Done    |
| Phase 5 | Static file serving, MIME types, template engine | 🔲 Planned |
| Phase 6 | Thread pool, mutex, graceful shutdown            | 🔲 Planned |

### v0.3 — Data & Auth

| Phase   | Description                                                | Status     |
| ------- | ---------------------------------------------------------- | ---------- |
| Phase 7 | SQLite integration, parameterized queries, connection pool | 🔲 Planned |
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

Raw POSIX socket server that accepts connections, reads incoming bytes, and sends responses back. Single-threaded for now — one request at a time.

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
    next(); // pass to next middleware
});
```

The router itself sits at the end of the pipeline as the final middleware.

---

## Folder Structure

```
kernel-web-framework/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── server/
│   │   └── TcpServer.hpp
│   ├── http/
│   │   ├── HttpRequest.hpp
│   │   ├── HttpResponse.hpp
│   │   └── HttpParser.hpp
│   ├── router/
│   │   └── Router.hpp
│   └── middleware/
│       └── Middleware.hpp
├── src/
│   ├── main.cpp
│   ├── server/
│   │   └── TcpServer.cpp
│   ├── http/
│   │   ├── HttpRequest.cpp
│   │   ├── HttpResponse.cpp
│   │   └── HttpParser.cpp
│   └── router/
│       └── Router.cpp
```

---

## Build

**Requirements**

- Linux (any distro) — or macOS
- GCC 11+ or Clang 14+
- CMake 3.16+

> **Not on Linux?** Here are your options:
>
> - **Docker (easiest)** — works on any OS, no setup needed:
>   ```bash
>   docker run -it --rm -v $(pwd):/app -w /app gcc:latest bash
>   ```
>   Then build normally from inside the container
> - **Windows** — use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) (Windows Subsystem for Linux), it gives you a full Linux environment inside Windows
> - **macOS** — works natively, just make sure you have Clang installed via Xcode Command Line Tools: `xcode-select --install`
> - **Any OS** — spin up a Linux VM with [VirtualBox](https://www.virtualbox.org/) or use a cloud VM (AWS, DigitalOcean, etc.)

**Build**

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

**Run**

```bash
./server
# Server listening on port 8080...
```

---

## Version History

| Version | Status         | Description                                            |
| ------- | -------------- | ------------------------------------------------------ |
| v0.1    | ✅ Complete    | TCP server + HTTP parser + router                      |
| v0.2    | 🔄 In progress | Middleware pipeline done — threading & templating next |
| v0.3    | 🔲 Planned     | Database + authentication                              |
| v0.4    | 🔲 Planned     | epoll event loop + performance                         |
| v0.5    | 🔲 Future      | TLS + HTTP/2                                           |

---

## Author

**Ahmed Gaber** — Backend Developer (Python | Django | C++)
