/**@file
 * @brief Single-header C++ framework for bridging between a Ragel-generated
 *   scanner and a Lemon-generated parser.*/

#ifndef LARGEMELON_LARGEMELON_HPP
#define LARGEMELON_LARGEMELON_HPP

#include <algorithm>
#include <cassert>
#include <functional>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace largemelon {
	
	
	
	/**@brief String representing a character's "escaped" representation.
	 * @param c Character.
	 * @return String representing a character's "escaped" representation,
	 *     including whitespace characters and other ANSI control codes.*/
	inline std::string escchr(const char c) {
		switch (c) {
			case '\n': return { '\\', 'n' };
			case '\r': return { '\\', 'r' };
			case '\f': return { '\\', 'f' };
			case '\v': return { '\\', 'v' };
			case '\t': return { '\\', 't' };
			case '\\': return { '\\', '\\' };
			default: return { c };
		}
	}
	
	/**@brief Copy of a given string, with certain (whitespace) characters
	 *     escaped.
	 * @param s String.
	 * @return Copy of @c s with certain characters escaped, namely
	 *     whitespace.*/
	inline std::string escstr(const std::string& s) {
		std::string es;
		es = "";
		for (auto si=s.cbegin(); si!=s.cend(); si++)
			es += escchr(*si);
		return es;
	}
	
	
	
	/**@brief Location of a span of text in source code.*/
	struct text_loc {
		/**@brief Line number of first character.*/
		size_t first_lno;
		/**@brief Column number of first character.*/
		size_t first_cno;
		/**@brief Line number of last character.*/
		size_t last_lno;
		/**@brief Column number of last character.*/
		size_t last_cno;
	};
	
	/**@brief Equality operator for @c text_loc.
	 * @param lhs First position in text.
	 * @param rhs Second position in text.
	 * @return @c true if @c lhs and @c rhs have identical data member values,
	 *     @c false otherwise.*/
	inline constexpr bool operator==(const text_loc &lhs,
		const text_loc &rhs) {
		return (lhs.first_lno == rhs.first_lno)
			&& (lhs.first_cno == rhs.first_cno)
			&& (lhs.last_lno == rhs.last_lno)
			&& (lhs.last_cno == rhs.last_cno);
	}
	
	/**@brief Inequality operator for @c text_loc.
	 * @param lhs First position in text.
	 * @param rhs Second position in text.
	 * @return Equivalent to <code>!(lhs == rhs)</code>.*/
	inline constexpr bool operator!=(const text_loc &lhs,
		const text_loc &rhs) {
		return !(lhs == rhs);
	}
	
	/**@brief Less-than operator for @c text_loc.
	 * @param lhs First position in text.
	 * @param rhs Second position in text.
	 * @return @c true if the span of text delimited by @c lhs comes entirely
	 *     before the span delimited by @c rhs, @c false otherwise.
	 * @note This is used to sort error messages.*/
	inline constexpr bool operator <(const text_loc &lhs,
		const text_loc &rhs) {
		return (lhs.last_lno < rhs.first_lno)
			|| (lhs.last_lno == rhs.first_lno
				&& lhs.last_cno <= rhs.first_cno);
	}
	
	/**@brief Serializes a @c text_loc instance to an output stream.*/
	inline std::ostream &operator <<(std::ostream &os, const text_loc &loc) {
		if (loc.first_lno == loc.last_lno) {
			os << loc.first_lno << ":" << loc.first_cno;
			if (loc.first_cno != loc.last_cno) {
				os << "-" << loc.last_cno;
			}
		}
		else {
			os << loc.first_lno << ":" << loc.first_cno
				<< "-" << loc.last_lno << ":" << loc.last_cno;
		}
		return os;
	}
	
	/**@brief Location of text "span" at the start of any file or string, from
	 *        which all other location values are calculated.*/
	static const text_loc FIRST_TEXT_LOC = { 1, 0, 1, 0 };
	
	/**@brief Empty, "uninitialized" @c text_loc value.*/
	static const text_loc EMPTY_TEXT_LOC = { 0, 0, 0, 0 };
	
	
	
	/**@brief Location of a given text string in a larger string being parsed.
	 * @param prev_loc Location of text previous to @c mtext.
	 * @param mtext    Text for which location is calculated.
	 * @return Location value with @c text_loc::first_lno and
	 *     @c text_loc::first_cno at the first character of @c mtext and
	 *     @c text_loc::last_lno and @c text_loc::last_cno at the last
	 *     character of @c mtext.
	 * @note Only @c prev_loc.last_lno and @c prev_loc.last_cno are used from
	 *     @c prev_loc.
	 * @todo Document @c mtext_loc() with an illustration of lines and column
	 *     numbers (e.g. with column @c 0 before the line).
	 * 
	 * @code
	 * text_loc prev_loc = {1, 26, 1, 29};
	 * text_loc loc = mtext_loc("Everything is creepy and dark and dirty.",
	 *   prev_loc);
	 * @endcode
	 * @dotfile mtext_loc_example_1.dot
	 */
	inline text_loc mtext_loc(const text_loc &prev_loc,
		const std::string &mtext) {
		
		static const std::regex RGX_NL = std::regex(R"(\r\n|\r|\n)");
		std::sregex_iterator nli, rend;
		text_loc loc;
		size_t num_newlines, tpos;
		
		// The new location starts just after the last position in `prev_loc`.
		
		loc.first_lno = prev_loc.last_lno;
		loc.first_cno = prev_loc.last_cno + 1;
			// what if this is past all characters on line?
			// if it is, then the newline sequence is coming up and that
			// erroneous extra column won't matter;
			// write unit tests to confirm this
		
		// The number of lines spanned by the new location is equal to the
		// number of newline character sequences in `mtext`.
		
		num_newlines = 0;
		tpos = 0;
		nli = std::sregex_iterator(mtext.cbegin(), mtext.cend(), RGX_NL);
		rend = std::sregex_iterator();
		for (; nli!=rend; nli++) {
			num_newlines++;
			tpos = nli->position(0) + nli->length(0);
		}
		
		// If any newlines were encountered in `mtext`, then the column number
		// of the last position spanned by the new location is equal to the
		// number of positions from the last newline in `mtext` to the end of
		// `mtext`.
		//
		// If no newlines were encountered, then the column number of the new
		// location is simply the value of the previous location's last
		// position plus the number of positions in `mtext`.
		
		if (num_newlines > 0) {
			loc.last_lno = loc.first_lno + num_newlines;
			loc.last_cno = mtext.length() - tpos;
		}
		else {
			loc.last_lno = loc.first_lno;
			loc.last_cno = prev_loc.last_cno + mtext.length();
		}
		
		return loc;
		
	}
	
	
	
	/**@brief Location spanning and including two text locations.
	 * @param first_loc  Location of first character in text span.
	 * @param last_loc   Location of last character in text span.
	 * @return Location value equal to @c first_loc.first_lno,
	 *     @c first_loc.first_cno, @c last_loc.last_lno, and
	 *     @c last_loc.last_cno.*/
	inline constexpr text_loc span_loc(const text_loc &first_loc,
		const text_loc &last_loc) {
		return { first_loc.first_lno, first_loc.first_cno,
			last_loc.last_lno, last_loc.last_cno };
	}
	
	
	
	/**@brief Marks a function parameter as unused, so that it doesn't trigger
	 *     a compilation warning. These can get numerous and cumbersome. (GCC
	 *     has an @c unused attribute specifically for this purpose, but that
	 *     is not implemented here.)
	 * @param V Function parameter.*/
	#define LARGEMELON_UNUSED_PARAM(V) do { (void)(V); } while (0);
	
	
	
	/**@brief A token provided by the lexer to the parser.
	 * @note Use the @c lex_token function to construct instances of this
	 *     type.*/
	struct lex_token {
		/**@brief Text matched and pushed to the parser.*/
		std::string mtext;
		/**@brief Path to source file being parsed.*/
		std::filesystem::path fpath;
		/**@brief Location of matched text in source file being parsed.*/
		text_loc loc;
		/**@brief Default constructor.
		 * @details @c mtext and @c fpath are set to zero-length values, and
		 *   @c loc is set to @ref largemelon::FIRST_TEXT_LOC.*/
		lex_token() : mtext(), fpath(), loc(FIRST_TEXT_LOC) {}
		/**@brief Constructor.
		 * @param mtext Text matched and pushed to the parser.
		 * @param fpath Path to source file being parsed.
		 * @param loc Location of matched text in source file being parsed.*/
		lex_token(const std::string& mtext, const std::filesystem::path& fpath,
			const text_loc& loc) : mtext(mtext), fpath(fpath), loc(loc) {}
	};
	
	/**@brief Serializes a @c lex_token instance.
	 * @param os Text output stream.
	 * @param token Lexer token.
	 * @return @c os, with @c token serialized and appended.*/
	inline std::ostream& operator<<(std::ostream& os, const lex_token& token) {
		os << "LEX_TOKEN(\"" << escstr(token.mtext) << "\",";
		if (! token.fpath.empty())
			os << "\"" << token.fpath.c_str() << "\",";
		os << token.loc << ")";
		return os;
	}
	
	/**@brief Returns the string spanning between two pointers in a
	 *     <code>char *</code> instance.
	 * @param ts First position.
	 * @param te Last position, plus 1.
	 * @return String starting with character at position @c ts and ending at
	 *     character at position just before @c te.
	 * @warning @c ts and @c te cannot be null pointers (@c nullptr). Also, the
	 *     result of <code>te - ts</code> must be zero or greater.*/
	inline std::string toktext(const char *ts, const char *te) {
		assert(ts != nullptr);
		assert(te != nullptr);
		assert((te - ts) >= 0);
		return std::string(ts, te - ts);
	}
	
	/**@brief Returns the string spanning between two pointers in a
	 *   <code>char *</code> instance, with some characters trimmed from the
	 *   beginning and end of the resultant string.
	 * @param ts Position of first character in returned string.
	 * @param te Position one past the last character in returned string.
	 * @param ltrim Number of characters to remove from beginning of string.
	 * @param rtrim Number of characters to remove from end of string.
	 * @return String starting @c ltrim characters after character at position
	 *   @c ts and ending @c rtrim characters before character at position
	 *   @c te.
	 * @warning @c ts and @c te cannot be null pointers (@c nullptr), and the
	 *   result of <code>(te - rtrim) - (ts + ltrim)</code> must be zero or
	 *   greater.*/
	inline std::string toktext(const char *ts, const char *te,
		const size_t ltrim, const size_t rtrim) {
		assert(ts != nullptr);
		assert(te != nullptr);
		assert((te - rtrim) - (ts + ltrim) >= 0);
		return std::string(ts + ltrim, te - ts - ltrim - rtrim);
	}
	
	
	
	/**@brief Data type for a function matching the function signature of
	 *   @c std::malloc.
	 * @note An explicit declaration is used here instead of just
	 *   <tt>decltype(std::malloc)</tt> in order to avoid a compiler warning
	 *   about ignoring attributes on a template argument.*/
	using malloc_func_type = void *(*)(size_t) noexcept;
	
	/**@brief Data type for a function matching the function signature of
	 *   @c std::free.
	 * @note As with @ref malloc_func_type, an explicit declaration is used
	 *   here instead of <tt>decltype(std::free)</tt>.*/
	using free_func_type = void (*)(void *) noexcept;
	
	/**@brief Data type for a function pointer with function signature matching
	 *     that of the <c>ParseAlloc()</c>-like function generated by the Lemon
	 *     parser generator.*/
	using lemon_parsealloc_func_type = std::function<void*(malloc_func_type)>;
	//~ using lemon_parsealloc_func_type = std::function<void*(*)(size_t)>;
	//~ void *(*malloc_func)(size_t)
	
	/**@brief Data type for a function pointer with function signature matching
	 *     that of the <c>ParseFree()</c>-like function generated by the Lemon
	 *     parser generator.*/
	using lemon_parsefree_func_type = std::function<void(void*,
		free_func_type)>;
	
	/**@brief Data type for a function pointer with function signature matching
	 *     that of the <c>Parse()</c>-like function generated by the Lemon
	 *     parser generator.
	 * @tparam ContextType Data type for the extra argument, passed by pointer
	 *     between calls to <c>Parse()</c> (or its renamed equivalent).*/
	template <typename ContextType>
	using lemon_parse_func_type = std::function<void(void *, int, lex_token *,
		ContextType *)>;
	
	/**@brief Data type for a function pointer with function signature matching
	 *     that of the <c>ParseTrace()</c>-like function generated by the Lemon
	 *     parser generator.*/
	using lemon_parsetrace_func_type = std::function<void(FILE *, char *)>;
	
	
	
	/**@brief Sets the matched text and current location in text for a single
	 *     parsing step.
	 * @param mtext Matched text, extracted from parsed text.
	 * @param loc Location of matched text, relative to parsed text.
	 * @param fpath Path to file being parsed.
	 * @param ts Pointer to position of first matched character in parsed text.
	 * @param te Pointer to position just after last matched character in
	 *     parsed text.
	 * @param ltrim Number of characters to trim from the front of the parsed
	 *     text, starting from @c ts.
	 * @param rtrim Number of characters to trim from the end of the parsed
	 *     text, starting from @c te.
	 * 
	 * The values of @c ltrim and @c rtrim do not affect the resultant value of
	 * @c loc, because those characters are still being consumed by the parsing
	 * step.*/
	inline void set_mtext_and_loc_trimmed(std::string& mtext, text_loc& loc,
		const std::filesystem::path& fpath, const char *ts, const char *te,
		const size_t& ltrim, const size_t& rtrim) {
		LARGEMELON_UNUSED_PARAM(fpath);
		assert(ts != nullptr);
		assert(te != nullptr);
		assert((te - rtrim) - (ts + ltrim) > 0);
		mtext = toktext(ts + ltrim, te - rtrim);
		loc = mtext_loc(loc, toktext(ts, te));
	}
	
	
	
	/**@brief Skips a token matched by a single parsing step.
	 * @param mtext Matched text, extracted from parsed text.
	 * @param loc Location of matched text, relative to parsed text.
	 * @param fpath Path to file being parsed.
	 * @param ts Pointer to position of first matched character in parsed text.
	 * @param te Pointer to position just after last matched character in
	 *     parsed text.
	 * @param ltrim Number of characters to trim from the front of the parsed
	 *     text, starting from @c ts.
	 * @param rtrim Number of characters to trim from the end of the parsed
	 *     text, starting from @c te.
	 * @param verbosity Level of debug output.
	 * 
	 * The token that is skipped is not passed to the parser. Considering that
	 * a lexer is what calls this function, it is a lexer that is not passing
	 * the token to the parser. So, use this function when the lexer needs to
	 * recognize the token but the parser must not. For example, in the C and
	 * C++ programming languages, C-style block comments can be inserted
	 * anywhere in a source file, because they'll be automatically removed
	 * (effectively, treated as a single token and then skipped by the parser).
	 * This function operates under the same principle.
	 * 
	 * The values of @c mtext and @c loc are still set, so that parsing can
	 * continue with the next-matched token.*/
	inline void skip_token(std::string& mtext, text_loc& loc,
		const std::filesystem::path& fpath, const char *ts, const char *te,
		const size_t& ltrim, const size_t& rtrim, const int& verbosity) {
		LARGEMELON_UNUSED_PARAM(ltrim);
		LARGEMELON_UNUSED_PARAM(rtrim);
		assert(ts != nullptr);
		assert(te != nullptr);
		set_mtext_and_loc_trimmed(mtext, loc, fpath, ts, te, 0, 0);
		if (verbosity >= 2) {
			std::cerr << "Skipping text `" << escstr(mtext) << "` at "
				<< loc << std::endl;
		}
	}
	
	
	
	/**@brief Executes lexer action code for a single parsing step.
	 * @tparam ContextType Data type for the context object being passed by
	 *     pointer between calls to the parser. This is the same as the data
	 *     type used in the <c>%extra_argument</c> directive in the Lemon
	 *     specification file, and thus passed as the fourth argument to the
	 *     function passed to @c parse_func.
	 * @param mtext Matched text, extracted from parsed text.
	 * @param loc Location of matched text, relative to parsed text.
	 * @param context Parsing context populated during each parsing step.
	 * @param fpath Path to file being parsed.
	 * @param parse_func <c>Parse()</c>-like function wrapped by this one.
	 * @param ts Pointer to position of first matched character in parsed text.
	 * @param te Pointer to position just after last matched character in
	 *     parsed text.
	 * @param pparser Pointer to allocated instance of parser.
	 * @param token_id Value of token, as defined in a Lemon-generated header
	 *     file, being passed to the parser.
	 * @param ltrim Number of characters to trim from the front of the parsed
	 *     text, starting from @c ts.
	 * @param rtrim Number of characters to trim from the end of the parsed
	 *     text, starting from @c te.
	 * @param verbosity Level of debug output.
	 * @warning This dynamically allocates a new @ref lex_token instance.
	 * @note Because @c pparser is just a <c>void *</c>, and because every
	 *     parser implemented by Lemon is allocated as a <c>void *</c>, <em>any
	 *     kind of Lemon-created parser can be used with this function</em>.
	 *     None of the machinery implementing this function is dependent on any
	 *     particular properties of a Ragel-derived scanner, on any formal
	 *     grammar definitions, or on the structure of the @c ContextType data
	 *     type that is passed to the @c parse_func call; this function can be
	 *     reused as-is.*/
	template <typename ContextType>
	inline void parse_token_trimmed(std::string& mtext, text_loc& loc,
		ContextType& context, const std::filesystem::path& fpath,
		lemon_parse_func_type<ContextType> parse_func, const char *ts,
		const char *te, void *const pparser, const size_t& token_id,
		const size_t& ltrim, const size_t& rtrim, const int& verbosity) {
		
		assert(ts != nullptr);
		assert(te != nullptr);
		assert(pparser != nullptr);
		set_mtext_and_loc_trimmed(mtext, loc, fpath, ts, te, ltrim, rtrim);
		if (verbosity >= 2) {
			std::cerr << "Passing token `" << escstr(mtext) << "` at " << loc
				<< " to the parser" << std::endl;
		}
		parse_func(pparser, token_id, new lex_token(mtext, fpath, loc),
			&context);
		
	}
	
	
	
	/**@brief Executes action code for a single parsing step, without building
	 *     a lexer token and passing the matched text into the parser
	 *     machinery.
	 * @tparam ContextType Data type for the context object being passed by
	 *     pointer between calls to the parser. This is the same as the data
	 *     type used in the <c>%extra_argument</c> directive in the Lemon
	 *     specification file, and thus passed as the fourth argument to the
	 *     function passed to @c parse_func.
	 * @param mtext Matched text, extracted from parsed text.
	 * @param loc Location of matched text, relative to parsed text.
	 * @param context Parsing context populated during each parsing step.
	 * @param fpath Path to file being parsed.
	 * @param parse_func <c>Parse()</c>-like function wrapped by this one.
	 * @param ts Pointer to position of first matched character in parsed text.
	 * @param te Pointer to position just after last matched character in
	 *     parsed text.
	 * @param pparser Pointer to allocated instance of parser.
	 * @param token_id Value of token, as defined in @c parser/src/parser.h,
	 *     being passed to the parser.
	 * @param verbosity Level of debug output.
	 * 
	 * This function is used when the actual contents of a matched token are
	 * irrelevant, yet the parser still needs to register that the token was
	 * matched for the grammar's sake.
	 * 
	 * @note As of this writing (2021-08-29), this function might be unused.
	 *     Its effectiveness is thus questionable.
	 * 
	 * @note The same restrictions -- or lack thereof -- on the usage of
	 *     @c parse_token_trimmed() apply to this function with respect to the
	 *     @c pparser argument.*/
	template <typename ContextType>
	inline void parse_null_token(std::string& mtext, text_loc& loc,
		ContextType& context, const std::filesystem::path& fpath,
		lemon_parse_func_type<ContextType> parse_func, const char *ts,
		const char *te, void *const pparser, const size_t& token_id,
		const int& verbosity) {
		
		assert(ts != nullptr);
		assert(te != nullptr);
		assert(pparser != nullptr);
		set_mtext_and_loc_trimmed(mtext, loc, fpath, ts, te, 0, 0);
		if (verbosity >= 2) {
			std::cerr << "Passing token `" << escstr(mtext) << "` at " << loc
				<< " to the parser as null" << std::endl;
		}
		parse_func(pparser, token_id, nullptr, &context);
		
	}
	
	
	
	template <typename AstEnumType>
	class ast_base_type;
	
	/**@brief Base class for all types of AST nodes.
	 * @tparam AstEnumType Data type for the enumerated AST node type. Must be
	 *   an integral (e.g., int, char), or enumeration type (e.g., enum, enum
	 *   class, enum struct).*/
	template <typename AstEnumType>
	class ast_base_type {
		static_assert(std::is_enum<AstEnumType>::value
			|| std::is_integral<AstEnumType>::value, "AstEnumType must be an "
			"integral or enumeration type");
		/**@brief Pointer to parent AST node.*/
		ast_base_type<AstEnumType>* parent_;
		/**@brief Pointers to child AST nodes.*/
		std::set<ast_base_type<AstEnumType>*> childs_;
		/**@brief Location in parsed source of text represented by this node.*/
		text_loc loc_;
	protected:
		/**@brief Constructor.
		 * @param loc Location of original text in parsed source.
		 * @details This node is set as the root node of its AST and with no
		 *   child nodes. This node's parent node is not set until this node is
		 *   set as a child node of another node.*/
		ast_base_type(const text_loc& loc) : parent_(this), childs_(),
			loc_(loc) {
			assert(parent_ != nullptr);
		}
		/**@brief Assigns an existing AST node as a child of this node, and
		 *   sets this node as the parent of that node.
		 * @param child Node to set as a child of this node.
		 * @details The parent node of @c child becomes this node.
		 * @warning @c child cannot be @c nullptr.*/
		void add_child(ast_base_type<AstEnumType>* const child) {
			child->parent_ = this;
			childs_.insert(child);
		}
		/**@brief Tail case for @ref add_childs. Does nothing.*/
		void add_childs() {}
		/**@brief Assigns multiple AST nodes as children of this node, and sets
		 *   this node as the parent of those nodes.
		 * @tparam ChildTypes Data types for @c childs. (They will always be
		 *   a parameter pack of @ref ast_base_type* @c const.)
		 * @param child First child node.
		 * @param childs Next child nodes.
		 * @warning @c child cannot be @c nullptr. None of the elements of
		 *   @c childs can contain @c nullptr.*/
		template <typename... ChildTypes>
		void add_childs(ast_base_type<AstEnumType>* const child,
			ChildTypes... childs) {
			add_child(child);
			add_childs(childs...);
		}
	public:
		/**@brief Enumerated value associated with AST nodes of this type.
		 * @details This is defined by every class derived from
		 *   @ref ast_base_type. Each derived type of AST node has an
		 *   associated enumerated value. This way, the AST node can be passed
		 *   around as an @ref ast_base_type pointer and then be
		 *   <tt>dynamic_cast</tt>-ed back to the class type associated with
		 *   its enumerated type.*/
		virtual constexpr AstEnumType type() const = 0;
		/**@brief Parent AST node.
		 * @note If <tt>this == this->parent()</tt>, then this is the root
		 *   node of its AST.*/
		ast_base_type<AstEnumType>* parent() const {
			assert(parent_ != nullptr);
			return parent_;
		}
		/**@brief Whether this node is the root node of its AST.
		 * @return @c true if <tt>this == parent()</tt>, @c false otherwise.*/
		bool is_root() const { return this == parent(); }
		/**@brief Root node of this AST, for which there is no parent node.*/
		const ast_base_type<AstEnumType>* root() const {
			const ast_base_type<AstEnumType>* n = this;
			do {
				assert(n != nullptr);
				if (n->is_root()) {
					break;
				}
				n = n->parent();
			} while (true);
			assert(n != nullptr);
			return n;
		}
		/**@brief Child AST nodes.*/
		std::set<ast_base_type<AstEnumType>*> childs() const {
			return childs_;
		}
		/**@brief Location of original text in parsed source.*/
		text_loc loc() const { return loc_; }
	};
	
	
	
	/**@brief Extends @c ast_base_type to associate it with an enumerated AST
	 *   node type and implement the @ref ast_base_type::type method.
	 * @tparam AstEnumType Data type for enumerated AST node type.
	 * @tparam N Enumerated value for AST node type.
	 * @details All other AST node types are derived from this template
	 *   class.*/
	template <typename AstEnumType, AstEnumType N>
	class ast_typed_base_type : public ast_base_type<AstEnumType> {
	protected:
		/**@brief Constructor.
		 * @param loc Location of original text in parsed source.*/
		ast_typed_base_type(const text_loc& loc)
			: ast_base_type<AstEnumType>(loc) {}
	public:
		/**@brief Enumerated AST node type.*/
		constexpr AstEnumType type() const override { return N; }
	};
	
	
	
	static_assert(std::is_base_of< ast_base_type<int>,
		ast_typed_base_type<int, 1> >::value, "expected template class "
		"ast_base_type<int> to be base class of derived template class "
		"ast_typed_base_type<int,1>");
	static_assert(!std::is_same< ast_base_type<int>,
		ast_typed_base_type<int, 1> >::value, "expected template class "
		"ast_base_type<int> to not be same as derived template class "
		"ast_typed_base_type<int,1>");
	
	
	
	/**@brief Base trait type for indicating whether a class is either:
	 *   1) a base AST node type for a given enumerated AST node type; or
	 *   2) one of that class' derived types.
	 * @tparam AstEnumType Enumerated AST node type.
	 * @tparam ClassType Class being interrogated.
	 * 
	 * To specialize this for an application with a particular enumerated AST
	 * node type:
	 * @code{.cpp}
	 * enum class nt {
	 *   ROOT = 1,
	 *   BOOL_LITERAL,
	 *   CONST_DECL,
	 * };
	 * template <typename ClassType>
	 * constexpr bool is_ast_node_class() const {
	 *   return is_ast_node_class_type<nt, ClassType>::value;
	 * }
	 * @endcode*/
	template <typename AstEnumType, typename ClassType>
	struct is_ast_node_class_type : public std::integral_constant<bool,
		std::is_base_of<ast_base_type<AstEnumType>, ClassType>::value
	> {};
	
	/**@brief Base trait type for indicating whether a class is derived from a
	 *   base AST node type for a given enumerated AST node type.
	 * @tparam AstEnumType Enumerated AST node type.
	 * @tparam ClassType Class being interrogated.
	 * 
	 * To specialize this for an application with an enumerated AST node type
	 * @c nt:
	 * @code{.cpp}
	 * template <typename ClassType>
	 * constexpr bool is_ast_node_subclass() const {
	 *   return is_ast_node_subclass_type<nt, ClassType>::value;
	 * }
	 * @endcode*/
	template <typename AstEnumType, typename ClassType>
	struct is_ast_node_subclass_type : public std::integral_constant<bool,
		std::is_base_of<ast_base_type<AstEnumType>, ClassType>::value
		&& (!std::is_same<ast_base_type<AstEnumType>, ClassType>::value)
	> {};
	
	
	
	/**@brief Collection of registers managed by a Ragel-generated scanner.
	 * @details To use this in the same C++ source file as a Ragel machine
	 *   instantiation:
	 * @code{.cpp}
	 * largemelon::ragel_scanner_pers_type rp;
	 * 
	 * %% access rp.;
	 * %% variable p   rp.p;
	 * %% variable pe  rp.pe;
	 * %% variable eof rp.eof;
	 * %% write data;
	 * 
	 * // set up parser...
	 * 
	 * %% write init;
	 * %% write exec;
	 * 
	 * // ...
	 * @endcode
	 * See the Ragel user manual for details.*/
	struct ragel_scanner_pers_type {
		/**@brief Pointer to first input character.*/
		const char *p;
		/**@brief Pointer to just after last input character.*/
		const char *pe;
		/**@brief Pointer to end of opened file, if applicable.*/
		const char *eof;
		/**@brief Register for current internal parser state.*/
		int cs;
		/**@brief Register for most recent successful pattern match.*/
		int act;
		/**@brief Pointer to first character of found token.*/
		const char *te;
		/**@brief Pointer to just after end of found token.*/
		const char *ts;
	};
	
	
	
} // namespace largemelon



