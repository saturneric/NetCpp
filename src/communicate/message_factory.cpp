#include "communicate/commu_options.h"
#include "communicate/message.h"

#include <stdexcept>
#include <stdio.h>

namespace Net {

bool MessageFactory::encodeMessage(Message &msg, vector<char> &raw_data) {

  sign_gen.setRawData(msg.getData());
  msg.addOption("sign", sign_gen.getHex());

  encode_head(msg, raw_data);
  encode_options(msg, raw_data);
  encode_body(msg, raw_data);
  encode_tail(msg, raw_data);

  return true;
}

void MessageFactory::encode_head(const Message &msg, vector<char> &raw_data) {

  raw_data.insert(raw_data.end(), msg.msg_head.cbegin(), msg.msg_head.cend());

  raw_data.push_back(' ');

  // 协议版本
  const string s_version = std::to_string(msg.version);

  raw_data.insert(raw_data.end(), s_version.cbegin(), s_version.cend());

  raw_data.push_back(' ');

  // 消息类型
  const string s_type = std::to_string(msg.type);

  raw_data.insert(raw_data.end(), s_type.cbegin(), s_type.cend());

  raw_data.push_back(' ');

  // 会话ID
  const string s_tid = std::to_string(msg.tid);

  raw_data.insert(raw_data.end(), s_tid.cbegin(), s_tid.cend());

  raw_data.push_back('\r');
  raw_data.push_back('\n');
}

void MessageFactory::encode_options(const Message &msg,
                                    vector<char> &raw_data) {

  for (auto &option : msg.options) {
    raw_data.push_back('\"');

    const string &key = option.getKey();

    raw_data.insert(raw_data.end(), key.cbegin(), key.cend());

    raw_data.push_back('\"');
    raw_data.push_back(':');
    raw_data.push_back('\"');

    const string &value = option.getValue();

    raw_data.insert(raw_data.end(), value.cbegin(), value.cend());

    raw_data.push_back('\"');
    raw_data.push_back('\r');
    raw_data.push_back('\n');
  }

  // 计算头部和配置部分的哈希值
  int16_t sum_hash;
  
  calculate_options_hash(raw_data, raw_data.size() - 1, sum_hash);
  const string sum_hash_str = std::to_string(sum_hash);
  raw_data.insert(raw_data.end(), sum_hash_str.cbegin(), sum_hash_str.cend());

  raw_data.push_back('\r');
  raw_data.push_back('\n');

  raw_data.push_back('\r');
  raw_data.push_back('\n');
}

void MessageFactory::encode_body(const Message &msg, vector<char> &raw_data) {
  raw_data.insert(raw_data.end(), msg.data.begin(), msg.data.end());
  raw_data.push_back('\r');
  raw_data.push_back('\n');
}

void MessageFactory::encode_tail(const Message &msg, vector<char> &raw_data) {
  raw_data.insert(raw_data.end(), msg.msg_tail.cbegin(), msg.msg_tail.cend());
}

ssize_t MessageFactory::decode_head(const vector<char> &raw_data,
                                    Message &msg) {
  size_t index = 0;
  for (auto &c : Message::msg_head) {
    if (raw_data[index++] != c)
      return -1;
  }

  if (raw_data[index++] != ' ')
    return -1;

  string s_version;
  char c;
  while ((c = raw_data[index++]) != ' ') {
    s_version.push_back(c);
  }

  int16_t version = std::stoi(s_version);

  if (version != MSG_VERSION)
    return -1;

  string s_type;
  while ((c = raw_data[index++]) != ' ') {
    s_type.push_back(c);
  }

  int16_t type = std::stoi(s_type);

  string s_tid;

  while ((c = raw_data[index++]) != '\r') {
    s_tid.push_back(c);
  }

  int32_t tid = std::stoi(s_tid);

  if (raw_data[index++] != '\n')
    return -1;

  msg.setHead(tid, type);

  return index;
}

ssize_t MessageFactory::decode_options(const vector<char> &raw_data,
                                       size_t offset, Message &msg) {
  size_t index = offset;
  char c;
  while (raw_data[index++] == '\"') {

    string key;

    while ((c = raw_data[index++]) != '\"') {
      key.push_back(c);
    }

    if (raw_data[index++] != ':') {
      return -1;
    }

    if (raw_data[index++] != '\"') {
      return -1;
    }

    string value;

    while ((c = raw_data[index++]) != '\"') {
      value.push_back(c);
    }

    if (raw_data[index++] != '\r' || raw_data[index++] != '\n')
      return -1;

    msg.addOption(key.c_str(), value.c_str());
  }

  int sum_hash_range = index - 2;

  --index;

  string sum_hash_str;
  while((c = raw_data[index++]) != '\r') {
    sum_hash_str.push_back(c);
  }

  if(raw_data[index++] != '\n') {
    return -1;
  }

  int16_t sum_hash = std::stoi(sum_hash_str);
  int16_t sum_hash_check;

  calculate_options_hash(raw_data, sum_hash_range, sum_hash_check);

  if(sum_hash_check != sum_hash) return -1;

  if (raw_data[index++] != '\r' || raw_data[index++] != '\n')
    return -1;

  return index;
}

ssize_t MessageFactory::decode_body(const vector<char> &raw_data, size_t offset,
                                    Message &msg) {
  size_t index = offset;
  int32_t size = 0;
  string s_size;
  try {
    s_size = msg.getValue("size");
  } catch (std::runtime_error e) {
    return -1;
  }

  size = std::stoi(s_size);

  msg.pushData((void *)&raw_data[index], size);

  index += size;

  if (raw_data[index++] != '\r' || raw_data[index++] != '\n')
    return -1;

  return index;
}

ssize_t MessageFactory::decode_tail(const vector<char> &raw_data, size_t offset,
                                    Message &msg) {
  size_t index = offset;
  for (auto &c : Message::msg_tail) {
    if (raw_data[index++] != c)
      return -1;
  }

  return index;
}

void MessageFactory::calculate_options_hash(const vector<char> &raw_data, int32_t end_index, int16_t &sum_hash) {

  if(end_index > raw_data.size()) {
    throw std::runtime_error("end index out of range");
  }

  sum_hash = 0;

  int32_t index = 0;

  for(auto &c : raw_data) {
    sum_hash += static_cast<int16_t>(c);
    sum_hash %= 65535;
    if (index++ >= end_index) break;
  }

}

bool MessageFactory::decodeMessage(const vector<char> &raw_data, Message &msg) {
  msg.clear();

  ssize_t offset = decode_head(raw_data, msg);
  if (!~offset)
    return false;
  offset = decode_options(raw_data, offset, msg);
  if (!~offset)
    return false;
  offset = decode_body(raw_data, offset, msg);
  if (!~offset)
    return false;
  offset = decode_tail(raw_data, offset, msg);
  if (!~offset)
    return false;
  else
    return true;
}

bool MessageFactory::checkMessageSign(const Message &msg) {
  string sign;

  try {
    sign = msg.getValue("sign");
  } catch (std::runtime_error e) {
    return false;
  }

  sign_gen.setRawData(msg.getData());
  return sign_gen.getHex() == sign;
}

bool MessageFactory::decodeMessageHead(const vector<char> &raw_data, Message &msg) {
  if(!~decode_head(raw_data, msg)) return false;
  else return true;
}

bool MessageFactory::decodeMessageOption(const vector<char> &raw_data, Message &msg) {
  if(!~decode_options(raw_data, 0, msg)) return false;
  else return true;
}

bool MessageFactory::decodeMessageBody(const vector<char> &raw_data, Message &msg) {
  if(!~decode_body(raw_data, 0, msg)) return false;
  return true;
}

bool MessageFactory::decodeMessageTail(const vector<char> &raw_data, Message &msg) {
  if(!~decode_tail(raw_data, 0, msg)) return false;
  return true;
}

} // namespace Net
