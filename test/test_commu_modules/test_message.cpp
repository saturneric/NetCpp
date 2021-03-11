#include <gtest/gtest.h>

#include "debug_tools/print_tools.h"
#include "communicate/message.h"

using namespace Net;

TEST(Option_Test, base_test_1) {
  Option op = Option("Size", "123");
  Option op2 = std::move(op);

  ASSERT_EQ(op.getKey(), "Size");
  ASSERT_EQ(op.getValue(), "123");

  ASSERT_EQ(op2.getKey(), "Size");
  ASSERT_EQ(op2.getValue(), "123");
}

TEST(Message_Test, base_test_1) {
  Message msg = Message();
  msg.setHead(1, 0x1);
  string buf = "hello world, wellcome";
  msg.pushData((void *const)buf.data(), buf.size());
  vector<char> rd;;
  MessageFactory mf;
  mf.encodeMessage(msg, rd);
  PrintTools::printInfoBuffer(rd, "Message Raw Data");

  for(auto &c : rd) {
    printf("%c", c);
  }
  printf("\n");

  ASSERT_TRUE(mf.decodeMessage(rd, msg));

  ASSERT_EQ(msg.getTID(), 1);

  printf("SIGN: %s\n", msg.getValue("sign").c_str());;
  printf("DATE: %s\n", msg.getValue("date").c_str());;
  printf("SIZE: %s\n", msg.getValue("size").c_str());;

  ASSERT_EQ(msg.getData()[20], 'e');
  ASSERT_EQ(msg.getData()[19], 'm');
  ASSERT_EQ(msg.getData()[0], 'h');

  ASSERT_TRUE(mf.checkMessageSign(msg));

}

TEST(MessageParser_Test, base_test_1) {
  Message msg;
  msg.setHead(0x2, 0x1);
  msg.addOption("abc", "def");
  char buf[] = "Hello WWWWWWWW!";

  msg.pushData(buf, sizeof(buf));
  MessageFactory mf;
  vector<char> rd;
  mf.encodeMessage(msg, rd);
  MessageParser mp;
  mp.parse(rd.data(), rd.size());

  ASSERT_EQ(mp.getMessageCount(), 1);

  shared_ptr<Message> p_msg = mp.getMessage();
  
  ASSERT_EQ(p_msg->getData().size(), msg.getData().size());
  ASSERT_EQ(p_msg->getTID(), msg.getTID());
  ASSERT_EQ(p_msg->getValue("abc"), "def");

  mp.parse(&rd[0], 8);
  mp.parse(&rd[8], 32);
  mp.parse(&rd[40], 100);
  mp.parse(&rd[140], 27);
  
  ASSERT_EQ(mp.getMessageCount(), 1);

  mp.parse(&rd[0], 14);
  mp.parse(&rd[14], 1);
  mp.parse(&rd[15], 34);
  mp.parse(&rd[49], 11);
  mp.parse(&rd[60], 107);
  
  ASSERT_EQ(mp.getMessageCount(), 2);

  p_msg = mp.getMessage();
  
  ASSERT_EQ(p_msg->getData().size(), msg.getData().size());
  ASSERT_EQ(p_msg->getTID(), msg.getTID());
  ASSERT_EQ(p_msg->getValue("abc"), "def");

  ASSERT_EQ(mp.getMessageCount(), 1);

  mp.parse(&rd[0], 15);
  mp.parse(&rd[16], 34);
  mp.parse(&rd[50], 10);
  mp.parse(&rd[60], 107);

  ASSERT_EQ(mp.getMessageCount(), 1);
  
  mp.parse(&rd[0], 16);
  mp.parse(&rd[16], 34);
  mp.parse(&rd[50], 10);
  mp.parse(&rd[60], 107);

  ASSERT_EQ(mp.getMessageCount(), 1);
}
