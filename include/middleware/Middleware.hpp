#pragma once

#include <functional>
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"

using Middleware = std::function<
    void(
        HttpRequest&,
        HttpResponse&,
        std::function<void()>
    )
>;
