#ifndef _WEB_WEBSOCKET_FRAME_H_
#define _WEB_WEBSOCKET_FRAME_H_

#include <FruitBowl.h>

namespace Web {
namespace WebSocket {

class Frame {
public:
  Frame();
  ~Frame();

  Result decode(const uint8_t * begin, size_t length);

private:
  Result decode(const uint8_t c);

  enum class DecodeState_t : uint8_t {
    HEADER_OP_CODE,
    HEADER_PAYLOAD_LEN,
    HEADER_PAYLOAD_LEN_EXT,
    HEADER_PAYLOAD_MASK_KEY,
    DATA,
    COMPLETE
  };

  DecodeState_t state = DecodeState_t::HEADER_OP_CODE;

  enum class OpCode_t : uint8_t {
    CONTINUATION = 0x00,
    TEXT         = 0x01,
    BINARY       = 0x02,
    PING         = 0x09,
    PONG         = 0x0A,
  };

  OpCode_t    code = OpCode_t::CONTINUATION;
  uint64_t    payloadLength;
  uint32_t    maskingKey;
  std::string data;
};

} // namespace WebSocket
} // namespace Web

#endif /* _WEB_WEBSOCKET_FRAME_H_ */