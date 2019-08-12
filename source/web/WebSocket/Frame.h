#ifndef _WEB_WEBSOCKET_FRAME_H_
#define _WEB_WEBSOCKET_FRAME_H_

#include <FruitBowl.h>
#include <asio.hpp>

namespace Web {
namespace WebSocket {

enum class Opcode_t : uint8_t {
  CONTINUATION = 0x00,
  TEXT         = 0x01,
  BINARY       = 0x02,
  CLOSE        = 0x08,
  PING         = 0x09,
  PONG         = 0x0A,
};

class Frame {
public:
  Frame();
  ~Frame();

  Result decode(const uint8_t * begin, size_t length);

  void setOpcode(Opcode_t code);

  const Opcode_t      getOpcode() const;
  const std::string & getData() const;
  asio::const_buffer  toBuffer();

  void addData(const std::string & string);

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

  std::vector<uint8_t> buffer;

  Opcode_t    opcode        = Opcode_t::CONTINUATION;
  uint64_t    payloadLength = 0;
  uint32_t    maskingKey    = 0;
  bool        fin           = false;
  std::string data;
};

} // namespace WebSocket
} // namespace Web

#endif /* _WEB_WEBSOCKET_FRAME_H_ */