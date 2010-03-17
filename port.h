#ifndef PORT_H
#define PORT_H

// tr1/boost/std function objects.
#ifdef USE_BOOST
#include <boost/function.hpp>
namespace port = boost;
#elif USE_TR1
#include <tr1/functional>
#include <tr1/memory>
namespace port = std::tr1;
#else
#include <functional>
#include <memory>
namespace port = std;
#endif

#endif
