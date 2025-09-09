/**@file
 * @brief Unit tests for <tt>../largemelon.hpp</tt>.
 * @note It's difficult to unit-test the functions that directly bridge from
 *   the lexer to the parser, such as @ref largemelon::parse_token_trimmed.
 *   This is because they would require a working lexer and a working parser,
 *   and even small lexers and parsers take quite a bit of work to set up.
 *   Still, we may want to test certain lexer/parser pairs for correct behavior
 *   when using those functions. In those cases, we can define Doctest test
 *   fixtures within the @ref largemelon::test namespace which contain those
 *   lexers and parsers.*/

#include <doctest/doctest.h>
#include "../largemelon.hpp"
#include <memory> // std::unique_ptr
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <vector>



/**@namespace largemelon::test
 * @brief Namespace defining test-only, non-production code for
 *   @c largemelon.*/

namespace largemelon::test {
	
	
	
	/**@test The @c std::malloc function fits the function signature
	 *   represented by @ref malloc_func_type.*/
	TEST_CASE("malloc_type matches std::malloc") {
		std::type_index type_idx = typeid(*malloc_func_type());
		std::type_index func_idx = typeid(std::malloc);
		MESSAGE("typeid(*malloc_func_type()): ", type_idx.name());
		MESSAGE("typeid(std::malloc):         ", func_idx.name());
		// following line fails on Windows, for some reason
		CHECK_EQ(type_idx, func_idx);
	}
	
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
	
	/**@test */
	TEST_CASE("same current indent does not change block indents") {
		int indent_change, rc;
		std::vector<size_t> indents = {4, 4};
		rc = largemelon::update_block_indents(indent_change, indents, 8);
		CHECK(rc == 0);
		CHECK(indents.size() == 2);
		CHECK(indent_change == 0);
	}

	/**@test */
	TEST_CASE("no current indent clears block indents") {
		int indent_change, rc;
		std::vector<size_t> indents = {2, 6, 4};
		rc = largemelon::update_block_indents(indent_change, indents, 0);
		CHECK(rc == 0);
		CHECK(indents.empty());
		CHECK(indent_change == -3);
	}

	/**@test */
	TEST_CASE("larger current indent extends block indents") {
		int indent_change, rc;
		std::vector<size_t> indents = {2, 2, 2, 2};
		rc = largemelon::update_block_indents(indent_change, indents, 12);
		CHECK(rc == 0);
		CHECK(indents.size() == 5);
		CHECK(indents.back() == 4);
		CHECK(indent_change == 1);
	}

	/**@test */
	TEST_CASE("smaller non-aligned current indent causes error") {
		int indent_change, rc;
		std::vector<size_t> indents = {4, 4, 4};
		rc = largemelon::update_block_indents(indent_change, indents, 10);
		CHECK(rc != 0);
		// TODO: indicate specifically that it's a bad indent
	}

