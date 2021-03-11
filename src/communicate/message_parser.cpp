#include "communicate/message.h"
#include <stdexcept>
#include <string>

namespace Net {

void MessageParser::parse(const void *buf, size_t size) {

  // push all data in buff into queue named buffer
  const char *c_buf = (const char *)buf;
  for (int i = 0; i < size; i++) {
    buffer.push(c_buf[i]);
  }

  // process the data byte by byte until the queue is empty
  while(!buffer.empty()) {

    // if there is no message concentrated then create new message
    if (temp_msg == nullptr) {
      temp_msg = std::make_shared<Message>();
    }
    if (head_state < 0) {
      locate_head();
    } else if (option_state < 0) {
      locate_option();
    } else if (body_state != 0) {
      locate_body();
    }
    else if (tail_state < 0) {
      locate_tail();
    }

    // tail processing done
    if(!tail_state){
      // release the message concentrated
      msgs.push(temp_msg);
      temp_msg = nullptr;
      // reset the state of the parser
      reset_state();
    }
  }
}

void MessageParser::locate_head() {

  while (!buffer.empty()) {
    char c = buffer.front();
    buffer.pop();
    temp_buffer.push_back(c);
    if (head_state == -5) {
      if (c == 'M')
        head_state++;
      continue;
    }
    if (head_state == -4) {
      if (c != 'S') {
        reset_state();
      } else
        head_state++;
      continue;
    }
    if (head_state == -3) {
      if (c != 'G') {
        reset_state();
      } else
        head_state++;
      continue;
    }
    if (head_state == -2) {
      if (c == '\r') {
        head_state++;
      }
      continue;
    }

    if (head_state == -1) {
      if (c != '\n') {
        reset_state();
      } else {
        head_state++;
        if (!factory.decodeMessageHead(temp_buffer, *temp_msg)) {
          reset_state();
        }
        temp_buffer.clear();
        break;
      }
    }
  }
}

void MessageParser::locate_option() { 
  while(!buffer.empty()) {
    char c = buffer.front();
    buffer.pop();
    temp_buffer.push_back(c);
    if(option_state == -4) {
      if(c == '\"')
        option_state++;
      else if(c == '\r')
        option_state = -1;
      else if (c > 47 && c < 58)
        option_state++;
      continue;
    }
    if(option_state == -3) {
      if(c == '\r'){
        option_state++;
      }
      continue;
    }
    if(option_state == -2) {
      if(c == '\n'){
        option_state = -4;
      }
      else{
        reset_state();
        break;
      }
      continue;
    }
    if(option_state == -1) {
      if(c != '\n'){
        reset_state();
        break;
      }
      else{
        if(!factory.decodeMessageOption(temp_buffer, *temp_msg)) {
          reset_state();
          break;
        }
        option_state++;
        temp_buffer.clear();
        break;
      }
    }
  }
}

void MessageParser::locate_body() {
  while(!buffer.empty()) {
    char c = buffer.front();
    buffer.pop();
    temp_buffer.push_back(c);
    if(body_state == -1) {
      string s_size;
      try {
        s_size = temp_msg->getValue("size");
      } catch(std::runtime_error e) {
        reset_state();
        break;
      }
      body_state = std::stoul(s_size) + 2;
    }
    if(body_state > 1) {
      body_state--;
      continue;
    }
    if(body_state == 2) {
      if(c == '\r'){
        body_state--;
        continue;
      }
      else{
        reset_state();
        break;
      }
    }
    if(body_state == 1) {
      if(c != '\n'){
        reset_state();
        break;
      }
      else{
        if(!factory.decodeMessageBody(temp_buffer, *temp_msg)) {
          reset_state();
          break;
        }
        temp_buffer.clear();
        body_state--;
        break;
      }
    }
  }
}

void MessageParser::locate_tail() {
  while(!buffer.empty()) {
    char c = buffer.front();
    buffer.pop();
    temp_buffer.push_back(c);
    if(tail_state == -4) {
      if(c == 'G')
        tail_state++;
      else{
        reset_state();
        break;
      }
      continue;
    }
    if(tail_state == -3) {
      if(c == 'S')
        tail_state++;
      else{
        reset_state();
        break;
      }
      continue;
    }
    if(tail_state == -2) {
      if(c == 'M')
        tail_state++;
      else{
        reset_state();
        break;
      }
    }
    if(tail_state == -1) {
      if(!factory.decodeMessageTail(temp_buffer, *temp_msg)) {
        reset_state();
        break;
      }
      tail_state++;
      temp_buffer.clear();
      break;
    }
  }
}

void MessageParser::reset_state() {
  temp_buffer.clear();
  // reset the message concentrated
  if(temp_msg != nullptr)
    temp_msg->clear();
  // reset the state recorder
  head_state = -5;
  option_state = -4;
  body_state = -1;
  tail_state = -4;
}

shared_ptr<Message> MessageParser::getMessage() {
  shared_ptr<Message> p_msg = nullptr;
  if(!msgs.empty()){
    p_msg = msgs.front();
    msgs.pop();
  }
  return p_msg;
}

size_t MessageParser::getMessageCount() {
  return msgs.size();
}

} // namespace Net
