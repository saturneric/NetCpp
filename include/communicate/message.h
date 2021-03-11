#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>
#include <memory>

#include "communicate/commu_options.h"
#include "utils/sha256_generator.h"

using std::string;
using std::vector;
using std::shared_ptr;
using std::queue;

namespace Net {

// the structure to store the option part of a message
class Option {
public:
  Option(const string &key, const string &value);

  Option(const Option &o);

  Option(Option &&o) noexcept;

  Option& operator=(const Option &o);

  Option& operator=(Option &&o) noexcept;

  const string &getKey() const;

  const string &getValue() const;

  void updateValue(const string &value);

private:
  // the key is unique within a message
  shared_ptr<string> key;
  shared_ptr<string> value;
};

class MessageFactory;

class Message {
public:
  
  Message() noexcept;

  void setHead(int32_t tid, int16_t type);

  void addOption(const string &key, const string &value);

  const string &getValue(const string &key) const;

  void pushData(void *const buf, size_t size);

  int32_t getTID() const;

  const vector<char> &getData() const;

  void clear();

private:
  friend MessageFactory;

  int32_t tid = -1;
  int16_t version = MSG_VERSION;
  int16_t type = 0x0;
  vector<Option> options = vector<Option>();
  vector<char> data = vector<char>();
  static const string msg_head;
  static const string msg_tail;

};

class MessageFactory {
public:
  bool encodeMessage(Message &msg, vector<char> &raw_data);

  bool decodeMessage(const vector<char> &raw_data, Message &msg);

  bool decodeMessageHead(const vector<char> &raw_data, Message &msg);

  bool decodeMessageOption(const vector<char> &raw_data, Message &msg);

  bool decodeMessageBody(const vector<char> &raw_data, Message &msg);

  bool decodeMessageTail(const vector<char> &raw_data, Message &msg); 

  bool checkMessageSign(const Message &msg);

private:
  void encode_head(const Message &msg, vector<char> &raw_data);

  void encode_options(const Message &msg, vector<char> &raw_data);

  void encode_body(const Message &msg, vector<char> &raw_data);

  void encode_tail(const Message &msg, vector<char> &raw_data);

  ssize_t decode_head(const vector<char> &raw_data, Message &msg);

  ssize_t decode_options(const vector<char> &raw_data, size_t offset, Message &msg);

  ssize_t decode_body(const vector<char> &raw_data, size_t offset, Message &msg);

  ssize_t decode_tail(const vector<char> &raw_Data, size_t offset, Message &msg);

  void calculate_options_hash(const vector<char> &raw_data, int32_t end_index, int16_t &sum_hash);

  SHA256Generator sign_gen;
};

class MessageParser {
public:
 
  // send a received buffer to parse
  void parse(const void *buf, size_t size);

  // Get a parsed message from the queue
  shared_ptr<Message> getMessage();

  // Get the count of the parsed meaasge
  size_t getMessageCount();

private:

  // state recorder of a special part of a message
  int head_state = -5;
  int option_state = -4;
  ssize_t body_state = -1;
  int tail_state = -4;

  // factory used to read data and form the message step by step
  MessageFactory factory;

  // store the message parsed
  queue<shared_ptr<Message>> msgs;
  // buffer to store the data give by 
  queue<char> buffer;
  // buffer to temporarily store the data of the certain part related to the state of the parser
  vector<char> temp_buffer;

  // temporarily record the message concentrated
  shared_ptr<Message> temp_msg; 
  
  // reset all state and refresh the parser
  void reset_state();

  void locate_head();

  void locate_option();

  void locate_body();

  void locate_tail();

};

} // namespace Net
