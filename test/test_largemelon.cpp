/**@file
 * @brief Unit tests for <tt>../largemelon.hpp</tt>.*/

#include <doctest/doctest.h>
#include "../largemelon.hpp"



/**@test An empty string has no characters to escape.*/
TEST_CASE("empty string with escaped characters") {
	std::string s = largemelon::escstr("");
	CHECK(s == "");
}

/**@test If a string has no characters to escape, then the string is not
 *     changed at all.*/
TEST_CASE("string with non-escaped characters") {
	std::string s_orig = "the wheels on the bus";
	std::string s_new = largemelon::escstr(s_orig);
	CHECK(s_new == s_orig);
}

/**@test The @c escstr function escapes the @c '\\n' character.*/
TEST_CASE("string with escaped whitespace characters") {
	std::string s = largemelon::escstr("Median\nNarrative");
	CHECK(s == R"(Median\nNarrative)");
}



/**@test A location in text is considered "less than" another location if
 *     both of the boundaries of the first location come before the
 *     boundaries of the second location.*/
TEST_CASE("text span is 'less-than' another if it completely precedes "
	"it") {
	const largemelon::text_loc before = { 1, 1, 1, 8 };
	const largemelon::text_loc after = { 2, 5, 2, 18 };
	CHECK(before < after);
	CHECK(!( after < before));
}

/**@test It's unexpected during normal use that a parsed text span would
 *     both start after another one's start and end after that same one's
 *     end. If two @c text_loc instances are partially entwined, then the
 *     'less-than' comparison returns @c false regardless of the order.*/
TEST_CASE("text locations is not 'less than' another if they're "
	"entwined") {
	const largemelon::text_loc first = { 5, 9, 5, 18 };
	const largemelon::text_loc second = { 5, 11, 5, 24 };
	CHECK(first != second);
	CHECK( !(first < second) );
	CHECK( !(second < first) );
}



/**@test */
TEST_CASE("toktext on a Contrapoints phrase") {
	const std::string text = "no emotions in this video";
	std::string sample = largemelon::toktext(text.c_str(), text.c_str() + 11);
	CHECK(sample == "no emotions");
	sample = largemelon::toktext(text.c_str() + 3, text.c_str() + 11);
	CHECK(sample == "emotions");
}

