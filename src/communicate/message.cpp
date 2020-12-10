#include "communicate/message.h"
#include "communicate/commu_options.h"

#include <time.h>

namespace Net {

const string Message::msg_head = "MSG";
const string Message::msg_tail = "GSM";

void Message::addOption(const string &key, const string &value) {
  for (Option &option : options) {
    if (option.getKey() == key) {
      option.updateValue(value);
      return;
    }
  }
  options.push_back(Option(key, value));
}

const string &Message::getValue(const string &key) const {
  for (Option option : options) {
    if (option.getKey() == key) {
      return option.getValue();
    }
  }
  throw new std::runtime_error("key not found");
}

void Message::pushData(void *const buf, size_t size) {
  char *const c_buf = (char *const)buf;
  for (size_t i = 0; i < size; i++) {
    data.push_back(c_buf[i]);
  }

  addOption("size", std::to_string(data.size()));

}

void Message::clear() {
  this->data.clear();
  this->options.clear();
  this->tid = 0;
  this->type = 0x1;
}

void Message::setHead(int32_t tid, int16_t type) {
  this->tid = tid;
  this->type = type;
}

int32_t Message::getTID() const {
  return tid;
}

const vector<char> &Message::getData() const {
  return data;
}

Message::Message() noexcept {

  time_t t = time(nullptr);
  struct tm *p_tm = gmtime(&t);
  char buf[22];
  size_t w_bit = strftime(&buf[0], sizeof(buf), "%D %T %Z", p_tm);
  
  addOption("date", buf);

}
} // namespace Net
