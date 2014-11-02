#include <CwshI.h>
#include <CFileMatch.h>

#ifdef COS_TERM
#include <COSTerm.h>
#endif

CwshMatch::
CwshMatch(Cwsh *cwsh) :
 cwsh_(cwsh) {
}

bool
CwshMatch::
showMatch(const string &line)
{
  string word;

  CwshComplete complete(cwsh_, line);

  CwshCompletionType type = complete.getCompletionType(&word);

  if (type == CWSH_COMPLETION_TYPE_NONE) {
    cwsh_->beep();

    return false;
  }

  word += "*";

  vector<string> words;

  if      (type == CWSH_COMPLETION_TYPE_COMMAND)
    getPathMatch(word, words);
  else if (type == CWSH_COMPLETION_TYPE_FILE) {
    string::size_type pos = word.rfind('/');

    if (pos != string::npos) {
      string lhs = word.substr(0, pos);
      string rhs = word.substr(pos + 1);

      CDir::enter(lhs);

      getFileMatch(rhs, words);

      CDir::leave();
    }
    else
      getFileMatch(word, words);
  }
  else if (type == CWSH_COMPLETION_TYPE_VAR)
    getVarMatch(word, words);
  else if (type == CWSH_COMPLETION_TYPE_USERS)
    getUsersMatch(word, words);
  else
    return false;

  CStrUtil::sort(words);

  vector<string> uniq_words;

  CStrUtil::uniq(words, uniq_words);

  std::cout << std::endl;

  print(uniq_words);

  return true;
}

bool
CwshMatch::
getPathMatch(const string &pattern_str, vector<string> &words)
{
  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandPath(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getFileMatch(const string &pattern_str, vector<string> &words)
{
  CFileMatch fileMatch;

  if (! fileMatch.matchPrefix(pattern_str, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getVarMatch(const string &pattern_str, vector<string> &words)
{
  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandVar(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getUsersMatch(const string &pattern_str, vector<string> &words)
{
  if (! CwshString::matchUsers(pattern_str, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

void
CwshMatch::
print(vector<string> &words)
{
  int max_len = CStrUtil::maxLen(words);

  int screen_width = 80;

#ifdef COS_TERM
  int screen_height;

  COSTerm::getCharSize(&screen_width, &screen_height);
#endif

  int num_words = words.size();

  int words_per_line = std::max(screen_width / (max_len + 1), 1);

  int num_lines = num_words / words_per_line;

  if ((num_words % words_per_line) != 0)
    ++num_lines;

  int i = 0;
  int j = 0;

  while (i < num_words && j < num_lines) {
    int len = words[i].size();

    std::cout << words[i];

    for (int k = 0; k <= max_len - len; ++k)
      std::cout << " ";

    i += num_lines;

    if (i >= num_words) {
      std::cout << std::endl;

      ++j;

      i = j;
    }
  }
}
