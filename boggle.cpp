#include <cstdio>
#include <iostream>
#include <memory>
#include <cstring>
#include <cctype>
#include <fstream>
#include <string>
#include <set>

// ---------------------------------------------------------

#define DEFAULT_DICTIONARY_FILENAME "words.txt"
#define DEFAULT_BOARD_FILENAME "board.txt"

// ---------------------------------------------------------

struct Dictionary
{
	bool is_self_word;

	Dictionary();

	bool add_word(const char* word, int length);
	bool is_word(const char* word, int length) const;

	Dictionary* get_sub_dictionary(char c);
	const Dictionary* get_sub_dictionary(char c) const;
private:
	std::unique_ptr<Dictionary> letters[26];
};

// ---------------------------------------------------------

Dictionary::Dictionary()
	: is_self_word(false)
{
}

Dictionary* Dictionary::get_sub_dictionary(char c)
{
	return letters[toupper(c) - 'A'].get();
}

const Dictionary* Dictionary::get_sub_dictionary(char c) const
{
	return letters[toupper(c) - 'A'].get();
}

bool Dictionary::add_word(const char* word, const int length)
{
	const char letter = *word;

	if (!isalpha(letter))
		return false;

	auto& sub_dictionary = letters[toupper(letter) - 'A'];

	if (sub_dictionary == nullptr)
		sub_dictionary = std::make_unique<Dictionary>();

	if (length == 1)
	{
		sub_dictionary->is_self_word = true;
		return true;
	}
	
	return sub_dictionary->add_word(word + 1, length - 1);
}

bool Dictionary::is_word(const char* word, const int length) const
{
	const char letter = *word;

	if (!isalpha(letter))
		return false;

	const auto* sub_dictionary = get_sub_dictionary(letter);

	if (sub_dictionary == nullptr)
		return false;

	return length == 1 ? sub_dictionary->is_self_word : sub_dictionary->is_word(word + 1, length - 1);
}

// ---------------------------------------------------------

void search_board(const char (&board)[4][4], bool (&marked)[4][4], int i, int j, char* word, int len, const Dictionary* dict, std::set<std::string>& words)
{
	if (marked[i][j])
		return;

	const char letter = board[i][j];

	const auto* sub_dict = dict->get_sub_dictionary(letter);

	if (sub_dict == nullptr)
		return;

	word[len++] = letter;

	if (letter == 'Q')
	{
		word[len++] = 'U';
		sub_dict = sub_dict->get_sub_dictionary('U');

		if (sub_dict == nullptr)
		{
			word[--len] = '\0';
			return;
		}
	}

	marked[i][j] = true;

	if (sub_dict->is_self_word)
	{
		words.emplace(word, len);
	}

	if (i > 0)
	{
		if (j > 0) search_board(board, marked, i - 1, j - 1, word, len, sub_dict, words);
		           search_board(board, marked, i - 1, j    , word, len, sub_dict, words);
		if (j < 3) search_board(board, marked, i - 1, j + 1, word, len, sub_dict, words);
	}

	if (j > 0)     search_board(board, marked, i    , j - 1, word, len, sub_dict, words);
	if (j < 3)     search_board(board, marked, i    , j + 1, word, len, sub_dict, words);

	if (i < 3)
	{
		if (j > 0) search_board(board, marked, i + 1, j - 1, word, len, sub_dict, words);
		           search_board(board, marked, i + 1, j    , word, len, sub_dict, words);
		if (j < 3) search_board(board, marked, i + 1, j + 1, word, len, sub_dict, words);
	}

	marked[i][j] = false;
	word[--len] = '\0';
}

// ---------------------------------------------------------

int main(int argc, const char* argv[])
{
	const char* board_filename = argc > 1 ? argv[1] : DEFAULT_BOARD_FILENAME;
	const char* words_filename = argc > 2 ? argv[2] : DEFAULT_DICTIONARY_FILENAME;

	Dictionary dict;
 	
	{
		std::ifstream words_file(words_filename);

		if (!words_file)
		{
			printf("failed to read file \"%s\"", words_filename);
			return 1;
		}

		std::string line;
		while (std::getline(words_file, line))
			dict.add_word(line.c_str(), static_cast<int>(line.size()));
	}

	char board[4][4];

	{
		std::ifstream board_file(board_filename);

		if (!board_file)
		{
			printf("failed to read file \"%s\"", words_filename);
			return 1;
		}

		int n = 0;
		std::string line;
		while (std::getline(board_file, line))
		{
			if (line.size() != 4)
			{
				printf("invalid board format (not enough columns, expecting 4x4)");
				return 1;
			}

			for (int i = 0; i < 4; i++)
				board[n][i] = line[i];

			if (++n == 4)
				break;
		}

		if (n < 4)
		{
			printf("invalid board format (not enough rows, expecting 4x4)");
			return 1;
		}
	}

	char word[50] = {};
	bool marked[4][4] = {};
	std::set<std::string> found_words;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			search_board(board, marked, i, j, word, 0, &dict, found_words);
		}
	}

	for (const auto& word : found_words)
	{
		puts(word.c_str());
	}

	return 0;
}