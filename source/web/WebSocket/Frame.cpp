#include "Frame.h"

#include <spdlog/spdlog.h>

namespace Web {
namespace WebSocket {

/**
 * @brief Construct a new Frame:: Frame object
 *
 */
Frame::Frame() {}

/**
 * @brief Destroy the Frame:: Frame object
 *
 */
Frame::~Frame() {}

/**
 * @brief Decode a frame from a character string and populate the appropriate
 * fields
 *
 * @param begin character
 * @param length of buffer
 * @return Result error code
 */
Result Frame::decode(const uint8_t * begin, size_t length) {
  Result result;

  while (length > 0) {
    result = decode(*begin);
    if (!result)
      return result + "WebSocket frame decode";
    ++begin;
    --length;
  }

  if (state != DecodeState_t::COMPLETE)
    return ResultCode_t::INCOMPLETE;

  spdlog::debug("WebSocket Frame: \"{}\"", data);

  return ResultCode_t::SUCCESS;
}

/**
 * @brief Decode a character into the appropriate fields
 *
 * @param c character
 * @return Result error code
 */
Result Frame::decode(const uint8_t c) {
  switch (state) {
    case DecodeState_t::HEADER_OP_CODE:
      code  = static_cast<OpCode_t>(c & 0x0F);
      state = DecodeState_t::HEADER_PAYLOAD_LEN;
      break;
    case DecodeState_t::HEADER_PAYLOAD_LEN:
      if ((c & 0x80) == 0)
        return ResultCode_t::INVALID_DATA + "WebSocket Frame was not masked";
      // Shift until the FF is discard, add the next 4 bytes
      maskingKey = 0x000000FF;

      payloadLength = static_cast<uint64_t>(c & 0x7F);
      if (payloadLength == 126) {
        state = DecodeState_t::HEADER_PAYLOAD_LEN_EXT;
        // Shift until the FF is discard, add the next 2 bytes
        payloadLength = 0x00FF000000000000;
      } else if (payloadLength == 127) {
        state = DecodeState_t::HEADER_PAYLOAD_LEN_EXT;
        // Shift until the FF is discard, add the next 8 bytes
        payloadLength = 0x00000000000000FF;
      } else {
        state = DecodeState_t::HEADER_PAYLOAD_MASK_KEY;
      }
      break;
    case DecodeState_t::HEADER_PAYLOAD_LEN_EXT:
      if ((payloadLength >> 56) == 0xFF)
        state = DecodeState_t::HEADER_PAYLOAD_MASK_KEY;
      payloadLength = (payloadLength << 8) | static_cast<uint64_t>(c);
      break;
    case DecodeState_t::HEADER_PAYLOAD_MASK_KEY:
      if ((maskingKey >> 24) == 0xFF)
        state = DecodeState_t::DATA;
      maskingKey = (maskingKey << 8) | static_cast<uint32_t>(c);
      break;
    case DecodeState_t::DATA: {
      uint8_t key = maskingKey >> 24; // MSB
      data.push_back(static_cast<char>(c ^ key));
      maskingKey = (maskingKey << 8) | key; // Circularly shift to the next byte
      --payloadLength;
      if (payloadLength == 0) {
        if (code == OpCode_t::CONTINUATION)
          // This message is fragmented, add on to this one
          state = DecodeState_t::HEADER_OP_CODE;
        else
          state = DecodeState_t::COMPLETE;
      }
    } break;
    case DecodeState_t::COMPLETE:
    default:
      return ResultCode_t::INVALID_STATE +
             ("WebSocket frame decode: " +
                 std::to_string(static_cast<uint8_t>(state)));
  }
  return ResultCode_t::SUCCESS;
}

} // namespace WebSocket
} // namespace Web