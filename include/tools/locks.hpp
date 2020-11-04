#pragma once

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

#define TOOLS_SHARED_LOCK(mutex) boost::shared_lock<decltype(mutex)> shared_lock_##mutex(mutex);
#define TOOLS_UNIQUE_LOCK(mutex) boost::unique_lock<decltype(mutex)> unique_lock_##mutex(mutex);