	/**@test */
	TEST_CASE("smaller aligned current indent trims block indents") {
		int indent_change, rc;
		std::vector<size_t> indents = {4, 4, 4};
		rc = largemelon::update_block_indents(indent_change, indents, 4);
		CHECK(rc == 0);
		CHECK(indents.size() == 1);
		CHECK(indent_change == -2);
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
	
	
	
	/**@brief The @ref largemelon::span_loc function is robust enough that text
	 *   location spans can be blindly thrown at it to quickly get the spans of
	 *   large AST nodes with a lot of child nodes.*/
	TEST_SUITE("span_loc tests") {
		/**@test */
		TEST_CASE("null case") {
			CHECK_EQ(span_loc(EMPTY_TEXT_LOC, EMPTY_TEXT_LOC), EMPTY_TEXT_LOC);
		}
		/**@test The "span" across two equal text locations is equal to that
		 *   location.*/
		TEST_CASE("identity case") {
			CHECK_EQ(span_loc(FIRST_TEXT_LOC, FIRST_TEXT_LOC), FIRST_TEXT_LOC);
		}
		/**@test Given a location span @c A that surrounds and encases another
		 *   span @c B, their cumulative span is equal to @c A.*/
		TEST_CASE("encapsulation case") {
			const largemelon::text_loc A = { 2, 7, 4, 0 };
			const largemelon::text_loc B = { 2, 11, 3, 31 };
			CHECK_EQ(span_loc(A, B), A);
		}
	}
	
	
	
	/**@test */
	TEST_CASE("toktext on a Contrapoints phrase") {
		const std::string text = "no emotions in this video";
		std::string sample = largemelon::toktext(text.c_str(),
			text.c_str() + 11);
		CHECK(sample == "no emotions");
		sample = largemelon::toktext(text.c_str() + 3, text.c_str() + 11);
		CHECK(sample == "emotions");
	}
	
	
	
	/**@brief Enumerated node type.*/
	enum class nt {
		ROOT = 1,     ///< Root node.
		BOOL_LITERAL, ///< Boolean literal.
		BINOP_LOGOR,  ///< Logical-or.
		DATA_DECL,    ///< Data declaration.
	};
	
	/**@brief Base AST node type, from which all other AST node types are
	 *   derived.*/
	using ast_base = largemelon::ast_base_type<nt>;
	
	/**@brief Extends @c ast_base to associate it with @c nt.*/
	template <nt N>
	using ast_typed_base = largemelon::ast_typed_base_type<nt, N>;
	
	/**@brief Whether a given class is an AST node class type.
	 * @tparam T Class.*/
	template <typename T>
	constexpr bool is_ast_node_class
		= largemelon::is_ast_node_class_type<nt, ast_base>::value;
	
	/**@brief Whether a given class is an AST node sub-class type, derived from
	 *   @c ast_base.
	 * @tparam T Class.*/
	template <typename T>
	constexpr bool is_ast_node_subclass
		= largemelon::is_ast_node_subclass_type<nt, T>::value;
	
	
	
	/**@brief AST node for a Boolean literal.*/
	class ast_bool_literal : public ast_typed_base<nt::BOOL_LITERAL> {
		/**@brief Boolean value represented by this node.*/
		bool value_;
	public:
		/**@brief Constructor.
		 * @param loc Original location of text in parsed source.
		 * @param value Boolean value.*/
		ast_bool_literal(const largemelon::text_loc& loc, const bool value)
			: ast_typed_base<nt::BOOL_LITERAL>(loc), value_(value) {}
		/**@brief Boolean value represented by this node.*/
		bool value() const { return value_; }
	};
	
	static_assert(
		largemelon::is_ast_node_class_type<nt, ast_bool_literal>::value,
		"expected ast_bool_literal to be an AST node class");
	
	class ast_binop_logor : public ast_typed_base<nt::BINOP_LOGOR> {
		/**@brief Left-hand expression.*/
		std::unique_ptr<ast_base> lexpr_;
		/**@brief Right-hand expression.*/
		std::unique_ptr<ast_base> rexpr_;
	public:
		ast_binop_logor(const largemelon::text_loc& loc, ast_base* const lexpr,
			ast_base* const rexpr)
			: ast_typed_base<nt::BINOP_LOGOR>(loc),
			  lexpr_(std::unique_ptr<ast_base>(lexpr)),
			  rexpr_(std::unique_ptr<ast_base>(rexpr)) {
			add_childs(lexpr_.get(), rexpr_.get());
		}
		ast_base* lexpr() const { return lexpr_.get(); }
		ast_base* rexpr() const { return rexpr_.get(); }
	};
	
	/**@brief AST node for a data declaration.*/
	class ast_data_decl : public ast_typed_base<nt::DATA_DECL> {
		/**@brief Declaration name.*/
		std::string name_;
		/**@brief Expression.*/
		std::unique_ptr<ast_base> expr_;
	public:
		/**@brief Constructor.
		 * @param loc Original location of text in parsed source.
		 * @param name Declaration name.
		 * @param expr Expression.*/
		ast_data_decl(const largemelon::text_loc& loc, const std::string& name,
			ast_base* const expr)
			: ast_typed_base<nt::DATA_DECL>(loc), name_(name),
			  expr_(std::unique_ptr<ast_base>(expr)) {
			add_child(expr);
		}
		/**@brief Declaration name.*/
		std::string_view name() const { return name_; }
		/**@brief Expression.*/
		ast_base* expr() const { return expr_.get(); }
	};
	
	
	
	static_assert( is_ast_node_class<ast_base>,
		"expected ast_base to be an AST node class type");
	static_assert( is_ast_node_class<ast_bool_literal>,
		"expected ast_bool_literal to be an AST node class type");
	static_assert( is_ast_node_class<ast_data_decl>,
		"expected ast_data_decl to be an AST node class type");
	
	static_assert(!is_ast_node_subclass<ast_base>,
		"expected ast_base to not be an AST node sub-class (of itself)");
	static_assert(
		std::is_base_of< ast_base, ast_typed_base<nt::ROOT> >::value
		&& (!std::is_same< ast_base, ast_typed_base<nt::ROOT> >::value),
		"expected ast_typed_base<nt::ROOT> to be sub-class of ast_base");
	static_assert(
		is_ast_node_subclass_type< nt, ast_typed_base<nt::ROOT> >::value,
		"expected ast_typed_base<nt::ROOT> to be sub-class of ast_base");
	
	static_assert(
		largemelon::is_ast_node_subclass_type<nt, ast_bool_literal>::value,
		"expected ast_bool_literal to be sub-class of ast_base");
	static_assert( is_ast_node_subclass<ast_bool_literal>,
		"expected ast_bool_literal to be an AST node sub-class");
	static_assert( is_ast_node_subclass<ast_data_decl>,
		"expected ast_data_decl to be an AST node sub-class");
	
	
	
	/**@test If an AST node has no parent node, then it is the root of its
	 *   AST.*/
	TEST_CASE("Boolean literal with no parent node") {
		using namespace largemelon::test;
		ast_bool_literal node({1,0,1,3}, true);
		CHECK_EQ(node.type(), nt::BOOL_LITERAL);
		CHECK_EQ(node.value(), true);
		CHECK_EQ(node.parent(), &node);
		CHECK(node.is_root());
	}
	
	/**@test */
	TEST_CASE("binary addition operator") {
		using namespace largemelon::test;
		auto lexpr = new ast_bool_literal({1,0,1,3}, true);
		auto rexpr = new ast_bool_literal({1,8,1,12}, false);
		auto binop = new ast_binop_logor({1,0,1,12}, lexpr, rexpr);
		CHECK_EQ(binop->is_root(), true);
		CHECK_EQ(binop->lexpr()->is_root(), false);
		CHECK_EQ(binop->rexpr()->is_root(), false);
		delete binop;
	}
	
	/**@test */
	TEST_CASE("data declaration root with expression sub-node") {
		using namespace largemelon::test;
		auto expr = new ast_bool_literal({0,0,0,0}, false);
		auto decl = new ast_data_decl({0,0,0,0}, "unifying_force", expr);
		CHECK_EQ(decl->is_root(), true);
		CHECK_EQ(decl->expr(), expr);
		CHECK_NE(decl->expr()->parent(), decl->expr());
		CHECK_EQ(expr->is_root(), false);
		CHECK_EQ(decl->expr()->parent(), decl);
		CHECK_EQ(decl->childs().count(decl->expr()), 1);
		CHECK_EQ(decl->childs().count(expr), 1);
		delete decl;
	}
	
	
	
} // namespace largemelon::test
