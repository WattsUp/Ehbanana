#ifndef _EHBANANA_ADD_OUTPUT_H_
#define _EHBANANA_ADD_OUTPUT_H_

#include "Ehbanana.h"

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const uint8_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::UINT8, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const int8_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::INT8, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const uint16_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::UINT16, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const int16_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::INT16, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const uint32_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::UINT32, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const int32_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::INT32, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const uint64_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::UINT64, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const int64_t value) {
  return EBAddOutput(output, id, property, EBValueType_t::INT64, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const float value) {
  return EBAddOutput(output, id, property, EBValueType_t::FLOAT, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const double value) {
  return EBAddOutput(output, id, property, EBValueType_t::DOUBLE, &value);
}

inline EBError_t EBAddOutput(EBOutput_t output, const char * id,
    const char * property, const char * value) {
  return EBAddOutput(output, id, property, EBValueType_t::CSTRING, value);
}

template <typename T>
void EBAddOutputEx(
    EBOutput_t output, const char * id, const char * property, const T value) {
  EBError_t error = EBAddOutput(output, id, property, value);
  if (EB_FAILED(error))
    throw std::exception(EBErrorName(error));
}

#endif /* _EHBANANA_ADD_OUTPUT_H_ */