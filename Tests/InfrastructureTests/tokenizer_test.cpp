#include "stdafx.h"

#include <infrastructure/tokenizer.h>
#include <infrastructure/exception.h>

TEST(TokenizerTest, TestSingleQuotes) {
	// Note the escaped double and single quotes inside the string and 
	// the unterminated single quote after that
	Tokenizer tokenizer("   'a simple\"\"\\' quoted string'  'unterminated ");

	ASSERT_TRUE(tokenizer.NextToken());
	ASSERT_TRUE(tokenizer.IsQuotedString());
	ASSERT_EQ("a simple\"\"' quoted string", tokenizer.GetTokenText());

}

TEST(TokenizerTest, TestDoubleQuotes) {
	Tokenizer tokenizer("   \"a simple\\\"\\\"' quoted string\"   ");

	ASSERT_TRUE(tokenizer.NextToken());
	ASSERT_TRUE(tokenizer.IsQuotedString());
	ASSERT_EQ("a simple\"\"' quoted string", tokenizer.GetTokenText());

}

TEST(TokenizerTest, TestUnterminatedSingleQuotes) {
	Tokenizer tokenizer("   \"    ");
	ASSERT_THROW(tokenizer.NextToken(), TempleException);
}

TEST(TokenizerTest, TestIdentifier) {
	Tokenizer tokenizer("_ABC");

	ASSERT_TRUE(tokenizer.NextToken());
	ASSERT_TRUE(tokenizer.IsIdentifier("_abc"));
}
