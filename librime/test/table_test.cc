// vim: set sts=2 sw=2 et:
// encoding: utf-8
//
// Copyleft 2011 RIME Developers
// License: GPLv3
// 
// 2011-07-03 GONG Chen <chen.sst@gmail.com>
//
#include <gtest/gtest.h>
#include <rime/impl/table.h>

TEST(RimeTableTest, IntegrityTest) {
  const char file_name[] = "table_test.bin";
  rime::scoped_ptr<rime::Table> table;
  table.reset(new rime::Table(file_name));
  ASSERT_TRUE(table);
  table->Remove();

  rime::Syllabary syll;
  rime::Vocabulary voc;
  rime::DictEntry d;
  syll.insert("0");
  // no entries for '0', however
  syll.insert("1");
  d.code.push_back(1);
  d.text = "yi";
  d.weight = 1.0;
  voc[1].entries.push_back(d);
  syll.insert("2");
  d.code.back() = 2;
  d.text = "er";
  voc[2].entries.push_back(d);
  d.text = "liang";
  voc[2].entries.push_back(d);
  d.text = "lia";
  voc[2].entries.push_back(d);
  syll.insert("3");
  d.code.back() = 3;
  d.text = "san";
  voc[3].entries.push_back(d);
  d.text = "sa";
  voc[3].entries.push_back(d);
  syll.insert("4");
  rime::Vocabulary *lv2 = new rime::Vocabulary;
  voc[1].next_level.reset(lv2);
  rime::Vocabulary *lv3 = new rime::Vocabulary;
  (*lv2)[2].next_level.reset(lv3);
  d.code.back() = 1;
  d.code.push_back(2);
  d.code.push_back(3);
  d.code.push_back(4);
  d.text = "yi-er-san-si";
  (*lv3)[3].entries.push_back(d);
  
  ASSERT_TRUE(table->Build(syll, voc, 7));
  ASSERT_TRUE(table->Save());
  table.reset();

  table.reset(new rime::Table(file_name));
  ASSERT_TRUE(table);
  ASSERT_TRUE(table->Load());

  EXPECT_STREQ("0", table->GetSyllableById(0));
  EXPECT_STREQ("3", table->GetSyllableById(3));
  EXPECT_STREQ("4", table->GetSyllableById(4));
  
  rime::Table::Cluster cluster = table->GetEntries(1);
  rime::table::Entry *e = cluster.first;
  ASSERT_TRUE(e);
  ASSERT_EQ(1, cluster.second);
  EXPECT_STREQ("yi", e[0].text.c_str());
  EXPECT_EQ(1.0, e[0].weight);
  
  cluster = table->GetEntries(2);
  e = cluster.first;
  ASSERT_TRUE(e);
  ASSERT_EQ(3, cluster.second);
  EXPECT_STREQ("er", e[0].text.c_str());
  EXPECT_STREQ("liang", e[1].text.c_str());
  EXPECT_STREQ("lia", e[2].text.c_str());
  
  cluster = table->GetEntries(3);
  e = cluster.first;
  ASSERT_TRUE(e);
  ASSERT_EQ(2, cluster.second);
  EXPECT_STREQ("san", e[0].text.c_str());
  EXPECT_STREQ("sa", e[1].text.c_str());

  rime::Code code;
  code.push_back(1);
  code.push_back(2);
  code.push_back(3);
  rime::TableVisitor v(table->Query(code));
  ASSERT_FALSE(v.exhausted());
  ASSERT_EQ(1, v.remaining());
  ASSERT_TRUE(v.entry());
  EXPECT_STREQ("yi-er-san-si", v.entry()->text.c_str());
  ASSERT_TRUE(v.extra_code());
  ASSERT_EQ(1, v.extra_code()->size);
  EXPECT_EQ(4, *v.extra_code()->at);
  EXPECT_FALSE(v.Next());
}
