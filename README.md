# kernel-web-framework

> A backend web framework built from scratch in C++.

---

## What Is This?

kernel-web-framework is a backend web framework written in C++ from the ground up — no dependencies, no abstractions borrowed from existing frameworks.

Every layer is implemented manually:

- Raw TCP server using POSIX sockets
- HTTP/1.1 request parser
- Request/response abstraction
- Router with parameterized routes
- Composable middleware pipeline
- Static file serving and templating
- Thread pool for concurrent connections
- SQLite database integration with connection pooling
- JWT-based authentication
- epoll-based async event loop

---

## Roadmap

### v0.1 — Foundation
| Phase | Description |
|---|---|
| Phase 0 | Toolchain setup, TCP echo server, POSIX socket API |
| Phase 1 | HTTP/1.1 request parser |
| Phase 2 | HttpRequest / HttpResponse abstractions |
| Phase 3 | Router with exact and parameterized route matching |

### v0.2 — Framework
| Phase | Description |
|---|---|
| Phase 4 | Middleware pipeline with `next()` pattern |
| Phase 5 | Static file serving, MIME types, template engine |
| Phase 6 | Thread pool, mutex, graceful shutdown |

### v0.3 — Data & Auth
| Phase | Description |
|---|---|
| Phase 7 | SQLite integration, parameterized queries, connection pool |
| Phase 8 | Password hashing, JWT generation and verification |

### v0.4 — Scale
| Phase | Description |
|---|---|
| Phase 9 | epoll-based async event loop, non-blocking sockets |
| Phase 10 | HTTP keep-alive, rate limiting, benchmarking |

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

## Folder Structure

```
kernel-web-framework/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── server/
│   │   ├── Server.hpp
│   │   └── Server.cpp
│   ├── http/
│   │   ├── HttpRequest.hpp
│   │   ├── HttpRequest.cpp
│   │   ├── HttpResponse.hpp
│   │   ├── HttpResponse.cpp
│   │   └── HttpParser.cpp
│   ├── router/
│   │   ├── Router.hpp
│   │   └── Router.cpp
│   ├── middleware/
│   │   └── Middleware.hpp
│   ├── db/
│   │   └── Database.hpp
│   ├── auth/
│   │   └── Auth.hpp
│   └── utils/
│       └── StringUtils.hpp
├── tests/
│   └── test_http_parser.cpp
├── public/
└── templates/
```

---

## Build

**Requirements**
- Linux (Ubuntu 22.04+)
- GCC 11+ or Clang 14+
- CMake 3.16+
- GDB, Valgrind
- libsqlite3-dev, libssl-dev

**Install dependencies**
```bash
sudo apt update
sudo apt install -y build-essential cmake gdb valgrind libsqlite3-dev libssl-dev
```

**Build**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

**Run**
```bash
./server
# Listening on http://localhost:8080
```

---

## Version History

| Version | Status | Description |
|---|---|---|
| v0.1 | 🔲 In progress | TCP server + HTTP parser + router |
| v0.2 | 🔲 Planned | Middleware + threading + templating |
| v0.3 | 🔲 Planned | Database + authentication |
| v0.4 | 🔲 Planned | epoll event loop + performance |
| v0.5 | 🔲 Future | TLS + HTTP/2 |

---

## Author

**Ahmed Gaber** — Backend Developer (Python | Django | C++)
