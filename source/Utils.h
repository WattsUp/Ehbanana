#ifndef _EHBANANA_UTILS_H_
#define _EHBANANA_UTILS_H_

#include <asio.hpp>
#include <chrono>

namespace asio {
namespace ip {
typedef tcp::acceptor acceptor_t;
typedef tcp::endpoint endpoint_t;
typedef tcp::socket   socket_t;
} // namespace ip
} // namespace asio

namespace Ehbanana {

typedef ::std::chrono::system_clock sysclk_t;

template <typename T> using timepoint_t = ::std::chrono::time_point<T>;

typedef ::std::chrono::milliseconds millis_t;
typedef ::std::chrono::seconds      seconds_t;

namespace Net = ::asio::ip;

/**
 * @brief Construct a string representation of an endpoint
 *
 * @param endpoint
 * @return std::string "[xxx.xxx.xxx.xxx]:[xxxxx]"
 */
inline ::std::string endpointStr(const Net::endpoint_t & endpoint) {
  return endpoint.address().to_string() + ":" +
         ::std::to_string(endpoint.port());
}

::std::string fileExtension(const ::std::string & filename);

} // namespace Ehbanana

#endif /* _EHBANANA_UTILS_H_ */