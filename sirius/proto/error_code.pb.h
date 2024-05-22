// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: sirius/proto/error_code.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_sirius_2fproto_2ferror_5fcode_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_sirius_2fproto_2ferror_5fcode_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021007 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_sirius_2fproto_2ferror_5fcode_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_sirius_2fproto_2ferror_5fcode_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_sirius_2fproto_2ferror_5fcode_2eproto;
PROTOBUF_NAMESPACE_OPEN
PROTOBUF_NAMESPACE_CLOSE
namespace sirius {
namespace proto {

enum ErrCode : int {
  SUCCESS = 0,
  UNKNOWN_REQ_TYPE = 1,
  INTERNAL_ERROR = 2,
  NOT_LEADER = 3,
  RETRY_LATER = 4,
  EXEC_FAIL = 5,
  PEER_NOT_EQUAL = 6,
  PARSE_FROM_PB_FAIL = 11,
  PARSE_TO_PB_FAIL = 12,
  JSON_FROM_PB_FAIL = 13,
  PB_TO_JSON_FAIL = 14,
  INPUT_PARAM_ERROR = 21,
  HAVE_NOT_INIT = 22,
  CONFIG_EXISTS = 31,
  CONFIG_VERSION_OLD = 32,
  CONFIG_NOT_EXISTS = 33,
  CONFIG_NOT_EXISTS_VERSION = 34
};
bool ErrCode_IsValid(int value);
constexpr ErrCode ErrCode_MIN = SUCCESS;
constexpr ErrCode ErrCode_MAX = CONFIG_NOT_EXISTS_VERSION;
constexpr int ErrCode_ARRAYSIZE = ErrCode_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ErrCode_descriptor();
template<typename T>
inline const std::string& ErrCode_Name(T enum_t_value) {
  static_assert(::std::is_same<T, ErrCode>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function ErrCode_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    ErrCode_descriptor(), enum_t_value);
}
inline bool ErrCode_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, ErrCode* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<ErrCode>(
    ErrCode_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace proto
}  // namespace sirius

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::sirius::proto::ErrCode> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::sirius::proto::ErrCode>() {
  return ::sirius::proto::ErrCode_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_sirius_2fproto_2ferror_5fcode_2eproto