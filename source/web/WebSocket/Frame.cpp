#include "Frame.h"

namespace Ehbanana {
namespace Web {
namespace WebSocket {

/**
 * @brief Construct a new Frame:: Frame object
 *
 */
Frame::Frame(std::shared_ptr<Ehbanana::MessageOut> message) :
  messageOut(message) {}

/**
 * @brief Destroy the Frame:: Frame object
 *
 */
Frame::~Frame() {}

/**
 * @brief Decode a frame from a character string and populate the appropriate
 * fields
 * String may contain a fragment of the frame, a fragment frame, or multiple
 * frames
 *
 * @param begin character
 * @param length of buffer
 * @return bool true if frame is completely decoded, false otherwise
 */
bool Frame::decode(const uint8_t *& begin, size_t & length) {
  while (length > 0 && state != DecodeState_t::COMPLETE) {
    decode(*begin);
    ++begin;
    --length;
  }

  if (state != DecodeState_t::COMPLETE)
    return false;

  return true;
}

/**
 * @brief Decode a character into the appropriate fields
 *
 * @param c character
 * @throw std::exception Thrown on failure
 */
void Frame::decode(const uint8_t c) {
  switch (state) {
    case DecodeState_t::HEADER_OP_CODE:
      fin = (c & 0x80) == 0x80;
      if ((c & 0xF) != static_cast<uint8_t>(Opcode_t::CONTINUATION)) {
        // Set the op code if not a continuation
        opcode = static_cast<Opcode_t>(c & 0x0F);
        if (opcode == Opcode_t::BINARY)
          throw std::exception("Binary is not supported");
      }
      state = DecodeState_t::HEADER_PAYLOAD_LEN;
      break;
    case DecodeState_t::HEADER_PAYLOAD_LEN:
      if ((c & 0x80) == 0)
        throw std::exception("WebSocket frame was not masked");
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
      string.push_back(static_cast<char>(c ^ key));
      maskingKey = (maskingKey << 8) | key; // Circularly shift to the next byte
      --payloadLength;
      if (payloadLength == 0) {
        if (fin)
          state = DecodeState_t::COMPLETE;
        else
          // This message is fragmented, add on to this one
          state = DecodeState_t::HEADER_OP_CODE;
      }
    } break;
    case DecodeState_t::COMPLETE:
    default:
      throw std::exception(("Invalid WebSocket state during RX: " +
                            std::to_string(static_cast<uint8_t>(state)))
                               .c_str());
  }
}

/**
 * @brief Add data to the frame
 *
 * @param string to append
 */
void Frame::addData(const std::string & data) {
  string += data;
}

/**
 * @brief Get the string of the frame
 *
 * @return const std::string&
 */
const std::string & Frame::getString() const {
  return string;
}

/**
 * @brief Set the opcode of the frame
 *
 * @param code
 */
void Frame::setOpcode(Opcode_t code) {
  opcode = code;
}

/**
 * @brief Get the opcode of the frame
 *
 * @return const Opcode_t
 */
const Opcode_t Frame::getOpcode() const {
  return opcode;
}

/**
 * @brief Get the next set of buffers ready to send
 *
 * @return std::vector<asio::const_buffer> buffers
 */
const std::vector<asio::const_buffer> & Frame::getBuffers() {
  // If the buffers vector is empty, populate first
  if (buffers.empty()) {
    header.push_back(0x80 | static_cast<uint8_t>(opcode)); // FIN = 1
    if (!string.empty())
      payloadLength = string.size();
    else if (messageOut != nullptr)
      payloadLength = messageOut->size();
    // No masking
    if (payloadLength < 126) {
      // 7b payloadLength
      header.push_back(static_cast<uint8_t>(payloadLength));
    } else if (payloadLength <= 0x10000) {
      header.push_back(126); // 16b payload length
      header.push_back(static_cast<uint8_t>((payloadLength >> 8) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 0) & 0xFF));
    } else if (payloadLength <= 0x100000000) {
      header.push_back(127); // 64b payload length
      header.push_back(static_cast<uint8_t>((payloadLength >> 56) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 48) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 40) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 32) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 24) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 16) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 8) & 0xFF));
      header.push_back(static_cast<uint8_t>((payloadLength >> 0) & 0xFF));
    } else
      throw std::exception("WebSocket payload larger than 4GB not supported");
    buffers.push_back(asio::buffer(header));
    if (!string.empty())
      buffers.push_back(asio::buffer(string));
    else if (messageOut != nullptr)
      buffers.push_back(asio::buffer(
          messageOut->getString(), static_cast<size_t>(payloadLength)));
  }
  return buffers;
}

} // namespace WebSocket
} // namespace Web
} // namespace Ehbanana