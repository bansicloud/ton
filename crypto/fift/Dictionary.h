/*
    This file is part of TON Blockchain Library.

    TON Blockchain Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    TON Blockchain Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TON Blockchain Library.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2017-2020 Telegram Systems LLP
*/
#pragma once

#include <functional>
#include <map>

#include "IntCtx.h"

namespace fift {
using td::Ref;
/*
 *
 *    WORD CLASSES
 *
 */

typedef std::function<void(vm::Stack&)> StackWordFunc;
typedef std::function<void(IntCtx&)> CtxWordFunc;

class WordDef : public td::CntObject {
 public:
  WordDef() = default;
  virtual ~WordDef() override = default;
  virtual Ref<WordDef> run_tail(IntCtx& ctx) const = 0;
  void run(IntCtx& ctx) const;
  virtual bool is_list() const {
    return false;
  }
  virtual long long list_size() const {
    return -1;
  }
  virtual const std::vector<Ref<WordDef>>* get_list() const {
    return nullptr;
  }
};

class StackWord : public WordDef {
  StackWordFunc f;

 public:
  StackWord(StackWordFunc _f) : f(std::move(_f)) {
  }
  ~StackWord() override = default;
  Ref<WordDef> run_tail(IntCtx& ctx) const override;
};

class CtxWord : public WordDef {
  CtxWordFunc f;

 public:
  CtxWord(CtxWordFunc _f) : f(std::move(_f)) {
  }
  ~CtxWord() override = default;
  Ref<WordDef> run_tail(IntCtx& ctx) const override;
};

typedef std::function<Ref<WordDef>(IntCtx&)> CtxTailWordFunc;

class CtxTailWord : public WordDef {
  CtxTailWordFunc f;

 public:
  CtxTailWord(CtxTailWordFunc _f) : f(std::move(_f)) {
  }
  ~CtxTailWord() override = default;
  Ref<WordDef> run_tail(IntCtx& ctx) const override;
};

class WordList : public WordDef {
  std::vector<Ref<WordDef>> list;

 public:
  ~WordList() override = default;
  WordList() = default;
  WordList(std::vector<Ref<WordDef>>&& _list);
  WordList(const std::vector<Ref<WordDef>>& _list);
  WordList& push_back(Ref<WordDef> word_def);
  WordList& push_back(WordDef& wd);
  Ref<WordDef> run_tail(IntCtx& ctx) const override;
  void close();
  bool is_list() const override {
    return true;
  }
  long long list_size() const override {
    return (long long)list.size();
  }
  const std::vector<Ref<WordDef>>* get_list() const override {
    return &list;
  }
  WordList& append(const std::vector<Ref<WordDef>>& other);
  WordList* make_copy() const override {
    return new WordList(list);
  }
};

class DictEntry {
  Ref<WordDef> def;
  bool active;

 public:
  DictEntry() = delete;
  DictEntry(const DictEntry& ref) = default;
  DictEntry(DictEntry&& ref) = default;
  DictEntry(Ref<WordDef> _def, bool _act = false);
  DictEntry(StackWordFunc func);
  DictEntry(CtxWordFunc func, bool _act = false);
  DictEntry(CtxTailWordFunc func, bool _act = false);
  //DictEntry(const std::vector<Ref<WordDef>>& word_list);
  //DictEntry(std::vector<Ref<WordDef>>&& word_list);
  DictEntry& operator=(const DictEntry&) = default;
  DictEntry& operator=(DictEntry&&) = default;
  Ref<WordDef> get_def() const&;
  Ref<WordDef> get_def() &&;
  void operator()(IntCtx& ctx) const;
  bool is_active() const;
  ~DictEntry() = default;
};

/*
DictEntry::DictEntry(const std::vector<Ref<WordDef>>& word_list) : def(Ref<WordList>{true, word_list}) {
}

DictEntry::DictEntry(std::vector<Ref<WordDef>>&& word_list) : def(Ref<WordList>{true, std::move(word_list)}) {
}
*/

/*
 *
 *    DICTIONARIES
 *
 */

class Dictionary {
 public:
  DictEntry* lookup(td::Slice name);
  void def_ctx_word(std::string name, CtxWordFunc func);
  void def_ctx_tail_word(std::string name, CtxTailWordFunc func);
  void def_active_word(std::string name, CtxWordFunc func);
  void def_stack_word(std::string name, StackWordFunc func);
  void def_word(std::string name, DictEntry word);
  void undef_word(td::Slice name);

  auto begin() const {
    return words_.begin();
  }
  auto end() const {
    return words_.end();
  }

  static Ref<WordDef> nop_word_def;

 private:
  std::map<std::string, DictEntry, std::less<>> words_;
};

/*
 *
 *      AUX FUNCTIONS FOR WORD DEFS
 *
 */

Ref<WordDef> pop_exec_token(vm::Stack& stack);
Ref<WordList> pop_word_list(vm::Stack& stack);
void push_argcount(vm::Stack& stack, int args);
}  // namespace fift