/**@brief Macro for the declaration of a <tt>ParseAlloc</tt>-like function
 *   generated by Lemon.
 * @param PREFIX Value of the <tt>%name</tt> directive in the Lemon parser
 *   specification.
 * @details This resolves to a function with name <a>PREFIX</a>Alloc and call
 *   signature matching @ref largemelon::lemon_parsealloc_func_type.*/
#define LARGEMELON_LEMON_PARSEALLOC_DECL(PREFIX) \
extern "C" void * PREFIX##Alloc(void*(*)(size_t));

/**@brief Macro for the declaration of a <tt>ParseFree</tt>-like function
 *   generated by Lemon.
 * @param PREFIX Value of the <tt>%name</tt> directive in the Lemon parser
 *   specification.
 * @details This resolves to a function with name <a>PREFIX</a>Free and call
 *   signature matching @ref largemelon::lemon_parsefree_func_type.*/
#define LARGEMELON_LEMON_PARSEFREE_DECL(PREFIX) \
extern "C" void * PREFIX##Free(void *, void (*)(void *));

/**@brief Macro for the declaration of a <tt>Parse</tt>-like function generated
 *   by Lemon.
 * @param PREFIX Value of the <tt>%name</tt> directive in the Lemon parser
 *   specification.
 * @param CONTEXT_TYPE Type for the <tt>%extra_argument</tt> directive in the
 *   Lemon parser specification.
 * @details This resolves to a function with name <a>PREFIX</a> and call
 *   signature matching @ref largemelon::lemon_parse_func_type.*/
