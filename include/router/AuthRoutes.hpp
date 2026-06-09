#pragma once

#include "Router.hpp"
#include "../database/ConnectionPool.hpp"

void registerAuthRoutes(Router& router, ConnectionPool& pool);
