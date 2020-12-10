#include "communicate/message.h"

namespace Net {

Option::Option(const string &key, const string &value)
    : key(std::make_shared<string>(key)), value(std::make_shared<string>(value)) {}

const string &Option::getKey() const { return *key; }

const string &Option::getValue() const { return *value; }

void Option::updateValue(const string &value) { *this->value = value; }

Option::Option(const Option &o) {
  if(this == &o) return;
  key = o.key;
  value = o.value;
}

Option::Option(Option &&o) noexcept {
  if(this == &o) return;
  key = o.key;
  value = o.value;
}

Option& Option::operator=(const Option &o) {
  if(this == &o) return *this;
  key = o.key;
  value = o.value;
  return *this;
}

Option& Option::operator=(Option &&o) noexcept {
  if(this == &o) return *this;
  key = o.key;
  value = o.value;
  return *this;
}

} // namespace Net