#define LARGEMELON_LEMON_PARSE_DECL(PREFIX,CONTEXT_TYPE) \
extern "C" void PREFIX (void*, int, largemelon::lex_token*, CONTEXT_TYPE);

/**@brief Macro for the declaration of a <tt>ParseTrace</tt>-like function
 *   generated by Lemon.
 * @param PREFIX Value of the <tt>%name</tt> directive in the Lemon parser
 *   specification.
 * @details This resolves to a function with name <a>PREFIX</a>Trace and call
 *   signature matching @ref largemelon::lemon_parsetrace_func_type.*/
#define LARGEMELON_LEMON_PARSETRACE_DECL(PREFIX) \
extern "C" void PREFIX##Trace(FILE *stream, char *prefix);

/**@brief Macro for the declarations of the Lemon-generated functions with
 *  default names @c ParseAlloc, @c ParseFree, @c Parse, and @c ParseTrace.
 * @param PREFIX Value of the <tt>%name</tt> directive in the Lemon parser
 *   specification.
 * @param CONTEXT_TYPE Type for the <tt>%extra_argument</tt> directive in the
 *   Lemon parser specification.
 * @details When the Lemon generates a parser, it instantiates the
 *   @c ParseAlloc, @c ParseFree, and @c Parse functions (as named by default).
 *   (The @c ParseTrace function may be unavailable if @c NDEBUG is not
 *   defined.) To use those functions outside of <tt>%code { ... }</tt> block
 *   directives within the parser specification, they must be forward-declared.
 *   This macro resolves to the forward declarations of those four functions.
 *   
 *   If using this macro within C++, wrap this macro instantiation in an
 *   <tt>extern "C" { ... }</tt> block.*/
#define LARGEMELON_LEMON_PARSER_DECLS(PREFIX,CONTEXT_TYPE) \
LARGEMELON_LEMON_PARSEALLOC_DECL(PREFIX) \
LARGEMELON_LEMON_PARSEFREE_DECL(PREFIX) \
LARGEMELON_LEMON_PARSE_DECL(PREFIX,CONTEXT_TYPE) \
LARGEMELON_LEMON_PARSETRACE_DECL(PREFIX)



#endif // LARGEMELON_LARGEMELON_HPP
